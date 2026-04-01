package sei

import (
	"encoding/binary"
	"fmt"
)

// MasteringDisplayColourVolumeSEI is HEVC SEI Message 137.
// Defined in ISO/IEC 23008-2 D.2.28
type MasteringDisplayColourVolumeSEI struct {
	DisplayPrimariesX            [3]uint16
	DisplayPrimariesY            [3]uint16
	WhitePointX                  uint16
	WhitePointY                  uint16
	MaxDisplayMasteringLuminance uint32
	MinDisplayMasteringLuminance uint32
}

func (m MasteringDisplayColourVolumeSEI) Type() uint {
	return SEIMasteringDisplayColourVolumeType
}

func (m MasteringDisplayColourVolumeSEI) Size() uint {
	return 24
}

func (m MasteringDisplayColourVolumeSEI) Payload() []byte {
	pl := make([]byte, m.Size())
	pos := 0
	for i := 0; i < 3; i++ {
		binary.BigEndian.PutUint16(pl[pos:pos+2], m.DisplayPrimariesX[i])
		pos += 2
		binary.BigEndian.PutUint16(pl[pos:pos+2], m.DisplayPrimariesY[i])
		pos += 2
	}
	binary.BigEndian.PutUint16(pl[pos:pos+2], m.WhitePointX)
	pos += 2
	binary.BigEndian.PutUint16(pl[pos:pos+2], m.WhitePointY)
	pos += 2
	binary.BigEndian.PutUint32(pl[pos:pos+4], m.MaxDisplayMasteringLuminance)
	pos += 4
	binary.BigEndian.PutUint32(pl[pos:pos+4], m.MinDisplayMasteringLuminance)
	return pl
}

func (m MasteringDisplayColourVolumeSEI) String() string {
	msgType := SEIType(m.Type()).String()
	return fmt.Sprintf("%s %dB: primaries=(%d, %d) (%d, %d) (%d, %d), whitePoint=(%d, %d), maxLum=%d, minLum=%d",
		msgType, m.Size(),
		m.DisplayPrimariesX[0], m.DisplayPrimariesY[0],
		m.DisplayPrimariesX[1], m.DisplayPrimariesY[1],
		m.DisplayPrimariesX[2], m.DisplayPrimariesY[2],
		m.WhitePointX, m.WhitePointY,
		m.MaxDisplayMasteringLuminance, m.MinDisplayMasteringLuminance)
}

// DecodeUserDataUnregisteredSEI - Decode an unregistered SEI message (type 5)
func DecodeMasteringDisplayColourVolumeSEI(sd *SEIData) (SEIMessage, error) {
	m := MasteringDisplayColourVolumeSEI{}
	data := sd.Payload()
	if len(data) != int(m.Size()) {
		return nil, fmt.Errorf("sei message size mismatch: %d instead of %d", len(data), m.Size())
	}
	pos := 0
	for i := 0; i < 3; i++ {
		m.DisplayPrimariesX[i] = binary.BigEndian.Uint16(data[pos:])
		pos += 2
		m.DisplayPrimariesY[i] = binary.BigEndian.Uint16(data[pos:])
		pos += 2
	}
	m.WhitePointX = binary.BigEndian.Uint16(data[pos:])
	pos += 2
	m.WhitePointY = binary.BigEndian.Uint16(data[pos:])
	pos += 2
	m.MaxDisplayMasteringLuminance = binary.BigEndian.Uint32(data[pos:])
	pos += 4
	m.MinDisplayMasteringLuminance = binary.BigEndian.Uint32(data[pos:])
	return &m, nil
}
