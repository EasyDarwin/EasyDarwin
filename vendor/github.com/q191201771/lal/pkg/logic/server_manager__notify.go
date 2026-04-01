// Copyright 2023, Chef.  All rights reserved.
// https://github.com/q191201771/lal
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

package logic

import (
	"github.com/q191201771/lal/pkg/base"
	"github.com/q191201771/naza/pkg/taskpool"
)

// server_manager__notify.go
//
// NotifyHandler部分。
//
// 注意，所有Notify回调都异步化在单独的协程中执行。
// 使得外部的NotifyHandler实现不会影响到内部逻辑的执行。比如避免回调在锁内触发，回调中调用API又再次拿锁的情况。
//

func (sm *ServerManager) nhInitNotifyHandler() {
	// TODO(chef): [opt] 这里已经做了异步化处理，http notify那边的异步可以去掉 202304

	// 如果外部没有传入，则使用默认的http notify handler
	if sm.option.NotifyHandler == nil {
		sm.option.NotifyHandler = NewHttpNotify(sm.config.HttpNotifyConfig, sm.config.ServerId)
	}

	sm.notifyHandlerThread, _ = taskpool.NewPool(func(option *taskpool.Option) {
		option.InitWorkerNum = 1
		option.MaxWorkerNum = 1
	})
}

func (sm *ServerManager) nhOnServerStart(info base.LalInfo) {
	sm.notifyHandlerThread.Go(func(param ...interface{}) {
		p := param[0].(base.LalInfo)
		sm.option.NotifyHandler.OnServerStart(p)
	}, info)
}

func (sm *ServerManager) nhOnUpdate(info base.UpdateInfo) {
	sm.notifyHandlerThread.Go(func(param ...interface{}) {
		p := param[0].(base.UpdateInfo)
		sm.option.NotifyHandler.OnUpdate(p)
	}, info)
}

func (sm *ServerManager) nhOnPubStart(info base.PubStartInfo) {
	sm.notifyHandlerThread.Go(func(param ...interface{}) {
		p := param[0].(base.PubStartInfo)
		sm.option.NotifyHandler.OnPubStart(p)
	}, info)
}

func (sm *ServerManager) nhOnPubStop(info base.PubStopInfo) {
	sm.notifyHandlerThread.Go(func(param ...interface{}) {
		p := param[0].(base.PubStopInfo)
		sm.option.NotifyHandler.OnPubStop(p)
	}, info)
}

func (sm *ServerManager) nhOnSubStart(info base.SubStartInfo) {
	sm.notifyHandlerThread.Go(func(param ...interface{}) {
		p := param[0].(base.SubStartInfo)
		sm.option.NotifyHandler.OnSubStart(p)
	}, info)
}

func (sm *ServerManager) nhOnSubStop(info base.SubStopInfo) {
	sm.notifyHandlerThread.Go(func(param ...interface{}) {
		p := param[0].(base.SubStopInfo)
		sm.option.NotifyHandler.OnSubStop(p)
	}, info)
}

func (sm *ServerManager) nhOnRelayPullStart(info base.PullStartInfo) {
	sm.notifyHandlerThread.Go(func(param ...interface{}) {
		p := param[0].(base.PullStartInfo)
		sm.option.NotifyHandler.OnRelayPullStart(p)
	}, info)
}

func (sm *ServerManager) nhOnRelayPullStop(info base.PullStopInfo) {
	sm.notifyHandlerThread.Go(func(param ...interface{}) {
		p := param[0].(base.PullStopInfo)
		sm.option.NotifyHandler.OnRelayPullStop(p)
	}, info)
}

func (sm *ServerManager) nhOnRtmpConnect(info base.RtmpConnectInfo) {
	sm.notifyHandlerThread.Go(func(param ...interface{}) {
		p := param[0].(base.RtmpConnectInfo)
		sm.option.NotifyHandler.OnRtmpConnect(p)
	}, info)
}

func (sm *ServerManager) nhOnHlsMakeTs(info base.HlsMakeTsInfo) {
	sm.notifyHandlerThread.Go(func(param ...interface{}) {
		p := param[0].(base.HlsMakeTsInfo)
		sm.option.NotifyHandler.OnHlsMakeTs(p)
	}, info)
}
