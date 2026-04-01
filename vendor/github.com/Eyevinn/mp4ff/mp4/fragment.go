package mp4

import (
	"fmt"
	"io"
	"sort"

	"github.com/Eyevinn/mp4ff/bits"
)

// Fragment - MP4 Fragment ([prft] + moof + mdat)
type Fragment struct {
	Emsgs       []*EmsgBox
	Prft        *PrftBox
	Moof        *MoofBox
	Mdat        *MdatBox
	Children    []Box       // All top-level boxes in order
	nextTrunNr  uint32      // To handle multi-trun cases
	EncOptimize EncOptimize // Bit field with optimizations being done at encoding
	StartPos    uint64      // Start position in file added by parser
}

// NewFragment creates an empty MP4 Fragment.
func NewFragment() *Fragment {
	return &Fragment{}
}

// CreateFragment creates a single track fragment
func CreateFragment(seqNumber uint32, trackID uint32) (*Fragment, error) {
	f := Fragment{Children: make([]Box, 0, 2)}
	moof := &MoofBox{}
	f.AddChild(moof)
	mfhd := CreateMfhd(seqNumber)
	_ = moof.AddChild(mfhd)
	traf := &TrafBox{}
	_ = moof.AddChild(traf) // Can only have error when adding second track
	tfhd := CreateTfhd(trackID)
	_ = traf.AddChild(tfhd)
	tfdt := &TfdtBox{} // Data will be provided by first sample
	_ = traf.AddChild(tfdt)
	trun := CreateTrun(f.nextTrunNr)
	f.nextTrunNr++
	_ = traf.AddChild(trun)
	mdat := &MdatBox{}
	f.AddChild(mdat)

	return &f, nil
}

// CreateMultiTrackFragment creates a multi-track fragment without trun boxes.
func CreateMultiTrackFragment(seqNumber uint32, trackIDs []uint32) (*Fragment, error) {
	f := NewFragment()
	moof := &MoofBox{}
	f.AddChild(moof)
	mfhd := CreateMfhd(seqNumber)
	_ = moof.AddChild(mfhd)
	for _, trackID := range trackIDs {
		traf := &TrafBox{}
		_ = moof.AddChild(traf) // Can only have error when adding second track
		tfhd := CreateTfhd(trackID)
		_ = traf.AddChild(tfhd)
		tfdt := &TfdtBox{} // Data will be provided by first sample
		_ = traf.AddChild(tfdt)
		// Don't add trun, but let that happen in write order
	}
	mdat := &MdatBox{}
	f.AddChild(mdat)

	return f, nil
}

// AddChild adds a top-level box to Fragment. Add in proper order.
func (f *Fragment) AddChild(b Box) {
	switch box := b.(type) {
	case *EmsgBox:
		f.Emsgs = append(f.Emsgs, box)
	case *PrftBox:
		f.Prft = box
	case *MoofBox:
		f.Moof = box
	case *MdatBox:
		f.Mdat = box
	}
	f.Children = append(f.Children, b)
}

// AddEmsg inserts an emsg box at the end of a sequence of emsg boxes at the start of the fragment.
func (f *Fragment) AddEmsg(emsg *EmsgBox) {
	prevEmsg := -1
	for i, c := range f.Children {
		if _, ok := c.(*EmsgBox); ok {
			prevEmsg = i
		}
	}
	newIdx := prevEmsg + 1
	f.Children = append(f.Children[:newIdx+1], f.Children[newIdx:]...)
	f.Children[newIdx] = emsg
}

// Size - return size of fragment including all boxes.
// Be aware that TrafBox.OptimizeTfhdTrun() can change size
func (f *Fragment) Size() uint64 {
	var size uint64 = 0
	for _, c := range f.Children {
		size += c.Size()
	}
	return size
}

// GetFullSamples - Get full samples including media and accumulated time
func (f *Fragment) GetFullSamples(trex *TrexBox) ([]FullSample, error) {
	moof := f.Moof
	mdat := f.Mdat
	//seqNr := moof.Mfhd.SequenceNumber
	var traf *TrafBox
	foundTrak := false
	if trex != nil {
		for _, traf = range moof.Trafs {
			if traf.Tfhd.TrackID == trex.TrackID {
				foundTrak = true
				break
			}
		}
		if !foundTrak {
			return nil, nil // This trackID may not exist for this fragment
		}
	} else {
		traf = moof.Traf // The first one
	}
	tfhd := traf.Tfhd
	var baseTime uint64
	if traf.Tfdt != nil {
		baseTime = traf.Tfdt.BaseMediaDecodeTime()
	}
	moofStartPos := moof.StartPos
	var samples []FullSample
	for _, trun := range traf.Truns {
		totalDur := trun.AddSampleDefaultValues(tfhd, trex)
		// The default is moofStartPos according to Section 8.8.7.1
		baseOffset := moofStartPos
		if tfhd.HasBaseDataOffset() {
			baseOffset = tfhd.BaseDataOffset
		} else if tfhd.DefaultBaseIfMoof() {
			baseOffset = moofStartPos
		}
		if trun.HasDataOffset() {
			baseOffset = uint64(int64(trun.DataOffset) + int64(baseOffset))
		}
		mdatDataLength := uint64(len(mdat.Data)) // len should be fine for 64-bit
		var offsetInMdat uint64
		if baseOffset > 0 {
			offsetInMdat = baseOffset - mdat.PayloadAbsoluteOffset()
			if offsetInMdat > mdatDataLength {
				return nil, fmt.Errorf("offset in mdata beyond size")
			}
		} else {
			offsetInMdat = 0
		}
		samples = append(samples, trun.GetFullSamples(uint32(offsetInMdat), baseTime, mdat)...)
		baseTime += totalDur // Next trun start after this
	}

	return samples, nil
}

// AddFullSample - add a full sample to the first (and only) trun of a track
// AddFullSampleToTrack is the more general function
func (f *Fragment) AddFullSample(s FullSample) {
	trun := f.Moof.Traf.Trun
	if trun.SampleCount() == 0 {
		tfdt := f.Moof.Traf.Tfdt
		tfdt.SetBaseMediaDecodeTime(s.DecodeTime)
	}
	trun.AddSample(s.Sample)
	mdat := f.Mdat
	mdat.AddSampleData(s.Data)
}

// AddFullSampleToTrack - allows for adding samples to any track
// New trun boxes will be created if latest trun of fragment is not in this track
func (f *Fragment) AddFullSampleToTrack(s FullSample, trackID uint32) error {
	err := f.AddSampleToTrack(s.Sample, trackID, s.DecodeTime)
	if err != nil {
		return err
	}
	mdat := f.Mdat
	mdat.lazyDataSize = 0
	mdat.AddSampleData(s.Data)

	return nil
}

// AddSample - add a sample to the first (and only) trun of a track
// AddSampleToTrack is the more general function
func (f *Fragment) AddSample(s Sample, baseMediaDecodeTime uint64) {
	trun := f.Moof.Traf.Trun
	if trun.SampleCount() == 0 {
		tfdt := f.Moof.Traf.Tfdt
		tfdt.SetBaseMediaDecodeTime(baseMediaDecodeTime)
	}
	trun.AddSample(s)
	f.Mdat.lazyDataSize += uint64(s.Size)
}

// AddSamples - add a slice of Sample to the first (and only) trun of a track
func (f *Fragment) AddSamples(ss []Sample, baseMediaDecodeTime uint64) {
	trun := f.Moof.Traf.Trun
	if trun.SampleCount() == 0 {
		tfdt := f.Moof.Traf.Tfdt
		tfdt.SetBaseMediaDecodeTime(baseMediaDecodeTime)
	}
	trun.AddSamples(ss)
	var accSize uint64 = 0
	for _, s := range ss {
		accSize += uint64(s.Size)
	}
	f.Mdat.lazyDataSize += accSize
}

// AddSampleToTrack - allows for adding samples to any track
// New trun boxes will be created if latest trun of fragment is not in this track
// baseMediaDecodeTime will be used only for first sample in a trun
func (f *Fragment) AddSampleToTrack(s Sample, trackID uint32, baseMediaDecodeTime uint64) error {
	var traf *TrafBox
	for _, traf = range f.Moof.Trafs {
		if traf.Tfhd.TrackID == trackID {
			break
		}
	}
	if traf == nil {
		return fmt.Errorf("no track with trackID=%d", trackID)
	}
	if len(traf.Truns) == 0 { // Create first trun if needed
		trun := CreateTrun(f.nextTrunNr)
		f.nextTrunNr++
		err := traf.AddChild(trun)
		if err != nil {
			return err
		}
	}
	if len(traf.Truns) == 1 && traf.Trun.SampleCount() == 0 {
		tfdt := traf.Tfdt
		tfdt.SetBaseMediaDecodeTime(baseMediaDecodeTime)
	}
	trun := traf.Truns[len(traf.Truns)-1] // latest of this track
	if trun.writeOrderNr != f.nextTrunNr-1 {
		// We are not in the latest trun. Must make a new one
		trun = CreateTrun(f.nextTrunNr)
		f.nextTrunNr++
		err := traf.AddChild(trun)
		if err != nil {
			return err
		}
	}
	trun.AddSample(s)
	f.Mdat.lazyDataSize += uint64(s.Size)
	return nil
}

// Encode - write fragment via writer
func (f *Fragment) Encode(w io.Writer) error {
	if f.Moof == nil {
		return fmt.Errorf("moof not set in fragment")
	}
	traf := f.Moof.Traf
	if f.EncOptimize&OptimizeTrun != 0 {
		err := traf.OptimizeTfhdTrun()
		if err != nil {
			return err
		}
	}
	if f.Mdat == nil {
		return fmt.Errorf("mdat not set in fragment")
	}
	f.SetTrunDataOffsets()
	for _, b := range f.Children {
		err := b.Encode(w)
		if err != nil {
			return err
		}
	}
	return nil
}

// EncodeSW - write fragment via SliceWriter
func (f *Fragment) EncodeSW(sw bits.SliceWriter) error {
	if f.Moof == nil {
		return fmt.Errorf("moof not set in fragment")
	}
	traf := f.Moof.Traf
	if f.EncOptimize&OptimizeTrun != 0 {
		err := traf.OptimizeTfhdTrun()
		if err != nil {
			return err
		}
	}
	if f.Mdat == nil {
		return fmt.Errorf("mdat not set in fragment")
	}
	f.SetTrunDataOffsets()
	for _, c := range f.Children {
		err := c.EncodeSW(sw)
		if err != nil {
			return err
		}
	}
	return nil
}

// Info - write box-specific information
func (f *Fragment) Info(w io.Writer, specificBoxLevels, indent, indentStep string) error {
	for _, box := range f.Children {
		err := box.Info(w, specificBoxLevels, indent, indentStep)
		if err != nil {
			return err
		}
	}
	return nil
}

// GetChildren - return children boxes
func (f *Fragment) GetChildren() []Box {
	return f.Children
}

// SetTrunDataOffsets - if writeOrder available, sort and set dataOffset in truns
func (f *Fragment) SetTrunDataOffsets() {
	nrTruns := 0
	writeOrderSet := false
	for _, traf := range f.Moof.Trafs {
		for _, trun := range traf.Truns {
			nrTruns++
			if trun.writeOrderNr != 0 {
				writeOrderSet = true
			}
		}
	}
	if !writeOrderSet && nrTruns > 1 {
		return
	}

	truns := make([]*TrunBox, 0, nrTruns)
	for _, traf := range f.Moof.Trafs {
		truns = append(truns, traf.Truns...)
	}
	sort.Slice(truns, func(i, j int) bool {
		return truns[i].writeOrderNr < truns[j].writeOrderNr
	})
	dataOffset := f.Moof.Size() + f.Mdat.HeaderSize()
	for _, trun := range truns {
		trun.DataOffset = int32(dataOffset)
		dataOffset += trun.SizeOfData()
	}
}

// GetSampleNrFromTime - look up sample number from a specified time. Return error if no matching time
func (f *Fragment) GetSampleNrFromTime(trex *TrexBox, sampleTime uint64) (uint32, error) {
	if len(f.Moof.Trafs) != 1 {
		return 0, fmt.Errorf("not exactly one traf")
	}
	traf := f.Moof.Traf
	if len(traf.Truns) != 1 {
		return 0, fmt.Errorf("not exactly one trun")
	}
	baseDecodeTime := traf.Tfdt.BaseMediaDecodeTime()
	if baseDecodeTime > sampleTime {
		return 0, fmt.Errorf("sampleTime %d less that baseMediaDecodeTime %d", sampleTime, baseDecodeTime)
	}
	defaultSampleDuration := uint32(0)
	deltaTime := sampleTime - baseDecodeTime
	if trex != nil {
		defaultSampleDuration = trex.DefaultSampleDuration
	}
	if traf.Tfhd.HasDefaultSampleDuration() {
		defaultSampleDuration = traf.Tfhd.DefaultSampleDuration
	}
	return traf.Trun.GetSampleNrForRelativeTime(deltaTime, defaultSampleDuration)
}

// GetSampleInterval - get SampleInterval for a fragment with only one track
func (f *Fragment) GetSampleInterval(trex *TrexBox, startSampleNr, endSampleNr uint32) (SampleInterval, error) {
	moof := f.Moof
	if len(moof.Trafs) != 1 {
		return SampleInterval{}, fmt.Errorf("not exactly one track in fragment")
	}
	traf := moof.Traf
	if len(traf.Truns) != 1 {
		return SampleInterval{}, fmt.Errorf("not exactly 1, but %d trun boxes", len(traf.Truns))
	}
	tfhd, trun := traf.Tfhd, traf.Trun
	moofStartPos := moof.StartPos
	_ = trun.AddSampleDefaultValues(tfhd, trex)
	var baseOffset uint64
	if tfhd.HasBaseDataOffset() {
		baseOffset = tfhd.BaseDataOffset
	} else if tfhd.DefaultBaseIfMoof() {
		baseOffset = moofStartPos
	}
	if trun.HasDataOffset() {
		baseOffset = uint64(int64(trun.DataOffset) + int64(baseOffset))
	}
	offsetInMdat := uint32(baseOffset - f.Mdat.PayloadAbsoluteOffset())
	return trun.GetSampleInterval(startSampleNr, endSampleNr, traf.Tfdt.BaseMediaDecodeTime(), f.Mdat, offsetInMdat)
}

// AddSampleInterval - add SampleInterval for a fragment with only one track
func (f *Fragment) AddSampleInterval(sItvl SampleInterval) error {
	moof := f.Moof
	traf := moof.Traf
	trun := traf.Trun
	if len(moof.Trafs) != 1 || len(traf.Truns) != 1 {
		return fmt.Errorf("not exactly one track and one trun in fragment")
	}
	if trun.SampleCount() == 0 {
		traf.Tfdt.SetBaseMediaDecodeTime(sItvl.FirstDecodeTime)
	}
	trun.AddSamples(sItvl.Samples)
	f.Mdat.AddSampleDataPart(sItvl.Data)
	return nil
}

// CommonSampleDuration returns a common non-zero sample duration for a track defined by trex if available.
func (f *Fragment) CommonSampleDuration(trex *TrexBox) (uint32, error) {
	if trex == nil {
		return 0, fmt.Errorf("trex not set")
	}
	moof := f.Moof
	var traf *TrafBox
	trackID := trex.TrackID
	for _, t := range moof.Trafs {
		if t.Tfhd.TrackID == trackID {
			traf = t
			break
		}
	}
	if traf == nil {
		return 0, fmt.Errorf("no track with trex trackID=%d", trackID)
	}
	commonDur := trex.DefaultSampleDuration
	if traf.Tfhd.HasDefaultSampleDuration() {
		commonDur = traf.Tfhd.DefaultSampleDuration
	}
	for _, trun := range traf.Truns {
		cDur := trun.CommonSampleDuration(commonDur)
		if commonDur != 0 {
			if cDur != commonDur {
				commonDur = 0
			}
		} else {
			commonDur = cDur
		}
		if commonDur == 0 {
			break
		}
	}
	if commonDur == 0 {
		return 0, fmt.Errorf("no common sample duration")
	}
	return commonDur, nil
}
