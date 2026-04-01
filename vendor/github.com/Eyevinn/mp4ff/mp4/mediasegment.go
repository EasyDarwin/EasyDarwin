package mp4

import (
	"fmt"
	"io"

	"github.com/Eyevinn/mp4ff/bits"
)

// MediaSegment is an MP4 Media Segment with one or more Fragments.
type MediaSegment struct {
	Styp        *StypBox
	Sidx        *SidxBox   // The first sidx box in a segment
	Sidxs       []*SidxBox // All sidx boxes in a segment
	Fragments   []*Fragment
	EncOptimize EncOptimize
	StartPos    uint64 // Start position in file
}

// NewMediaSegment - create empty MediaSegment with CMAF styp box
func NewMediaSegment() *MediaSegment {
	return &MediaSegment{
		Styp:        CreateStyp(),
		Fragments:   nil,
		EncOptimize: OptimizeNone,
	}
}

// NewMediaSegmentWithStyp - create empty MediaSegment with styp box
func NewMediaSegmentWithStyp(styp *StypBox) *MediaSegment {
	return &MediaSegment{
		Styp:        styp,
		Fragments:   nil,
		EncOptimize: OptimizeNone,
	}
}

// NewMediaSegmentWithoutStyp - create empty media segment with no styp box
func NewMediaSegmentWithoutStyp() *MediaSegment {
	return &MediaSegment{
		Styp:        nil,
		Fragments:   nil,
		EncOptimize: OptimizeNone,
	}
}

// AddSidx adds a sidx box to the MediaSegment.
func (s *MediaSegment) AddSidx(sidx *SidxBox) {
	if len(s.Sidxs) == 0 {
		s.Sidx = sidx
	}
	s.Sidxs = append(s.Sidxs, sidx)
}

// AddFragment - Add a fragment to a MediaSegment
func (s *MediaSegment) AddFragment(f *Fragment) {
	s.Fragments = append(s.Fragments, f)
}

// LastFragment returns the currently last fragment, or nil if no fragments.
func (s *MediaSegment) LastFragment() *Fragment {
	if len(s.Fragments) == 0 {
		return nil
	}
	return s.Fragments[len(s.Fragments)-1]
}

// Size - return size of media segment
func (s *MediaSegment) Size() uint64 {
	var size uint64 = 0
	if s.Styp != nil {
		size += s.Styp.Size()
	}
	if s.Sidx != nil {
		size += s.Sidx.Size()
	}
	for _, f := range s.Fragments {
		size += f.Size()
	}
	return size
}

// Encode - Write MediaSegment via writer
func (s *MediaSegment) Encode(w io.Writer) error {
	if s.Styp != nil {
		err := s.Styp.Encode(w)
		if err != nil {
			return err
		}
	}
	if len(s.Sidxs) > 0 {
		for i := range s.Sidxs {
			err := s.Sidxs[i].Encode(w)
			if err != nil {
				return err
			}
		}
	}
	for _, f := range s.Fragments {
		f.EncOptimize = s.EncOptimize
		err := f.Encode(w)
		if err != nil {
			return err
		}
	}
	return nil
}

// EncodeSW - Write MediaSegment via SliceWriter
func (s *MediaSegment) EncodeSW(sw bits.SliceWriter) error {
	if s.Styp != nil {
		err := s.Styp.EncodeSW(sw)
		if err != nil {
			return err
		}
	}
	if len(s.Sidxs) > 0 {
		for i := range s.Sidxs {
			err := s.Sidxs[i].EncodeSW(sw)
			if err != nil {
				return err
			}
		}
	}
	for _, f := range s.Fragments {
		f.EncOptimize = s.EncOptimize
		err := f.EncodeSW(sw)
		if err != nil {
			return err
		}
	}
	return nil
}

// Info - write box tree with indent for each level
func (s *MediaSegment) Info(w io.Writer, specificBoxLevels, indent, indentStep string) error {
	if s.Styp != nil {
		err := s.Styp.Info(w, specificBoxLevels, indent, indentStep)
		if err != nil {
			return err
		}
	}
	if len(s.Sidxs) > 0 {
		for i := range s.Sidxs {
			err := s.Sidxs[i].Info(w, specificBoxLevels, indent, indentStep)
			if err != nil {
				return err
			}
		}
	}
	for _, f := range s.Fragments {
		err := f.Info(w, specificBoxLevels, indent, indentStep)
		if err != nil {
			return err
		}
	}
	return nil
}

// Fragmentify - Split into multiple fragments. Assume single mdat and trun for now
func (s *MediaSegment) Fragmentify(timescale uint64, trex *TrexBox, duration uint32) ([]*Fragment, error) {
	inFragments := s.Fragments
	outFragments := make([]*Fragment, 0)
	var of *Fragment

	var cumDur uint32 = 0

	for _, inFrag := range inFragments {
		trackID := inFrag.Moof.Traf.Tfhd.TrackID

		samples, err := inFrag.GetFullSamples(trex)
		if err != nil {
			return nil, err
		}
		for _, s := range samples {
			if cumDur == 0 {
				var err error
				of, err = CreateFragment(inFrag.Moof.Mfhd.SequenceNumber, trackID)
				if err != nil {
					return nil, err
				}
				outFragments = append(outFragments, of)
			}
			//of.AddFullSample(s)
			err = of.AddFullSampleToTrack(s, trackID)
			if err != nil {
				return nil, err
			}
			cumDur += s.Dur
			if cumDur >= duration {
				// fmt.Printf("Wrote fragment with duration %d\n", cumDur)
				cumDur = 0
			}
		}
	}
	return outFragments, nil
}

// CommonSampleDuration returns a common non-zero sample duration for a track defined by trex if available.
func (s *MediaSegment) CommonSampleDuration(trex *TrexBox) (uint32, error) {
	if trex == nil {
		return 0, fmt.Errorf("trex not set")
	}
	var commonDur uint32
	for i, frag := range s.Fragments {
		cDur, err := frag.CommonSampleDuration(trex)
		if err != nil {
			return 0, fmt.Errorf("fragment.CommonSampleDuration: %w", err)
		}
		if i == 0 {
			commonDur = cDur
		} else if commonDur != cDur {
			return 0, fmt.Errorf("different common sample duration in fragment %d", i+1)
		}
	}
	return commonDur, nil
}

// FirstBox returns the first box in the segment, or an error if no boxes are found.
func (s *MediaSegment) FirstBox() (Box, error) {
	if s.Styp != nil {
		return s.Styp, nil
	}
	if len(s.Sidxs) > 0 {
		return s.Sidxs[0], nil
	}
	if len(s.Fragments) > 0 {
		if len(s.Fragments[0].Children) > 0 {
			return s.Fragments[0].Children[0], nil
		}
	}
	return nil, fmt.Errorf("no boxes in segment")
}
