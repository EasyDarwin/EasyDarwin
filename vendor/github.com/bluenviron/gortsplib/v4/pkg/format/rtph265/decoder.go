package rtph265

import (
	"errors"
	"fmt"

	"github.com/pion/rtp"

	"github.com/bluenviron/mediacommon/pkg/codecs/h265"
)

// ErrMorePacketsNeeded is returned when more packets are needed.
var ErrMorePacketsNeeded = errors.New("need more packets")

// ErrNonStartingPacketAndNoPrevious is returned when we received a non-starting
// packet of a fragmented NALU and we didn't received anything before.
// It's normal to receive this when decoding a stream that has been already
// running for some time.
var ErrNonStartingPacketAndNoPrevious = errors.New(
	"received a non-starting fragment without any previous starting fragment")

func joinFragments(fragments [][]byte, size int) []byte {
	ret := make([]byte, size)
	n := 0
	for _, p := range fragments {
		n += copy(ret[n:], p)
	}
	return ret
}

// Decoder is a RTP/H265 decoder.
// Specification: https://datatracker.ietf.org/doc/html/rfc7798
type Decoder struct {
	// indicates that NALUs have an additional field that specifies the decoding order.
	MaxDONDiff int

	firstPacketReceived bool
	fragmentsSize       int
	fragments           [][]byte

	// for Decode()
	frameBuffer     [][]byte
	frameBufferLen  int
	frameBufferSize int
}

// Init initializes the decoder.
func (d *Decoder) Init() error {
	if d.MaxDONDiff != 0 {
		return fmt.Errorf("MaxDONDiff != 0 is not supported (yet)")
	}
	return nil
}

func (d *Decoder) decodeNALUs(pkt *rtp.Packet) ([][]byte, error) {
	if len(pkt.Payload) < 2 {
		d.fragments = d.fragments[:0] // discard pending fragments
		return nil, fmt.Errorf("payload is too short")
	}

	typ := h265.NALUType((pkt.Payload[0] >> 1) & 0b111111)
	var nalus [][]byte

	switch typ {
	case h265.NALUType_AggregationUnit:
		d.fragments = d.fragments[:0] // discard pending fragments

		payload := pkt.Payload[2:]

		for {
			if len(payload) < 2 {
				return nil, fmt.Errorf("invalid aggregation unit (invalid size)")
			}

			size := uint16(payload[0])<<8 | uint16(payload[1])
			payload = payload[2:]

			if int(size) > len(payload) {
				return nil, fmt.Errorf("invalid aggregation unit (invalid size)")
			}

			nalus = append(nalus, payload[:size])
			payload = payload[size:]

			if len(payload) == 0 {
				break
			}
		}

		if nalus == nil {
			return nil, fmt.Errorf("aggregation unit doesn't contain any NALU")
		}

		d.firstPacketReceived = true

	case h265.NALUType_FragmentationUnit:
		if len(pkt.Payload) < 3 {
			d.fragments = d.fragments[:0] // discard pending fragments
			return nil, fmt.Errorf("payload is too short")
		}

		start := pkt.Payload[2] >> 7
		end := (pkt.Payload[2] >> 6) & 0x01

		if start == 1 {
			d.fragments = d.fragments[:0] // discard pending fragments

			if end != 0 {
				return nil, fmt.Errorf("invalid fragmentation unit (can't contain both a start and end bit)")
			}

			typ := pkt.Payload[2] & 0b111111
			head := uint16(pkt.Payload[0]&0b10000001)<<8 | uint16(typ)<<9 | uint16(pkt.Payload[1])
			d.fragmentsSize = len(pkt.Payload[1:])
			d.fragments = append(d.fragments, []byte{byte(head >> 8), byte(head)}, pkt.Payload[3:])
			d.firstPacketReceived = true

			return nil, ErrMorePacketsNeeded
		}

		if len(d.fragments) == 0 {
			if !d.firstPacketReceived {
				return nil, ErrNonStartingPacketAndNoPrevious
			}

			return nil, fmt.Errorf("invalid fragmentation unit (non-starting)")
		}

		d.fragmentsSize += len(pkt.Payload[3:])
		if d.fragmentsSize > h265.MaxAccessUnitSize {
			d.fragments = d.fragments[:0]
			return nil, fmt.Errorf("NALU size (%d) is too big, maximum is %d", d.fragmentsSize, h265.MaxAccessUnitSize)
		}

		d.fragments = append(d.fragments, pkt.Payload[3:])

		if end != 1 {
			return nil, ErrMorePacketsNeeded
		}

		nalus = [][]byte{joinFragments(d.fragments, d.fragmentsSize)}
		d.fragments = d.fragments[:0]

	case h265.NALUType_PACI:
		d.fragments = d.fragments[:0] // discard pending fragments
		d.firstPacketReceived = true
		return nil, fmt.Errorf("PACI packets are not supported (yet)")

	default:
		d.fragments = d.fragments[:0] // discard pending fragments
		d.firstPacketReceived = true
		nalus = [][]byte{pkt.Payload}
	}

	return nalus, nil
}

// Decode decodes an access unit from a RTP packet.
func (d *Decoder) Decode(pkt *rtp.Packet) ([][]byte, error) {
	nalus, err := d.decodeNALUs(pkt)
	if err != nil {
		return nil, err
	}
	l := len(nalus)

	if (d.frameBufferLen + l) > h265.MaxNALUsPerAccessUnit {
		d.frameBuffer = nil
		d.frameBufferLen = 0
		d.frameBufferSize = 0
		return nil, fmt.Errorf("NALU count exceeds maximum allowed (%d)",
			h265.MaxNALUsPerAccessUnit)
	}

	addSize := 0

	for _, nalu := range nalus {
		addSize += len(nalu)
	}

	if (d.frameBufferSize + addSize) > h265.MaxAccessUnitSize {
		d.frameBuffer = nil
		d.frameBufferLen = 0
		d.frameBufferSize = 0
		return nil, fmt.Errorf("access unit size (%d) is too big, maximum is %d",
			d.frameBufferSize+addSize, h265.MaxAccessUnitSize)
	}

	d.frameBuffer = append(d.frameBuffer, nalus...)
	d.frameBufferLen += l
	d.frameBufferSize += addSize

	if !pkt.Marker {
		return nil, ErrMorePacketsNeeded
	}

	ret := d.frameBuffer

	// do not reuse frameBuffer to avoid race conditions
	d.frameBuffer = nil
	d.frameBufferLen = 0
	d.frameBufferSize = 0

	return ret, nil
}
