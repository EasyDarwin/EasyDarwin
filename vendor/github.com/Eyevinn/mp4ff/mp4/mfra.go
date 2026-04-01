package mp4

import (
	"io"

	"github.com/Eyevinn/mp4ff/bits"
)

// MfraBox - Movie Fragment Random Access Box (mfra)
// Container for TfraBox(es) that can be used to find sync samples
type MfraBox struct {
	Tfra     *TfraBox
	Tfras    []*TfraBox
	Mfro     *MfroBox
	Children []Box
	StartPos uint64
}

// DecodeMfra - box-specific decode
func DecodeMfra(hdr BoxHeader, startPos uint64, r io.Reader) (Box, error) {
	children, err := DecodeContainerChildren(hdr, startPos+8, startPos+hdr.Size, r)
	if err != nil {
		return nil, err
	}
	m := &MfraBox{}
	m.StartPos = startPos
	for _, box := range children {
		err := m.AddChild(box)
		if err != nil {
			return nil, err
		}
	}
	return m, nil
}

// DecodeMfraSR - box-specific decode
func DecodeMfraSR(hdr BoxHeader, startPos uint64, sr bits.SliceReader) (Box, error) {
	children, err := DecodeContainerChildrenSR(hdr, startPos+8, startPos+hdr.Size, sr)
	if err != nil {
		return nil, err
	}
	m := &MfraBox{}
	m.StartPos = startPos
	for _, box := range children {
		err := m.AddChild(box)
		if err != nil {
			return nil, err
		}
	}
	return m, nil
}

// AddChild - add child box
func (m *MfraBox) AddChild(child Box) error {
	switch box := child.(type) {
	case *TfraBox:
		if m.Tfra == nil {
			m.Tfra = box
		}
		m.Tfras = append(m.Tfras, box)
	case *MfroBox:
		m.Mfro = box
	}
	m.Children = append(m.Children, child)
	return nil
}

// Type - returns box type
func (m *MfraBox) Type() string {
	return "mfra"
}

// Size - returns calculated size
func (m *MfraBox) Size() uint64 {
	return containerSize(m.Children)
}

// Encode - write mfra container to w
func (m *MfraBox) Encode(w io.Writer) error {
	return EncodeContainer(m, w)
}

// EncodeSW- write mfra container via sw
func (m *MfraBox) EncodeSW(sw bits.SliceWriter) error {
	return EncodeContainerSW(m, sw)
}

// GetChildren - list of child boxes
func (m *MfraBox) GetChildren() []Box {
	return m.Children
}

// Info - write box-specific information
func (m *MfraBox) Info(w io.Writer, specificBoxLevels, indent, indentStep string) error {
	return ContainerInfo(m, w, specificBoxLevels, indent, indentStep)
}

// FindEntry - find tfra entry for given moof start offset and trackID. Return nil if not found.
func (m *MfraBox) FindEntry(moofStart uint64, trackID uint32) *TfraEntry {
	for _, tfra := range m.Tfras {
		if tfra.TrackID == trackID {
			return tfra.FindEntry(moofStart)
		}
	}
	return nil
}
