package mp4

import (
	"fmt"
	"io"

	"github.com/Eyevinn/mp4ff/bits"
)

// MoofBox -  Movie Fragment Box (moof)
//
// Contains all meta-data. To be able to stream a file, the moov box should be placed before the mdat box.
type MoofBox struct {
	Mfhd     *MfhdBox
	Traf     *TrafBox // The first traf child box
	Trafs    []*TrafBox
	Pssh     *PsshBox
	Psshs    []*PsshBox
	Children []Box
	StartPos uint64
}

// DecodeMoof - box-specific decode
func DecodeMoof(hdr BoxHeader, startPos uint64, r io.Reader) (Box, error) {
	data := make([]byte, hdr.payloadLen())
	_, err := io.ReadFull(r, data)
	if err != nil {
		return nil, err
	}
	sr := bits.NewFixedSliceReader(data)
	children, err := DecodeContainerChildrenSR(hdr, startPos+8, startPos+hdr.Size, sr)
	if err != nil {
		return nil, err
	}
	m := MoofBox{Children: make([]Box, 0, len(children))}
	m.StartPos = startPos
	for _, c := range children {
		err := m.AddChild(c)
		if err != nil {
			return nil, err
		}
	}

	return &m, nil
}

// DecodeMoofSR - box-specific decode
func DecodeMoofSR(hdr BoxHeader, startPos uint64, sr bits.SliceReader) (Box, error) {
	children, err := DecodeContainerChildrenSR(hdr, startPos+8, startPos+hdr.Size, sr)
	if err != nil {
		return nil, err
	}
	m := MoofBox{Children: make([]Box, 0, len(children))}
	m.StartPos = startPos
	for _, c := range children {
		err := m.AddChild(c)
		if err != nil {
			return nil, err
		}
	}

	return &m, sr.AccError()
}

// AddChild - add child box
func (m *MoofBox) AddChild(child Box) error {
	switch box := child.(type) {
	case *MfhdBox:
		m.Mfhd = box
	case *TrafBox:
		if m.Traf == nil {
			m.Traf = box
		}
		m.Trafs = append(m.Trafs, box)
	case *PsshBox:
		if m.Pssh == nil {
			m.Pssh = box
		}
		m.Psshs = append(m.Psshs, box)
	}
	m.Children = append(m.Children, child)
	return nil
}

// Type - returns box type
func (m *MoofBox) Type() string {
	return "moof"
}

// Size - returns calculated size
func (m *MoofBox) Size() uint64 {
	return containerSize(m.Children)
}

// Encode - write moof after updating trun dataoffset
func (m *MoofBox) Encode(w io.Writer) error {
	for _, trun := range m.Traf.Truns {
		if trun.HasDataOffset() && trun.DataOffset == 0 {
			return fmt.Errorf("dataoffset in trun not set")
		}
	}
	err := EncodeHeader(m, w)
	if err != nil {
		return err
	}
	for _, b := range m.Children {
		err = b.Encode(w)
		if err != nil {
			return err
		}
	}
	return nil
}

// Encode - write moof after updating trun dataoffset
func (m *MoofBox) EncodeSW(sw bits.SliceWriter) error {
	for _, trun := range m.Traf.Truns {
		if trun.HasDataOffset() && trun.DataOffset == 0 {
			return fmt.Errorf("dataoffset in trun not set")
		}
	}
	err := EncodeHeaderSW(m, sw)
	if err != nil {
		return err
	}
	for _, c := range m.Children {
		err = c.EncodeSW(sw)
		if err != nil {
			return err
		}
	}
	return nil
}

// GetChildren - list of child boxes
func (m *MoofBox) GetChildren() []Box {
	return m.Children
}

// Info - write box-specific information
func (m *MoofBox) Info(w io.Writer, specificBoxLevels, indent, indentStep string) error {
	return ContainerInfo(m, w, specificBoxLevels, indent, indentStep)
}

// RemovePsshs - remove and return all psshs children boxes
func (m *MoofBox) RemovePsshs() (psshs []*PsshBox, totalSize uint64) {
	if m.Pssh == nil {
		return nil, 0
	}
	psshs = m.Psshs
	newChildren := make([]Box, 0, len(m.Children)-len(m.Psshs))
	for i := range m.Children {
		if m.Children[i].Type() != "pssh" {
			newChildren = append(newChildren, m.Children[i])
		}
	}
	m.Children = newChildren
	m.Pssh = nil
	m.Psshs = nil

	for _, pssh := range psshs {
		totalSize += pssh.Size()
	}

	return psshs, totalSize
}
