// Copyright 2020, Chef.  All rights reserved.
// https://github.com/q191201771/lal
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

package rtsp

import (
	"sync"

	"github.com/q191201771/lal/pkg/base"
	"github.com/q191201771/lal/pkg/sdp"
	"github.com/q191201771/naza/pkg/nazaerrors"
	"github.com/q191201771/naza/pkg/nazanet"
)

type IPullSessionObserver interface {
	IBaseInSessionObserver
}

type PullSessionOption struct {
	// 从调用Pull函数，到接收音视频数据的前一步，也即收到rtsp play response的超时时间
	// 如果为0，则没有超时时间
	PullTimeoutMs int

	OverTcp bool // 是否使用interleaved模式，也即是否通过rtsp command tcp连接传输rtp/rtcp数据
}

var defaultPullSessionOption = PullSessionOption{
	PullTimeoutMs: 10000,
	OverTcp:       false,
}

type PullSession struct {
	onDescribeResponse func()

	cmdSession    *ClientCommandSession
	baseInSession *BaseInSession

	disposeOnce sync.Once
	waitChan    chan error
}

type ModPullSessionOption func(option *PullSessionOption)

func NewPullSession(observer IPullSessionObserver, modOptions ...ModPullSessionOption) *PullSession {
	// TODO(chef): refactor 把observer从New中移除到With的函数中

	option := defaultPullSessionOption
	for _, fn := range modOptions {
		fn(&option)
	}

	s := &PullSession{
		onDescribeResponse: defaultOnDescribeResponse,
		waitChan:           make(chan error, 1),
	}
	baseInSession := NewBaseInSessionWithObserver(base.SessionTypeRtspPull, s, observer)
	cmdSession := NewClientCommandSession(CcstPullSession, baseInSession.UniqueKey(), s, func(opt *ClientCommandSessionOption) {
		opt.DoTimeoutMs = option.PullTimeoutMs
		opt.OverTcp = option.OverTcp
	})
	s.baseInSession = baseInSession
	s.cmdSession = cmdSession
	Log.Infof("[%s] lifecycle new rtsp PullSession. session=%p", baseInSession.UniqueKey(), s)
	return s
}

func (session *PullSession) WithOnDescribeResponse(onDescribeResponse func()) *PullSession {
	session.onDescribeResponse = onDescribeResponse
	return session
}

// Pull 阻塞直到和对端完成拉流前，握手部分的工作（也即收到RTSP Play response），或者发生错误
func (session *PullSession) Pull(rawUrl string) error {
	Log.Debugf("[%s] pull. url=%s", session.UniqueKey(), rawUrl)
	if err := session.cmdSession.Do(rawUrl); err != nil {
		return err
	}

	// 管理内部的多个资源，确保:
	// 1. 一个资源销毁后，其他资源也被销毁
	// 2. 所有资源都销毁后才通知上层
	go func() {
		var cmdSessionDisposed, baseInSessionDisposed bool
		var retErr error
		var retErrFlag bool
	LOOP:
		for {
			var err error
			select {
			case err = <-session.cmdSession.WaitChan():
				if err != nil {
					_ = session.baseInSession.Dispose()
				}
				if cmdSessionDisposed {
					Log.Errorf("[%s] cmd session disposed already.", session.UniqueKey())
				}
				cmdSessionDisposed = true
			case err = <-session.baseInSession.WaitChan():
				// err是nil时，表示是被PullSession::Dispose主动销毁，那么cmdSession也会被销毁，就不需要我们再调用cmdSession.Dispose了
				if err != nil {
					_ = session.cmdSession.Dispose()
				}
				if baseInSessionDisposed {
					Log.Errorf("[%s] base in session disposed already.", session.UniqueKey())
				}
				baseInSessionDisposed = true
			} // select loop

			// 第一个错误作为返回值
			if !retErrFlag {
				retErr = err
				retErrFlag = true
			}
			if cmdSessionDisposed && baseInSessionDisposed {
				break LOOP
			}
		} // for loop

		session.waitChan <- retErr
	}()

	return nil
}

func (session *PullSession) GetSdp() sdp.LogicContext {
	return session.baseInSession.GetSdp()
}

// ---------------------------------------------------------------------------------------------------------------------
// IClientSessionLifecycle interface
// ---------------------------------------------------------------------------------------------------------------------

// Dispose 文档请参考： IClientSessionLifecycle interface
func (session *PullSession) Dispose() error {
	return session.dispose(nil)
}

// WaitChan 文档请参考： IClientSessionLifecycle interface
func (session *PullSession) WaitChan() <-chan error {
	return session.waitChan
}

// ---------------------------------------------------------------------------------------------------------------------

// Url 文档请参考： interface ISessionUrlContext
func (session *PullSession) Url() string {
	return session.cmdSession.Url()
}

// AppName 文档请参考： interface ISessionUrlContext
func (session *PullSession) AppName() string {
	return session.cmdSession.AppName()
}

// StreamName 文档请参考： interface ISessionUrlContext
func (session *PullSession) StreamName() string {
	return session.cmdSession.StreamName()
}

// RawQuery 文档请参考： interface ISessionUrlContext
func (session *PullSession) RawQuery() string {
	return session.cmdSession.RawQuery()
}

// UniqueKey 文档请参考： interface IObject
func (session *PullSession) UniqueKey() string {
	return session.baseInSession.UniqueKey()
}

// ----- ISessionStat --------------------------------------------------------------------------------------------------

// GetStat 文档请参考： interface ISessionStat
func (session *PullSession) GetStat() base.StatSession {
	stat := session.baseInSession.GetStat()
	stat.RemoteAddr = session.cmdSession.RemoteAddr()
	return stat
}

// UpdateStat 文档请参考： interface ISessionStat
func (session *PullSession) UpdateStat(intervalSec uint32) {
	session.baseInSession.UpdateStat(intervalSec)
}

// IsAlive 文档请参考： interface ISessionStat
func (session *PullSession) IsAlive() (readAlive, writeAlive bool) {
	return session.baseInSession.IsAlive()
}

// ---------------------------------------------------------------------------------------------------------------------

// OnConnectResult IClientCommandSessionObserver, callback by ClientCommandSession
func (session *PullSession) OnConnectResult() {
	// noop
}

// OnDescribeResponse IClientCommandSessionObserver, callback by ClientCommandSession
func (session *PullSession) OnDescribeResponse(sdpCtx sdp.LogicContext) {
	session.onDescribeResponse()
	session.baseInSession.InitWithSdp(sdpCtx)
}

// OnSetupWithConn IClientCommandSessionObserver, callback by ClientCommandSession
func (session *PullSession) OnSetupWithConn(uri string, rtpConn, rtcpConn *nazanet.UdpConnection) {
	_ = session.baseInSession.SetupWithConn(uri, rtpConn, rtcpConn)
}

// OnSetupWithChannel IClientCommandSessionObserver, callback by ClientCommandSession
func (session *PullSession) OnSetupWithChannel(uri string, rtpChannel, rtcpChannel int) {
	_ = session.baseInSession.SetupWithChannel(uri, rtpChannel, rtcpChannel)
}

// OnSetupResult IClientCommandSessionObserver, callback by ClientCommandSession
func (session *PullSession) OnSetupResult() {
	session.baseInSession.WriteRtpRtcpDummy()
}

// OnInterleavedPacket IClientCommandSessionObserver, callback by ClientCommandSession
func (session *PullSession) OnInterleavedPacket(packet []byte, channel int) {
	session.baseInSession.HandleInterleavedPacket(packet, channel)
}

// WriteInterleavedPacket IInterleavedPacketWriter, callback by BaseInSession
func (session *PullSession) WriteInterleavedPacket(packet []byte, channel int) error {
	return session.cmdSession.WriteInterleavedPacket(packet, channel)
}

func (session *PullSession) dispose(err error) error {
	var retErr error
	session.disposeOnce.Do(func() {
		Log.Infof("[%s] lifecycle dispose rtsp PullSession. session=%p", session.UniqueKey(), session)
		e1 := session.cmdSession.Dispose()
		e2 := session.baseInSession.Dispose()
		retErr = nazaerrors.CombineErrors(e1, e2)
	})
	return retErr
}

func defaultOnDescribeResponse() {

}
