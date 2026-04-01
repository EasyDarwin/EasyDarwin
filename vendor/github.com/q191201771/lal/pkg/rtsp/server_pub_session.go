// Copyright 2020, Chef.  All rights reserved.
// https://github.com/q191201771/lal
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

package rtsp

import (
	"github.com/q191201771/naza/pkg/nazaerrors"
	"github.com/q191201771/naza/pkg/nazanet"

	"github.com/q191201771/lal/pkg/base"
	"github.com/q191201771/lal/pkg/sdp"
)

type IPubSessionObserver interface {
	IBaseInSessionObserver
}

type PubSession struct {
	urlCtx        base.UrlContext
	cmdSession    *ServerCommandSession
	baseInSession *BaseInSession

	observer IPubSessionObserver
}

func NewPubSession(urlCtx base.UrlContext, cmdSession *ServerCommandSession) *PubSession {
	s := &PubSession{
		urlCtx:     urlCtx,
		cmdSession: cmdSession,
	}
	baseInSession := NewBaseInSession(base.SessionTypeRtspPub, s)
	s.baseInSession = baseInSession
	Log.Infof("[%s] lifecycle new rtsp PubSession. session=%p, streamName=%s", s.UniqueKey(), s, urlCtx.LastItemOfPath)
	return s
}

func (session *PubSession) InitWithSdp(sdpCtx sdp.LogicContext) {
	session.baseInSession.InitWithSdp(sdpCtx)
}

func (session *PubSession) SetObserver(observer IPubSessionObserver) {
	session.baseInSession.SetObserver(observer)
}

func (session *PubSession) SetupWithConn(uri string, rtpConn, rtcpConn *nazanet.UdpConnection) error {
	return session.baseInSession.SetupWithConn(uri, rtpConn, rtcpConn)
}

func (session *PubSession) SetupWithChannel(uri string, rtpChannel, rtcpChannel int) error {
	return session.baseInSession.SetupWithChannel(uri, rtpChannel, rtcpChannel)
}

func (session *PubSession) Dispose() error {
	Log.Infof("[%s] lifecycle dispose rtsp PubSession. session=%p", session.UniqueKey(), session)
	e1 := session.cmdSession.Dispose()
	e2 := session.baseInSession.Dispose()
	return nazaerrors.CombineErrors(e1, e2)
}

func (session *PubSession) GetSdp() sdp.LogicContext {
	return session.baseInSession.GetSdp()
}

func (session *PubSession) HandleInterleavedPacket(b []byte, channel int) {
	session.baseInSession.HandleInterleavedPacket(b, channel)
}

func (session *PubSession) Url() string {
	return session.urlCtx.Url
}

func (session *PubSession) AppName() string {
	return session.urlCtx.PathWithoutLastItem
}

func (session *PubSession) StreamName() string {
	return session.urlCtx.LastItemOfPath
}

func (session *PubSession) RawQuery() string {
	return session.urlCtx.RawQuery
}

func (session *PubSession) UniqueKey() string {
	return session.baseInSession.UniqueKey()
}

// ----- ISessionStat --------------------------------------------------------------------------------------------------

func (session *PubSession) GetStat() base.StatSession {
	stat := session.baseInSession.GetStat()
	stat.RemoteAddr = session.cmdSession.RemoteAddr()
	return stat
}

func (session *PubSession) UpdateStat(intervalSec uint32) {
	session.baseInSession.UpdateStat(intervalSec)
}

func (session *PubSession) IsAlive() (readAlive, writeAlive bool) {
	return session.baseInSession.IsAlive()
}

// ---------------------------------------------------------------------------------------------------------------------

// WriteInterleavedPacket IInterleavedPacketWriter, callback by BaseInSession
func (session *PubSession) WriteInterleavedPacket(packet []byte, channel int) error {
	return session.cmdSession.WriteInterleavedPacket(packet, channel)
}
