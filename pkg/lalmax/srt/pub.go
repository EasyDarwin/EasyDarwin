package srt

import (
	"bufio"
	"context"
	srt "github.com/datarhei/gosrt"
	"github.com/q191201771/lal/pkg/aac"
	"github.com/q191201771/lal/pkg/base"
	"github.com/q191201771/lal/pkg/logic"
	"github.com/q191201771/naza/pkg/nazalog"
	codec "github.com/yapingcat/gomedia/go-codec"
	ts "github.com/yapingcat/gomedia/go-mpeg2"
)

type Publisher struct {
	ctx         context.Context
	srv         *SrtServer
	ss          logic.ICustomizePubSessionContext
	streamName  string
	demuxer     *ts.TSDemuxer
	conn        srt.Conn
	subscribers []*Subscriber
}

func NewPublisher(ctx context.Context, conn srt.Conn, streamName string, srv *SrtServer) *Publisher {
	pub := &Publisher{
		ctx:        ctx,
		srv:        srv,
		streamName: streamName,
		conn:       conn,
		demuxer:    ts.NewTSDemuxer(),
	}

	nazalog.Infof("create srt publisher, streamName:%s", streamName)
	return pub
}

func (p *Publisher) SetSession(session logic.ICustomizePubSessionContext) {
	p.ss = session
}

func (p *Publisher) Run() {
	defer func() {
		p.conn.Close()
		p.srv.Remove(p.streamName, p.ss)
	}()
	audioSampleRate := uint32(0)
	var foundAudio bool
	p.demuxer.OnFrame = func(cid ts.TS_STREAM_TYPE, frame []byte, pts uint64, dts uint64) {
		var pkt base.AvPacket
		if cid == ts.TS_STREAM_AAC {
			if !foundAudio {
				if asc, err := codec.ConvertADTSToASC(frame); err != nil {
					return
				} else {
					p.ss.FeedAudioSpecificConfig(asc.Encode())
					audioSampleRate = uint32(codec.AACSampleIdxToSample(int(asc.Sample_freq_index)))
				}

				foundAudio = true
			}

			var preAudioDts uint64
			ctx := aac.AdtsHeaderContext{}
			for len(frame) > aac.AdtsHeaderLength {
				ctx.Unpack(frame[:])
				if preAudioDts == 0 {
					preAudioDts = dts
				} else {
					preAudioDts += uint64(1024 * 1000 / audioSampleRate)
				}

				aacPacket := base.AvPacket{
					Timestamp:   int64(preAudioDts),
					PayloadType: base.AvPacketPtAac,
					Pts:         int64(preAudioDts),
				}
				if len(frame) >= int(ctx.AdtsLength) {
					Payload := frame[aac.AdtsHeaderLength:ctx.AdtsLength]
					if len(frame) > int(ctx.AdtsLength) {
						frame = frame[ctx.AdtsLength:]
					} else {
						frame = frame[0:0]
					}
					aacPacket.Payload = Payload
					p.ss.FeedAvPacket(aacPacket)
				}

			}
		} else if cid == ts.TS_STREAM_H264 {
			pkt.Payload = frame
			pkt.PayloadType = base.AvPacketPtAvc
			pkt.Pts = int64(pts)
			pkt.Timestamp = int64(dts)
			p.ss.FeedAvPacket(pkt)
		} else if cid == ts.TS_STREAM_H265 {
			pkt.Payload = frame
			pkt.PayloadType = base.AvPacketPtHevc
			pkt.Pts = int64(pts)
			pkt.Timestamp = int64(dts)
			p.ss.FeedAvPacket(pkt)
		}
	}
	err := p.demuxer.Input(bufio.NewReader(p.conn))
	if err != nil {
		nazalog.Infof("stream [%s] disconnected", p.streamName)
	}
	return
}
