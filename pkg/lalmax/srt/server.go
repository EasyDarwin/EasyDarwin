package srt

import (
	"context"
	"strings"
	"time"

	srt "github.com/datarhei/gosrt"
	"github.com/q191201771/lal/pkg/base"
	"github.com/q191201771/lal/pkg/logic"
	"github.com/q191201771/naza/pkg/nazalog"
)

type SrtServer struct {
	addr      string
	lalServer logic.ILalServer
	srtOpt    SrtOption
}
type SrtOption struct {
	Latency           int
	RecvLatency       int
	PeerLatency       int
	TlpktDrop         bool
	TsbpdMode         bool
	RecvBuf           int
	SendBuf           int
	MaxSendPacketSize int
}

var defaultSrtOption = SrtOption{
	Latency:           300,
	RecvLatency:       300,
	PeerLatency:       300,
	TlpktDrop:         true,
	TsbpdMode:         true,
	RecvBuf:           2 * 1024 * 1024,
	SendBuf:           2 * 1024 * 1024,
	MaxSendPacketSize: 4,
}

type ModSrtOption func(option *SrtOption)

func NewSrtServer(addr string, lal logic.ILalServer, modOptions ...ModSrtOption) *SrtServer {
	opt := defaultSrtOption
	for _, fn := range modOptions {
		fn(&opt)
	}
	svr := &SrtServer{
		addr:      addr,
		lalServer: lal,
		srtOpt:    opt,
	}

	nazalog.Info("create srt server")
	return svr
}

func (s *SrtServer) Run(ctx context.Context) {
	conf := srt.DefaultConfig()
	conf.Latency = time.Millisecond * time.Duration(s.srtOpt.Latency)
	conf.ReceiverLatency = time.Millisecond * time.Duration(s.srtOpt.RecvLatency)
	conf.PeerLatency = time.Millisecond * time.Duration(s.srtOpt.PeerLatency)
	conf.TooLatePacketDrop = s.srtOpt.TlpktDrop
	conf.TSBPDMode = s.srtOpt.TsbpdMode
	conf.SendBufferSize = uint32(s.srtOpt.SendBuf)
	conf.ReceiverBufferSize = uint32(s.srtOpt.RecvBuf)

	srtlistener, err := srt.Listen("srt", s.addr, conf)
	if err != nil {
		panic(err)
	}

	defer srtlistener.Close()

	nazalog.Info("srt server listen addr:", s.addr)

	for {
		select {
		case <-ctx.Done():
			return
		default:

		}

		var info StreamInfo
		conn, mode, err := srtlistener.Accept(func(req srt.ConnRequest) srt.ConnType {
			info = getStreamInfo(req.StreamId())
			return info.Mode
		})

		if err != nil {
			// rejected connection, ignore
			continue
		}

		if mode == srt.REJECT {
			// rejected connection, ignore
			continue
		}

		if info.Mode == srt.PUBLISH {
			go s.handlePublish(ctx, conn, info.StreamName)
		} else {
			go s.handleSubcribe(ctx, conn, info.StreamName)
		}
	}
}

func (s *SrtServer) handlePublish(ctx context.Context, conn srt.Conn, streamid string) {
	publisher := NewPublisher(ctx, conn, streamid, s)
	session, err := s.lalServer.AddCustomizePubSession(streamid)
	if err != nil {
		nazalog.Error(err)
	}

	if session != nil {
		session.WithOption(func(option *base.AvPacketStreamOption) {
			option.VideoFormat = base.AvPacketStreamVideoFormatAnnexb
		})
	}

	publisher.SetSession(session)
	publisher.Run()
}

func (s *SrtServer) handleSubcribe(ctx context.Context, conn srt.Conn, streamid string) {
	subscriber := NewSubscriber(ctx, conn, streamid, s.srtOpt.MaxSendPacketSize)
	subscriber.Run()
}

func (s *SrtServer) Remove(host string, ss logic.ICustomizePubSessionContext) {
	s.lalServer.DelCustomizePubSession(ss)
}

type StreamInfo struct {
	StreamName string
	Mode    srt.ConnType
}

func getStreamInfo(streamid string) StreamInfo {
	info := StreamInfo{
		Mode: srt.REJECT,
	}

	s := strings.TrimLeft(streamid, "#!::")
	values := strings.Split(s, ",")
	for _, v := range values {
		ss := strings.Split(v, "=")
		name := ss[0]
		switch name {
		case "h":
			info.StreamName = ss[1]
		case "m":
			switch ss[1] {
			case "publish":
				info.Mode = srt.PUBLISH
			case "request":
				info.Mode = srt.SUBSCRIBE
			}
		}
	}

	return info
}
