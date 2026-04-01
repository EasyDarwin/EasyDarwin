// Copyright 2022, Chef.  All rights reserved.
// https://github.com/q191201771/lal
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

package logic

import (
	"github.com/q191201771/lal/pkg/base"
	"github.com/q191201771/naza/pkg/bininfo"
	"math"
)

// server_manager__api.go
//
// 支持http-api功能的部分
//

func (sm *ServerManager) StatLalInfo() base.LalInfo {
	var lalInfo base.LalInfo
	lalInfo.BinInfo = bininfo.StringifySingleLine()
	lalInfo.LalVersion = base.LalVersion
	lalInfo.ApiVersion = base.HttpApiVersion
	lalInfo.NotifyVersion = base.HttpNotifyVersion
	lalInfo.WebUiVersion = base.HttpWebUiVersion
	lalInfo.StartTime = sm.serverStartTime
	lalInfo.ServerId = sm.config.ServerId
	return lalInfo
}

func (sm *ServerManager) StatAllGroup() (sgs []base.StatGroup) {
	sm.mutex.Lock()
	defer sm.mutex.Unlock()
	sm.groupManager.Iterate(func(group *Group) bool {
		sgs = append(sgs, group.GetStat(math.MaxInt32))
		return true
	})
	return
}

func (sm *ServerManager) StatGroup(streamName string) *base.StatGroup {
	sm.mutex.Lock()
	defer sm.mutex.Unlock()
	g := sm.getGroup("", streamName)
	if g == nil {
		return nil
	}
	// copy
	var ret base.StatGroup
	ret = g.GetStat(math.MaxInt32)
	return &ret
}

func (sm *ServerManager) CtrlStartRelayPull(info base.ApiCtrlStartRelayPullReq) (ret base.ApiCtrlStartRelayPullResp) {
	sm.mutex.Lock()
	defer sm.mutex.Unlock()

	streamName := info.StreamName
	if streamName == "" {
		ctx, err := base.ParseUrl(info.Url, -1)
		if err != nil {
			ret.ErrorCode = base.ErrorCodeStartRelayPullFail
			ret.Desp = err.Error()
			return
		}
		streamName = ctx.LastItemOfPath
	}

	// 注意，如果group不存在，我们依然relay pull
	g := sm.getOrCreateGroup("", streamName)

	sessionId, err := g.StartPull(info)
	if err != nil {
		ret.ErrorCode = base.ErrorCodeStartRelayPullFail
		ret.Desp = err.Error()
	} else {
		ret.ErrorCode = base.ErrorCodeSucc
		ret.Desp = base.DespSucc
		ret.Data.StreamName = streamName
		ret.Data.SessionId = sessionId
	}
	return
}

// CtrlStopRelayPull
//
// TODO(chef): 整理错误值
func (sm *ServerManager) CtrlStopRelayPull(streamName string) (ret base.ApiCtrlStopRelayPullResp) {
	sm.mutex.Lock()
	defer sm.mutex.Unlock()

	g := sm.getGroup("", streamName)
	if g == nil {
		ret.ErrorCode = base.ErrorCodeGroupNotFound
		ret.Desp = base.DespGroupNotFound
		return
	}

	ret.Data.SessionId = g.StopPull()
	if ret.Data.SessionId == "" {
		ret.ErrorCode = base.ErrorCodeSessionNotFound
		ret.Desp = base.DespSessionNotFound
		return
	}

	ret.ErrorCode = base.ErrorCodeSucc
	ret.Desp = base.DespSucc
	return
}

// CtrlKickSession
//
// TODO(chef): refactor 不要返回http结果，返回error吧
func (sm *ServerManager) CtrlKickSession(info base.ApiCtrlKickSessionReq) (ret base.ApiCtrlKickSessionResp) {
	sm.mutex.Lock()
	defer sm.mutex.Unlock()
	g := sm.getGroup("", info.StreamName)
	if g == nil {
		ret.ErrorCode = base.ErrorCodeGroupNotFound
		ret.Desp = base.DespGroupNotFound
		return
	}

	if !g.KickSession(info.SessionId) {
		ret.ErrorCode = base.ErrorCodeSessionNotFound
		ret.Desp = base.DespSessionNotFound
		return
	}

	ret.ErrorCode = base.ErrorCodeSucc
	ret.Desp = base.DespSucc
	return
}

func (sm *ServerManager) CtrlStartRtpPub(info base.ApiCtrlStartRtpPubReq) (ret base.ApiCtrlStartRtpPubResp) {
	sm.mutex.Lock()
	defer sm.mutex.Unlock()

	// 注意，如果group不存在，我们依然relay pull
	g := sm.getOrCreateGroup("", info.StreamName)
	ret = g.StartRtpPub(info)

	return
}
