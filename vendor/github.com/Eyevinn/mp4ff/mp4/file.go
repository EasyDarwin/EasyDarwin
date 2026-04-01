package mp4

import (
	"fmt"
	"io"
	"os"
	"strings"

	"github.com/Eyevinn/mp4ff/bits"
)

// File - an MPEG-4 file asset
//
// A progressive MPEG-4 file contains three main boxes:
//
//	ftyp : the file type box
//	moov : the movie box (meta-data)
//	mdat : the media data (chunks and samples). Only used for pror
//
// where mdat may come before moov.
// If fragmented, there are many more boxes and they are collected
// in the InitSegment, Segment and Segments structures.
// The sample metadata in the fragments in the Segments will be
// optimized unless EncModeBoxTree is set.
// To Encode the same data as Decoded, this flag must therefore be set.
// In all cases, Children contain all top-level boxes
type File struct {
	Ftyp         *FtypBox
	Moov         *MoovBox
	Mdat         *MdatBox        // mdat box for non-fragmented files. Extra empty boxes allowed.
	Init         *InitSegment    // Init data (ftyp + moov for fragmented file)
	Sidx         *SidxBox        // The first sidx box for a DASH OnDemand file
	Sidxs        []*SidxBox      // All sidx boxes for a DASH OnDemand file
	tfra         *TfraBox        // Single tfra box read first if DecISMFlag set
	Mfra         *MfraBox        // MfraBox for ISM files
	Segments     []*MediaSegment // Media segments
	Children     []Box           // All top-level boxes in order
	FragEncMode  EncFragFileMode // Determine how fragmented files are encoded
	EncOptimize  EncOptimize     // Bit field with optimizations being done at encoding
	fileDecFlags DecFileFlags    // Bit field with flags for decoding
	isFragmented bool
	fileDecMode  DecFileMode
}

// EncFragFileMode - mode for writing file
type EncFragFileMode byte

const (
	// EncModeSegment - only encode boxes that are part of Init and MediaSegments
	EncModeSegment = EncFragFileMode(0)
	// EncModeBoxTree - encode all boxes in file tree
	EncModeBoxTree = EncFragFileMode(1)
)

// DecFileMode - mode for decoding file
type DecFileMode byte

const (
	// DecModeNormal - read Mdat data into memory during file decoding.
	DecModeNormal DecFileMode = iota
	// DecModeLazyMdat - do not read mdat data into memory.
	// Thus, decode process requires less memory and faster.
	DecModeLazyMdat
)

// DecFileFlags can be combined for special decoding options
type DecFileFlags uint32

const (
	DecNoFlags DecFileFlags = 0
	// DecISMFlag tries to read mfra box at end to find segment boundaries (for ISM files)
	DecISMFlag DecFileFlags = (1 << 0)
	// DecStartOnMoof starts a segment at each moof boundary
	// This is provided no styp, or sidx/mfra box gives other information
	DecStartOnMoof = (1 << 1)
	// if no styp box, or sidx/mfra strudture
)

// EncOptimize - encoder optimization mode
type EncOptimize uint32

const (
	// OptimizeNone - no optimization
	OptimizeNone = EncOptimize(0)
	// OptimizeTrun - optimize trun box by moving default values to tfhd
	OptimizeTrun = EncOptimize(1 << 0)
)

func (eo EncOptimize) String() string {
	var optList []string
	msg := "OptimizeNone"
	if eo&OptimizeTrun != 0 {
		optList = append(optList, "OptimizeTrun")
	}
	if len(optList) > 0 {
		msg = strings.Join(optList, " | ")
	}
	return msg
}

// NewFile - create MP4 file
func NewFile() *File {
	return &File{
		FragEncMode: EncModeSegment,
		EncOptimize: OptimizeNone,
		fileDecMode: DecModeNormal,
		Children:    make([]Box, 0, 8), // Reasonable number of children
	}
}

// ReadMP4File - read an mp4 file from path
func ReadMP4File(path string) (*File, error) {
	f, err := os.Open(path)
	if err != nil {
		return nil, err
	}
	defer f.Close()

	mp4Root, err := DecodeFile(f, WithDecodeFlags(DecISMFlag))
	if err != nil {
		return nil, err
	}
	return mp4Root, nil
}

// BoxStructure represent a box or similar entity such as a Segment
type BoxStructure interface {
	Encode(w io.Writer) error
}

// WriteToFile - write a box structure to a file at filePath
func WriteToFile(boxStructure BoxStructure, filePath string) error {
	ofd, err := os.Create(filePath)
	if err != nil {
		return err
	}
	defer ofd.Close()
	err = boxStructure.Encode(ofd)
	return err
}

// AddMediaSegment - add a mediasegment to file f
func (f *File) AddMediaSegment(m *MediaSegment) {
	f.Segments = append(f.Segments, m)
}

// DecodeFile - parse and decode a file from reader r with optional file options.
// For example, the file options overwrite the default decode or encode mode.
func DecodeFile(r io.Reader, options ...Option) (*File, error) {
	f := NewFile()

	// apply options to change the default decode or encode mode
	f.ApplyOptions(options...)

	var boxStartPos uint64 = 0
	lastBoxType := ""

	var rs io.ReadSeeker
	if f.fileDecMode == DecModeLazyMdat {
		ok := false
		rs, ok = r.(io.ReadSeeker)
		if !ok {
			return nil, fmt.Errorf("expecting readseeker when decoding file lazily, but got %T", r)
		}
	}

	if (f.fileDecFlags & DecISMFlag) != 0 {
		err := f.findAndReadMfra(r)
		if err != nil {
			return nil, fmt.Errorf("checkMfra: %w", err)
		}
	}

LoopBoxes:
	for {
		var box Box
		var err error
		switch f.fileDecMode {
		case DecModeLazyMdat:
			box, err = DecodeBoxLazyMdat(boxStartPos, rs)
		case DecModeNormal:
			box, err = DecodeBox(boxStartPos, r)
		default:
			return nil, fmt.Errorf("unknown DecFileMode=%d", f.fileDecMode)
		}
		if err == io.EOF {
			break LoopBoxes
		}
		if err != nil {
			return nil, err
		}
		boxType, boxSize := box.Type(), box.Size()
		switch boxType {
		case "mdat":
			if f.isFragmented {
				if lastBoxType != "moof" {
					return nil, fmt.Errorf("does not support %v between moof and mdat", lastBoxType)
				}
			} else {
				if f.Mdat != nil {
					oldPayloadSize := f.Mdat.Size() - f.Mdat.HeaderSize()
					newMdat := box.(*MdatBox)
					newPayloadSize := newMdat.Size() - newMdat.HeaderSize()
					if oldPayloadSize > 0 && newPayloadSize > 0 {
						return nil, fmt.Errorf("only one non-empty mdat box supported (payload sizes %d and %d)",
							oldPayloadSize, newPayloadSize)
					}
				}
			}
		case "moof":
			moof := box.(*MoofBox)
			for _, traf := range moof.Trafs {
				if ok, parsed := traf.ContainsSencBox(); ok && !parsed {
					isEncrypted := true
					defaultIVSize := byte(0) // Should get this from tenc in sinf
					if f.Moov != nil {
						trackID := traf.Tfhd.TrackID
						isEncrypted = f.Moov.IsEncrypted(trackID)
						sinf := f.Moov.GetSinf(trackID)
						if sinf != nil && sinf.Schi != nil && sinf.Schi.Tenc != nil {
							defaultIVSize = sinf.Schi.Tenc.DefaultPerSampleIVSize
						}
					}
					if isEncrypted { // Don't do if encryption boxes still remain, but are not
						err = traf.ParseReadSenc(defaultIVSize, moof.StartPos)
						if err != nil {
							return nil, err
						}
					}
				}
			}
		}
		f.AddChild(box, boxStartPos)
		lastBoxType = boxType
		boxStartPos += boxSize
	}
	f.tfra = nil // Not needed anymore
	return f, nil
}

// Size - total size of all boxes
func (f *File) Size() uint64 {
	var totSize uint64 = 0
	for _, f := range f.Children {
		totSize += f.Size()
	}
	return totSize
}

// AddChild - add child with start position
func (f *File) AddChild(child Box, boxStartPos uint64) {
	switch box := child.(type) {
	case *FtypBox:
		f.Ftyp = box
	case *MoovBox:
		f.Moov = box
		if len(f.Moov.Trak.Mdia.Minf.Stbl.Stts.SampleCount) == 0 {
			f.isFragmented = true
			f.Init = NewMP4Init()
			f.Init.AddChild(f.Ftyp)
			f.Init.AddChild(f.Moov)
		}
	case *SidxBox:
		// sidx boxes are either added to the File or to the current media segment.
		// Since sidx boxes for a segment come before the moof, it is important that a new
		// segment is started with a styp box for the sidx to be associated with the
		// right segment.
		// A more general solution could possibly be implemented by looking at the
		// sidx details like reference_ID to understand the sidx chain structure,
		// and/or by waiting with associating the sidx box until more boxes are read.
		// Given the rareness of multiple sidx boxes and the complexity of implementing
		// and testing such a solution, that track is not deemed worth the effort for now.
		if len(f.Segments) == 0 {
			// Add sidx to top level until we know that a segment has started
			f.AddSidx(box)
		} else {
			currSeg := f.Segments[len(f.Segments)-1]
			currSeg.AddSidx(box)
		}
	case *StypBox:
		// Starts a new segment
		f.isFragmented = true
		f.AddMediaSegment(&MediaSegment{Styp: box, StartPos: boxStartPos})
	case *EmsgBox:
		// emsg box is only added at the start of a fragment (inside a segment).
		// The case that a segment starts without an emsg is also handled.
		f.startSegmentIfNeeded(box, boxStartPos)
		lastSeg := f.LastSegment()
		if len(lastSeg.Fragments) == 0 {
			lastSeg.AddFragment(&Fragment{StartPos: boxStartPos})
		}
		frag := lastSeg.LastFragment()
		frag.AddChild(box)
	case *MoofBox:
		f.isFragmented = true
		moof := box
		moof.StartPos = boxStartPos
		f.startSegmentIfNeeded(moof, boxStartPos)
		currSeg := f.LastSegment()
		lastFrag := currSeg.LastFragment()
		if lastFrag == nil || lastFrag.Moof != nil {
			currSeg.AddFragment(&Fragment{StartPos: boxStartPos})
		}
		frag := currSeg.LastFragment()
		frag.AddChild(moof)
	case *MdatBox:
		if !f.isFragmented { // Only add if previous mdat is nil or empty
			if f.Mdat == nil || f.Mdat.Size()-f.Mdat.HeaderSize() == 0 {
				f.Mdat = box
			}
		} else {
			currentFragment := f.LastSegment().LastFragment()
			currentFragment.AddChild(box)
		}
	case *MfraBox:
		f.Mfra = box
	}
	f.Children = append(f.Children, child)
}

// startSegmentIfNeeded starts a new segment if there is none or if position match with sidx of tfra.
func (f *File) startSegmentIfNeeded(b Box, boxStartPos uint64) {
	segStart := false
	segIdx := len(f.Segments)
	switch {
	case f.Sidx != nil:
		idx := 0
	sidxLoop:
		for _, sx := range f.Sidxs {
			startPos := sx.AnchorPoint
			for _, ref := range sx.SidxRefs {
				if ref.ReferenceType == 1 {
					continue sidxLoop
				}
				if boxStartPos == startPos && idx == segIdx {
					segStart = true
					break sidxLoop
				}
				startPos += uint64(ref.ReferencedSize)
				idx++
			}
		}
	case f.tfra != nil:
		if boxStartPos == uint64(f.tfra.Entries[segIdx].MoofOffset) {
			segStart = true
		}
	case (f.fileDecFlags & DecStartOnMoof) != 0:
		segStart = true
	default:
		segStart = (segIdx == 0)
	}
	if segStart {
		f.isFragmented = true
		ms := MediaSegment{
			Styp:        nil,
			Fragments:   nil,
			EncOptimize: OptimizeNone,
			StartPos:    boxStartPos,
		}
		f.AddMediaSegment(&ms)
		return
	}
}

// findAndReadMfra tries to find a tfra box inside an mfra box at the end of the file
// If no mfro box is found, no error is reported.
func (f *File) findAndReadMfra(r io.Reader) error {
	rs, ok := r.(io.ReadSeeker)
	if !ok {
		return fmt.Errorf("expecting readseeker when decoding ISM file")
	}
	mfroSize := int64(16) // This is the fixed size of the mfro box
	pos, err := rs.Seek(-mfroSize, io.SeekEnd)
	if err != nil {
		return fmt.Errorf("could not seek %d bytes from end: %w", mfroSize, err)
	}
	mfro, err := TryDecodeMfro(uint64(pos), rs) // mfro
	if err != nil {
		// Not an mfro box here, just reset and return
		_, err = rs.Seek(0, io.SeekStart)
		return err
	}
	mfraSize := int64(mfro.ParentSize)
	pos, err = rs.Seek(-mfraSize, io.SeekEnd)
	if err != nil {
		return fmt.Errorf("could not seek %d bytes from end: %w", mfraSize, err)
	}
	b, err := DecodeBox(uint64(pos), rs) // mfra
	if err != nil {
		return fmt.Errorf("could not decode mfra box: %w", err)
	}
	mfra, ok := b.(*MfraBox)
	if !ok {
		return fmt.Errorf("expecting mfra box, but got %T", b)
	}
	f.tfra = mfra.Tfras[0]
	for i := 1; i < len(mfra.Tfras); i++ {
		if mfra.Tfras[i].TrackID == f.tfra.TrackID {
			return fmt.Errorf("only one tfra box per trackID is supported")
		}
		if len(mfra.Tfras[i].Entries) != len(f.tfra.Entries) {
			return fmt.Errorf("tfra boxes with different number of entries are not supported")
		}
		for j := 0; j < len(mfra.Tfras[i].Entries); j++ {
			if mfra.Tfras[i].Entries[j].MoofOffset != f.tfra.Entries[j].MoofOffset {
				return fmt.Errorf("tfra boxes with different moof offsets for different tracks are not supported")
			}
		}
	}
	_, err = rs.Seek(0, io.SeekStart)
	return err
}

// AddSidx adds a sidx box to the File and not a MediaSegment.
func (f *File) AddSidx(sidx *SidxBox) {
	if len(f.Sidxs) == 0 {
		f.Sidx = sidx
	}
	f.Sidxs = append(f.Sidxs, sidx)
}

// Encode - encode a file to a Writer
// Fragmented files are encoded based on InitSegment and MediaSegments, unless EncModeBoxTree is set.
func (f *File) Encode(w io.Writer) error {
	if f.isFragmented {
		switch f.FragEncMode {
		case EncModeSegment:
			if f.Init != nil {
				err := f.Init.Encode(w)
				if err != nil {
					return err
				}
			}
			if len(f.Sidxs) > 0 {
				for i := range f.Sidxs {
					err := f.Sidxs[i].Encode(w)
					if err != nil {
						return err
					}
				}
			}
			for _, seg := range f.Segments {
				if f.EncOptimize&OptimizeTrun != 0 {
					seg.EncOptimize = f.EncOptimize
				}
				err := seg.Encode(w)
				if err != nil {
					return err
				}
			}
			if f.Mfra != nil {
				err := f.Mfra.Encode(w)
				if err != nil {
					return err
				}
			}
		case EncModeBoxTree:
			for _, b := range f.Children {
				err := b.Encode(w)
				if err != nil {
					return err
				}
			}
		default:
			return fmt.Errorf("unknown FragEncMode=%d", f.FragEncMode)
		}
		return nil
	}
	// Progressive file
	for _, b := range f.Children {
		err := b.Encode(w)
		if err != nil {
			return err
		}
	}
	return nil
}

// EncodeSW - encode a file to a SliceWriter
// Fragmented files are encoded based on InitSegment and MediaSegments, unless EncModeBoxTree is set.
func (f *File) EncodeSW(sw bits.SliceWriter) error {
	if f.isFragmented {
		switch f.FragEncMode {
		case EncModeSegment:
			if f.Init != nil {
				err := f.Init.EncodeSW(sw)
				if err != nil {
					return err
				}
			}
			if len(f.Sidxs) > 0 {
				for i := range f.Sidxs {
					err := f.Sidxs[i].EncodeSW(sw)
					if err != nil {
						return err
					}
				}
			}
			for _, seg := range f.Segments {
				if f.EncOptimize&OptimizeTrun != 0 {
					seg.EncOptimize = f.EncOptimize
				}
				err := seg.EncodeSW(sw)
				if err != nil {
					return err
				}
			}
		case EncModeBoxTree:
			for _, b := range f.Children {
				err := b.EncodeSW(sw)
				if err != nil {
					return err
				}
			}
		default:
			return fmt.Errorf("unknown FragEncMode=%d", f.FragEncMode)
		}
		return nil
	}
	// Progressive file
	for _, b := range f.Children {
		err := b.EncodeSW(sw)
		if err != nil {
			return err
		}
	}
	return nil
}

// Info - write box tree with indent for each level
func (f *File) Info(w io.Writer, specificBoxLevels, indent, indentStep string) error {
	for _, box := range f.Children {
		err := box.Info(w, specificBoxLevels, indent, indentStep)
		if err != nil {
			return err
		}
	}
	return nil
}

// LastSegment - Currently last segment
func (f *File) LastSegment() *MediaSegment {
	if len(f.Segments) == 0 {
		return nil
	}
	return f.Segments[len(f.Segments)-1]
}

// IsFragmented - is file made of multiple segments (Mp4 fragments)
func (f *File) IsFragmented() bool {
	return f.isFragmented
}

// ApplyOptions - applies options for decoding or encoding a file
func (f *File) ApplyOptions(opts ...Option) {
	for _, opt := range opts {
		opt(f)
	}
}

// Option is function signature of file options.
// The design follows functional options pattern.
type Option func(f *File)

// WithEncodeMode sets up EncFragFileMode
func WithEncodeMode(mode EncFragFileMode) Option {
	return func(f *File) { f.FragEncMode = mode }
}

// WithDecodeMode sets up DecFileMode
func WithDecodeMode(mode DecFileMode) Option {
	return func(f *File) { f.fileDecMode = mode }
}

// WithDecodeFlags sets up DecodeFlags
func WithDecodeFlags(flags DecFileFlags) Option {
	return func(f *File) { f.fileDecFlags = flags }
}

// CopySampleData copies sample data from a track in a progressive mp4 file to w.
// Use rs for lazy read and workSpace as an intermediate storage to avoid memory allocations.
func (f *File) CopySampleData(w io.Writer, rs io.ReadSeeker, trak *TrakBox,
	startSampleNr, endSampleNr uint32, workSpace []byte) error {
	if f.isFragmented {
		return fmt.Errorf("only available for progressive files")
	}
	mdat := f.Mdat

	if mdat.IsLazy() && rs == nil {
		return fmt.Errorf("no ReadSeeker for lazy mdat")
	}
	mdatPayloadStart := mdat.PayloadAbsoluteOffset()

	stbl := trak.Mdia.Minf.Stbl
	chunks, err := stbl.Stsc.GetContainingChunks(startSampleNr, endSampleNr)
	if err != nil {
		return err
	}
	var getChunkOffset func(chunkNr int) (uint64, error)
	switch {
	case stbl.Stco != nil:
		getChunkOffset = stbl.Stco.GetOffset
	case stbl.Co64 != nil:
		getChunkOffset = stbl.Co64.GetOffset
	default:
		return fmt.Errorf("neither stco nor co64 available")
	}
	var startNr, endNr uint32
	var offset uint64
	workPos := 0
	workLen := len(workSpace)
	for i, chunk := range chunks {
		startNr = chunk.StartSampleNr
		endNr = startNr + chunk.NrSamples - 1
		offset, err = getChunkOffset(int(chunk.ChunkNr))
		if err != nil {
			return fmt.Errorf("getChunkOffset: %w", err)
		}
		if i == 0 {
			for sNr := chunk.StartSampleNr; sNr < startSampleNr; sNr++ {
				offset += uint64(stbl.Stsz.GetSampleSize(int(sNr)))
			}
			startNr = startSampleNr
		}

		if i == len(chunks)-1 {
			endNr = endSampleNr
		}
		var size int64
		for sNr := startNr; sNr <= endNr; sNr++ {
			size += int64(stbl.Stsz.GetSampleSize(int(sNr)))
		}
		if mdat.IsLazy() {
			_, err := rs.Seek(int64(offset), io.SeekStart)
			if err != nil {
				return err
			}
			if workLen == 0 {
				n, err := io.CopyN(w, rs, size)
				if err != nil {
					return fmt.Errorf("copyN: %w", err)
				}
				if n != size {
					return fmt.Errorf("wrote %d instead of %d bytes", n, size)
				}
			} else {
				nrLeft := int(size)
				nrRead := 0
				for {
					end := min(workLen, workPos+nrLeft)
					n, err := rs.Read(workSpace[workPos:end])
					if err != nil {
						return err
					}
					nrLeft -= n
					workPos += n
					nrRead += n
					if nrLeft == 0 {
						break
					}
					if workPos == workLen {
						n, err := w.Write(workSpace)
						if n != workPos {
							return fmt.Errorf("finished match %d written instead of instead of %d", n, workPos)
						}
						if err != nil {
							return fmt.Errorf("write error: %w", err)
						}
						workPos = 0
					}
				}
			}
		} else {
			offsetInMdatData := offset - mdatPayloadStart
			n, err := w.Write(mdat.Data[offsetInMdatData : offsetInMdatData+uint64(size)])
			if err != nil {
				return err
			}
			if int64(n) != size {
				return fmt.Errorf("copied %d bytes instead of %d", n, size)
			}
		}
	}
	if workPos > 0 {
		n, err := w.Write(workSpace[:workPos])
		if n != workPos {
			return fmt.Errorf("finished match %d written instead of instead of %d", n, workPos)
		}
		if err != nil {
			return fmt.Errorf("write error: %w", err)
		}
	}
	return nil
}

func (f *File) UpdateSidx(addIfNotExists, nonZeroEPT bool) error {

	if !f.IsFragmented() {
		return fmt.Errorf("input file is not fragmented")
	}

	initSeg := f.Init
	if initSeg == nil {
		return fmt.Errorf("input file does not have an init segment")
	}

	segs := f.Segments
	if len(segs) == 0 {
		return fmt.Errorf("input file does not have any media segments")
	}
	exists := f.Sidx != nil
	if !exists && !addIfNotExists {
		return nil
	}

	refTrak := findReferenceTrak(initSeg)
	trex, ok := initSeg.Moov.Mvex.GetTrex(refTrak.Tkhd.TrackID)
	if !ok {
		return fmt.Errorf("no trex box found for track %d", refTrak.Tkhd.TrackID)
	}
	segDatas, err := findSegmentData(segs, refTrak, trex)
	if err != nil {
		return fmt.Errorf("failed to find segment data: %w", err)
	}

	var sidx *SidxBox
	if exists {
		sidx = f.Sidx
	} else {
		sidx = &SidxBox{}
	}
	fillSidx(sidx, refTrak, segDatas, nonZeroEPT)
	if !exists {
		err = insertSidx(f, segDatas, sidx)
		if err != nil {
			return fmt.Errorf("failed to insert sidx box: %w", err)
		}
	}
	return nil
}

func findReferenceTrak(initSeg *InitSegment) *TrakBox {
	var trak *TrakBox
	for _, trak = range initSeg.Moov.Traks {
		if trak.Mdia.Hdlr.HandlerType == "vide" {
			return trak
		}
	}
	for _, trak = range initSeg.Moov.Traks {
		if trak.Mdia.Hdlr.HandlerType == "soun" {
			return trak
		}
	}
	return initSeg.Moov.Traks[0]
}

type segData struct {
	startPos         uint64
	presentationTime uint64
	baseDecodeTime   uint64
	dur              uint32
	size             uint32
}

// findSegmentData returns a slice of segment media data using a reference track.
func findSegmentData(segs []*MediaSegment, refTrak *TrakBox, trex *TrexBox) ([]segData, error) {
	segDatas := make([]segData, 0, len(segs))
	for _, seg := range segs {
		var firstCompositionTimeOffest int64
		dur := uint32(0)
		var baseTime uint64
		for fIdx, frag := range seg.Fragments {
			for _, traf := range frag.Moof.Trafs {
				tfhd := traf.Tfhd
				if tfhd.TrackID == refTrak.Tkhd.TrackID { // Find track that gives sidx time values
					if fIdx == 0 {
						baseTime = traf.Tfdt.BaseMediaDecodeTime()
					}
					for i, trun := range traf.Truns {
						trun.AddSampleDefaultValues(tfhd, trex)
						samples := trun.GetSamples()
						for j, sample := range samples {
							if fIdx == 0 && i == 0 && j == 0 {
								firstCompositionTimeOffest = int64(sample.CompositionTimeOffset)
							}
							dur += sample.Dur
						}
					}
				}
			}
		}
		sd := segData{
			startPos:         seg.StartPos,
			presentationTime: uint64(int64(baseTime) + firstCompositionTimeOffest),
			baseDecodeTime:   baseTime,
			dur:              dur,
			size:             uint32(seg.Size()),
		}
		segDatas = append(segDatas, sd)
	}
	return segDatas, nil
}

func fillSidx(sidx *SidxBox, refTrak *TrakBox, segDatas []segData, nonZeroEPT bool) {
	ept := uint64(0)
	if nonZeroEPT {
		ept = segDatas[0].presentationTime
	}
	sidx.Version = 1
	sidx.Timescale = refTrak.Mdia.Mdhd.Timescale
	sidx.ReferenceID = 1
	sidx.EarliestPresentationTime = ept
	sidx.FirstOffset = 0
	sidx.SidxRefs = make([]SidxRef, 0, len(segDatas))

	for _, segData := range segDatas {
		size := segData.size
		sidx.SidxRefs = append(sidx.SidxRefs, SidxRef{
			ReferencedSize:     size,
			SubSegmentDuration: segData.dur,
			StartsWithSAP:      1,
			SAPType:            1,
		})
	}
}

func insertSidx(inFile *File, segDatas []segData, sidx *SidxBox) error {
	// insert sidx box before first media segment
	// TODO. Handle case where startPos is not reliable. Maybe first box of first segment
	firstMediaBox, err := inFile.Segments[0].FirstBox()
	if err != nil {
		return fmt.Errorf("could not find position to insert sidx box: %w", err)
	}
	var mediaStartIdx = 0
	for i, ch := range inFile.Children {
		if ch == firstMediaBox {
			mediaStartIdx = i
			break
		}
	}
	if mediaStartIdx == 0 {
		return fmt.Errorf("could not find position to insert sidx box")
	}
	inFile.Children = append(inFile.Children[:mediaStartIdx], append([]Box{sidx}, inFile.Children[mediaStartIdx:]...)...)
	inFile.Sidx = sidx
	inFile.Sidxs = []*SidxBox{sidx}
	return nil
}

func min(a, b int) int {
	if a < b {
		return a
	}
	return b
}
