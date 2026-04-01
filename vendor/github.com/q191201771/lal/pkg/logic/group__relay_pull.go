// Copyright 2022, Chef.  All rights reserved.
// https://github.com/q191201771/lal
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

package logic

import (
	"errors"
	"fmt"
	"strings"
	"time"

	"github.com/q191201771/lal/pkg/base"
	"github.com/q191201771/lal/pkg/rtsp"
	"github.com/q191201771/naza/pkg/nazalog"

	"github.com/q191201771/lal/pkg/rtmp"
)

// StartPull 外部命令主动触发pull拉流
func (group *Group) StartPull(info base.ApiCtrlStartRelayPullReq) (string, error) {
	group.mutex.Lock()
	defer group.mutex.Unlock()

	group.pullProxy.apiEnable = true
	group.pullProxy.pullUrl = info.Url
	group.pullProxy.pullTimeoutMs = info.PullTimeoutMs
	group.pullProxy.pullRetryNum = info.PullRetryNum
	group.pullProxy.autoStopPullAfterNoOutMs = info.AutoStopPullAfterNoOutMs
	group.pullProxy.rtspMode = info.RtspMode
	group.pullProxy.debugDumpPacket = info.DebugDumpPacket

	return group.pullIfNeeded()
}

// StopPull
//
// @return 如果PullSession存在，返回它的unique key
func (group *Group) StopPull() string {
	group.mutex.Lock()
	defer group.mutex.Unlock()

	group.pullProxy.apiEnable = false
	return group.stopPull()
}

// ---------------------------------------------------------------------------------------------------------------------

type pullProxy struct {
	staticRelayPullEnable    bool // 是否开启pull TODO(chef): refactor 这两个bool可以考虑合并成一个
	apiEnable                bool
	pullUrl                  string
	pullTimeoutMs            int
	pullRetryNum             int
	autoStopPullAfterNoOutMs int // 没有观看者时，是否自动停止pull
	rtspMode                 int
	debugDumpPacket          string

	startCount   int
	lastHasOutTs int64

	isSessionPulling bool // 是否正在pull，注意，这是一个内部状态，表示的是session的状态，而不是整体任务应该处于的状态
	rtmpSession      *rtmp.PullSession
	rtspSession      *rtsp.PullSession
}

// initRelayPullByConfig 根据配置文件中的静态回源配置来初始化回源设置
func (group *Group) initRelayPullByConfig() {
	enable := group.config.StaticRelayPullConfig.Enable
	addr := group.config.StaticRelayPullConfig.Addr
	appName := group.appName
	streamName := group.streamName

	group.pullProxy = &pullProxy{
		startCount:   0,
		lastHasOutTs: time.Now().UnixNano() / 1e6,
	}

	var pullUrl string
	if enable {
		pullUrl = fmt.Sprintf("rtmp://%s/%s/%s", addr, appName, streamName)
	}

	group.pullProxy.pullUrl = pullUrl
	group.pullProxy.staticRelayPullEnable = enable
	group.pullProxy.pullTimeoutMs = StaticRelayPullTimeoutMs
	group.pullProxy.pullRetryNum = staticRelayPullRetryNum
	group.pullProxy.autoStopPullAfterNoOutMs = staticRelayPullAutoStopPullAfterNoOutMs
}

func (group *Group) setRtmpPullSession(session *rtmp.PullSession) {
	group.pullProxy.rtmpSession = session
}

func (group *Group) setRtspPullSession(session *rtsp.PullSession) {
	group.pullProxy.rtspSession = session
	if group.pullProxy.debugDumpPacket != "" {
		group.rtspPullDumpFile = base.NewDumpFile()
		if err := group.rtspPullDumpFile.OpenToWrite(group.pullProxy.debugDumpPacket); err != nil {
			Log.Errorf("%+v", err)
		}
	}
}

func (group *Group) resetRelayPullSession() {
	group.pullProxy.isSessionPulling = false
	group.pullProxy.rtmpSession = nil
	group.pullProxy.rtspSession = nil
	if group.rtspPullDumpFile != nil {
		group.rtspPullDumpFile.Close()
		group.rtspPullDumpFile = nil
	}
}

func (group *Group) getStatPull() base.StatPull {
	if group.pullProxy.rtmpSession != nil {
		return base.Session2StatPull(group.pullProxy.rtmpSession)
	}
	if group.pullProxy.rtspSession != nil {
		return base.Session2StatPull(group.pullProxy.rtspSession)
	}
	return base.StatPull{}
}

func (group *Group) disposeInactivePullSession() {
	if group.pullProxy.rtmpSession != nil {
		if readAlive, _ := group.pullProxy.rtmpSession.IsAlive(); !readAlive {
			Log.Warnf("[%s] session timeout. session=%s", group.UniqueKey, group.pullProxy.rtmpSession.UniqueKey())
			group.pullProxy.rtmpSession.Dispose()
		}
	}
	if group.pullProxy.rtspSession != nil {
		if readAlive, _ := group.pullProxy.rtspSession.IsAlive(); !readAlive {
			Log.Warnf("[%s] session timeout. session=%s", group.UniqueKey, group.pullProxy.rtspSession.UniqueKey())
			group.pullProxy.rtspSession.Dispose()
		}
	}
}

func (group *Group) updatePullSessionStat() {
	if group.pullProxy.rtmpSession != nil {
		group.pullProxy.rtmpSession.UpdateStat(calcSessionStatIntervalSec)
	}
	if group.pullProxy.rtspSession != nil {
		group.pullProxy.rtspSession.UpdateStat(calcSessionStatIntervalSec)
	}
}

func (group *Group) isPullModuleAlive() bool {
	if group.hasPullSession() || group.pullProxy.isSessionPulling {
		return true
	}
	flag, _ := group.shouldStartPull()
	return flag
}

func (group *Group) tickPullModule() {
	if group.hasSubSession() {
		group.pullProxy.lastHasOutTs = time.Now().UnixNano() / 1e6
	}

	if group.shouldAutoStopPull() {
		group.stopPull()
	} else {
		group.pullIfNeeded()
	}
}

func (group *Group) hasPullSession() bool {
	return group.pullProxy.rtmpSession != nil || group.pullProxy.rtspSession != nil
}

func (group *Group) pullSessionUniqueKey() string {
	if group.pullProxy.rtmpSession != nil {
		return group.pullProxy.rtmpSession.UniqueKey()
	}
	if group.pullProxy.rtspSession != nil {
		return group.pullProxy.rtspSession.UniqueKey()
	}
	return ""
}

// kickPull
//
// @return 返回true，表示找到对应的session，并关闭
func (group *Group) kickPull(sessionId string) bool {
	if (group.pullProxy.rtmpSession != nil && group.pullProxy.rtmpSession.UniqueKey() == sessionId) ||
		(group.pullProxy.rtspSession != nil && group.pullProxy.rtspSession.UniqueKey() == sessionId) {
		group.pullProxy.apiEnable = false
		group.stopPull()
		return true
	}
	return false
}

// 判断是否需要pull从远端拉流至本地，如果需要，则触发pull
//
// 当前调用时机：
// 1. 添加新sub session
// 2. 外部命令，比如http api
// 3. 定时器，比如pull的连接断了，通过定时器可以重启触发pull
func (group *Group) pullIfNeeded() (string, error) {
	if flag, err := group.shouldStartPull(); !flag {
		return "", err
	}

	Log.Infof("[%s] start relay pull. url=%s", group.UniqueKey, group.pullProxy.pullUrl)

	group.pullProxy.isSessionPulling = true
	group.pullProxy.startCount++

	isPullByRtmp := strings.HasPrefix(group.pullProxy.pullUrl, "rtmp")

	var rtmpSession *rtmp.PullSession
	var rtspSession *rtsp.PullSession
	var uk string

	if isPullByRtmp {
		rtmpSession = rtmp.NewPullSession(func(option *rtmp.PullSessionOption) {
			option.PullTimeoutMs = group.pullProxy.pullTimeoutMs
		}).WithOnPullSucc(func() {
			err := group.AddRtmpPullSession(rtmpSession)
			if err != nil {
				rtmpSession.Dispose()
				return
			}
		}).WithOnReadRtmpAvMsg(group.OnReadRtmpAvMsg)

		uk = rtmpSession.UniqueKey()
	} else {
		rtspSession = rtsp.NewPullSession(group, func(option *rtsp.PullSessionOption) {
			option.PullTimeoutMs = group.pullProxy.pullTimeoutMs
			option.OverTcp = group.pullProxy.rtspMode == 0
		}).WithOnDescribeResponse(func() {
			err := group.AddRtspPullSession(rtspSession)
			if err != nil {
				rtspSession.Dispose()
				return
			}
		})

		uk = rtspSession.UniqueKey()
	}

	go func(rtPullUrl string, rtIsPullByRtmp bool, rtRtmpSession *rtmp.PullSession, rtRtspSession *rtsp.PullSession) {
		if rtIsPullByRtmp {
			// TODO(chef): 处理数据回调，是否应该等待Add成功之后。避免竞态条件中途加入了其他in session
			err := rtRtmpSession.Pull(rtPullUrl)
			if err != nil {
				Log.Errorf("[%s] relay pull fail. err=%v", rtRtmpSession.UniqueKey(), err)
				group.DelRtmpPullSession(rtRtmpSession)
				return
			}

			err = <-rtRtmpSession.WaitChan()
			Log.Infof("[%s] relay pull done. err=%v", rtRtmpSession.UniqueKey(), err)
			group.DelRtmpPullSession(rtRtmpSession)
			return
		}

		err := rtRtspSession.Pull(rtPullUrl)
		if err != nil {
			Log.Errorf("[%s] relay pull fail. err=%v", rtRtspSession.UniqueKey(), err)
			group.DelRtspPullSession(rtRtspSession)
			return
		}

		err = <-rtRtspSession.WaitChan()
		Log.Infof("[%s] relay pull done. err=%v", rtRtspSession.UniqueKey(), err)
		group.DelRtspPullSession(rtRtspSession)
		return
	}(group.pullProxy.pullUrl, isPullByRtmp, rtmpSession, rtspSession)

	return uk, nil
}

func (group *Group) stopPull() string {
	// 关闭时，清空用于重试的计数
	group.pullProxy.startCount = 0

	if group.pullProxy.rtmpSession != nil {
		Log.Infof("[%s] stop pull session.", group.UniqueKey)
		group.pullProxy.rtmpSession.Dispose()
		return group.pullProxy.rtmpSession.UniqueKey()
	}
	if group.pullProxy.rtspSession != nil {
		Log.Infof("[%s] stop pull session.", group.UniqueKey)
		group.pullProxy.rtspSession.Dispose()
		return group.pullProxy.rtspSession.UniqueKey()
	}
	return ""
}

func (group *Group) shouldStartPull() (bool, error) {
	// 如果本地已经有输入型的流，就不需要pull了
	if group.hasInSession() {
		return false, base.ErrDupInStream
	}

	// 已经在pull中，就不需要pull了
	if group.pullProxy.isSessionPulling {
		return false, base.ErrDupInStream
	}

	if !group.pullProxy.staticRelayPullEnable && !group.pullProxy.apiEnable {
		return false, errors.New("relay pull not enable")
	}

	// 没人观看自动停的逻辑，是否满足并且需要触发
	if group.shouldAutoStopPull() {
		return false, errors.New("should auto stop pull")
	}

	// 检查重试次数
	if group.pullProxy.pullRetryNum >= 0 {
		if group.pullProxy.startCount > group.pullProxy.pullRetryNum {
			return false, errors.New("relay pull retry limited")
		}
	} else {
		// 负数永远都重试
	}

	return true, nil
}

// shouldAutoStopPull 是否需要自动停，根据没人观看停的逻辑
func (group *Group) shouldAutoStopPull() bool {
	// 没开启
	if group.pullProxy.autoStopPullAfterNoOutMs < 0 {
		return false
	}

	// 还有观众
	if group.hasOutSession() {
		return false
	}

	// 没有观众，并且设置为立即关闭
	if group.pullProxy.autoStopPullAfterNoOutMs == 0 {
		return true
	}

	// 是否达到时间阈值
	nazalog.Debugf("%d %d %d", group.pullProxy.lastHasOutTs, time.Now().UnixNano(), group.pullProxy.autoStopPullAfterNoOutMs)
	return group.pullProxy.lastHasOutTs != -1 && time.Now().UnixNano()/1e6-group.pullProxy.lastHasOutTs >= int64(group.pullProxy.autoStopPullAfterNoOutMs)
}
