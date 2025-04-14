package hook

import (
	"sync"
	"time"

	"easydarwin/pkg/lalmax/fmp4/hls"

	"github.com/q191201771/lal/pkg/base"
	"github.com/q191201771/naza/pkg/nazalog"
)

var _ base.ISession = (*consumerInfo)(nil)

type IHookSessionSubscriber interface {
	OnMsg(msg base.RtmpMsg)
	OnStop()
}

type HookSession struct {
	uniqueKey  string
	streamName string
	consumers  sync.Map
	hlssvr     *hls.HlsServer
	gopCache   *GopCache
	hasVideo   bool
}

type consumerInfo struct {
	subscriber   IHookSessionSubscriber
	hasSendVideo bool

	base.StatSession
}

// AppName implements base.ISession.
func (c *consumerInfo) AppName() string {
	return c.SessionId
}

// GetStat implements base.ISession.
func (c *consumerInfo) GetStat() base.StatSession {
	return c.StatSession
}

// IsAlive implements base.ISession.
func (c *consumerInfo) IsAlive() (readAlive bool, writeAlive bool) {
	return true, true
}

// RawQuery implements base.ISession.
func (c *consumerInfo) RawQuery() string {
	return ""
}

// StreamName implements base.ISession.
func (c *consumerInfo) StreamName() string {
	return c.SessionId
}

// UniqueKey implements base.ISession.
func (c *consumerInfo) UniqueKey() string {
	return c.SessionId
}

// UpdateStat implements base.ISession.
func (c *consumerInfo) UpdateStat(intervalSec uint32) {
}

// Url implements base.ISession.
func (*consumerInfo) Url() string {
	return ""
}

func NewHookSession(uniqueKey, streamName string, hlssvr *hls.HlsServer, gopNum, singleGopMaxFrameNum int) *HookSession {
	s := &HookSession{
		uniqueKey:  uniqueKey,
		streamName: streamName,
		hlssvr:     hlssvr,
		gopCache:   NewGopCache(gopNum, singleGopMaxFrameNum),
	}

	if s.hlssvr != nil {
		s.hlssvr.NewHlsSession(streamName)
	}

	nazalog.Infof("create hook session, uniqueKey:%s, streamName:%s", uniqueKey, streamName)

	GetHookSessionManagerInstance().SetHookSession(streamName, s)
	return s
}

func (session *HookSession) OnMsg(msg base.RtmpMsg) {
	if session.hlssvr != nil {
		session.hlssvr.OnMsg(session.streamName, msg)
	}

	session.consumers.Range(func(key, value interface{}) bool {
		c := value.(*consumerInfo)

		gopCount := session.gopCache.GetGopCount()
		if !c.hasSendVideo && gopCount > 0 {
			if v := session.GetVideoSeqHeaderMsg(); v != nil {
				c.subscriber.OnMsg(*v)
			}
			if v := session.GetAudioSeqHeaderMsg(); v != nil {
				c.subscriber.OnMsg(*v)
			}
			for i := 0; i < gopCount; i++ {
				for _, item := range session.gopCache.GetGopDataAt(i) {
					c.subscriber.OnMsg(item)
				}
			}
			c.hasSendVideo = true
		}

		if msg.Header.MsgTypeId == base.RtmpTypeIdVideo {
			if !c.hasSendVideo {
				if !msg.IsVideoKeyNalu() {
					return true
				}
				if v := session.GetVideoSeqHeaderMsg(); v != nil {
					c.subscriber.OnMsg(*v)
				}
				if v := session.GetAudioSeqHeaderMsg(); v != nil {
					c.subscriber.OnMsg(*v)
				}
				c.hasSendVideo = true
			}

			c.subscriber.OnMsg(msg)
		} else if msg.Header.MsgTypeId == base.RtmpTypeIdAudio {
			if !session.hasVideo || c.hasSendVideo {
				c.subscriber.OnMsg(msg)
			}
		}
		return true
	})

	if !session.hasVideo && msg.IsVideoKeyNalu() {
		session.hasVideo = true
	}

	session.gopCache.Feed(msg)
}

func (session *HookSession) OnStop() {
	if session.hlssvr != nil {
		session.hlssvr.OnStop(session.streamName)
	}

	nazalog.Debugf("OnStop, uniqueKey:%s, streamName:%s", session.uniqueKey, session.streamName)
	session.consumers.Range(func(key, value interface{}) bool {
		c := value.(*consumerInfo)
		c.subscriber.OnStop()
		return true
	})

	GetHookSessionManagerInstance().RemoveHookSession(session.streamName)
}

func (session *HookSession) AddConsumer(consumerId string, subscriber IHookSessionSubscriber) {

	info := &consumerInfo{
		subscriber: subscriber,
		StatSession: base.StatSession{
			SessionId: consumerId,
			StartTime: time.Now().Format(time.DateTime),
			// Protocol: , TODO: (xugo)需要传递更多的参数来填充数据
		},
	}

	nazalog.Info("AddConsumer, consumerId:", consumerId)
	session.consumers.Store(consumerId, info)
}

func (session *HookSession) GetAllConsumer() []base.StatSub {
	out := make([]base.StatSub, 0, 10)
	session.consumers.Range(func(key, value any) bool {
		v, ok := value.(*consumerInfo)
		if ok {
			// TODO: (xugo)先简单实现，此处需要优化数据准确性
			out = append(out, base.Session2StatSub(v))
		}
		return true
	})
	return out
}

func (session *HookSession) RemoveConsumer(consumerId string) {
	_, ok := session.consumers.Load(consumerId)
	if ok {
		nazalog.Info("RemoveConsumer, consumerId:", consumerId)
		session.consumers.Delete(consumerId)
	}
}

func (session *HookSession) GetVideoSeqHeaderMsg() *base.RtmpMsg {
	return session.gopCache.videoheader
}

func (session *HookSession) GetAudioSeqHeaderMsg() *base.RtmpMsg {
	return session.gopCache.audioheader
}
