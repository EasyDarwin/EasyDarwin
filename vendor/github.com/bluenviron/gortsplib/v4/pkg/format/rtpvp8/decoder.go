package rtpvp8

import (
	"errors"
	"fmt"

	"github.com/bluenviron/mediacommon/pkg/codecs/vp8"
	"github.com/pion/rtp"
	"github.com/pion/rtp/codecs"
)

// ErrMorePacketsNeeded is returned when more packets are needed.
var ErrMorePacketsNeeded = errors.New("need more packets")

// ErrNonStartingPacketAndNoPrevious is returned when we received a non-starting
// packet of a fragmented frame and we didn't received anything before.
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

// Decoder is a RTP/VP8 decoder.
// Specification: https://datatracker.ietf.org/doc/html/rfc7741
type Decoder struct {
	firstPacketReceived bool
	fragmentsSize       int
	fragments           [][]byte
}

// Init initializes the decoder.
func (d *Decoder) Init() error {
	return nil
}

// Decode decodes a VP8 frame from a RTP packet.
func (d *Decoder) Decode(pkt *rtp.Packet) ([]byte, error) {
	var vpkt codecs.VP8Packet
	_, err := vpkt.Unmarshal(pkt.Payload)
	if err != nil {
		d.fragments = d.fragments[:0] // discard pending fragments
		return nil, err
	}

	if vpkt.PID != 0 {
		d.fragments = d.fragments[:0] // discard pending fragments
		return nil, fmt.Errorf("packets containing single partitions are not supported")
	}

	var frame []byte

	if vpkt.S == 1 {
		d.fragments = d.fragments[:0] // discard pending fragments
		d.firstPacketReceived = true

		if !pkt.Marker {
			d.fragmentsSize = len(vpkt.Payload)
			d.fragments = append(d.fragments, vpkt.Payload)
			return nil, ErrMorePacketsNeeded
		}

		frame = vpkt.Payload
	} else {
		if len(d.fragments) == 0 {
			if !d.firstPacketReceived {
				return nil, ErrNonStartingPacketAndNoPrevious
			}

			return nil, fmt.Errorf("received a non-starting fragment")
		}

		d.fragmentsSize += len(vpkt.Payload)

		if d.fragmentsSize > vp8.MaxFrameSize {
			d.fragments = d.fragments[:0] // discard pending fragments
			return nil, fmt.Errorf("frame size (%d) is too big, maximum is %d", d.fragmentsSize, vp8.MaxFrameSize)
		}

		d.fragments = append(d.fragments, vpkt.Payload)

		if !pkt.Marker {
			return nil, ErrMorePacketsNeeded
		}

		frame = joinFragments(d.fragments, d.fragmentsSize)
		d.fragments = d.fragments[:0]
	}

	return frame, nil
}
