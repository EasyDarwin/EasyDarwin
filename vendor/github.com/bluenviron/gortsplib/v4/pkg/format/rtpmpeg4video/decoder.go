package rtpmpeg4video

import (
	"errors"
	"fmt"

	"github.com/pion/rtp"

	"github.com/bluenviron/mediacommon/pkg/codecs/mpeg4video"
)

// ErrMorePacketsNeeded is returned when more packets are needed.
var ErrMorePacketsNeeded = errors.New("need more packets")

func joinFragments(fragments [][]byte, size int) []byte {
	ret := make([]byte, size)
	n := 0
	for _, p := range fragments {
		n += copy(ret[n:], p)
	}
	return ret
}

// Decoder is a RTP/MPEG-4 Video decoder.
// Specification: https://datatracker.ietf.org/doc/html/rfc6416
type Decoder struct {
	fragments     [][]byte
	fragmentsSize int
}

// Init initializes the decoder.
func (d *Decoder) Init() error {
	return nil
}

// Decode decodes a frame from a RTP packet.
func (d *Decoder) Decode(pkt *rtp.Packet) ([]byte, error) {
	var frame []byte

	if len(d.fragments) == 0 {
		if pkt.Marker {
			frame = pkt.Payload
		} else {
			d.fragmentsSize = len(pkt.Payload)
			d.fragments = append(d.fragments, pkt.Payload)
			return nil, ErrMorePacketsNeeded
		}
	} else {
		d.fragmentsSize += len(pkt.Payload)
		if d.fragmentsSize > mpeg4video.MaxFrameSize {
			d.fragments = d.fragments[:0] // discard pending fragments
			return nil, fmt.Errorf("frame size (%d) is too big, maximum is %d", d.fragmentsSize, mpeg4video.MaxFrameSize)
		}

		d.fragments = append(d.fragments, pkt.Payload)

		if !pkt.Marker {
			return nil, ErrMorePacketsNeeded
		}

		frame = joinFragments(d.fragments, d.fragmentsSize)
		d.fragments = d.fragments[:0]
	}

	return frame, nil
}
