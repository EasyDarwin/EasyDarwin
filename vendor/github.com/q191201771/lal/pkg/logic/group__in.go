// Copyright 2022, Chef.  All rights reserved.
// https://github.com/q191201771/lal
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

package logic

import (
	"github.com/q191201771/lal/pkg/gb28181"
	"github.com/q191201771/naza/pkg/nazalog"
	"time"

	"github.com/q191201771/lal/pkg/base"
	"github.com/q191201771/lal/pkg/remux"
	"github.com/q191201771/lal/pkg/rtmp"
	"github.com/q191201771/lal/pkg/rtsp"
)

func (group *Group) AddCustomizePubSession(streamName string) (ICustomizePubSessionContext, error) {
	group.mutex.Lock()
	defer group.mutex.Unlock()

	if group.hasInSession() {
		Log.Errorf("[%s] in stream already exist at group. add customize pub session, exist=%s",
			group.UniqueKey, group.inSessionUniqueKey())
		return nil, base.ErrDupInStream
	}

	group.customizePubSession = NewCustomizePubSessionContext(streamName)
	Log.Debugf("[%s] [%s] add customize pub session into group.", group.UniqueKey, group.customizePubSession.UniqueKey())

	group.addIn()

	if group.shouldStartRtspRemuxer() {
		group.rtmp2RtspRemuxer = remux.NewRtmp2RtspRemuxer(
			group.onSdpFromRemux,
			group.onRtpPacketFromRemux,
		)
	}

	group.customizePubSession.WithOnRtmpMsg(group.OnReadRtmpAvMsg)

	return group.customizePubSession, nil
}

func (group *Group) AddRtmpPubSession(session *rtmp.ServerSession) error {
	group.mutex.Lock()
	defer group.mutex.Unlock()

	if group.hasInSession() {
		Log.Errorf("[%s] in stream already exist at group. add=%s, exist=%s",
			group.UniqueKey, session.UniqueKey(), group.inSessionUniqueKey())
		return base.ErrDupInStream
	}

	Log.Debugf("[%s] [%s] add rtmp pub session into group.", group.UniqueKey, session.UniqueKey())

	group.rtmpPubSession = session
	group.addIn()

	if group.shouldStartRtspRemuxer() {
		group.rtmp2RtspRemuxer = remux.NewRtmp2RtspRemuxer(
			group.onSdpFromRemux,
			group.onRtpPacketFromRemux,
		)
	}

	session.SetPubSessionObserver(group)

	return nil
}

// AddRtspPubSession TODO chef: rtsp package中，增加回调返回值判断，如果是false，将连接关掉
func (group *Group) AddRtspPubSession(session *rtsp.PubSession) error {
	group.mutex.Lock()
	defer group.mutex.Unlock()

	if group.hasInSession() {
		Log.Errorf("[%s] in stream already exist at group. wanna add=%s", group.UniqueKey, session.UniqueKey())
		return base.ErrDupInStream
	}

	Log.Debugf("[%s] [%s] add RTSP PubSession into group.", group.UniqueKey, session.UniqueKey())

	group.rtspPubSession = session
	group.addIn()

	group.rtsp2RtmpRemuxer = remux.NewAvPacket2RtmpRemuxer().WithOnRtmpMsg(group.onRtmpMsgFromRemux)
	session.SetObserver(group)

	return nil
}

func (group *Group) StartRtpPub(req base.ApiCtrlStartRtpPubReq) (ret base.ApiCtrlStartRtpPubResp) {
	group.mutex.Lock()
	defer group.mutex.Unlock()

	if group.hasInSession() {
		// TODO(chef): [fix] 处理已经有输入session的情况 202207
	}

	if req.DebugDumpPacket != "" {
		group.psPubDumpFile = base.NewDumpFile()
		if err := group.psPubDumpFile.OpenToWrite(req.DebugDumpPacket); err != nil {
			Log.Errorf("%+v", err)
		}
	}

	pubSession := gb28181.NewPubSession().WithStreamName(req.StreamName).WithOnAvPacket(group.OnAvPacketFromPsPubSession)
	pubSession.WithHookReadPacket(func(b []byte) {
		if group.psPubDumpFile != nil {
			group.psPubDumpFile.WriteWithType(b, base.DumpTypePsRtpData)
		}
	})

	Log.Debugf("[%s] [%s] add RTP PubSession into group.", group.UniqueKey, pubSession.UniqueKey())

	group.psPubSession = pubSession
	group.psPubTimeoutSec = uint32(req.TimeoutMs / 1000)
	group.addIn()

	group.rtsp2RtmpRemuxer = remux.NewAvPacket2RtmpRemuxer()
	group.rtsp2RtmpRemuxer.WithOption(func(option *base.AvPacketStreamOption) {
		option.VideoFormat = base.AvPacketStreamVideoFormatAnnexb
		option.AudioFormat = base.AvPacketStreamAudioFormatAdtsAac
	})
	group.rtsp2RtmpRemuxer.WithOnRtmpMsg(group.onRtmpMsgFromRemux)

	if group.shouldStartRtspRemuxer() {
		group.rtmp2RtspRemuxer = remux.NewRtmp2RtspRemuxer(
			group.onSdpFromRemux,
			group.onRtpPacketFromRemux,
		)
	}

	port, err := pubSession.Listen(req.Port, req.IsTcpFlag != 0)
	if err != nil {
		group.delPsPubSession(pubSession)

		ret.ErrorCode = base.ErrorCodeListenUdpPortFail
		ret.Desp = err.Error()
		return
	}

	go func() {
		runErr := pubSession.RunLoop()
		nazalog.Debugf("[%s] [%s] ps PubSession run loop exit, err=%v", group.UniqueKey, pubSession.UniqueKey(), runErr)
		group.DelPsPubSession(pubSession)
	}()

	ret.ErrorCode = base.ErrorCodeSucc
	ret.Desp = base.DespSucc
	ret.Data.SessionId = pubSession.UniqueKey()
	ret.Data.StreamName = pubSession.StreamName()
	ret.Data.Port = port
	return
}

func (group *Group) AddRtmpPullSession(session *rtmp.PullSession) error {
	group.mutex.Lock()
	defer group.mutex.Unlock()

	if group.hasInSession() {
		Log.Errorf("[%s] in stream already exist. wanna add=%s", group.UniqueKey, session.UniqueKey())
		return base.ErrDupInStream
	}

	Log.Debugf("[%s] [%s] add PullSession into group.", group.UniqueKey, session.UniqueKey())

	group.setRtmpPullSession(session)
	group.addIn()

	if group.shouldStartRtspRemuxer() {
		group.rtmp2RtspRemuxer = remux.NewRtmp2RtspRemuxer(
			group.onSdpFromRemux,
			group.onRtpPacketFromRemux,
		)
	}

	var info base.PullStartInfo
	info.SessionId = session.UniqueKey()
	info.Url = session.Url()
	info.Protocol = session.GetStat().Protocol
	info.RemoteAddr = session.GetStat().RemoteAddr
	info.AppName = session.AppName()
	info.StreamName = session.StreamName()
	info.UrlParam = session.RawQuery()
	info.HasInSession = group.hasInSession()
	info.HasOutSession = group.hasOutSession()
	group.observer.OnRelayPullStart(info)

	return nil
}

func (group *Group) AddRtspPullSession(session *rtsp.PullSession) error {
	group.mutex.Lock()
	defer group.mutex.Unlock()

	if group.hasInSession() {
		Log.Errorf("[%s] in stream already exist. wanna add=%s", group.UniqueKey, session.UniqueKey())
		return base.ErrDupInStream
	}

	Log.Debugf("[%s] [%s] add PullSession into group.", group.UniqueKey, session.UniqueKey())

	group.setRtspPullSession(session)
	group.addIn()

	group.rtsp2RtmpRemuxer = remux.NewAvPacket2RtmpRemuxer().WithOnRtmpMsg(group.onRtmpMsgFromRemux)

	var info base.PullStartInfo
	info.SessionId = session.UniqueKey()
	info.Url = session.Url()
	info.Protocol = session.GetStat().Protocol
	info.RemoteAddr = session.GetStat().RemoteAddr
	info.AppName = session.AppName()
	info.StreamName = session.StreamName()
	info.UrlParam = session.RawQuery()
	info.HasInSession = group.hasInSession()
	info.HasOutSession = group.hasOutSession()
	group.observer.OnRelayPullStart(info)

	return nil
}

// ---------------------------------------------------------------------------------------------------------------------

func (group *Group) DelPsPubSession(session *gb28181.PubSession) {
	group.mutex.Lock()
	defer group.mutex.Unlock()
	group.delPsPubSession(session)
}

func (group *Group) DelCustomizePubSession(sessionCtx ICustomizePubSessionContext) {
	group.mutex.Lock()
	defer group.mutex.Unlock()
	group.delCustomizePubSession(sessionCtx)
}

func (group *Group) DelRtmpPubSession(session *rtmp.ServerSession) {
	group.mutex.Lock()
	defer group.mutex.Unlock()
	group.delRtmpPubSession(session)
}

func (group *Group) DelRtspPubSession(session *rtsp.PubSession) {
	group.mutex.Lock()
	defer group.mutex.Unlock()
	group.delRtspPubSession(session)
}

func (group *Group) DelRtmpPullSession(session *rtmp.PullSession) {
	group.mutex.Lock()
	defer group.mutex.Unlock()
	group.delPullSession(session)

	var info base.PullStopInfo
	info.SessionId = session.UniqueKey()
	info.Url = session.Url()
	info.Protocol = session.GetStat().Protocol
	info.RemoteAddr = session.GetStat().RemoteAddr
	info.AppName = session.AppName()
	info.StreamName = session.StreamName()
	info.UrlParam = session.RawQuery()
	info.HasInSession = group.hasInSession()
	info.HasOutSession = group.hasOutSession()
	group.observer.OnRelayPullStop(info)
}

func (group *Group) DelRtspPullSession(session *rtsp.PullSession) {
	group.mutex.Lock()
	defer group.mutex.Unlock()
	group.delPullSession(session)

	var info base.PullStopInfo
	info.SessionId = session.UniqueKey()
	info.Url = session.Url()
	info.Protocol = session.GetStat().Protocol
	info.RemoteAddr = session.GetStat().RemoteAddr
	info.AppName = session.AppName()
	info.StreamName = session.StreamName()
	info.UrlParam = session.RawQuery()
	info.HasInSession = group.hasInSession()
	info.HasOutSession = group.hasOutSession()
	group.observer.OnRelayPullStop(info)
}

// ---------------------------------------------------------------------------------------------------------------------

func (group *Group) delPsPubSession(session *gb28181.PubSession) {
	Log.Debugf("[%s] [%s] del ps PubSession from group.", group.UniqueKey, session.UniqueKey())

	if session != group.psPubSession {
		Log.Warnf("[%s] del ps pub session but not match. del session=%s, group session=%p",
			group.UniqueKey, session.UniqueKey(), group.psPubSession)
		return
	}

	group.delIn()
}

func (group *Group) delCustomizePubSession(sessionCtx ICustomizePubSessionContext) {
	Log.Debugf("[%s] [%s] del customize PubSession from group.", group.UniqueKey, sessionCtx.UniqueKey())

	if sessionCtx != group.customizePubSession {
		Log.Warnf("[%s] del customize pub session but not match. del session=%s, group session=%p",
			group.UniqueKey, sessionCtx.UniqueKey(), group.customizePubSession)
		return
	}

	group.delIn()
}

func (group *Group) delRtmpPubSession(session *rtmp.ServerSession) {
	Log.Debugf("[%s] [%s] del rtmp PubSession from group.", group.UniqueKey, session.UniqueKey())

	if session != group.rtmpPubSession {
		Log.Warnf("[%s] del rtmp pub session but not match. del session=%s, group session=%p",
			group.UniqueKey, session.UniqueKey(), group.rtmpPubSession)
		return
	}

	group.delIn()

}

func (group *Group) delRtspPubSession(session *rtsp.PubSession) {
	Log.Debugf("[%s] [%s] del rtsp PubSession from group.", group.UniqueKey, session.UniqueKey())

	if session != group.rtspPubSession {
		Log.Warnf("[%s] del rtmp pub session but not match. del session=%s, group session=%p",
			group.UniqueKey, session.UniqueKey(), group.rtspPubSession)
		return
	}

	group.delIn()
}

func (group *Group) delPullSession(session base.IObject) {
	Log.Debugf("[%s] [%s] del PullSession from group.", group.UniqueKey, session.UniqueKey())

	group.resetRelayPullSession()
	group.delIn()
}

// ---------------------------------------------------------------------------------------------------------------------

// addIn 有pub或pull的输入型session加入时，需要调用该函数
func (group *Group) addIn() {
	now := time.Now().Unix()

	if group.shouldStartMpegtsRemuxer() {
		group.rtmp2MpegtsRemuxer = remux.NewRtmp2MpegtsRemuxer(group)
		nazalog.Debugf("[%s] [%s] NewRtmp2MpegtsRemuxer in group.", group.UniqueKey, group.rtmp2MpegtsRemuxer.UniqueKey())
	}

	if group.config.InSessionConfig.AddDummyAudioEnable {
		group.dummyAudioFilter = remux.NewDummyAudioFilter(group.UniqueKey, group.config.InSessionConfig.AddDummyAudioWaitAudioMs, group.broadcastByRtmpMsg)
	}

	if group.option.onHookSession != nil {
		group.customizeHookSessionContext = group.option.onHookSession(group.inSessionUniqueKey(), group.streamName)
	}

	group.startPushIfNeeded()
	group.startHlsIfNeeded()
	group.startRecordFlvIfNeeded(now)
	group.startRecordMpegtsIfNeeded(now)
}

// delIn 有pub或pull的输入型session离开时，需要调用该函数
func (group *Group) delIn() {
	// 注意，remuxer放前面，使得有机会将内部缓存的数据吐出来
	if group.rtmp2MpegtsRemuxer != nil {
		group.rtmp2MpegtsRemuxer.Dispose()
		group.rtmp2MpegtsRemuxer = nil
	}

	if group.customizeHookSessionContext != nil {
		group.customizeHookSessionContext.OnStop()
		group.customizeHookSessionContext = nil
	}

	group.stopPushIfNeeded()
	group.stopHlsIfNeeded()
	group.stopRecordFlvIfNeeded()
	group.stopRecordMpegtsIfNeeded()

	group.rtmpPubSession = nil
	group.rtspPubSession = nil
	group.customizePubSession = nil
	group.psPubSession = nil
	group.rtsp2RtmpRemuxer = nil
	group.rtmp2RtspRemuxer = nil
	group.dummyAudioFilter = nil

	if group.psPubDumpFile != nil {
		group.psPubDumpFile.Close()
		group.psPubDumpFile = nil
	}
	group.rtmpGopCache.Clear()
	group.httpflvGopCache.Clear()
	group.httptsGopCache.Clear()
	group.sdpCtx = nil
	group.patpmt = nil
}
