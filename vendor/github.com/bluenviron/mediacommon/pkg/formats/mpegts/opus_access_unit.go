package mpegts

import (
	"fmt"
)

type opusAccessUnit struct {
	ControlHeader opusControlHeader
	Packet        []byte
}

func (au *opusAccessUnit) unmarshal(buf []byte) (int, error) {
	n, err := au.ControlHeader.unmarshal(buf)
	if err != nil {
		return 0, fmt.Errorf("invalid control header: %w", err)
	}
	buf = buf[n:]

	if len(buf) < au.ControlHeader.PayloadSize {
		return 0, fmt.Errorf("buffer is too small")
	}

	au.Packet = buf[:au.ControlHeader.PayloadSize]

	return n + au.ControlHeader.PayloadSize, nil
}

func (au *opusAccessUnit) marshalSize() int {
	return au.ControlHeader.marshalSize() + au.ControlHeader.PayloadSize
}

func (au *opusAccessUnit) marshalTo(buf []byte) (int, error) {
	if au.ControlHeader.PayloadSize != len(au.Packet) {
		return 0, fmt.Errorf("invalid packet")
	}

	n, err := au.ControlHeader.marshalTo(buf)
	if err != nil {
		return 0, err
	}

	n += copy(buf[n:], au.Packet)

	return n, nil
}
