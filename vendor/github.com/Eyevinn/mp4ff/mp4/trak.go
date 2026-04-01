package mp4

import (
	"fmt"
	"io"

	"github.com/Eyevinn/mp4ff/bits"
)

// DefaultTrakID - trakID used when generating new fragmented content
const DefaultTrakID = 1

// TrakBox - Track Box (tkhd - mandatory)
//
// Contained in : Movie Box (moov)
//
// A media file can contain one or more tracks.
type TrakBox struct {
	Tkhd     *TkhdBox
	Edts     *EdtsBox
	Mdia     *MdiaBox
	Children []Box
}

// NewTrakBox - Make a new empty TrakBox
func NewTrakBox() *TrakBox {
	return &TrakBox{}
}

// AddChild - Add a child box
func (t *TrakBox) AddChild(child Box) {
	switch box := child.(type) {
	case *TkhdBox:
		t.Tkhd = box
	case *MdiaBox:
		t.Mdia = box
	case *EdtsBox:
		t.Edts = box
	}
	t.Children = append(t.Children, child)
}

// DecodeTrak - box-specific decode
func DecodeTrak(hdr BoxHeader, startPos uint64, r io.Reader) (Box, error) {
	children, err := DecodeContainerChildren(hdr, startPos+8, startPos+hdr.Size, r)
	if err != nil {
		return nil, err
	}
	t := TrakBox{Children: make([]Box, 0, len(children))}
	for _, c := range children {
		t.AddChild(c)
	}
	return &t, nil
}

// DecodeTrakSR - box-specific decode
func DecodeTrakSR(hdr BoxHeader, startPos uint64, sr bits.SliceReader) (Box, error) {
	children, err := DecodeContainerChildrenSR(hdr, startPos+8, startPos+hdr.Size, sr)
	if err != nil {
		return nil, err
	}
	t := TrakBox{Children: make([]Box, 0, len(children))}
	for _, c := range children {
		t.AddChild(c)
	}
	return &t, nil
}

// Type - box type
func (t *TrakBox) Type() string {
	return "trak"
}

// Size - calculated size of box
func (t *TrakBox) Size() uint64 {
	return containerSize(t.Children)
}

// GetChildren - list of child boxes
func (t *TrakBox) GetChildren() []Box {
	return t.Children
}

// Encode - write trak container to w
func (t *TrakBox) Encode(w io.Writer) error {
	return EncodeContainer(t, w)
}

// Encode - write trak container to sw
func (b *TrakBox) EncodeSW(sw bits.SliceWriter) error {
	return EncodeContainerSW(b, sw)
}

// Info - write box info to w
func (t *TrakBox) Info(w io.Writer, specificBoxLevels, indent, indentStep string) error {
	return ContainerInfo(t, w, specificBoxLevels, indent, indentStep)
}

// GetNrSamples - get number of samples for this track defined in the parent moov box.
func (t *TrakBox) GetNrSamples() uint32 {
	stbl := t.Mdia.Minf.Stbl
	return stbl.Stsz.GetNrSamples()
}

// GetSampleData - get sample metadata for a specific interval of samples defined in moov.
// If going outside the range of available samples, an error is returned.
func (t *TrakBox) GetSampleData(startSampleNr, endSampleNr uint32) ([]Sample, error) {
	stbl := t.Mdia.Minf.Stbl
	nrSamples := stbl.Stsz.GetNrSamples()
	if startSampleNr < 1 || endSampleNr > nrSamples {
		return nil, fmt.Errorf("sample interval %d-%d not inside available %d-%d", startSampleNr, endSampleNr, 1, nrSamples)
	}
	samples := make([]Sample, endSampleNr-startSampleNr+1)
	stts := stbl.Stts
	ctts := stbl.Ctts
	stss := stbl.Stss
	sdtp := stbl.Sdtp

	for nr := startSampleNr; nr <= endSampleNr; nr++ {
		var cto int32
		if ctts != nil {
			cto = ctts.GetCompositionTimeOffset(nr)
		}
		samples[nr-1] = Sample{
			Flags:                 createSampleFlagsFromProgressiveBoxes(stss, sdtp, nr),
			Dur:                   stts.GetDur(nr),
			Size:                  stbl.Stsz.GetSampleSize(int(nr)),
			CompositionTimeOffset: cto,
		}
	}
	return samples, nil
}

func createSampleFlagsFromProgressiveBoxes(stss *StssBox, sdtp *SdtpBox, sampleNr uint32) uint32 {
	var sampleFlags SampleFlags
	if stss != nil {
		isSync := stss.IsSyncSample(uint32(sampleNr))
		sampleFlags.SampleIsNonSync = !isSync
		if isSync {
			sampleFlags.SampleDependsOn = 2 //2 = does not depend on others (I-picture). May be overridden by sdtp entry
		}
	}
	if sdtp != nil {
		entry := sdtp.Entries[uint32(sampleNr)-1] // table starts at 0, but sampleNr is one-based
		sampleFlags.IsLeading = entry.IsLeading()
		sampleFlags.SampleDependsOn = entry.SampleDependsOn()
		sampleFlags.SampleHasRedundancy = entry.SampleHasRedundancy()
		sampleFlags.SampleIsDependedOn = entry.SampleIsDependedOn()
	}
	return sampleFlags.Encode()
}

// DataRange is a range for sample data in a file relative to file start
type DataRange struct {
	Offset uint64
	Size   uint64
}

// GetRangesForSampleInterval - get ranges inside file for sample range [startSampleNr, endSampleNr]
func (t *TrakBox) GetRangesForSampleInterval(startSampleNr, endSampleNr uint32) ([]DataRange, error) {
	stbl := t.Mdia.Minf.Stbl
	stsc := stbl.Stsc
	stco := stbl.Stco
	co64 := stbl.Co64
	stsz := stbl.Stsz
	nrSamples := stbl.Stsz.GetNrSamples()
	if startSampleNr < 1 || endSampleNr > nrSamples {
		return nil, fmt.Errorf("sample interval %d-%d not inside available %d-%d", startSampleNr, endSampleNr, 1, nrSamples)
	}
	chunks, err := stsc.GetContainingChunks(startSampleNr, endSampleNr)
	if err != nil {
		return nil, err
	}
	dataRanges := make([]DataRange, len(chunks))
	lastChunkIdx := len(chunks) - 1
	for idx, chunk := range chunks {
		var offset uint64
		if stco != nil {
			offset, err = stco.GetOffset(int(chunk.ChunkNr))
		} else if stbl.Co64 != nil {
			offset, err = co64.GetOffset(int(chunk.ChunkNr))
		}
		if err != nil {
			return nil, err
		}
		startNrInChunk := chunk.StartSampleNr
		endNrInChunk := chunk.StartSampleNr + chunk.NrSamples - 1
		if idx == 0 { // First chunk, adapt startPoint
			sizeUpToFirst, err := stsz.GetTotalSampleSize(chunk.StartSampleNr, startSampleNr-1)
			if err != nil {
				return nil, err
			}
			offset += sizeUpToFirst
			startNrInChunk = startSampleNr
		}
		if idx == lastChunkIdx {
			endNrInChunk = endSampleNr
		}
		size, err := stsz.GetTotalSampleSize(startNrInChunk, endNrInChunk)
		if err != nil {
			return nil, err
		}
		dataRanges[idx] = DataRange{Offset: offset, Size: size}
	}
	return dataRanges, nil
}
