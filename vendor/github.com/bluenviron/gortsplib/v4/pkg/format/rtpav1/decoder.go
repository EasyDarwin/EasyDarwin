package rtpav1

import (
	"errors"
	"fmt"

	"github.com/bluenviron/mediacommon/pkg/codecs/av1"
	"github.com/pion/rtp"
	"github.com/pion/rtp/codecs"
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

// Decoder is a RTP/AV1 decoder.
// Specification: https://aomediacodec.github.io/av1-rtp-spec/
type Decoder struct {
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
	return nil
}

func (d *Decoder) decodeOBUs(pkt *rtp.Packet) ([][]byte, error) {
	var av1header codecs.AV1Packet
	_, err := av1header.Unmarshal(pkt.Payload)
	if err != nil {
		d.fragments = d.fragments[:0] // discard pending fragments
		d.fragmentsSize = 0
		return nil, fmt.Errorf("invalid header: %w", err)
	}

	if av1header.Z {
		if len(d.fragments) == 0 {
			if !d.firstPacketReceived {
				return nil, ErrNonStartingPacketAndNoPrevious
			}

			return nil, fmt.Errorf("received a subsequent fragment without previous fragments")
		}

		d.fragmentsSize += len(av1header.OBUElements[0])
		if d.fragmentsSize > av1.MaxTemporalUnitSize {
			d.fragments = d.fragments[:0]
			d.fragmentsSize = 0
			return nil, fmt.Errorf("OBU size (%d) is too big, maximum is %d", d.fragmentsSize, av1.MaxTemporalUnitSize)
		}

		d.fragments = append(d.fragments, av1header.OBUElements[0])
		av1header.OBUElements = av1header.OBUElements[1:]
	}

	d.firstPacketReceived = true

	var obus [][]byte

	if len(av1header.OBUElements) > 0 {
		if d.fragmentsSize != 0 {
			obus = append(obus, joinFragments(d.fragments, d.fragmentsSize))
			d.fragments = d.fragments[:0]
			d.fragmentsSize = 0
		}

		if av1header.Y {
			elementCount := len(av1header.OBUElements)

			d.fragmentsSize += len(av1header.OBUElements[elementCount-1])
			if d.fragmentsSize > av1.MaxTemporalUnitSize {
				d.fragments = d.fragments[:0]
				d.fragmentsSize = 0
				return nil, fmt.Errorf("OBU size (%d) is too big, maximum is %d", d.fragmentsSize, av1.MaxTemporalUnitSize)
			}

			d.fragments = append(d.fragments, av1header.OBUElements[elementCount-1])
			av1header.OBUElements = av1header.OBUElements[:elementCount-1]
		}

		obus = append(obus, av1header.OBUElements...)
	} else if !av1header.Y {
		obus = append(obus, joinFragments(d.fragments, d.fragmentsSize))
		d.fragments = d.fragments[:0]
		d.fragmentsSize = 0
	}

	if len(obus) == 0 {
		return nil, ErrMorePacketsNeeded
	}

	return obus, nil
}

// Decode decodes a temporal unit from a RTP packet.
func (d *Decoder) Decode(pkt *rtp.Packet) ([][]byte, error) {
	obus, err := d.decodeOBUs(pkt)
	if err != nil {
		return nil, err
	}
	l := len(obus)

	if (d.frameBufferLen + l) > av1.MaxOBUsPerTemporalUnit {
		d.frameBuffer = nil
		d.frameBufferLen = 0
		d.frameBufferSize = 0
		return nil, fmt.Errorf("OBU count exceeds maximum allowed (%d)",
			av1.MaxOBUsPerTemporalUnit)
	}

	addSize := 0

	for _, obu := range obus {
		addSize += len(obu)
	}

	if (d.frameBufferSize + addSize) > av1.MaxTemporalUnitSize {
		d.frameBuffer = nil
		d.frameBufferLen = 0
		d.frameBufferSize = 0
		return nil, fmt.Errorf("temporal unit size (%d) is too big, maximum is %d",
			d.frameBufferSize+addSize, av1.MaxOBUsPerTemporalUnit)
	}

	d.frameBuffer = append(d.frameBuffer, obus...)
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
