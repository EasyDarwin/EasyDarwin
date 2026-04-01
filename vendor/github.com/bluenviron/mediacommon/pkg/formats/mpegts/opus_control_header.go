package mpegts

import (
	"fmt"

	"github.com/bluenviron/mediacommon/pkg/bits"
)

func unmarshalPayloadSize(buf []byte, pos *int) (int, error) {
	res := 0

	for {
		next, err := bits.ReadBits(buf, pos, 8)
		if err != nil {
			return 0, err
		}

		res += int(next)

		if next != 255 {
			break
		}
	}

	return res, nil
}

func marshalPayloadSizeSize(v int) int {
	return v/255 + 1
}

func marshalPayloadSize(v int, ss int, buf []byte) {
	for i := 0; i < (ss - 1); i++ {
		buf[i] = 255
	}
	buf[ss-1] = byte(v % 255)
}

type opusControlHeader struct {
	PayloadSize            int
	StartTrimFlag          bool
	EndTrimFlag            bool
	ControlExtensionFlag   bool
	StartTrim              uint16
	EndTrim                uint16
	ControlExtensionLength uint8
}

func (h *opusControlHeader) unmarshal(buf []byte) (int, error) {
	pos := 0

	err := bits.HasSpace(buf, pos, 16)
	if err != nil {
		return 0, err
	}

	prefix := bits.ReadBitsUnsafe(buf, &pos, 11)
	if prefix != 0x3ff {
		return 0, fmt.Errorf("invalid prefix")
	}

	h.StartTrimFlag = bits.ReadFlagUnsafe(buf, &pos)
	h.EndTrimFlag = bits.ReadFlagUnsafe(buf, &pos)
	h.ControlExtensionFlag = bits.ReadFlagUnsafe(buf, &pos)

	pos += 2 // reserved

	h.PayloadSize, err = unmarshalPayloadSize(buf, &pos)
	if err != nil {
		return 0, err
	}

	if h.StartTrimFlag {
		err := bits.HasSpace(buf, pos, 16)
		if err != nil {
			return 0, err
		}

		pos += 3 // reserved
		h.StartTrim = uint16(bits.ReadBitsUnsafe(buf, &pos, 13))
	}

	if h.EndTrimFlag {
		err := bits.HasSpace(buf, pos, 16)
		if err != nil {
			return 0, err
		}

		pos += 3 // reserved
		h.EndTrim = uint16(bits.ReadBitsUnsafe(buf, &pos, 13))
	}

	if h.ControlExtensionFlag {
		tmp, err := bits.ReadBits(buf, &pos, 8)
		if err != nil {
			return 0, err
		}
		h.ControlExtensionLength = uint8(tmp)

		space := 8 * int(h.ControlExtensionLength)
		err = bits.HasSpace(buf, pos, space)
		if err != nil {
			return 0, err
		}

		pos += space // reserved
	}

	return pos / 8, nil
}

func (h *opusControlHeader) marshalSize() int {
	n := 2 + marshalPayloadSizeSize(h.PayloadSize)
	if h.StartTrimFlag {
		n += 2
	}
	if h.EndTrimFlag {
		n += 2
	}
	if h.ControlExtensionFlag {
		n++
		n += int(h.ControlExtensionLength)
	}
	return n
}

func (h *opusControlHeader) marshalTo(buf []byte) (int, error) {
	buf[0] = 0b01111111

	buf[1] = 0b111 << 5
	if h.StartTrimFlag {
		buf[1] |= 1 << 4
	}
	if h.EndTrimFlag {
		buf[1] |= 1 << 3
	}
	if h.ControlExtensionFlag {
		buf[1] |= 1 << 2
	}

	ss := marshalPayloadSizeSize(h.PayloadSize)
	marshalPayloadSize(h.PayloadSize, ss, buf[2:])
	pos := 2 + ss

	if h.StartTrimFlag {
		buf[pos] = uint8(h.StartTrim >> 8)
		buf[pos+1] = uint8(h.StartTrim)
		pos += 2
	}

	if h.EndTrimFlag {
		buf[pos] = uint8(h.EndTrim >> 8)
		buf[pos+1] = uint8(h.EndTrim)
		pos += 2
	}

	if h.ControlExtensionFlag {
		buf[pos] = h.ControlExtensionLength
		pos++
		pos += int(h.ControlExtensionLength)
	}

	return pos, nil
}
