package svr

import (
	"fmt"
	"github.com/q191201771/lal/pkg/aac"
	"github.com/q191201771/lal/pkg/base"
	"github.com/q191201771/lal/pkg/logic"
	"github.com/q191201771/lal/pkg/rtprtcp"
	"github.com/q191201771/lal/pkg/rtsp"
	"github.com/q191201771/lal/pkg/sdp"
)

type StreamClient struct {
	ChannelID   int
	URL         string
	Session     logic.ICustomizePubSessionContext
	pullSession *rtsp.PullSession
	ascContext  *aac.AscContext
	videoIFrame []byte
}

func NewStreamClient(id int, rtspUrl string) *StreamClient {
	return &StreamClient{
		ChannelID: id,
		URL:       rtspUrl,
	}
}

const StreamName string = "stream_"

func (client *StreamClient) Open() {
	var err error
	customizePubStreamName := fmt.Sprintf("%s%d", StreamName, client.ChannelID)
	client.Session, err = Lals.GetILalServer().AddCustomizePubSession(customizePubStreamName)
	if err != nil {
		fmt.Printf("add customize pub session err %s", err.Error())
		return
	}
	if true {
		client.Session.WithOption(func(option *base.AvPacketStreamOption) {
			option.VideoFormat = base.AvPacketStreamVideoFormatAnnexb
			option.AudioFormat = base.AvPacketStreamAudioFormatRawAac
		})
	} else {
		client.Session.WithOption(func(option *base.AvPacketStreamOption) {
			option.VideoFormat = base.AvPacketStreamVideoFormatAnnexb
		})
	}
	client.pullSession = rtsp.NewPullSession(client, func(option *rtsp.PullSessionOption) {
		option.PullTimeoutMs = 10000
		option.OverTcp = true
	})
	err = client.pullSession.Pull(client.URL)
	if err != nil {
		return
	}
}

func (client *StreamClient) Stop() (err error) {
	client.pullSession.Dispose()
	client.pullSession = nil
	Lals.GetILalServer().DelCustomizePubSession(client.Session)
	client.Session = nil
	return
}

func (client *StreamClient) OnSdp(sdpCtx sdp.LogicContext) {
	if len(sdpCtx.Asc) > 0 {
		client.ascContext, _ = aac.NewAscContext(sdpCtx.Asc)
	}
}
func (client *StreamClient) OnRtpPacket(pkt rtprtcp.RtpPacket) {

}
func (client *StreamClient) OnAvPacket(pkt base.AvPacket) {
	fmt.Printf("--------------:%d\n", pkt.PayloadType)
	if client.Session == nil {
		return
	}
	if pkt.IsVideo() {
		if !(pkt.PayloadType == base.AvPacketPtAvc || pkt.PayloadType == base.AvPacketPtHevc) {
			fmt.Printf("pkt.PayloadType:%d\n", pkt.PayloadType)
			return
		}
		pkt.Payload[0] = 0
		pkt.Payload[1] = 0
		pkt.Payload[2] = 0
		pkt.Payload[3] = 1
		flag := false
		if pkt.PayloadType == base.AvPacketPtAvc {
			v := pkt.Payload[4] & 0x1f
			if v == 7 {
				flag = true
				client.videoIFrame = make([]byte, 0)
				client.videoIFrame = append(client.videoIFrame, pkt.Payload...)
			}
			if v == 8 {
				flag = true
				if len(client.videoIFrame) > 0 {
					client.videoIFrame = append(client.videoIFrame, pkt.Payload...)
				}
			}
			if v == 5 {
				flag = true
				if len(client.videoIFrame) > 0 {
					client.videoIFrame = append(client.videoIFrame, pkt.Payload...)
				}
			}
			if v == 6 {
				flag = true
			}
			if v == 1 {
				flag = true
			}
			if flag {
				err := client.Session.FeedAvPacket(pkt)
				if err != nil {
					fmt.Printf("stream client video av packet err :%v", err)
					return
				}
			}
		} else {
			v := (pkt.Payload[4] & 0x7E) >> 1
			if v == 32 {
				flag = true
				client.videoIFrame = make([]byte, 0)
				client.videoIFrame = append(client.videoIFrame, pkt.Payload...)
			}
			if v == 33 {
				flag = true
				if len(client.videoIFrame) > 0 {
					client.videoIFrame = append(client.videoIFrame, pkt.Payload...)
				}
			}
			if v == 34 {
				flag = true
				if len(client.videoIFrame) > 0 {
					client.videoIFrame = append(client.videoIFrame, pkt.Payload...)
				}
			}
			if v == 19 {
				flag = true
				if len(client.videoIFrame) > 0 {
					client.videoIFrame = append(client.videoIFrame, pkt.Payload...)
				}
			}
			if v == 39 {
				flag = true
			}
			if v == 1 {
				flag = true
			}
			if flag {
				err := client.Session.FeedAvPacket(pkt)
				if err != nil {
					fmt.Printf("stream client video av packet err :%v", err)
					return
				}
			}
		}
	} else if pkt.IsAudio() {
		if pkt.PayloadType != base.AvPacketPtAac {
			return
		}
		if client.ascContext != nil {
			out := client.ascContext.PackAdtsHeader(len(pkt.Payload))
			asc, err := aac.MakeAscWithAdtsHeader(out)
			err = client.Session.FeedAudioSpecificConfig(asc)
			if err != nil {
				fmt.Printf("stream client audio specific config err :%v", err)
				return
			}
			err = client.Session.FeedAvPacket(pkt)
			if err != nil {
				fmt.Printf("stream client audio av packet err :%v", err)
				return
			}
		}
	}
}
