package rtc

import (
	"github.com/gofrs/uuid"
	"github.com/pion/webrtc/v3"
	"github.com/q191201771/lal/pkg/base"
	"github.com/q191201771/lal/pkg/logic"
	"github.com/q191201771/naza/pkg/nazalog"
)

type whipSession struct {
	streamid      string
	pc            *peerConnection
	lalServer     logic.ILalServer
	lalSession    logic.ICustomizePubSessionContext
	videoUnpacker *UnPacker
	audioUnpacker *UnPacker
	pktChan       chan base.AvPacket
	closeChan     chan bool
	subscriberId  string
}

func NewWhipSession(streamid string, pc *peerConnection, lalServer logic.ILalServer) *whipSession {
	session, err := lalServer.AddCustomizePubSession(streamid)
	if err != nil {
		nazalog.Error(err)
		return nil
	}

	session.WithOption(func(option *base.AvPacketStreamOption) {
		option.VideoFormat = base.AvPacketStreamVideoFormatAnnexb
	})

	u, _ := uuid.NewV4()

	return &whipSession{
		streamid:     streamid,
		pc:           pc,
		lalServer:    lalServer,
		lalSession:   session,
		pktChan:      make(chan base.AvPacket, 100),
		closeChan:    make(chan bool, 2),
		subscriberId: u.String(),
	}
}

func (conn *whipSession) GetAnswerSDP(offer string) (sdp string) {
	gatherComplete := webrtc.GatheringCompletePromise(conn.pc.PeerConnection)

	conn.pc.SetRemoteDescription(webrtc.SessionDescription{
		Type: webrtc.SDPTypeOffer,
		SDP:  string(offer),
	})

	answer, err := conn.pc.CreateAnswer(nil)
	if err != nil {
		nazalog.Error(err)
		return
	}

	err = conn.pc.SetLocalDescription(answer)
	if err != nil {
		nazalog.Error(err)
		return
	}

	<-gatherComplete

	sdp = conn.pc.LocalDescription().SDP
	return
}

func (conn *whipSession) Run() {

	conn.pc.OnConnectionStateChange(func(state webrtc.PeerConnectionState) {
		nazalog.Info("peer connection state: ", state.String())

		switch state {
		case webrtc.PeerConnectionStateConnected:
		case webrtc.PeerConnectionStateDisconnected:
			fallthrough
		case webrtc.PeerConnectionStateFailed:
			fallthrough
		case webrtc.PeerConnectionStateClosed:
			conn.closeChan <- true
		}
	})

	var videoPt webrtc.PayloadType
	conn.pc.OnTrack(func(tr *webrtc.TrackRemote, receiver *webrtc.RTPReceiver) {
		switch tr.Kind() {
		case webrtc.RTPCodecTypeVideo:
			conn.videoUnpacker = NewUnPacker(tr.Codec().MimeType, tr.Codec().ClockRate, conn.pktChan)
			videoPt = tr.PayloadType()
		case webrtc.RTPCodecTypeAudio:
			mimeType := tr.Codec().MimeType
			if tr.Codec().MimeType == "" {
				// pt为0或者8按照G711U和G711A处理,提高兼容性
				if tr.PayloadType() == 0 {
					mimeType = webrtc.MimeTypePCMU
				} else if tr.PayloadType() == 8 {
					mimeType = webrtc.MimeTypePCMA
				}
			}
			conn.audioUnpacker = NewUnPacker(mimeType, tr.Codec().ClockRate, conn.pktChan)
		}

		for {
			pkt, _, err := tr.ReadRTP()
			if err != nil {
				nazalog.Error(err)
				return
			}

			if conn.videoUnpacker != nil && pkt.Header.PayloadType == uint8(videoPt) {
				conn.videoUnpacker.UnPack(pkt)
			} else if conn.audioUnpacker != nil {
				conn.audioUnpacker.UnPack(pkt)
			}
		}
	})

	for {
		select {
		case <-conn.closeChan:
			nazalog.Info("whip connect close, streamid:", conn.streamid)
			conn.lalServer.DelCustomizePubSession(conn.lalSession)
			return
		case pkt := <-conn.pktChan:
			conn.lalSession.FeedAvPacket(pkt)
		}
	}
}
