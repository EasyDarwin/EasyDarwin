package httpfmp4

import (
	"errors"
	"net/http"
	"strings"
	"sync"

	"easydarwin/pkg/lalmax/hook"

	"github.com/gofrs/uuid"
	"github.com/q191201771/naza/pkg/connection"

	"github.com/Eyevinn/mp4ff/mp4"
	"github.com/gin-gonic/gin"
	"github.com/q191201771/lal/pkg/aac"
	"github.com/q191201771/lal/pkg/avc"
	"github.com/q191201771/lal/pkg/base"
	"github.com/q191201771/lal/pkg/hevc"
	"github.com/q191201771/naza/pkg/nazalog"
)

var ErrWriteChanFull = errors.New("Fmp4  Session write channel full")

var (
	readBufSize = 4096 //  session connection读缓冲的大小
	wChanSize   = 256  //  session 发送数据时，channel 的大小
)

type HttpFmp4Session struct {
	streamid     string
	hooks        *hook.HookSession
	subscriberId string
	audioTrakId  uint32
	videoTrakId  uint32
	w            gin.ResponseWriter
	conn         connection.Connection
	disposeOnce  sync.Once
	initSegment  *mp4.InitSegment
	videoTrackId uint32
	audioTrackId uint32
	vfragment    *mp4.Fragment
	afragment    *mp4.Fragment
	lastVideoDts uint32
	lastAudioDts uint32
	seqNumber    uint32
	hasVideo     bool
}

func NewHttpFmp4Session(streamid string) *HttpFmp4Session {

	streamid = strings.TrimSuffix(streamid, ".mp4")
	u, _ := uuid.NewV4()

	session := &HttpFmp4Session{
		streamid:     streamid,
		subscriberId: u.String(),
		initSegment:  mp4.CreateEmptyInit(),
	}

	session.initSegment.Moov.Mvhd.NextTrackID = 1

	nazalog.Info("create http fmp4 seesion, streamid:", streamid)

	return session
}
func (session *HttpFmp4Session) Dispose() error {
	return session.dispose()
}
func (session *HttpFmp4Session) dispose() error {
	var retErr error
	session.disposeOnce.Do(func() {
		session.OnStop()
		if session.conn == nil {
			retErr = base.ErrSessionNotStarted
			return
		}
		retErr = session.conn.Close()
	})
	return retErr
}
func (session *HttpFmp4Session) handleSession(c *gin.Context) {
	ok, hooksession := hook.GetHookSessionManagerInstance().GetHookSession(session.streamid)
	if !ok {
		nazalog.Error("stream is not found, streamid:", session.streamid)
		c.Status(http.StatusNotFound)
		return
	}

	session.hooks = hooksession
	session.w = c.Writer

	vheader := hooksession.GetVideoSeqHeaderMsg()
	if vheader != nil {
		moov := session.initSegment.Moov
		session.videoTrackId = moov.Mvhd.NextTrackID
		moov.Mvhd.NextTrackID++
		newTrak := mp4.CreateEmptyTrak(session.videoTrackId, 1000, "video", "chi")
		moov.AddChild(newTrak)
		moov.Mvex.AddChild(mp4.CreateTrex(session.videoTrackId))
		switch vheader.VideoCodecId() {
		case base.RtmpCodecIdAvc:
			sps, pps, err := avc.ParseSpsPpsFromSeqHeader(vheader.Payload)
			if err != nil {
				nazalog.Error("ParseSpsPpsFromSeqHeader failed, err:", err)
				break
			}

			var spss, ppps [][]byte
			spss = append(spss, sps)
			ppps = append(ppps, pps)

			err = newTrak.SetAVCDescriptor("avc1", spss, ppps, true)
			if err != nil {
				nazalog.Error("SetAVCDescriptor failed, err:", err)
				break
			}

		case base.RtmpCodecIdHevc:
			var vpss, spss, ppss [][]byte
			var vps, sps, pps []byte
			var err error

			if vheader.IsEnhanced() {
				vps, sps, pps, err = hevc.ParseVpsSpsPpsFromEnhancedSeqHeader(vheader.Payload)
				if err != nil {
					nazalog.Error("ParseVpsSpsPpsFromEnhancedSeqHeader failed, err:", err)
					break
				}

			} else {
				vps, sps, pps, err = hevc.ParseVpsSpsPpsFromSeqHeader(vheader.Payload)
				if err != nil {
					nazalog.Error("ParseVpsSpsPpsFromSeqHeader failed, err:", err)
					break
				}
			}

			vpss = append(vpss, vps)
			spss = append(spss, sps)
			ppss = append(ppss, pps)

			err = newTrak.SetHEVCDescriptor("hvc1", vpss, spss, ppss, nil, true)
			if err != nil {
				nazalog.Error("SetHEVCDescriptor failed, err:", err)
				break
			}

		default:
			nazalog.Error("unknow video codecid:", vheader.VideoCodecId())
		}

		session.hasVideo = true
	}

	aheader := hooksession.GetAudioSeqHeaderMsg()
	if aheader != nil {
		moov := session.initSegment.Moov
		session.audioTrackId = moov.Mvhd.NextTrackID
		moov.Mvhd.NextTrackID++
		newTrak := mp4.CreateEmptyTrak(session.audioTrackId, 1000, "audio", "chi")
		moov.AddChild(newTrak)
		moov.Mvex.AddChild(mp4.CreateTrex(session.audioTrackId))

		switch aheader.AudioCodecId() {
		case base.RtmpSoundFormatAac:
			ascCtx, err := aac.NewAscContext(aheader.Payload[2:])
			if err != nil {
				nazalog.Error("NewAscContext failed, err:", err)
				return
			}

			samplerate, _ := ascCtx.GetSamplingFrequency()
			switch ascCtx.AudioObjectType {
			case 1:
				// HEAACv1
				newTrak.SetAACDescriptor(5, samplerate)
			case 2:
				// AAClc
				newTrak.SetAACDescriptor(2, samplerate)
			case 3:
				// HEAACv2
				newTrak.SetAACDescriptor(29, samplerate)
			}

		default:
			nazalog.Error("unknow audio codecid:", aheader.AudioCodecId())
		}
	}

	if vheader == nil && aheader == nil {
		c.Status(http.StatusNotFound)
		return
	}

	c.Header("Content-Type", "video/mp4")
	c.Header("Connection", "close")
	c.Header("Expires", "-1")
	h, ok := session.w.(http.Hijacker)
	if !ok {
		nazalog.Error("gin response does not implement http.Hijacker")
		return
	}

	conn, bio, err := h.Hijack()
	if err != nil {
		nazalog.Errorf("hijack failed. err=%+v", err)
		return
	}
	if bio.Reader.Buffered() != 0 || bio.Writer.Buffered() != 0 {
		nazalog.Errorf("hijack but buffer not empty. rb=%d, wb=%d", bio.Reader.Buffered(), bio.Writer.Buffered())
	}
	session.conn = connection.New(conn, func(option *connection.Option) {
		option.ReadBufSize = readBufSize
		option.WriteChanSize = wChanSize
	})
	if err = session.writeHttpHeader(session.w.Header()); err != nil {
		nazalog.Errorf("session writeHttpHeader. err=%+v", err)
		return
	}
	session.hooks.AddConsumer(session.subscriberId, session)
	session.initSegment.Encode(session.conn)

	readBuf := make([]byte, 1024)
	_, err = session.conn.Read(readBuf)
	session.dispose()
}

func (session *HttpFmp4Session) writeHttpHeader(header http.Header) error {
	p := make([]byte, 0, 1024)
	p = append(p, []byte("HTTP/1.1 200 OK\r\n")...)
	for k, vs := range header {
		for _, v := range vs {
			p = append(p, k...)
			p = append(p, ": "...)
			for i := 0; i < len(v); i++ {
				b := v[i]
				if b <= 31 {
					// prevent response splitting.
					b = ' '
				}
				p = append(p, b)
			}
			p = append(p, "\r\n"...)
		}
	}
	p = append(p, "\r\n"...)

	return session.write(p)
}
func (session *HttpFmp4Session) write(buf []byte) (err error) {
	if session.conn != nil {
		_, err = session.conn.Write(buf)
	}
	return err
}
func (session *HttpFmp4Session) OnMsg(msg base.RtmpMsg) {
	switch msg.Header.MsgTypeId {
	case base.RtmpTypeIdMetadata:
		return
	case base.RtmpTypeIdAudio:
		session.FeedAudio(msg)
	case base.RtmpTypeIdVideo:
		session.FeedVideo(msg)
	}
}

func (session *HttpFmp4Session) OnStop() {
	session.hooks.RemoveConsumer(session.subscriberId)
}

func (session *HttpFmp4Session) FeedVideo(msg base.RtmpMsg) {
	if msg.VideoCodecId() != base.RtmpCodecIdAvc && msg.VideoCodecId() != base.RtmpCodecIdHevc {
		return
	}

	flags := mp4.NonSyncSampleFlags
	if msg.IsVideoKeyNalu() {
		flags = mp4.SyncSampleFlags
	}

	var duration uint32
	if session.lastVideoDts == 0 {
		session.lastVideoDts = msg.Dts()
		duration = msg.Dts() - session.lastVideoDts
	} else {
		duration = msg.Dts() - session.lastVideoDts
		session.lastVideoDts = msg.Dts()
	}

	if session.vfragment != nil && len(session.vfragment.Moof.Traf.Trun.Samples) > 10 {
		session.vfragment.Encode(session.conn)
		session.vfragment = nil
	}

	if session.vfragment == nil {
		session.vfragment, _ = mp4.CreateFragment(session.GetSequenceNumber(), session.videoTrackId)
	}

	index := 5
	if msg.IsEnhanced() {
		index = msg.GetEnchanedHevcNaluIndex()
	}

	session.vfragment.AddFullSample(mp4.FullSample{
		Data:       msg.Payload[index:],
		DecodeTime: uint64(msg.Dts()),
		Sample: mp4.Sample{
			Flags:                 flags,
			Dur:                   duration,
			Size:                  uint32(len(msg.Payload[index:])),
			CompositionTimeOffset: int32(msg.Cts()),
		},
	})
}

func (session *HttpFmp4Session) FeedAudio(msg base.RtmpMsg) {
	if msg.AudioCodecId() != base.RtmpSoundFormatAac {
		return
	}

	var duration uint32
	if session.lastAudioDts == 0 {
		session.lastAudioDts = msg.Dts()
		duration = msg.Dts() - session.lastAudioDts
	} else {
		duration = msg.Dts() - session.lastAudioDts
		session.lastAudioDts = msg.Dts()
	}

	if session.afragment != nil && len(session.afragment.Moof.Traf.Trun.Samples) > 10 {
		session.afragment.Encode(session.conn)
		session.afragment = nil
	}

	if session.afragment == nil {
		session.afragment, _ = mp4.CreateFragment(session.GetSequenceNumber(), session.audioTrackId)
	}

	session.afragment.AddFullSample(mp4.FullSample{
		Data:       msg.Payload[2:],
		DecodeTime: uint64(msg.Dts()),
		Sample: mp4.Sample{
			Flags:                 mp4.NonSyncSampleFlags,
			Dur:                   duration,
			Size:                  uint32(len(msg.Payload[2:])),
			CompositionTimeOffset: 0,
		},
	})
}

func (session *HttpFmp4Session) GetSequenceNumber() uint32 {
	session.seqNumber++
	return session.seqNumber
}
