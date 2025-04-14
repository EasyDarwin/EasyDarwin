package hls

import (
	"time"

	config "easydarwin/pkg/lalmax/conf"

	"github.com/bluenviron/gohlslib"
	"github.com/bluenviron/gohlslib/pkg/codecs"
	"github.com/bluenviron/mediacommon/pkg/codecs/mpeg4audio"
	"github.com/gin-gonic/gin"
	"github.com/q191201771/lal/pkg/avc"
	"github.com/q191201771/lal/pkg/base"
	"github.com/q191201771/lal/pkg/hevc"
	"github.com/q191201771/naza/pkg/nazalog"
	uuid "github.com/satori/go.uuid"
)

type HlsSession struct {
	muxer               *gohlslib.Muxer
	done                bool
	data                []Frame
	audioCodecId        int
	videoCodecId        int
	maxMsgSize          int
	streamName          string
	sps                 []byte
	pps                 []byte
	vps                 []byte
	asc                 []byte
	startAudioPts       time.Duration
	startVideoPts       time.Duration
	audioStartPTSFilled bool
	videoStartPTSFilled bool
	SessionId           string
}

func NewHlsSession(streamName string, conf config.HlsConfig) *HlsSession {
	variant := gohlslib.MuxerVariantFMP4
	if conf.LowLatency {
		variant = gohlslib.MuxerVariantLowLatency
	}

	u, _ := uuid.NewV4()

	session := &HlsSession{
		muxer: &gohlslib.Muxer{
			Variant: variant,
		},
		audioCodecId: -1,
		videoCodecId: -1,
		maxMsgSize:   128,
		data:         make([]Frame, 10)[0:0],
		streamName:   streamName,
		SessionId:    u.String(),
	}

	uid, _ := uuid.NewV4()
	session.SessionId = uid.String()

	if !conf.LowLatency && conf.SegmentCount > 0 {
		// fmp4模式下可以设置分片个数
		session.muxer.SegmentCount = conf.SegmentCount
	}

	if conf.LowLatency && conf.PartDuration > 0 {
		// llhls设置part duration
		session.muxer.PartDuration = time.Millisecond * time.Duration(conf.PartDuration)
	}

	if conf.SegmentDuration > 0 {
		session.muxer.SegmentDuration = time.Second * time.Duration(conf.SegmentDuration)
	}

	return session
}

func (session *HlsSession) OnMsg(msg base.RtmpMsg) {
	if session.done {
		if msg.Header.MsgTypeId == base.RtmpTypeIdVideo {
			if msg.IsVideoKeySeqHeader() {
				session.videoCodecId = int(msg.VideoCodecId())
				if msg.IsAvcKeySeqHeader() {
					var err error
					session.sps, session.pps, err = avc.ParseSpsPpsFromSeqHeader(msg.Payload)
					if err != nil {
						nazalog.Error("ParseSpsPpsFromSeqHeader err:", err)
					}
				} else {
					session.vps, session.sps, session.pps, _ = hevc.ParseVpsSpsPpsFromSeqHeaderWithoutMalloc(msg.Payload)
				}
			} else {
				nals, err := avc.SplitNaluAvcc(msg.Payload[5:])
				if err != nil {
					nazalog.Error(err)
					return
				}

				var nalus [][]byte
				if msg.IsAvcKeyNalu() || msg.IsHevcKeyNalu() {
					if msg.IsAvcKeyNalu() {
						nalus = append(nalus, session.sps)
						nalus = append(nalus, session.pps)
					} else {
						nalus = append(nalus, session.vps)
						nalus = append(nalus, session.sps)
						nalus = append(nalus, session.pps)
					}
				}

				nalus = append(nalus, nals...)
				pts := time.Millisecond*time.Duration(msg.Pts()) - session.startVideoPts
				err = session.muxer.WriteH26x(time.Now(), pts, nalus)
				if err != nil {
					nazalog.Error("hls-fmp4 WriteH26x failed, err:", err)
				}
			}

		} else {
			if session.audioCodecId == int(base.RtmpSoundFormatAac) {
				pts := time.Millisecond*time.Duration(msg.Dts()) - session.startAudioPts
				err := session.muxer.WriteMPEG4Audio(time.Now(), pts, [][]byte{msg.Payload[2:]})
				if err != nil {
					nazalog.Error("hls-fmp4 WriteMPEG4Audio failed, err:", err)
				}
			} else if session.audioCodecId == int(base.RtmpSoundFormatOpus) {
				pts := time.Millisecond*time.Duration(msg.Dts()) - session.startAudioPts
				err := session.muxer.WriteOpus(time.Now(), pts, [][]byte{msg.Payload[1:]})
				if err != nil {
					nazalog.Error("hls-fmp4 WriteOpus failed, err:", err)
				}
			}
		}

		return
	}

	switch msg.Header.MsgTypeId {
	case base.RtmpTypeIdAudio:
		session.audioCodecId = int(msg.AudioCodecId())
		if session.audioCodecId == int(base.RtmpSoundFormatAac) {
			if msg.IsAacSeqHeader() {
				session.asc = msg.Payload[2:]
			} else {
				if !session.audioStartPTSFilled {
					session.startAudioPts = time.Millisecond * time.Duration(msg.Dts())
					session.audioStartPTSFilled = true
				}

				pts := time.Millisecond*time.Duration(msg.Dts()) - session.startAudioPts

				frame := Frame{
					ntp:       time.Now(),
					pts:       pts,
					au:        [][]byte{msg.Payload[2:]},
					codecType: msg.AudioCodecId(),
				}
				session.data = append(session.data, frame)
			}
		} else if session.audioCodecId == int(base.RtmpSoundFormatOpus) {
			if !session.audioStartPTSFilled {
				session.startAudioPts = time.Millisecond * time.Duration(msg.Dts())
				session.audioStartPTSFilled = true
			}

			pts := time.Millisecond*time.Duration(msg.Dts()) - session.startAudioPts
			frame := Frame{
				ntp:       time.Now(),
				pts:       pts,
				au:        [][]byte{msg.Payload[1:]},
				codecType: msg.AudioCodecId(),
			}
			session.data = append(session.data, frame)
		} else {
			return
		}
	case base.RtmpTypeIdVideo:
		if msg.IsVideoKeySeqHeader() {
			session.videoCodecId = int(msg.VideoCodecId())
			if msg.IsAvcKeySeqHeader() {
				var err error
				session.sps, session.pps, err = avc.ParseSpsPpsFromSeqHeader(msg.Payload)
				if err != nil {
					nazalog.Error("ParseSpsPpsFromSeqHeader err:", err)
				}
			} else {
				session.vps, session.sps, session.pps, _ = hevc.ParseVpsSpsPpsFromSeqHeaderWithoutMalloc(msg.Payload)
			}
		} else {
			nals, err := avc.SplitNaluAvcc(msg.Payload[5:])
			if err != nil {
				return
			}

			if !session.videoStartPTSFilled {
				session.startVideoPts = time.Millisecond * time.Duration(msg.Pts())
				session.videoStartPTSFilled = true
			}

			var nalus [][]byte
			if msg.IsAvcKeyNalu() || msg.IsHevcKeyNalu() {
				if msg.IsAvcKeyNalu() {
					nalus = append(nalus, session.sps)
					nalus = append(nalus, session.pps)
				} else {
					nalus = append(nalus, session.vps)
					nalus = append(nalus, session.sps)
					nalus = append(nalus, session.pps)
				}
			}

			nalus = append(nalus, nals...)

			pts := time.Millisecond*time.Duration(msg.Pts()) - session.startVideoPts

			frame := Frame{
				ntp:       time.Now(),
				pts:       pts,
				au:        nalus,
				codecType: msg.VideoCodecId(),
			}

			session.data = append(session.data, frame)
		}
	}

	if session.videoCodecId != -1 && session.audioCodecId != -1 {
		session.drain()
		return
	}

	if len(session.data) >= session.maxMsgSize {
		session.drain()
		return
	}
}

func (session *HlsSession) drain() {
	if session.videoCodecId != -1 {
		if session.videoCodecId == int(base.RtmpCodecIdAvc) {
			session.muxer.VideoTrack = &gohlslib.Track{
				Codec: &codecs.H264{
					SPS: session.sps,
					PPS: session.pps,
				},
			}
		} else if session.videoCodecId == int(base.RtmpCodecIdHevc) {
			session.muxer.VideoTrack = &gohlslib.Track{
				Codec: &codecs.H265{
					VPS: session.vps,
					SPS: session.sps,
					PPS: session.pps,
				},
			}
		}
	}

	if session.audioCodecId != -1 {
		if session.audioCodecId == int(base.RtmpSoundFormatAac) {
			var mpegConf mpeg4audio.Config
			err := mpegConf.Unmarshal(session.asc)
			if err != nil {
				nazalog.Error(err)
				return
			}

			session.muxer.AudioTrack = &gohlslib.Track{
				Codec: &codecs.MPEG4Audio{
					Config: mpegConf,
				},
			}
		} else if session.audioCodecId == int(base.RtmpSoundFormatOpus) {
			session.muxer.AudioTrack = &gohlslib.Track{
				Codec: &codecs.Opus{
					ChannelCount: 1,
				},
			}
		}
	}

	if err := session.muxer.Start(); err != nil {
		nazalog.Error(err)
		return
	}

	for _, data := range session.data {
		if (data.codecType == base.RtmpCodecIdAvc || data.codecType == base.RtmpCodecIdHevc) && session.videoCodecId != -1 {
			err := session.muxer.WriteH26x(data.ntp, data.pts, data.au)
			if err != nil {
				nazalog.Error("hls-fmp4 WriteH26x failed, err:", err)
				continue
			}
		} else if session.audioCodecId != -1 {
			if data.codecType == base.RtmpSoundFormatAac {
				err := session.muxer.WriteMPEG4Audio(data.ntp, data.pts, data.au)
				if err != nil {
					nazalog.Error("hls-fmp4 WriteMPEG4Audio failed, err:", err)
					continue
				}
			} else if data.codecType == base.RtmpSoundFormatOpus {
				err := session.muxer.WriteOpus(data.ntp, data.pts, data.au)
				if err != nil {
					nazalog.Error("hls-fmp4 WriteMPEG4Audio failed, err:", err)
					continue
				}
			} else {
				// gohlslib不支持g711
				continue
			}
		}
	}

	session.done = true
}

func (session *HlsSession) OnStop() {
	if session.done {
		session.muxer.Close()
	}
}

func (session *HlsSession) HandleRequest(ctx *gin.Context) {
	nazalog.Info("handle hls request, streamName:", session.streamName, " path:", ctx.Request.URL.Path)
	session.muxer.Handle(ctx.Writer, ctx.Request)
}

type Frame struct {
	ntp       time.Time
	pts       time.Duration
	au        [][]byte
	codecType uint8
}
