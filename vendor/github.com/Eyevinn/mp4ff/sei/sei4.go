package sei

import (
	"encoding/binary"
	"encoding/hex"
	"fmt"
)

// DecodeUserDataRegisteredSEI decodes a SEI message of type 4.
func DecodeUserDataRegisteredSEI(sd *SEIData) (SEIMessage, error) {
	itutData := ITUData{
		CountryCode:      sd.payload[0],
		ProviderCode:     binary.BigEndian.Uint16(sd.payload[1:3]),
		UserIdentifier:   binary.BigEndian.Uint32(sd.payload[3:7]),
		UserDataTypeCode: sd.payload[7],
	}
	if itutData.IsCEA608() {
		return ExtractCEA608sei(sd)
	}
	return NewRegisteredSEI(sd, itutData), nil
}

// ITUData identifies registered payload in SEI of type 4 (User data registered by ITU-T Rec T 35).
type ITUData struct {
	CountryCode      byte
	UserDataTypeCode byte
	ProviderCode     uint16
	UserIdentifier   uint32
}

// IsCEA608 checks if ITU-T data corresponds to CEA-608.
func (i ITUData) IsCEA608() bool {
	return (i.CountryCode == 0xb5 &&
		i.ProviderCode == 0x31 &&
		i.UserIdentifier == 0x47413934 &&
		i.UserDataTypeCode == 0x3)
}

// RegisteredSEI is user_data_registered_itu_t_t35 (type 4) SEI message.
type RegisteredSEI struct {
	payload  []byte
	ITUTData ITUData
}

// NewRegisteredSEI creates an ITU-T registered SEI message (type 4).
func NewRegisteredSEI(sd *SEIData, ituData ITUData) *RegisteredSEI {
	return &RegisteredSEI{
		payload:  sd.payload,
		ITUTData: ituData,
	}
}

// Type returns the SEI payload type.
func (s *RegisteredSEI) Type() uint {
	return SEIUserDataRegisteredITUtT35Type
}

// Size returns size in bytes of raw SEI message rbsp payload.
func (s *RegisteredSEI) Size() uint {
	return uint(len(s.payload))
}

// String provides a short description of the SEI message.
func (s *RegisteredSEI) String() string {
	return fmt.Sprintf("SEI type %d, size=%d, %v", s.Type(), s.Size(), s.ITUTData)
}

// Payload returns the SEI raw rbsp payload.
func (s *RegisteredSEI) Payload() []byte {
	return s.payload
}

// ExtractCEA608sei returns payload and parsed field for CEA 608 SEI message.
// CEA-608 encapsulation in SEI nal unit is defined in ATSC-120 and further
// in CTA-708 specification (previously CEA-708).
func ExtractCEA608sei(sd *SEIData) (*CEA608sei, error) {
	field1, field2, err := ParseCEA608(sd.payload[8:])
	if err != nil {
		return nil, err
	}
	return &CEA608sei{
		payload: sd.payload,
		Field1:  field1,
		Field2:  field2,
	}, nil
}

// CEA608sei data structure.
type CEA608sei struct {
	payload []byte // full raw payload
	Field1  []byte
	Field2  []byte
}

// Type returns the SEI payload type.
func (s *CEA608sei) Type() uint {
	return SEIUserDataRegisteredITUtT35Type
}

// Size is size in bytes of raw SEI message rbsp payload.
func (s *CEA608sei) Size() uint {
	return uint(len(s.payload))
}

// String provides a simple representation of the CEA608 data.
func (s *CEA608sei) String() string {
	return fmt.Sprintf("SEI type %d CEA-608, size=%d, field1: %q, field2: %q", s.Type(), s.Size(),
		hex.EncodeToString(s.Field1), hex.EncodeToString(s.Field2))
}

// Payload returns the SEI raw rbsp payload.
func (s *CEA608sei) Payload() []byte {
	return s.payload
}

// ParseCEA608 parsers the the fields of data from CEA-708 encapsulation.
// This is specified in Section 4.3 of ANSI/CTA-708-E R-2018.
func ParseCEA608(payload []byte) ([]byte, []byte, error) {
	pos := 0
	ccCount := payload[pos] & 0x1f
	pos += 2 // Advance 1 and skip reserved byte
	var field1 []byte
	var field2 []byte

	for i := byte(0); i < ccCount; i++ {
		if len(payload) < pos+3 {
			return nil, nil, fmt.Errorf("not enough data for CEA-708 parsing")
		}
		b := payload[pos]
		ccValid := b & 0x4
		ccType := b & 0x3
		pos++
		ccData1 := payload[pos] // Keep parity bit
		pos++
		ccData2 := payload[pos] // Keep parity bit
		pos++
		if ccValid != 0 && ((ccData1&0x7f)+(ccData2&0x7f) != 0) { //Check validity and non-empty data
			if ccType == 0 {
				field1 = append(field1, ccData1)
				field1 = append(field1, ccData2)
			} else if ccType == 1 {
				field2 = append(field2, ccData1)
				field2 = append(field2, ccData2)
			}
		}
	}
	// There should also be a 0xff marker bits byte before the end of the NALU
	return field1, field2, nil
}
