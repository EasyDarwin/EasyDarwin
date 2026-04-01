package sei

import (
	"encoding/hex"
	"fmt"
)

// UnregisteredSEI is SEI message of type 5.
type UnregisteredSEI struct {
	UUID    []byte
	payload []byte // raw rbsp payload including UUID
}

// Type returns SEI payload type.
func (s *UnregisteredSEI) Type() uint {
	return SEIUserDataUnregisteredType
}

// Size returns size in bytes of raw SEI message rbsp payload.
func (s *UnregisteredSEI) Size() uint {
	return uint(len(s.payload))
}

// String provides a short description of the SEI message.
func (s *UnregisteredSEI) String() string {
	payloadAfterUUID := string(s.payload[16:])
	return fmt.Sprintf("SEI type %d, size=%d, uuid=%q, payload=%q",
		s.Type(), s.Size(), hex.EncodeToString(s.UUID), payloadAfterUUID)
}

// Payload returns the SEI raw rbsp payload.
func (s *UnregisteredSEI) Payload() []byte {
	return s.payload
}

// DecodeUserDataUnregisteredSEI decodes an unregistered SEI message (type 5).
func DecodeUserDataUnregisteredSEI(sd *SEIData) (SEIMessage, error) {
	uuid := sd.payload[:16]
	return NewUnregisteredSEI(sd, uuid), nil
}

// NewUnregisteredSEI creates an unregistered SEI message (type 5).
func NewUnregisteredSEI(sd *SEIData, uuid []byte) *UnregisteredSEI {
	return &UnregisteredSEI{
		UUID:    uuid,
		payload: sd.payload,
	}
}
