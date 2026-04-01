package rtpmpeg4audio

import (
	"fmt"

	"github.com/bluenviron/mediacommon/pkg/codecs/mpeg4audio"
	"github.com/pion/rtp"
)

func (d *Decoder) decodeLATM(pkt *rtp.Packet) ([][]byte, error) {
	var au []byte
	buf := pkt.Payload

	if len(d.fragments) == 0 {
		pl, n, err := payloadLengthInfoDecode(buf)
		if err != nil {
			return nil, err
		}

		buf = buf[n:]
		bl := len(buf)

		if pl <= bl {
			au = buf[:pl]
			// there could be other data, due to otherDataPresent. Ignore it.
		} else {
			if pl > mpeg4audio.MaxAccessUnitSize {
				d.fragments = d.fragments[:0] // discard pending fragments
				return nil, fmt.Errorf("access unit size (%d) is too big, maximum is %d",
					pl, mpeg4audio.MaxAccessUnitSize)
			}

			d.fragments = append(d.fragments, buf)
			d.fragmentsSize = pl
			d.fragmentsExpected = pl - bl
			return nil, ErrMorePacketsNeeded
		}
	} else {
		bl := len(buf)

		if d.fragmentsExpected > bl {
			d.fragments = append(d.fragments, buf)
			d.fragmentsExpected -= bl
			return nil, ErrMorePacketsNeeded
		}

		d.fragments = append(d.fragments, buf[:d.fragmentsExpected])
		// there could be other data, due to otherDataPresent. Ignore it.

		au = joinFragments(d.fragments, d.fragmentsSize)
		d.fragments = d.fragments[:0]
	}

	return [][]byte{au}, nil
}
