// Copyright 2020, Chef.  All rights reserved.
// https://github.com/q191201771/lal
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

package rtsp

import (
	"github.com/q191201771/lal/pkg/base"
	"github.com/q191201771/lal/pkg/rtprtcp"
	"github.com/q191201771/lal/pkg/sdp"
	"github.com/q191201771/naza/pkg/nazaatomic"
	"github.com/q191201771/naza/pkg/nazaerrors"
	"github.com/q191201771/naza/pkg/nazanet"
)

type SubSessionStage int

const (
	SubSessionStageReadDescribe int32 = 0 // 初时阶段，已收到 describe
	SubSessionStageWriteSdp           = 1 // 已发送 sdp
	SubSessionStageReadPlay           = 2 // 已收到 play
)

type SubSession struct {
	urlCtx         base.UrlContext
	cmdSession     *ServerCommandSession
	baseOutSession *BaseOutSession

	ShouldWaitVideoKeyFrame bool

	Stage nazaatomic.Int32 // 见 SubSessionStageReadDescribe 等常量定义
}

func NewSubSession(urlCtx base.UrlContext, cmdSession *ServerCommandSession) *SubSession {
	s := &SubSession{
		urlCtx:     urlCtx,
		cmdSession: cmdSession,

		ShouldWaitVideoKeyFrame: true,
	}
	s.Stage.Store(SubSessionStageReadDescribe)
	baseOutSession := NewBaseOutSession(base.SessionTypeRtspSub, s)
	s.baseOutSession = baseOutSession
	Log.Infof("[%s] lifecycle new rtsp SubSession. session=%p, streamName=%s", s.UniqueKey(), s, urlCtx.LastItemOfPath)
	return s
}

// FeedSdp 供上层调用
func (session *SubSession) FeedSdp(sdpCtx sdp.LogicContext) {
	session.Stage.Store(SubSessionStageWriteSdp)
	session.cmdSession.FeedSdp(sdpCtx.RawSdp)
}

// InitWithSdp 供 ServerCommandSession 调用
func (session *SubSession) InitWithSdp(sdpCtx sdp.LogicContext) {
	session.Stage.Store(SubSessionStageWriteSdp)
	session.baseOutSession.InitWithSdp(sdpCtx)
}

func (session *SubSession) SetupWithConn(uri string, rtpConn, rtcpConn *nazanet.UdpConnection) error {
	return session.baseOutSession.SetupWithConn(uri, rtpConn, rtcpConn)
}

func (session *SubSession) SetupWithChannel(uri string, rtpChannel, rtcpChannel int) error {
	return session.baseOutSession.SetupWithChannel(uri, rtpChannel, rtcpChannel)
}

func (session *SubSession) WriteRtpPacket(packet rtprtcp.RtpPacket) {
	stage := session.Stage.Load()
	if stage != SubSessionStageReadPlay {
		//Log.Warnf("[%s] write rtp packet is not as expected, stage is not ready yet.. stage=%d", session.UniqueKey(), stage)
		return
	}
	session.baseOutSession.WriteRtpPacket(packet)
}

func (session *SubSession) Dispose() error {
	Log.Infof("[%s] lifecycle dispose rtsp SubSession. session=%p", session.UniqueKey(), session)
	e1 := session.baseOutSession.Dispose()
	e2 := session.cmdSession.Dispose()
	return nazaerrors.CombineErrors(e1, e2)
}

func (session *SubSession) HandleInterleavedPacket(b []byte, channel int) {
	session.baseOutSession.HandleInterleavedPacket(b, channel)
}

func (session *SubSession) Url() string {
	return session.urlCtx.Url
}

func (session *SubSession) AppName() string {
	return session.urlCtx.PathWithoutLastItem
}

func (session *SubSession) StreamName() string {
	return session.urlCtx.LastItemOfPath
}

func (session *SubSession) RawQuery() string {
	return session.urlCtx.RawQuery
}

func (session *SubSession) UniqueKey() string {
	return session.baseOutSession.UniqueKey()
}

func (session *SubSession) GetStat() base.StatSession {
	stat := session.baseOutSession.GetStat()
	stat.RemoteAddr = session.cmdSession.RemoteAddr()
	return stat
}

func (session *SubSession) UpdateStat(intervalSec uint32) {
	session.baseOutSession.UpdateStat(intervalSec)
}

func (session *SubSession) IsAlive() (readAlive, writeAlive bool) {
	return session.baseOutSession.IsAlive()
}

// WriteInterleavedPacket IInterleavedPacketWriter, callback by BaseOutSession
func (session *SubSession) WriteInterleavedPacket(packet []byte, channel int) error {
	return session.cmdSession.WriteInterleavedPacket(packet, channel)
}
