package mp4

import (
	"errors"
	"fmt"
	"io"

	"github.com/Eyevinn/mp4ff/bits"
)

// TrafBox - Track Fragment Box (traf)
//
// Contained in : Movie Fragment Box (moof)
type TrafBox struct {
	Tfhd     *TfhdBox
	Tfdt     *TfdtBox
	Saiz     *SaizBox
	Saio     *SaioBox
	Sbgp     *SbgpBox
	Sgpd     *SgpdBox
	Senc     *SencBox
	UUIDSenc *UUIDBox // A PIFF box of subtype senc
	Trun     *TrunBox // The first TrunBox
	Truns    []*TrunBox
	Children []Box
}

// DecodeTraf - box-specific decode
func DecodeTraf(hdr BoxHeader, startPos uint64, r io.Reader) (Box, error) {
	children, err := DecodeContainerChildren(hdr, startPos+8, startPos+hdr.Size, r)
	if err != nil {
		return nil, err
	}
	t := &TrafBox{Children: make([]Box, 0, len(children))}
	for _, child := range children {
		err := t.AddChild(child)
		if err != nil {
			return nil, err
		}
	}
	return t, nil
}

// DecodeTrafSR - box-specific decode
func DecodeTrafSR(hdr BoxHeader, startPos uint64, sr bits.SliceReader) (Box, error) {
	children, err := DecodeContainerChildrenSR(hdr, startPos+8, startPos+hdr.Size, sr)
	if err != nil {
		return nil, err
	}
	t := &TrafBox{Children: make([]Box, 0, len(children))}
	for _, child := range children {
		err := t.AddChild(child)
		if err != nil {
			return nil, err
		}
	}
	return t, nil
}

// ContainsSencBox - is there a senc box in traf and is it parsed
// If not parsed, call ParseReadSenc to parse it
func (t *TrafBox) ContainsSencBox() (ok, parsed bool) {
	for _, c := range t.Children {
		switch box := c.(type) {
		case *SencBox:
			return true, !box.readButNotParsed
		case *UUIDBox: // PIFF
			if box.SubType() == "senc" {
				return true, !box.Senc.readButNotParsed
			}
		}
	}
	return false, false
}

// ParseReadSenc makes a second round to parse a senc box previously read
func (t *TrafBox) ParseReadSenc(defaultIVSize byte, moofStartPos uint64) error {
	if t.Senc == nil && t.UUIDSenc == nil {
		return fmt.Errorf("no senc box or uuid senc box")
	}
	var senc *SencBox
	if t.Senc != nil {
		senc = t.Senc
	} else {
		senc = t.UUIDSenc.Senc
	}
	if t.Saio != nil {
		// saio should be present, but we try without it, if it doesn't exist
		posFromSaio := t.Saio.Offset[0] + int64(moofStartPos)
		if uint64(posFromSaio) != senc.StartPos+16 {
			//TODO. Reenable
			return fmt.Errorf("offset from saio (%d) relative moof start differs from senc data start %d", posFromSaio, senc.StartPos+16)
			//fmt.Printf("offset from saio (%d) and moof differs from senc data start %d\n", posFromSaio, senc.StartPos+16)

		}
	}
	perSampleIVSize := defaultIVSize
	sbgp, sgpd := t.Sbgp, t.Sgpd
	if sbgp != nil && sbgp.GroupingType == "seig" && sgpd != nil && sgpd.GroupingType == "seig" {
		nrSbgpEntries := len(sbgp.SampleCounts)
		if nrSbgpEntries != 1 {
			return fmt.Errorf("sbgp entries = %d, only 1 supported for now", nrSbgpEntries)
		}
		sgpdEntryNr := sbgp.GroupDescriptionIndices[0]
		if sgpdEntryNr != sbgpInsideOffset+1 {
			return fmt.Errorf("sgpd entry number must be first inside = 65536 + 1")
		}
		sgpdEntry := sgpd.SampleGroupEntries[sgpdEntryNr-sbgpInsideOffset-1]
		seigEntry := sgpdEntry.(*SeigSampleGroupEntry)
		perSampleIVSize = seigEntry.PerSampleIVSize
	}
	err := senc.ParseReadBox(perSampleIVSize, t.Saiz)
	if err != nil {
		return err
	}
	return nil
}

// AddChild - add child box
func (t *TrafBox) AddChild(child Box) error {
	switch box := child.(type) {
	case *TfhdBox:
		t.Tfhd = box
	case *TfdtBox:
		t.Tfdt = box
	case *SaizBox:
		t.Saiz = box
	case *SaioBox:
		t.Saio = box
	case *SbgpBox:
		t.Sbgp = box
	case *SgpdBox:
		t.Sgpd = box
	case *SencBox:
		t.Senc = box
	case *TrunBox:
		if t.Trun == nil {
			t.Trun = box
		}
		t.Truns = append(t.Truns, box)
	case *UUIDBox:
		if box.SubType() == "senc" {
			t.UUIDSenc = box
		}
	default:
	}
	t.Children = append(t.Children, child)
	return nil
}

// Type - return box type
func (t *TrafBox) Type() string {
	return "traf"
}

// Size - return calculated size
func (t *TrafBox) Size() uint64 {
	return containerSize(t.Children)
}

// GetChildren - list of child boxes
func (t *TrafBox) GetChildren() []Box {
	return t.Children
}

// Encode - write box to w
func (t *TrafBox) Encode(w io.Writer) error {
	return EncodeContainer(t, w)
}

// Encode - write minf container to sw
func (b *TrafBox) EncodeSW(sw bits.SliceWriter) error {
	return EncodeContainerSW(b, sw)
}

// Info - write box-specific information
func (t *TrafBox) Info(w io.Writer, specificBoxLevels, indent, indentStep string) error {
	return ContainerInfo(t, w, specificBoxLevels, indent, indentStep)
}

// OptimizeTfhdTrun - optimize trun by default values in tfhd box
// Only look at first trun, even if there is more than one
// Don't optimize again, if already done so that no data is present
func (t *TrafBox) OptimizeTfhdTrun() error {
	tfhd := t.Tfhd
	trun := t.Trun
	if len(trun.Samples) == 0 {
		return errors.New("no samples in trun")
	}
	if len(trun.Samples) == 1 {
		return nil // No need to optimize
	}

	if trun.HasSampleDuration() {
		hasCommonDur := true
		commonDur := trun.Samples[0].Dur
		for _, s := range trun.Samples {
			if s.Dur != commonDur {
				hasCommonDur = false
				break
			}
		}
		if hasCommonDur {
			// Set defaultSampleDuration in tfhd and remove from trun
			tfhd.Flags = tfhd.Flags | defaultSampleDurationPresent
			tfhd.DefaultSampleDuration = commonDur
			trun.Flags = trun.Flags & ^TrunSampleDurationPresentFlag
		}
	}

	if trun.HasSampleSize() {
		hasCommonSize := true
		commonSize := trun.Samples[0].Size
		for _, s := range trun.Samples {
			if s.Size != commonSize {
				hasCommonSize = false
				break
			}
		}
		if hasCommonSize {
			// Set defaultSampleSize in tfhd and remove from trun
			tfhd.Flags = tfhd.Flags | defaultSampleSizePresent
			tfhd.DefaultSampleSize = commonSize
			trun.Flags = trun.Flags & ^TrunSampleSizePresentFlag
		}
	}

	if trun.HasSampleFlags() {
		firstSampleFlags := trun.Samples[0].Flags
		hasCommonFlags := true
		commonSampleFlags := trun.Samples[1].Flags
		for i, s := range trun.Samples {
			if i >= 1 {
				if s.Flags != commonSampleFlags {
					hasCommonFlags = false
					break
				}
			}
		}
		if hasCommonFlags {
			if firstSampleFlags != commonSampleFlags {
				trun.SetFirstSampleFlags(firstSampleFlags)
			}
			tfhd.Flags = tfhd.Flags | defaultSampleFlagsPresent
			tfhd.DefaultSampleFlags = commonSampleFlags
			trun.Flags = trun.Flags & ^TrunSampleFlagsPresentFlag
		}
	}

	if trun.HasSampleCompositionTimeOffset() {
		allZeroCTO := true
		for _, s := range trun.Samples {
			if s.CompositionTimeOffset != 0 {
				allZeroCTO = false
				break
			}
		}
		if allZeroCTO {
			trun.Flags = trun.Flags & ^TrunSampleCompositionTimeOffsetPresentFlag
		}
	}
	return nil
}

// RemoveEncryptionBoxes - remove encryption boxes and return number of bytes removed
func (t *TrafBox) RemoveEncryptionBoxes() uint64 {
	remainingChildren := make([]Box, 0, len(t.Children))
	var nrBytesRemoved uint64 = 0
	for _, ch := range t.Children {
		switch box := ch.(type) {
		case *SaizBox:
			nrBytesRemoved += ch.Size()
			t.Saiz = nil
		case *SaioBox:
			nrBytesRemoved += ch.Size()
			t.Saio = nil
		case *SencBox:
			nrBytesRemoved += ch.Size()
			t.Senc = nil
		case *UUIDBox:
			if box.SubType() == "senc" {
				nrBytesRemoved += ch.Size()
				t.UUIDSenc = nil
			}
		default:
			remainingChildren = append(remainingChildren, ch)
		}
	}
	t.Children = remainingChildren
	return nrBytesRemoved
}
