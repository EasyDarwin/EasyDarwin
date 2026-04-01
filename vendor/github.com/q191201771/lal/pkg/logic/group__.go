// Copyright 2019, Chef.  All rights reserved.
// https://github.com/q191201771/lal
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

package logic

import (
	"encoding/json"
	"strings"
	"sync"
	"time"

	"github.com/q191201771/lal/pkg/gb28181"

	"github.com/q191201771/lal/pkg/base"
	"github.com/q191201771/lal/pkg/hls"
	"github.com/q191201771/lal/pkg/httpflv"
	"github.com/q191201771/lal/pkg/httpts"
	"github.com/q191201771/lal/pkg/mpegts"
	"github.com/q191201771/lal/pkg/remux"
	"github.com/q191201771/lal/pkg/rtmp"
	"github.com/q191201771/lal/pkg/rtsp"
	"github.com/q191201771/lal/pkg/sdp"
)

// ---------------------------------------------------------------------------------------------------------------------
// 输入流需要做的事情
// TODO(chef): [refactor] 考虑抽象出通用接口 202208
//
// checklist表格
// | .                                           | rtmp pub | ps pub |
// | 添加到group中                                | Y        | Y      |
// | 到输出流的转换路径关系                         | Y        | Y      |
// | 删除                                        | Y        | Y      |
// | group.hasPubSession()                       | Y        | Y      |
// | group.disposeInactiveSessions()检查超时并清理 | Y        | Y      |
// | group.Dispose()时销毁                        | Y        | Y      |
// | group.GetStat()时获取信息                     | Y        | Y      |
// | group.KickSession()时踢出                    | Y        | Y      |
// | group.updateAllSessionStat()更新信息         | Y        | Y      |
// | group.inSessionUniqueKey()                  | Y        | Y      |

// TODO(chef): [refactor] 整理sub类型流接入需要做的事情的文档 202211

// ---------------------------------------------------------------------------------------------------------------------
// 输入流到输出流的转换路径关系（一共6种输入）：
//
// rtmpPullSession.WithOnReadRtmpAvMsg  ->
// rtmpPubSession.SetPubSessionObserver ->
//    customizePubSession.WithOnRtmpMsg -> OnReadRtmpAvMsg(enter Lock) -> [dummyAudioFilter] -> broadcastByRtmpMsg -> rtmp, http-flv
//                                                                                                                 -> rtmp2RtspRemuxer -> rtsp
//                                                                                                                 -> rtmp2MpegtsRemuxer -> ts, hls
//
// ---------------------------------------------------------------------------------------------------------------------
// rtspPullSession ->
//  rtspPubSession -> OnRtpPacket(enter Lock) -> rtsp
//                 -> OnAvPacket(enter Lock) -> rtsp2RtmpRemuxer -> onRtmpMsgFromRemux -> [dummyAudioFilter] -> broadcastByRtmpMsg -> rtmp, http-flv
//                                                                                                           -> rtmp2MpegtsRemuxer -> ts, hls
//
// ---------------------------------------------------------------------------------------------------------------------
// psPubSession -> OnAvPacketFromPsPubSession(enter Lock) -> rtsp2RtmpRemuxer -> onRtmpMsgFromRemux -> [dummyAudioFilter] -> broadcastByRtmpMsg -> ...
//                                                                                                                                              -> ...
//                                                                                                                                              -> ...

type GroupOption struct {
	onHookSession func(uniqueKey string, streamName string) ICustomizeHookSessionContext
}

type IGroupObserver interface {
	CleanupHlsIfNeeded(appName string, streamName string, path string)
	OnHlsMakeTs(info base.HlsMakeTsInfo)
	OnRelayPullStart(info base.PullStartInfo) // TODO(chef): refactor me
	OnRelayPullStop(info base.PullStopInfo)
}

type Group struct {
	UniqueKey  string // const after init
	appName    string // const after init
	streamName string // const after init TODO chef: 和stat里的字段重复，可以删除掉
	config     *Config

	option                      GroupOption
	customizeHookSessionContext ICustomizeHookSessionContext

	observer IGroupObserver

	exitChan chan struct{}

	mutex sync.Mutex
	// pub
	rtmpPubSession      *rtmp.ServerSession
	rtspPubSession      *rtsp.PubSession
	customizePubSession *CustomizePubSessionContext
	psPubSession        *gb28181.PubSession
	rtsp2RtmpRemuxer    *remux.AvPacket2RtmpRemuxer // TODO(chef): [refactor] 重命名为avPacket2RtmpRemuxer，因为除了rtsp，customize pub和gb28181 pub都是 202208
	rtmp2RtspRemuxer    *remux.Rtmp2RtspRemuxer
	rtmp2MpegtsRemuxer  *remux.Rtmp2MpegtsRemuxer
	// pull
	pullProxy *pullProxy
	// rtmp pub使用 TODO(chef): [doc] 更新这个注释，是共同使用 202210
	dummyAudioFilter *remux.DummyAudioFilter
	// ps pub使用
	psPubTimeoutSec            uint32 // 超时时间
	psPubPrevInactiveCheckTick int64  // 上次检查时间
	// rtmp sub使用
	rtmpGopCache *remux.GopCache
	// httpflv sub使用
	httpflvGopCache *remux.GopCache
	// httpts sub使用
	httptsGopCache *remux.GopCacheMpegts
	// rtsp使用
	sdpCtx *sdp.LogicContext
	// mpegts使用
	patpmt []byte
	// sub
	rtmpSubSessionSet    map[*rtmp.ServerSession]struct{}
	httpflvSubSessionSet map[*httpflv.SubSession]struct{}
	httptsSubSessionSet  map[*httpts.SubSession]struct{}
	rtspSubSessionSet    map[*rtsp.SubSession]struct{} // 注意，使用这个容器时，一定要注意 session 的 Stage 属性
	hlsSubSessionSet     map[*hls.SubSession]struct{}
	// push
	pushEnable    bool
	url2PushProxy map[string]*pushProxy
	// hls
	hlsMuxer *hls.Muxer
	// record
	recordFlv    *httpflv.FlvFileWriter
	recordMpegts *mpegts.FileWriter
	// rtmp sub使用
	rtmpMergeWriter *base.MergeWriter // TODO(chef): 后面可以在业务层加一个定时Flush
	//
	stat base.StatGroup
	//
	inVideoFpsRecords base.PeriodRecord
	//
	hlsCalcSessionStatIntervalSec uint32
	//
	psPubDumpFile    *base.DumpFile
	rtspPullDumpFile *base.DumpFile
}

func NewGroup(appName string, streamName string, config *Config, option GroupOption, observer IGroupObserver) *Group {
	uk := base.GenUkGroup()

	g := &Group{
		UniqueKey:  uk,
		appName:    appName,
		streamName: streamName,
		config:     config,
		option:     option,
		observer:   observer,
		stat: base.StatGroup{
			StreamName: streamName,
			AppName:    appName,
		},
		exitChan:                   make(chan struct{}, 1),
		rtmpSubSessionSet:          make(map[*rtmp.ServerSession]struct{}),
		httpflvSubSessionSet:       make(map[*httpflv.SubSession]struct{}),
		httptsSubSessionSet:        make(map[*httpts.SubSession]struct{}),
		rtspSubSessionSet:          make(map[*rtsp.SubSession]struct{}),
		hlsSubSessionSet:           make(map[*hls.SubSession]struct{}),
		rtmpGopCache:               remux.NewGopCache("rtmp", uk, config.RtmpConfig.GopNum, config.RtmpConfig.SingleGopMaxFrameNum),
		httpflvGopCache:            remux.NewGopCache("httpflv", uk, config.HttpflvConfig.GopNum, config.HttpflvConfig.SingleGopMaxFrameNum),
		httptsGopCache:             remux.NewGopCacheMpegts(uk, config.HttptsConfig.GopNum, config.HttptsConfig.SingleGopMaxFrameNum),
		psPubPrevInactiveCheckTick: -1,
		inVideoFpsRecords:          base.NewPeriodRecord(32),
	}

	g.hlsCalcSessionStatIntervalSec = uint32(config.HlsConfig.FragmentDurationMs / 100) // equals to (ms/1000) * 10
	if g.hlsCalcSessionStatIntervalSec == 0 {
		g.hlsCalcSessionStatIntervalSec = defaultHlsCalcSessionStatIntervalSec
	}

	g.initRelayPushByConfig()
	g.initRelayPullByConfig()

	if config.RtmpConfig.MergeWriteSize > 0 {
		g.rtmpMergeWriter = base.NewMergeWriter(g.writev2RtmpSubSessions, config.RtmpConfig.MergeWriteSize)
	}

	Log.Infof("[%s] lifecycle new group. group=%p, appName=%s, streamName=%s", uk, g, appName, streamName)
	return g
}

func (group *Group) RunLoop() {
	<-group.exitChan
}

// Tick 定时器
//
// @param tickCount 当前时间，单位秒。注意，不一定是Unix时间戳，可以是从0开始+1秒递增的时间
func (group *Group) Tick(tickCount uint32) {
	group.mutex.Lock()
	defer group.mutex.Unlock()

	group.tickPullModule()
	group.startPushIfNeeded()

	// 定时关闭没有数据的session
	group.disposeInactiveSessions(tickCount)

	// 定时计算session bitrate
	if tickCount%calcSessionStatIntervalSec == 0 {
		group.updateAllSessionStat()
	}

	// because hls make multiple separate http request to get stream content and gap between request base on hls segment duration
	// if we update every 5s can cause bitrateKbit equal to 0 if within 5s do not have any ts http request is make
	if tickCount%group.hlsCalcSessionStatIntervalSec == 0 {
		for session := range group.hlsSubSessionSet {
			session.UpdateStat(group.hlsCalcSessionStatIntervalSec)
		}
	}
}

// Dispose ...
func (group *Group) Dispose() {
	Log.Infof("[%s] lifecycle dispose group.", group.UniqueKey)
	group.exitChan <- struct{}{}

	group.mutex.Lock()
	defer group.mutex.Unlock()

	if group.rtmpPubSession != nil {
		group.rtmpPubSession.Dispose()
	}
	if group.rtspPubSession != nil {
		group.rtspPubSession.Dispose()
	}
	if group.psPubSession != nil {
		group.psPubSession.Dispose()
	}

	for session := range group.rtmpSubSessionSet {
		session.Dispose()
	}
	group.rtmpSubSessionSet = nil

	for session := range group.rtspSubSessionSet {
		session.Dispose()
	}
	group.rtspSubSessionSet = nil

	for session := range group.httpflvSubSessionSet {
		session.Dispose()
	}
	group.httpflvSubSessionSet = nil

	for session := range group.httptsSubSessionSet {
		session.Dispose()
	}
	group.httptsSubSessionSet = nil

	group.delIn()
}

// ---------------------------------------------------------------------------------------------------------------------

func (group *Group) StringifyDebugStats(maxsub int) string {
	b, _ := json.Marshal(group.GetStat(maxsub))
	return string(b)
}

func (group *Group) GetStat(maxsub int) base.StatGroup {
	// TODO(chef): [refactor] param maxsub

	group.mutex.Lock()
	defer group.mutex.Unlock()

	if group.rtmpPubSession != nil {
		group.stat.StatPub = base.Session2StatPub(group.rtmpPubSession)
	} else if group.rtspPubSession != nil {
		group.stat.StatPub = base.Session2StatPub(group.rtspPubSession)
	} else if group.psPubSession != nil {
		group.stat.StatPub = base.Session2StatPub(group.psPubSession)
	} else {
		group.stat.StatPub = base.StatPub{}
	}

	group.stat.StatPull = group.getStatPull()

	group.stat.StatSubs = nil
	var statSubCount int
	for s := range group.rtmpSubSessionSet {
		statSubCount++
		if statSubCount > maxsub {
			break
		}
		group.stat.StatSubs = append(group.stat.StatSubs, base.Session2StatSub(s))
	}
	for s := range group.httpflvSubSessionSet {
		statSubCount++
		if statSubCount > maxsub {
			break
		}
		group.stat.StatSubs = append(group.stat.StatSubs, base.Session2StatSub(s))
	}
	for s := range group.httptsSubSessionSet {
		statSubCount++
		if statSubCount > maxsub {
			break
		}
		group.stat.StatSubs = append(group.stat.StatSubs, base.Session2StatSub(s))
	}
	for s := range group.rtspSubSessionSet {
		statSubCount++
		if statSubCount > maxsub {
			break
		}
		group.stat.StatSubs = append(group.stat.StatSubs, base.Session2StatSub(s))
	}

	for s := range group.hlsSubSessionSet {
		statSubCount++
		if statSubCount > maxsub {
			break
		}
		group.stat.StatSubs = append(group.stat.StatSubs, base.Session2StatSub(s))
	}

	group.stat.GetFpsFrom(&group.inVideoFpsRecords, time.Now().Unix())

	return group.stat
}

func (group *Group) KickSession(sessionId string) bool {
	group.mutex.Lock()
	defer group.mutex.Unlock()

	Log.Infof("[%s] kick session. session id=%s", group.UniqueKey, sessionId)

	if strings.HasPrefix(sessionId, base.UkPreRtmpServerSession) {
		if group.rtmpPubSession != nil && group.rtmpPubSession.UniqueKey() == sessionId {
			group.rtmpPubSession.Dispose()
			return true
		}
		for s := range group.rtmpSubSessionSet {
			if s.UniqueKey() == sessionId {
				s.Dispose()
				return true
			}
		}
	} else if strings.HasPrefix(sessionId, base.UkPreRtmpPullSession) || strings.HasPrefix(sessionId, base.UkPreRtspPullSession) {
		return group.kickPull(sessionId)
	} else if strings.HasPrefix(sessionId, base.UkPreRtspPubSession) {
		if group.rtspPubSession != nil && group.rtspPubSession.UniqueKey() == sessionId {
			group.rtspPubSession.Dispose()
			return true
		}
	} else if strings.HasPrefix(sessionId, base.UkPrePsPubSession) {
		if group.psPubSession != nil && group.psPubSession.UniqueKey() == sessionId {
			group.psPubSession.Dispose()
			return true
		}
	} else if strings.HasPrefix(sessionId, base.UkPreFlvSubSession) {
		// TODO chef: 考虑数据结构改成sessionIdzuokey的map
		for s := range group.httpflvSubSessionSet {
			if s.UniqueKey() == sessionId {
				s.Dispose()
				return true
			}
		}
	} else if strings.HasPrefix(sessionId, base.UkPreTsSubSession) {
		for s := range group.httptsSubSessionSet {
			if s.UniqueKey() == sessionId {
				s.Dispose()
				return true
			}
		}
	} else if strings.HasPrefix(sessionId, base.UkPreRtspSubSession) {
		for s := range group.rtspSubSessionSet {
			if s.UniqueKey() == sessionId {
				s.Dispose()
				return true
			}
		}
	} else {
		Log.Errorf("[%s] kick session while session id format invalid. %s", group.UniqueKey, sessionId)
	}

	return false
}

func (group *Group) IsInactive() bool {
	group.mutex.Lock()
	defer group.mutex.Unlock()
	return group.isTotalEmpty() && !group.isPullModuleAlive()
}

func (group *Group) HasInSession() bool {
	group.mutex.Lock()
	defer group.mutex.Unlock()
	return group.hasInSession()
}

func (group *Group) HasOutSession() bool {
	group.mutex.Lock()
	defer group.mutex.Unlock()
	return group.hasOutSession()
}

func (group *Group) OutSessionNum() int {
	// TODO(chef): 没有包含hls的播放者

	group.mutex.Lock()
	defer group.mutex.Unlock()

	pushNum := 0
	for _, item := range group.url2PushProxy {
		// TODO(chef): [refactor] 考虑只判断session是否为nil 202205
		if item.isPushing && item.pushSession != nil {
			pushNum++
		}
	}
	return len(group.rtmpSubSessionSet) + len(group.rtspSubSessionSet) +
		len(group.httpflvSubSessionSet) + len(group.httptsSubSessionSet) + pushNum
}

// ---------------------------------------------------------------------------------------------------------------------

// disposeInactiveSessions 关闭不活跃的session
func (group *Group) disposeInactiveSessions(tickCount uint32) {
	if group.psPubSession != nil {
		if group.psPubTimeoutSec == 0 {
			// noop
			// 没有超时逻辑
		} else {
			if group.psPubPrevInactiveCheckTick == -1 ||
				tickCount-uint32(group.psPubPrevInactiveCheckTick) >= group.psPubTimeoutSec {

				if readAlive, _ := group.psPubSession.IsAlive(); !readAlive {
					Log.Warnf("[%s] session timeout. session=%s", group.UniqueKey, group.psPubSession.UniqueKey())
					group.psPubSession.Dispose()
				}

				group.psPubPrevInactiveCheckTick = int64(tickCount)
			}
		}
	}

	// 以下都是以 CheckSessionAliveIntervalSec 为间隔的清理逻辑

	if tickCount%CheckSessionAliveIntervalSec != 0 {
		return
	}

	if group.rtmpPubSession != nil {
		if readAlive, _ := group.rtmpPubSession.IsAlive(); !readAlive {
			Log.Warnf("[%s] session timeout. session=%s", group.UniqueKey, group.rtmpPubSession.UniqueKey())
			group.rtmpPubSession.Dispose()
		}
	}
	if group.rtspPubSession != nil {
		if readAlive, _ := group.rtspPubSession.IsAlive(); !readAlive {
			Log.Warnf("[%s] session timeout. session=%s", group.UniqueKey, group.rtspPubSession.UniqueKey())
			group.rtspPubSession.Dispose()
		}
	}

	group.disposeInactivePullSession()

	for session := range group.rtmpSubSessionSet {
		if _, writeAlive := session.IsAlive(); !writeAlive {
			Log.Warnf("[%s] session timeout. session=%s", group.UniqueKey, session.UniqueKey())
			session.Dispose()
		}
	}
	for session := range group.rtspSubSessionSet {
		if _, writeAlive := session.IsAlive(); !writeAlive {
			Log.Warnf("[%s] session timeout. session=%s", group.UniqueKey, session.UniqueKey())
			session.Dispose()
		}
	}

	for session := range group.httpflvSubSessionSet {
		if _, writeAlive := session.IsAlive(); !writeAlive {
			Log.Warnf("[%s] session timeout. session=%s", group.UniqueKey, session.UniqueKey())
			session.Dispose()
		}
	}
	for session := range group.httptsSubSessionSet {
		if _, writeAlive := session.IsAlive(); !writeAlive {
			Log.Warnf("[%s] session timeout. session=%s", group.UniqueKey, session.UniqueKey())
			session.Dispose()
		}
	}
	for _, item := range group.url2PushProxy {
		session := item.pushSession
		if item.isPushing && session != nil {
			if _, writeAlive := session.IsAlive(); !writeAlive {
				Log.Warnf("[%s] session timeout. session=%s", group.UniqueKey, session.UniqueKey())
				session.Dispose()
			}
		}
	}
}

// updateAllSessionStat 更新所有session的状态
func (group *Group) updateAllSessionStat() {
	if group.rtmpPubSession != nil {
		group.rtmpPubSession.UpdateStat(calcSessionStatIntervalSec)
	}
	if group.rtspPubSession != nil {
		group.rtspPubSession.UpdateStat(calcSessionStatIntervalSec)
	}
	if group.psPubSession != nil {
		group.psPubSession.UpdateStat(calcSessionStatIntervalSec)
	}

	group.updatePullSessionStat()

	for session := range group.rtmpSubSessionSet {
		session.UpdateStat(calcSessionStatIntervalSec)
	}
	for session := range group.httpflvSubSessionSet {
		session.UpdateStat(calcSessionStatIntervalSec)
	}
	for session := range group.httptsSubSessionSet {
		session.UpdateStat(calcSessionStatIntervalSec)
	}
	for session := range group.rtspSubSessionSet {
		session.UpdateStat(calcSessionStatIntervalSec)
	}

	for _, item := range group.url2PushProxy {
		session := item.pushSession
		if item.isPushing && session != nil {
			session.UpdateStat(calcSessionStatIntervalSec)
		}
	}
}

func (group *Group) hasPubSession() bool {
	return group.rtmpPubSession != nil || group.rtspPubSession != nil || group.customizePubSession != nil ||
		group.psPubSession != nil
}

func (group *Group) hasSubSession() bool {
	return len(group.rtmpSubSessionSet) != 0 ||
		len(group.httpflvSubSessionSet) != 0 ||
		len(group.httptsSubSessionSet) != 0 ||
		len(group.rtspSubSessionSet) != 0 ||
		len(group.hlsSubSessionSet) != 0 ||
		group.customizeHookSessionContext != nil
}

func (group *Group) hasPushSession() bool {
	for _, item := range group.url2PushProxy {
		if item.isPushing && item.pushSession != nil {
			return true
		}
	}
	return false
}

func (group *Group) hasInSession() bool {
	return group.hasPubSession() || group.hasPullSession()
}

// hasOutSession 是否还有out往外发送音视频数据的session
func (group *Group) hasOutSession() bool {
	return group.hasSubSession() || group.hasPushSession()
}

// isTotalEmpty 当前group是否完全没有流了
func (group *Group) isTotalEmpty() bool {
	return !group.hasInSession() && !group.hasOutSession()
}

func (group *Group) inSessionUniqueKey() string {
	if group.rtmpPubSession != nil {
		return group.rtmpPubSession.UniqueKey()
	}
	if group.rtspPubSession != nil {
		return group.rtspPubSession.UniqueKey()
	}
	if group.psPubSession != nil {
		return group.psPubSession.UniqueKey()
	}
	return group.pullSessionUniqueKey()
}

func (group *Group) shouldStartRtspRemuxer() bool {
	return group.config.RtspConfig.Enable || group.config.RtspConfig.RtspsEnable
}

func (group *Group) shouldStartMpegtsRemuxer() bool {
	return (group.config.HlsConfig.Enable || group.config.HlsConfig.EnableHttps) ||
		(group.config.HttptsConfig.Enable || group.config.HttptsConfig.EnableHttps) ||
		group.config.RecordConfig.EnableMpegts
}

func (group *Group) OnHlsMakeTs(info base.HlsMakeTsInfo) {
	group.observer.OnHlsMakeTs(info)
}
