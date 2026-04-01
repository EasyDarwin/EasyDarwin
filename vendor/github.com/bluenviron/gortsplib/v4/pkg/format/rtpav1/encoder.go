package rtpav1

import (
	"crypto/rand"

	"github.com/bluenviron/mediacommon/pkg/codecs/av1"
	"github.com/pion/rtp"
)

const (
	rtpVersion            = 2
	defaultPayloadMaxSize = 1460 // 1500 (UDP MTU) - 20 (IP header) - 8 (UDP header) - 12 (RTP header)
)

func randUint32() (uint32, error) {
	var b [4]byte
	_, err := rand.Read(b[:])
	if err != nil {
		return 0, err
	}
	return uint32(b[0])<<24 | uint32(b[1])<<16 | uint32(b[2])<<8 | uint32(b[3]), nil
}

// Encoder is a RTP/AV1 encoder.
// Specification: https://aomediacodec.github.io/av1-rtp-spec/
type Encoder struct {
	// payload type of packets.
	PayloadType uint8

	// SSRC of packets (optional).
	// It defaults to a random value.
	SSRC *uint32

	// initial sequence number of packets (optional).
	// It defaults to a random value.
	InitialSequenceNumber *uint16

	// maximum size of packet payloads (optional).
	// It defaults to 1460.
	PayloadMaxSize int

	sequenceNumber uint16
}

// Init initializes the encoder.
func (e *Encoder) Init() error {
	if e.SSRC == nil {
		v, err := randUint32()
		if err != nil {
			return err
		}
		e.SSRC = &v
	}
	if e.InitialSequenceNumber == nil {
		v, err := randUint32()
		if err != nil {
			return err
		}
		v2 := uint16(v)
		e.InitialSequenceNumber = &v2
	}
	if e.PayloadMaxSize == 0 {
		e.PayloadMaxSize = defaultPayloadMaxSize
	}

	e.sequenceNumber = *e.InitialSequenceNumber
	return nil
}

// Encode encodes OBUs into RTP packets.
func (e *Encoder) Encode(obus [][]byte) ([]*rtp.Packet, error) {
	isKeyFrame, err := av1.ContainsKeyFrame(obus)
	if err != nil {
		return nil, err
	}

	var curPacket *rtp.Packet
	var packets []*rtp.Packet
	curPayloadLen := 0

	createNewPacket := func(z bool) {
		curPacket = &rtp.Packet{
			Header: rtp.Header{
				Version:        rtpVersion,
				PayloadType:    e.PayloadType,
				SequenceNumber: e.sequenceNumber,
				SSRC:           *e.SSRC,
			},
			Payload: []byte{0},
		}
		e.sequenceNumber++
		packets = append(packets, curPacket)
		curPayloadLen = 1

		if z {
			curPacket.Payload[0] |= 1 << 7
		}
	}

	finalizeCurPacket := func(y bool) {
		if y {
			curPacket.Payload[0] |= 1 << 6
		}
	}

	createNewPacket(false)

	for _, obu := range obus {
		for {
			avail := e.PayloadMaxSize - curPayloadLen
			obuLen := len(obu)
			needed := obuLen + 2

			if needed <= avail {
				buf := make([]byte, av1.LEB128MarshalSize(uint(obuLen)))
				av1.LEB128MarshalTo(uint(obuLen), buf)
				curPacket.Payload = append(curPacket.Payload, buf...)
				curPacket.Payload = append(curPacket.Payload, obu...)
				curPayloadLen += len(buf) + obuLen
				break
			}

			if avail > 2 {
				fragmentLen := avail - 2
				buf := make([]byte, av1.LEB128MarshalSize(uint(fragmentLen)))
				av1.LEB128MarshalTo(uint(fragmentLen), buf)
				curPacket.Payload = append(curPacket.Payload, buf...)
				curPacket.Payload = append(curPacket.Payload, obu[:fragmentLen]...)
				obu = obu[fragmentLen:]
			}

			finalizeCurPacket(true)
			createNewPacket(true)
		}
	}

	finalizeCurPacket(false)

	if isKeyFrame {
		packets[0].Payload[0] |= 1 << 3
	}

	packets[len(packets)-1].Marker = true

	return packets, nil
}
