package sei

import (
	"encoding/binary"
	"fmt"
)

// ContentLightLevelInformationSEI is HEVC SEI Message 144.
// Defined in ISO/IEC 23008-2 D.2.35
type ContentLightLevelInformationSEI struct {
	MaxContentLightLevel    uint16
	MaxPicAverageLightLevel uint16
}

func (c ContentLightLevelInformationSEI) Type() uint {
	return SEIContentLightLevelInformationType
}

func (c ContentLightLevelInformationSEI) Size() uint {
	return 4
}

func (c ContentLightLevelInformationSEI) Payload() []byte {
	pl := make([]byte, c.Size())
	binary.BigEndian.PutUint16(pl[:2], c.MaxContentLightLevel)
	binary.BigEndian.PutUint16(pl[2:4], c.MaxPicAverageLightLevel)
	return pl
}

func (c ContentLightLevelInformationSEI) String() string {
	msgType := SEIType(c.Type()).String()
	return fmt.Sprintf("%s %dB: maxContentLightLevel=%d, maxPicAverageLightLevel=%d",
		msgType, c.Size(), c.MaxContentLightLevel, c.MaxPicAverageLightLevel)
}

// DecodeContentLightLevelInformationSEI decodes HEVC SEI 144.
func DecodeContentLightLevelInformationSEI(sd *SEIData) (SEIMessage, error) {
	c := ContentLightLevelInformationSEI{}
	data := sd.Payload()
	if len(data) != int(c.Size()) {
		return nil, fmt.Errorf("sei message size mismatch: %d instead of %d", len(data), c.Size())
	}
	c.MaxContentLightLevel = binary.BigEndian.Uint16(data[:2])
	c.MaxPicAverageLightLevel = binary.BigEndian.Uint16(data[2:4])
	return &c, nil
}
