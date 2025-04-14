package rtc

import (
	"errors"
	"fmt"
	"time"

	"github.com/bluenviron/gortsplib/v4/pkg/format"
	"github.com/bluenviron/gortsplib/v4/pkg/format/rtph264"
	"github.com/bluenviron/gortsplib/v4/pkg/format/rtph265"
	"github.com/bluenviron/gortsplib/v4/pkg/format/rtplpcm"
	"github.com/bluenviron/gortsplib/v4/pkg/format/rtpsimpleaudio"
	"github.com/bluenviron/gortsplib/v4/pkg/rtpreorderer"
	"github.com/bluenviron/gortsplib/v4/pkg/rtptime"
	"github.com/pion/rtp"
	"github.com/pion/webrtc/v3"
	"github.com/q191201771/lal/pkg/avc"
	"github.com/q191201771/lal/pkg/base"
	"github.com/q191201771/naza/pkg/nazalog"
)

var ErrNeedMoreFrames = errors.New("need more frames")

type UnPacker struct {
	reorderer   *rtpreorderer.Reorderer
	payloadType base.AvPacketPt
	clockRate   uint32
	pktChan     chan<- base.AvPacket
	timeDecoder *rtptime.GlobalDecoder
	format      format.Format
	dec         IRtpDecoder
}

func NewUnPacker(mimeType string, clockRate uint32, pktChan chan<- base.AvPacket) *UnPacker {
	un := &UnPacker{
		reorderer:   rtpreorderer.New(),
		clockRate:   clockRate,
		pktChan:     pktChan,
		timeDecoder: rtptime.NewGlobalDecoder(),
	}

	switch mimeType {
	case webrtc.MimeTypeH264:
		un.payloadType = base.AvPacketPtAvc
		un.format = &format.H264{}
		un.dec = NewH264RtpDecoder(un.format)
	case webrtc.MimeTypePCMA:
		un.payloadType = base.AvPacketPtG711A
		un.format = &format.G711{}
	case webrtc.MimeTypePCMU:
		un.payloadType = base.AvPacketPtG711U
		un.format = &format.G711{}
	case webrtc.MimeTypeOpus:
		un.payloadType = base.AvPacketPtOpus
		un.format = &format.Opus{}
		un.dec = NewOpusRtpDecoder(un.format)
	case webrtc.MimeTypeH265:
		un.payloadType = base.AvPacketPtHevc
		un.format = &format.H265{}
		un.dec = NewH265RtpDecoder(un.format)
	default:
		nazalog.Error("unsupport mineType:", mimeType)
	}

	nazalog.Info("create rtp unpacker, mimeType:", mimeType)

	return un
}

func (un *UnPacker) UnPack(pkt *rtp.Packet) (err error) {
	packets, lost := un.reorderer.Process(pkt)
	if lost != 0 {
		nazalog.Error("rtp lost")
		return
	}

	for _, rtppkt := range packets {
		pts, ok := un.timeDecoder.Decode(un.format, rtppkt)
		if !ok {
			continue
		}

		frame, err := un.dec.Decode(rtppkt)
		if err != nil {
			if err != ErrNeedMoreFrames {
				nazalog.Error("rtp dec Decode failed:", err)
				return err
			}

			continue
		}

		var pkt base.AvPacket
		pkt.PayloadType = un.payloadType
		pkt.Timestamp = int64(pts / time.Millisecond)
		pkt.Pts = pkt.Timestamp
		pkt.Payload = append(pkt.Payload, frame...)

		un.pktChan <- pkt
	}

	return
}

type IRtpDecoder interface {
	Decode(pkt *rtp.Packet) ([]byte, error)
}

type H264RtpDecoder struct {
	IRtpDecoder
	dec *rtph264.Decoder
}

func NewH264RtpDecoder(f format.Format) *H264RtpDecoder {
	dec, _ := f.(*format.H264).CreateDecoder()
	return &H264RtpDecoder{
		dec: dec,
	}
}

func (r *H264RtpDecoder) Decode(pkt *rtp.Packet) ([]byte, error) {
	nalus, err := r.dec.Decode(pkt)
	if err != nil {
		return nil, ErrNeedMoreFrames
	}

	if len(nalus) == 0 {
		err = fmt.Errorf("invalid frame")
		return nil, err
	}

	var frame []byte
	for _, nalu := range nalus {
		frame = append(frame, avc.NaluStartCode4...)
		frame = append(frame, nalu...)
	}

	return frame, nil
}

type G711RtpDecoder struct {
	IRtpDecoder
	dec *rtplpcm.Decoder
}

func NewG711RtpDecoder(f format.Format) *G711RtpDecoder {
	dec, _ := f.(*format.G711).CreateDecoder()
	return &G711RtpDecoder{
		dec: dec,
	}
}

func (r *G711RtpDecoder) Decode(pkt *rtp.Packet) ([]byte, error) {
	frame, err := r.dec.Decode(pkt)
	if err != nil {
		nazalog.Error(err)
		return nil, err
	}

	return frame, nil
}

type OpusRtpDecoder struct {
	IRtpDecoder
	dec *rtpsimpleaudio.Decoder
}

func NewOpusRtpDecoder(f format.Format) *OpusRtpDecoder {
	dec, _ := f.(*format.Opus).CreateDecoder()
	return &OpusRtpDecoder{
		dec: dec,
	}
}

func (r *OpusRtpDecoder) Decode(pkt *rtp.Packet) ([]byte, error) {
	frame, err := r.dec.Decode(pkt)
	if err != nil {
		nazalog.Error(err)
		return nil, err
	}

	return frame, nil
}

type H265RtpDecoder struct {
	IRtpDecoder
	dec *rtph265.Decoder
}

func NewH265RtpDecoder(f format.Format) *H265RtpDecoder {
	dec, _ := f.(*format.H265).CreateDecoder()
	return &H265RtpDecoder{
		dec: dec,
	}
}

func (r *H265RtpDecoder) Decode(pkt *rtp.Packet) ([]byte, error) {
	nalus, err := r.dec.Decode(pkt)
	if err != nil {
		return nil, ErrNeedMoreFrames
	}

	if len(nalus) == 0 {
		err = fmt.Errorf("invalid frame")
		return nil, err
	}

	var frame []byte
	for _, nalu := range nalus {
		frame = append(frame, avc.NaluStartCode4...)
		frame = append(frame, nalu...)
	}

	return frame, nil
}
