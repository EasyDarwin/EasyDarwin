package mp4

import (
	"fmt"
	"io"

	"github.com/Eyevinn/mp4ff/bits"
)

// StscBox is Sample To Chunk Box in progressive file.
//
// A chunk contains samples. This table defines to which chunk a sample is associated.
// Each entry is defined by :
//
//   - first chunk : all chunks starting at this index up to the next first chunk have the same sample count/description
//   - samples per chunk : number of samples in the chunk
//   - sample description id : description (see the sample description box - stsd)
//     this value is most often the same for all samples, so it is stored as a single value if possible.
//
// FirstSampleNr is a helper value for fast lookup. Somthing that is often a bottleneck.
type StscBox struct {
	Version                   byte
	Flags                     uint32
	singleSampleDescriptionID uint32 // Used instead of slice if all values are the same
	Entries                   []StscEntry
	SampleDescriptionID       []uint32
}

type StscEntry struct {
	FirstChunk      uint32
	SamplesPerChunk uint32
	FirstSampleNr   uint32
}

// DecodeStsc - box-specific decode
func DecodeStsc(hdr BoxHeader, startPos uint64, r io.Reader) (Box, error) {
	data, err := readBoxBody(r, hdr)
	if err != nil {
		return nil, err
	}
	sr := bits.NewFixedSliceReader(data)
	return DecodeStscSR(hdr, startPos, sr)
}

// DecodeStscSR - box-specific decode
func DecodeStscSR(hdr BoxHeader, startPos uint64, sr bits.SliceReader) (Box, error) {
	versionAndFlags := sr.ReadUint32()
	entryCount := sr.ReadUint32()
	b := StscBox{
		Version: byte(versionAndFlags >> 24),
		Flags:   versionAndFlags & flagsMask,
		Entries: make([]StscEntry, entryCount),
	}

	var accSampleNr uint32 = 1

	for i := 0; i < int(entryCount); i++ {
		b.Entries[i].FirstChunk = sr.ReadUint32()
		b.Entries[i].SamplesPerChunk = sr.ReadUint32()
		if i > 0 {
			accSampleNr += (b.Entries[i].FirstChunk - b.Entries[i-1].FirstChunk) * b.Entries[i-1].SamplesPerChunk
		}
		b.Entries[i].FirstSampleNr = accSampleNr

		sdi := sr.ReadUint32()
		if i == 0 {
			b.singleSampleDescriptionID = sdi
		} else {
			if sdi != b.singleSampleDescriptionID {
				if b.singleSampleDescriptionID != 0 {
					b.SampleDescriptionID = make([]uint32, entryCount)
					for j := 0; j < i; j++ {
						b.SampleDescriptionID[i] = sdi
					}
					b.singleSampleDescriptionID = 0
				}
				b.SampleDescriptionID[i] = sdi
			}
		}
	}
	return &b, nil
}

// Type box-specific type
func (b *StscBox) Type() string {
	return "stsc"
}

// Size - box-specific size
func (b *StscBox) Size() uint64 {
	return uint64(boxHeaderSize + 8 + len(b.Entries)*12)
}

// Encode - write box to w
func (b *StscBox) Encode(w io.Writer) error {
	sw := bits.NewFixedSliceWriter(int(b.Size()))
	err := b.EncodeSW(sw)
	if err != nil {
		return err
	}
	_, err = w.Write(sw.Bytes())
	return err
}

// EncodeSW - box-specific encode to slicewriter
func (b *StscBox) EncodeSW(sw bits.SliceWriter) error {
	err := EncodeHeaderSW(b, sw)
	if err != nil {
		return err
	}
	versionAndFlags := (uint32(b.Version) << 24) + b.Flags
	sw.WriteUint32(versionAndFlags)
	sw.WriteUint32(uint32(len(b.Entries)))
	for i := range b.Entries {
		sw.WriteUint32(b.Entries[i].FirstChunk)
		sw.WriteUint32(b.Entries[i].SamplesPerChunk)
		if b.singleSampleDescriptionID != 0 {
			sw.WriteUint32(b.singleSampleDescriptionID)
		} else {
			sw.WriteUint32(b.SampleDescriptionID[i])
		}
	}
	return sw.AccError()
}

// Info - write specific box info to w
func (b *StscBox) Info(w io.Writer, specificBoxLevels, indent, indentStep string) error {
	bd := newInfoDumper(w, indent, b, int(b.Version), b.Flags)
	if len(b.Entries) > 0 {
		bd.write(" - entryCount: %d", len(b.Entries))
	}
	level := getInfoLevel(b, specificBoxLevels)
	if level >= 1 {
		for i := range b.Entries {
			bd.write(" - entry[%d]: firstChunk=%d samplesPerChunk=%d sampleDescriptionID=%d",
				i+1, b.Entries[i].FirstChunk, b.Entries[i].SamplesPerChunk, b.GetSampleDescriptionID(i+1))
		}
	}
	return bd.err
}

// AddEntry adds a new entry and calculates helper values.
func (b *StscBox) AddEntry(firstChunk, samplesPerChunk, sampleDescriptionID uint32) error {
	switch {
	case len(b.Entries) == 0:
		if firstChunk != 1 {
			return fmt.Errorf("first stsc entry does not have firstChunk == 1, but %d", firstChunk)
		}
		b.Entries = append(b.Entries, StscEntry{firstChunk, samplesPerChunk, 1})
		b.singleSampleDescriptionID = sampleDescriptionID
	default:
		nrEntries := len(b.Entries)
		if sampleDescriptionID != b.singleSampleDescriptionID {
			if b.singleSampleDescriptionID != 0 {
				b.SampleDescriptionID = make([]uint32, nrEntries)
				for i := 0; i < nrEntries; i++ {
					b.SampleDescriptionID[i] = b.singleSampleDescriptionID
				}
				b.singleSampleDescriptionID = 0
			}
			b.SampleDescriptionID = append(b.SampleDescriptionID, sampleDescriptionID)
		}
		lastEntry := b.Entries[len(b.Entries)-1]
		firstSampleNr := lastEntry.FirstSampleNr + (firstChunk-lastEntry.FirstChunk)*lastEntry.SamplesPerChunk
		b.Entries = append(b.Entries, StscEntry{firstChunk, samplesPerChunk, firstSampleNr})
	}
	return nil
}

// GetSampleDescriptionID returns the sample description ID from common or individual values for chunk.
// chunkNr is 1-based.
func (b *StscBox) GetSampleDescriptionID(chunkNr int) uint32 {
	if b.singleSampleDescriptionID != 0 {
		return b.singleSampleDescriptionID
	}
	return b.SampleDescriptionID[chunkNr-1]
}

// SetSingleSampleDescriptionID - use this for efficiency if all samples have same sample description
func (b *StscBox) SetSingleSampleDescriptionID(sampleDescriptionID uint32) {
	b.singleSampleDescriptionID = sampleDescriptionID
	b.SampleDescriptionID = nil
}

// ChunkNrFromSampleNr - get chunk number from sampleNr (one-based)
func (b *StscBox) ChunkNrFromSampleNr(sampleNr int) (chunkNr, firstSampleInChunk int, err error) {
	entryNr := b.FindEntryNrForSampleNr(uint32(sampleNr), 0)
	entry := b.Entries[entryNr]
	nrInEntry := (uint32(sampleNr) - entry.FirstSampleNr) / entry.SamplesPerChunk
	chunkNr = int(entry.FirstChunk + nrInEntry)
	firstSampleInChunk = int(entry.FirstSampleNr + nrInEntry*entry.SamplesPerChunk)
	return chunkNr, firstSampleInChunk, nil
}

// Chunk defines a chunk with number, starting sampleNr and nrSamples.
type Chunk struct {
	ChunkNr       uint32
	StartSampleNr uint32
	NrSamples     uint32
}

// GetContainingChunks returns chunks containing the sample interval including endSampleNr.
// startSampleNr and endSampleNr are 1-based.
func (b *StscBox) GetContainingChunks(startSampleNr, endSampleNr uint32) ([]Chunk, error) {
	if startSampleNr == 0 || endSampleNr < startSampleNr {
		return nil, fmt.Errorf("bad sample interval %d-%d", startSampleNr, endSampleNr)
	}
	nrEntries := uint32(len(b.Entries))

	startEntryNr := b.FindEntryNrForSampleNr(startSampleNr, 0)
	endEntryNr := b.FindEntryNrForSampleNr(endSampleNr, startEntryNr)

	startEntry := b.Entries[startEntryNr]
	endEntry := b.Entries[endEntryNr]
	startChunkNr := (startSampleNr-startEntry.FirstSampleNr)/startEntry.SamplesPerChunk + startEntry.FirstChunk
	endChunkNr := (endSampleNr-endEntry.FirstSampleNr)/endEntry.SamplesPerChunk + endEntry.FirstChunk

	chunks := make([]Chunk, 0, endChunkNr-startChunkNr+1)

	entryNr := startEntryNr
	entry := b.Entries[entryNr]
	for chunkNr := startChunkNr; chunkNr <= endChunkNr; chunkNr++ {
		chunk := Chunk{chunkNr, entry.FirstSampleNr + (chunkNr-entry.FirstChunk)*entry.SamplesPerChunk, entry.SamplesPerChunk}
		chunks = append(chunks, chunk)
		if entryNr < nrEntries-1 {
			if chunkNr+1 == b.Entries[entryNr+1].FirstChunk {
				entryNr++
				entry = b.Entries[entryNr]
			}
		}
	}
	return chunks, nil
}

// GetChunk returns chunk for chunkNr (one-based).
func (b *StscBox) GetChunk(chunkNr uint32) Chunk {
	if chunkNr == 0 {
		panic("ChunkNr set to 0 but is one-based")
	}
	chunk := Chunk{
		ChunkNr:       chunkNr,
		StartSampleNr: 1,
		NrSamples:     0,
	}
	entryNr := b.findEntryNrForChunkNr(chunkNr)
	entry := b.Entries[entryNr]
	chunk.NrSamples = entry.SamplesPerChunk
	chunk.StartSampleNr = (chunkNr-entry.FirstChunk)*entry.SamplesPerChunk + entry.FirstSampleNr
	return chunk
}

// findEntryNrForChunkNr returns the entry where chunkNr belongs.
// The resulting entryNr is 0-based index.
func (b *StscBox) findEntryNrForChunkNr(chunkNr uint32) uint32 {
	// The following is essentially the sort.Search() code specialized to this case
	low, high := 0, len(b.Entries)
	for low < high {
		mid := int(uint(low+high) >> 1) // avoid overflow when computing h
		// low ≤ mid < high
		if b.Entries[mid].FirstChunk > chunkNr {
			high = mid
		} else {
			low = mid + 1
		}
	}
	return uint32(low - 1)
}

// FindEntryNrForSampleNr returns the entry where sampleNr belongs. lowEntryIdx is entry index (zero-based).
// The resulting entryNr is 0-based index.
func (b *StscBox) FindEntryNrForSampleNr(sampleNr, lowEntryIdx uint32) uint32 {
	// The following is essentially the sort.Search() code specialized to this case
	low, high := lowEntryIdx, uint32(len(b.Entries))
	for low < high {
		mid := uint32(uint(low)+uint(high)) >> 1
		// low ≤ mid < high
		if b.Entries[mid].FirstSampleNr > sampleNr {
			high = mid
		} else {
			low = mid + 1
		}
	}
	return low - 1
}
