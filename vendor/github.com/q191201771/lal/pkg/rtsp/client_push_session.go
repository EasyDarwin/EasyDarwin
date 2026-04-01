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
	"github.com/q191201771/lal/pkg/rtprtcp"
	"github.com/q191201771/lal/pkg/sdp"
	"github.com/q191201771/naza/pkg/nazaerrors"
	"github.com/q191201771/naza/pkg/nazanet"
)

type PushSessionOption struct {
	PushTimeoutMs int
	OverTcp       bool
}

var defaultPushSessionOption = PushSessionOption{
	PushTimeoutMs: 10000,
	OverTcp:       false,
}

type PushSession struct {
	cmdSession     *ClientCommandSession
	baseOutSession *BaseOutSession

	disposeOnce sync.Once
	waitChan    chan error
}

type ModPushSessionOption func(option *PushSessionOption)

func NewPushSession(modOptions ...ModPushSessionOption) *PushSession {
	option := defaultPushSessionOption
	for _, fn := range modOptions {
		fn(&option)
	}

	s := &PushSession{
		waitChan: make(chan error, 1),
	}
	baseOutSession := NewBaseOutSession(base.SessionTypeRtspPush, s)
	cmdSession := NewClientCommandSession(CcstPushSession, baseOutSession.UniqueKey(), s, func(opt *ClientCommandSessionOption) {
		opt.DoTimeoutMs = option.PushTimeoutMs
		opt.OverTcp = option.OverTcp
	})
	s.cmdSession = cmdSession
	s.baseOutSession = baseOutSession
	Log.Infof("[%s] lifecycle new rtsp PushSession. session=%p", baseOutSession.UniqueKey(), s)
	return s
}

// Push 阻塞直到和对端完成推流前，握手部分的工作（也即收到RTSP Record response），或者发生错误
func (session *PushSession) Push(rawUrl string, sdpCtx sdp.LogicContext) error {
	Log.Debugf("[%s] push. url=%s", session.UniqueKey(), rawUrl)
	session.cmdSession.InitWithSdp(sdpCtx)
	session.baseOutSession.InitWithSdp(sdpCtx)
	if err := session.cmdSession.Do(rawUrl); err != nil {
		return err
	}

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
					_ = session.baseOutSession.Dispose()
				}
				if cmdSessionDisposed {
					Log.Errorf("[%s] cmd session disposed already.", session.UniqueKey())
				}
				cmdSessionDisposed = true
			case err = <-session.baseOutSession.WaitChan():
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

func (session *PushSession) WriteRtpPacket(packet rtprtcp.RtpPacket) error {
	return session.baseOutSession.WriteRtpPacket(packet)
}

// ---------------------------------------------------------------------------------------------------------------------
// IClientSessionLifecycle interface
// ---------------------------------------------------------------------------------------------------------------------

// Dispose 文档请参考： IClientSessionLifecycle interface
func (session *PushSession) Dispose() error {
	return session.dispose(nil)
}

// WaitChan 文档请参考： IClientSessionLifecycle interface
func (session *PushSession) WaitChan() <-chan error {
	return session.waitChan
}

// ---------------------------------------------------------------------------------------------------------------------

// Url 文档请参考： interface ISessionUrlContext
func (session *PushSession) Url() string {
	return session.cmdSession.Url()
}

// AppName 文档请参考： interface ISessionUrlContext
func (session *PushSession) AppName() string {
	return session.cmdSession.AppName()
}

// StreamName 文档请参考： interface ISessionUrlContext
func (session *PushSession) StreamName() string {
	return session.cmdSession.StreamName()
}

// RawQuery 文档请参考： interface ISessionUrlContext
func (session *PushSession) RawQuery() string {
	return session.cmdSession.RawQuery()
}

// UniqueKey 文档请参考： interface IObject
func (session *PushSession) UniqueKey() string {
	return session.baseOutSession.UniqueKey()
}

// ----- ISessionStat --------------------------------------------------------------------------------------------------

// GetStat 文档请参考： interface ISessionStat
func (session *PushSession) GetStat() base.StatSession {
	stat := session.baseOutSession.GetStat()
	stat.RemoteAddr = session.cmdSession.RemoteAddr()
	return stat
}

// UpdateStat 文档请参考： interface ISessionStat
func (session *PushSession) UpdateStat(intervalSec uint32) {
	session.baseOutSession.UpdateStat(intervalSec)
}

// IsAlive 文档请参考： interface ISessionStat
func (session *PushSession) IsAlive() (readAlive, writeAlive bool) {
	return session.baseOutSession.IsAlive()
}

// ---------------------------------------------------------------------------------------------------------------------

// OnConnectResult IClientCommandSessionObserver, callback by ClientCommandSession
func (session *PushSession) OnConnectResult() {
	// noop
}

// OnDescribeResponse IClientCommandSessionObserver, callback by ClientCommandSession
func (session *PushSession) OnDescribeResponse(sdpCtx sdp.LogicContext) {
	// noop
}

// OnSetupWithConn IClientCommandSessionObserver, callback by ClientCommandSession
func (session *PushSession) OnSetupWithConn(uri string, rtpConn, rtcpConn *nazanet.UdpConnection) {
	_ = session.baseOutSession.SetupWithConn(uri, rtpConn, rtcpConn)
}

// OnSetupWithChannel IClientCommandSessionObserver, callback by ClientCommandSession
func (session *PushSession) OnSetupWithChannel(uri string, rtpChannel, rtcpChannel int) {
	_ = session.baseOutSession.SetupWithChannel(uri, rtpChannel, rtcpChannel)
}

// OnSetupResult IClientCommandSessionObserver, callback by ClientCommandSession
func (session *PushSession) OnSetupResult() {
	// noop
}

// OnInterleavedPacket IClientCommandSessionObserver, callback by ClientCommandSession
func (session *PushSession) OnInterleavedPacket(packet []byte, channel int) {
	session.baseOutSession.HandleInterleavedPacket(packet, channel)
}

// WriteInterleavedPacket IInterleavedPacketWriter, callback by BaseOutSession
func (session *PushSession) WriteInterleavedPacket(packet []byte, channel int) error {
	return session.cmdSession.WriteInterleavedPacket(packet, channel)
}

func (session *PushSession) dispose(err error) error {
	var retErr error
	session.disposeOnce.Do(func() {
		Log.Infof("[%s] lifecycle dispose rtsp PushSession. session=%p", session.UniqueKey(), session)
		e1 := session.cmdSession.Dispose()
		e2 := session.baseOutSession.Dispose()
		retErr = nazaerrors.CombineErrors(e1, e2)
	})
	return retErr
}
