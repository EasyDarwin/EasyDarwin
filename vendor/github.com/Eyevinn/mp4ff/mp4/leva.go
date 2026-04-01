package mp4

import (
	"fmt"
	"io"

	"github.com/Eyevinn/mp4ff/bits"
)

/*
Definition according to ISO/IEC 14496-12 Section 8.8.13.2
aligned(8) class LevelAssignmentBox extends FullBox('leva', 0, 0) {
  unsigned int(8) level_count;
  for (j=1; j <= level_count; j++) {
    unsigned int(32) track_ID;
    unsigned int(1) padding_flag;
    unsigned int(7) assignment_type;
    if (assignment_type == 0) {
      unsigned int(32) grouping_type;
    } else if (assignment_type == 1) {
      unsigned int(32) grouping_type;
      unsigned int(32) grouping_type_parameter;
    } else if (assignment_type == 2) {
      // no further syntax elements needed
    } else if (assignment_type == 3) {
      // no further syntax elements needed
	} else if (assignment_type == 4) {
      unsigned int(32) sub_track_ID;
    }
    // other assignment_type values are reserved
  }
}
*/

// LevaBox - Subsegment Index Box according to ISO/IEC 14496-12 Section 8.8.13.2.
type LevaBox struct {
	Version byte
	Flags   uint32
	Levels  []LevaLevel
}

// LevaLevel - level data for LevaBox
type LevaLevel struct {
	TrackID                  uint32
	GroupingType             uint32
	GroupingTypeParameter    uint32
	SubTrackID               uint32
	paddingAndAssignmentType byte
}

// PaddingFlag - return padding flag.
func (l LevaLevel) PaddingFlag() bool {
	return l.paddingAndAssignmentType&0x80 != 0
}

// AssignmentType - return assignment type.
func (l LevaLevel) AssignmentType() byte {
	return l.paddingAndAssignmentType & 0x7f
}

// Size - return calculated size.
func (l LevaLevel) Size() uint64 {
	size := uint64(5)
	switch l.paddingAndAssignmentType & 0x7f {
	case 0:
		size += 4
	case 1:
		size += 8
	case 4:
		size += 4
	}
	return size
}

// NewLevaLevel - create new level for LevaBox.
func NewLevaLevel(trackID uint32, paddingFlag bool, assignmentType byte,
	groupingType, groupingTypeParameter, subTrackID uint32) (LevaLevel, error) {
	ll := LevaLevel{
		TrackID: trackID,
	}
	if assignmentType > 4 {
		return ll, fmt.Errorf("assignmentType %d not supported", assignmentType)
	}
	data := assignmentType
	if paddingFlag {
		data |= 0x80
	}
	ll.paddingAndAssignmentType = data
	switch assignmentType {
	case 0:
		ll.GroupingType = groupingType
	case 1:
		ll.GroupingType = groupingType
		ll.GroupingTypeParameter = groupingTypeParameter
	case 2:
	case 3:
	case 4:
		ll.SubTrackID = subTrackID
	}
	return ll, nil
}

// DecodeLeva - box-specific decode
func DecodeLeva(hdr BoxHeader, startPos uint64, r io.Reader) (Box, error) {
	data, err := readBoxBody(r, hdr)
	if err != nil {
		return nil, err
	}
	sr := bits.NewFixedSliceReader(data)
	return DecodeLevaSR(hdr, startPos, sr)
}

// DecodeLevaSR - box-specific decode
func DecodeLevaSR(hdr BoxHeader, startPos uint64, sr bits.SliceReader) (Box, error) {
	versionAndFlags := sr.ReadUint32()
	version := byte(versionAndFlags >> 24)

	b := &LevaBox{
		Version: version,
		Flags:   versionAndFlags & flagsMask,
	}
	levelCount := sr.ReadUint8()
	b.Levels = make([]LevaLevel, 0, levelCount)
	for j := 0; j < int(levelCount); j++ {
		trackID := sr.ReadUint32()
		paddingAndAssignmentType := sr.ReadUint8()
		lvl := LevaLevel{
			TrackID:                  trackID,
			paddingAndAssignmentType: paddingAndAssignmentType,
		}
		switch lvl.AssignmentType() {
		case 0:
			lvl.GroupingType = sr.ReadUint32()
		case 1:
			lvl.GroupingType = sr.ReadUint32()
			lvl.GroupingTypeParameter = sr.ReadUint32()
		case 4:
			lvl.SubTrackID = sr.ReadUint32()
		}
		b.Levels = append(b.Levels, lvl)
	}
	return b, sr.AccError()
}

// Type - return box type
func (b *LevaBox) Type() string {
	return "leva"
}

// Size - return calculated size
func (b *LevaBox) Size() uint64 {
	// Add up all fields depending on version
	size := uint64(boxHeaderSize + 4 + 1)
	for _, lvl := range b.Levels {
		size += lvl.Size()
	}
	return size
}

// Encode - write box to w
func (b *LevaBox) Encode(w io.Writer) error {
	sw := bits.NewFixedSliceWriter(int(b.Size()))
	err := b.EncodeSW(sw)
	if err != nil {
		return err
	}
	_, err = w.Write(sw.Bytes())
	return err
}

// EncodeSW - box-specific encode to slicewriter
func (b *LevaBox) EncodeSW(sw bits.SliceWriter) error {
	err := EncodeHeaderSW(b, sw)
	if err != nil {
		return err
	}
	versionAndFlags := (uint32(b.Version) << 24) + b.Flags
	sw.WriteUint32(versionAndFlags)
	sw.WriteUint8(uint8(len(b.Levels)))
	for _, lvl := range b.Levels {
		sw.WriteUint32(lvl.TrackID)
		sw.WriteUint8(lvl.paddingAndAssignmentType)
		switch lvl.AssignmentType() {
		case 0:
			sw.WriteUint32(lvl.GroupingType)
		case 1:
			sw.WriteUint32(lvl.GroupingType)
			sw.WriteUint32(lvl.GroupingTypeParameter)
		case 4:
			sw.WriteUint32(lvl.SubTrackID)
		}
	}
	return sw.AccError()
}

// Info - more info for level 1
func (b *LevaBox) Info(w io.Writer, specificBoxLevels, indent, indentStep string) error {
	bd := newInfoDumper(w, indent, b, int(b.Version), b.Flags)
	bd.write(" - levelCount: %d", len(b.Levels))
	level := getInfoLevel(b, specificBoxLevels)
	if level >= 1 {
		for i, lvl := range b.Levels {
			switch lvl.AssignmentType() {
			case 0:
				bd.write(" - level[%d]: trackID=%d paddingFlag=%t assignmentType=%d groupingType=%d",
					i+1, lvl.TrackID, lvl.PaddingFlag(), lvl.AssignmentType(), lvl.GroupingType)
			case 1:
				bd.write(" - level[%d]: trackID=%d paddingFlag=%t assignmentType=%d groupingType=%d groupingTypeParameter=%d",
					i+1, lvl.TrackID, lvl.PaddingFlag(), lvl.AssignmentType(), lvl.GroupingType, lvl.GroupingTypeParameter)
			case 4:
				bd.write(" - level[%d]: trackID=%d paddingFlag=%t assignmentType=%d subTrackID=%d",
					i+1, lvl.TrackID, lvl.PaddingFlag(), lvl.AssignmentType(), lvl.SubTrackID)
			default:
				bd.write(" - level[%d]: trackID=%d paddingFlag=%t assignmentType=%d",
					i+1, lvl.TrackID, lvl.PaddingFlag(), lvl.AssignmentType())
			}
		}
	}
	return bd.err
}
