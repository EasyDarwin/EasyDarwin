package av1

import (
	"fmt"
)

func obuRemoveSize(h *OBUHeader, sizeN int, ob []byte) []byte {
	newOBU := make([]byte, len(ob)-sizeN)
	newOBU[0] = (byte(h.Type) << 3)
	copy(newOBU[1:], ob[1+sizeN:])
	return newOBU
}

// BitstreamUnmarshal extracts a temporal unit from a bitstream.
// Optionally, it also removes the size field from OBUs.
// Specification: https://aomediacodec.github.io/av1-spec/#low-overhead-bitstream-format
func BitstreamUnmarshal(bs []byte, removeSizeField bool) ([][]byte, error) {
	var ret [][]byte

	for {
		var h OBUHeader
		err := h.Unmarshal(bs)
		if err != nil {
			return nil, err
		}

		if !h.HasSize {
			return nil, fmt.Errorf("OBU size not present")
		}

		size, sizeN, err := LEB128Unmarshal(bs[1:])
		if err != nil {
			return nil, err
		}

		obuLen := 1 + sizeN + int(size)
		if len(bs) < obuLen {
			return nil, fmt.Errorf("not enough bytes")
		}

		obu := bs[:obuLen]

		if removeSizeField {
			obu = obuRemoveSize(&h, sizeN, obu)
		}

		ret = append(ret, obu)
		bs = bs[obuLen:]

		if len(bs) == 0 {
			break
		}
	}

	return ret, nil
}

// BitstreamMarshal encodes a temporal unit into a bitstream.
// Specification: https://aomediacodec.github.io/av1-spec/#low-overhead-bitstream-format
func BitstreamMarshal(tu [][]byte) ([]byte, error) {
	n := 0

	for _, obu := range tu {
		n += len(obu)

		var h OBUHeader
		err := h.Unmarshal(obu)
		if err != nil {
			return nil, err
		}

		if !h.HasSize {
			size := len(obu) - 1
			n += LEB128MarshalSize(uint(size))
		}
	}

	buf := make([]byte, n)
	n = 0

	for _, obu := range tu {
		var h OBUHeader
		h.Unmarshal(obu) //nolint:errcheck

		if !h.HasSize {
			buf[n] = obu[0] | 0b00000010
			n++
			size := len(obu) - 1
			n += LEB128MarshalTo(uint(size), buf[n:])
			n += copy(buf[n:], obu[1:])
		}
	}

	return buf, nil
}
