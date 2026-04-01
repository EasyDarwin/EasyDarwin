// Copyright 2022, Chef.  All rights reserved.
// https://github.com/q191201771/lal
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

package logic

import (
	"net"

	"github.com/q191201771/lal/pkg/rtsp"

	"github.com/q191201771/lal/pkg/rtmp"
	"github.com/q191201771/naza/pkg/nazalog"

	"github.com/q191201771/lal/pkg/mpegts"

	"github.com/q191201771/lal/pkg/avc"
	"github.com/q191201771/lal/pkg/base"
	"github.com/q191201771/lal/pkg/hevc"
	"github.com/q191201771/lal/pkg/remux"
	"github.com/q191201771/lal/pkg/rtprtcp"
	"github.com/q191201771/lal/pkg/sdp"
)

// group__streaming.go
//
// 包含group中音视频数据转发、转封装协议的逻辑
//

// ---------------------------------------------------------------------------------------------------------------------

// OnReadRtmpAvMsg
//
// 输入rtmp数据.
// 来自 rtmp.ServerSession(Pub), rtmp.PullSession, CustomizePubSessionContext(remux.AvPacket2RtmpRemuxer), (remux.DummyAudioFilter) 的回调.
func (group *Group) OnReadRtmpAvMsg(msg base.RtmpMsg) {
	group.mutex.Lock()
	defer group.mutex.Unlock()
	if group.dummyAudioFilter != nil {
		group.dummyAudioFilter.Feed(msg)
	} else {
		group.broadcastByRtmpMsg(msg)
	}
}

// ---------------------------------------------------------------------------------------------------------------------

// OnSdp OnRtpPacket OnAvPacket
//
// 输入rtsp(rtp)和rtp合帧之后的数据.
// 来自 rtsp.PubSession 的回调.
func (group *Group) OnSdp(sdpCtx sdp.LogicContext) {
	group.mutex.Lock()
	defer group.mutex.Unlock()
	group.sdpCtx = &sdpCtx
	group.feedWaitRtspSubSessions()
	if group.rtsp2RtmpRemuxer != nil {
		group.rtsp2RtmpRemuxer.OnSdp(sdpCtx)
	}
	if group.rtspPullDumpFile != nil {
		group.rtspPullDumpFile.WriteWithType(sdpCtx.RawSdp, base.DumpTypeRtspSdpData)
	}
}

// OnRtpPacket ...
func (group *Group) OnRtpPacket(pkt rtprtcp.RtpPacket) {
	group.mutex.Lock()
	defer group.mutex.Unlock()
	group.feedRtpPacket(pkt)
	if group.rtspPullDumpFile != nil {
		group.rtspPullDumpFile.WriteWithType(pkt.Raw, base.DumpTypeRtspRtpData)
	}
}

// OnAvPacket ...
func (group *Group) OnAvPacket(pkt base.AvPacket) {
	group.mutex.Lock()
	defer group.mutex.Unlock()

	// 注意，由于rtsp pub的tcp命令连接和udp接收数据连接是并行的，
	// 可能发生rtsp pub已经回调告知结束，数据依然回调的现象，
	// 出于性能考虑，底层不判断，由上层按需判断
	if group.rtsp2RtmpRemuxer != nil {
		group.rtsp2RtmpRemuxer.OnAvPacket(pkt)
	}
}

// ---------------------------------------------------------------------------------------------------------------------

// OnAvPacketFromPsPubSession
//
// 来自 gb28181.PubSession 的回调.
func (group *Group) OnAvPacketFromPsPubSession(pkt *base.AvPacket) {
	// TODO(chef): [refactor] 统一所有回调，AvPacket和*AvPacket 202208

	group.mutex.Lock()
	defer group.mutex.Unlock()

	//Log.Debugf("Group::OnAvPacketFromPsPubSession. pkt=%s", pkt.DebugString())

	if group.rtsp2RtmpRemuxer != nil {
		group.rtsp2RtmpRemuxer.OnAvPacket(*pkt)
	}
}

// ---------------------------------------------------------------------------------------------------------------------

// OnPatPmt OnTsPackets
//
// 输入mpegts数据.
// 来自 remux.Rtmp2MpegtsRemuxer 的回调.
func (group *Group) OnPatPmt(b []byte) {
	group.patpmt = b

	if group.hlsMuxer != nil {
		group.hlsMuxer.FeedPatPmt(b)
	}

	if group.recordMpegts != nil {
		if err := group.recordMpegts.Write(b); err != nil {
			Log.Errorf("[%s] record mpegts write fragment header error. err=%+v", group.UniqueKey, err)
		}
	}
}

// OnTsPackets ...
func (group *Group) OnTsPackets(tsPackets []byte, frame *mpegts.Frame, boundary bool) {
	group.feedTsPackets(tsPackets, frame, boundary)
}

// ---------------------------------------------------------------------------------------------------------------------

// onRtmpMsgFromRemux
//
// 输入rtmp数据.
// 来自 remux.AvPacket2RtmpRemuxer 的回调.
func (group *Group) onRtmpMsgFromRemux(msg base.RtmpMsg) {
	if group.dummyAudioFilter != nil {
		group.dummyAudioFilter.Feed(msg)
	} else {
		group.broadcastByRtmpMsg(msg)
	}
}

// ---------------------------------------------------------------------------------------------------------------------

// onSdpFromRemux onRtpPacketFromRemux
//
// 输入rtsp(rtp)数据.
// 来自 remux.Rtmp2RtspRemuxer 的回调.
func (group *Group) onSdpFromRemux(sdpCtx sdp.LogicContext) {
	group.sdpCtx = &sdpCtx
	group.feedWaitRtspSubSessions()
}

// onRtpPacketFromRemux ...
func (group *Group) onRtpPacketFromRemux(pkt rtprtcp.RtpPacket) {
	group.feedRtpPacket(pkt)
}

// ---------------------------------------------------------------------------------------------------------------------

// OnFragmentOpen
//
// 来自 hls.Muxer 的回调
func (group *Group) OnFragmentOpen() {
	group.rtmp2MpegtsRemuxer.FlushAudio()
}

// ---------------------------------------------------------------------------------------------------------------------

// broadcastByRtmpMsg
//
// 使用rtmp类型的数据做为输入，广播给各协议的输出
//
// @param msg 调用结束后，内部不持有msg.Payload内存块
func (group *Group) broadcastByRtmpMsg(msg base.RtmpMsg) {
	//Log.Debugf("> broadcastByRtmpMsg. %s", msg.DebugString())

	if msg.Header.MsgLen != uint32(len(msg.Payload)) {
		Log.Errorf("[%s] diff. msgLen=%d, payload len=%d, %+v", group.UniqueKey, msg.Header.MsgLen, len(msg.Payload), msg.Header)
	}

	if msg.Header.MsgTypeId == base.RtmpTypeIdMetadata {
		m, err := rtmp.ParseMetadata(msg.Payload)
		nazalog.Debugf("[%s] metadata. err=%+v, len=%d, value=%s", group.UniqueKey, err, len(m), m.DebugString())
	}

	var (
		lazyRtmpChunkDivider remux.LazyRtmpChunkDivider
		lazyRtmpMsg2FlvTag   remux.LazyRtmpMsg2FlvTag
	)

	// 设置好用于发送的 rtmp 头部信息
	lazyRtmpChunkDivider.Init(msg)
	lazyRtmpMsg2FlvTag.Init(msg)

	// # 数据有效性检查
	if len(msg.Payload) == 0 {
		Log.Warnf("[%s] msg payload length is 0. %+v", group.UniqueKey, msg.Header)
		return
	}

	// TODO(chef): 暂时不打开，因为过滤掉了innertest中rtmp和flv的输出和输入就不完全相同了
	//if msg.Header.MsgTypeId == base.RtmpTypeIdAudio {
	//	if len(msg.Payload) <= 2 {
	//		// 注意，ffmpeg有时会发送这几种空数据，这种情况我们直接返回，不打印日志
	//		if bytes.Equal(msg.Payload, []byte{0xaf, 0x0}) {
	//			// noop
	//			return
	//		}
	//		Log.Errorf("[%s] invalid rtmp audio message. header=%+v, payload=%s",
	//			group.UniqueKey, msg.Header, hex.Dump(msg.Payload))
	//		return
	//	}
	//} else if msg.Header.MsgTypeId == base.RtmpTypeIdVideo {
	//	if len(msg.Payload) <= 5 {
	//		if bytes.Equal(msg.Payload, []byte{0x27, 0x02, 0x0, 0x0, 0x0}) ||
	//			bytes.Equal(msg.Payload, []byte{0x17, 0x02, 0x0, 0x0, 0x0}) {
	//			// noop
	//			return
	//		}
	//		Log.Errorf("[%s] invalid rtmp video message. header=%+v, payload=%s",
	//			group.UniqueKey, msg.Header, hex.Dump(msg.Payload))
	//		return
	//	}
	//}

	// # mpegts remuxer
	if group.rtmp2MpegtsRemuxer != nil {
		group.rtmp2MpegtsRemuxer.FeedRtmpMessage(msg)
	}

	// # rtsp
	if group.rtmp2RtspRemuxer != nil {
		group.rtmp2RtspRemuxer.FeedRtmpMsg(msg)
	}

	if group.customizeHookSessionContext != nil {
		group.customizeHookSessionContext.OnMsg(msg)
	}

	// # 广播。遍历所有 rtmp sub session，转发数据
	// ## 如果是新的 sub session，发送已缓存的信息
	for session := range group.rtmpSubSessionSet {
		if session.IsFresh {
			// TODO chef: 头信息和full gop也可以在SubSession刚加入时发送
			if group.rtmpGopCache.MetadataEnsureWithoutSetDataFrame != nil {
				Log.Debugf("[%s] [%s] write metadata", group.UniqueKey, session.UniqueKey())
				_ = session.Write(group.rtmpGopCache.MetadataEnsureWithoutSetDataFrame)
			}
			if group.rtmpGopCache.VideoSeqHeader != nil {
				Log.Debugf("[%s] [%s] write vsh", group.UniqueKey, session.UniqueKey())
				_ = session.Write(group.rtmpGopCache.VideoSeqHeader)
			}
			if group.rtmpGopCache.AacSeqHeader != nil {
				Log.Debugf("[%s] [%s] write ash", group.UniqueKey, session.UniqueKey())
				_ = session.Write(group.rtmpGopCache.AacSeqHeader)
			}
			gopCount := group.rtmpGopCache.GetGopCount()
			if gopCount > 0 {
				// GOP缓存中肯定包含了关键帧
				session.ShouldWaitVideoKeyFrame = false

				Log.Debugf("[%s] [%s] write gop cache. gop num=%d", group.UniqueKey, session.UniqueKey(), gopCount)
			}
			for i := 0; i < gopCount; i++ {
				for _, item := range group.rtmpGopCache.GetGopDataAt(i) {
					_ = session.Write(item)
				}
			}

			// 有新加入的sub session（本次循环的第一个新加入的sub session），把rtmp buf writer中的缓存数据全部广播发送给老的sub session
			// 从而确保新加入的sub session不会发送这部分脏的数据
			// 注意，此处可能被调用多次，但是只有第一次会实际flush缓存数据
			if group.rtmpMergeWriter != nil {
				group.rtmpMergeWriter.Flush()
			}

			session.IsFresh = false
		}

		if session.ShouldWaitVideoKeyFrame && msg.IsVideoKeyNalu() {
			// 有sub session在等待关键帧，并且当前是关键帧
			// 把rtmp buf writer中的缓存数据全部广播发送给老的sub session
			// 并且修改这个sub session的标志
			// 让rtmp buf writer来发送这个关键帧
			if group.rtmpMergeWriter != nil {
				group.rtmpMergeWriter.Flush()
			}
			session.ShouldWaitVideoKeyFrame = false
		}
	} // for loop iterate rtmpSubSessionSet

	// ## 转发本次数据
	if len(group.rtmpSubSessionSet) > 0 {
		if group.rtmpMergeWriter == nil {
			group.write2RtmpSubSessions(lazyRtmpChunkDivider.GetEnsureWithoutSdf())
		} else {
			group.rtmpMergeWriter.Write(lazyRtmpChunkDivider.GetEnsureWithoutSdf())
		}
	}

	// TODO chef: rtmp sub, rtmp push, httpflv sub 的发送逻辑都差不多，可以考虑封装一下
	if group.pushEnable {
		for _, v := range group.url2PushProxy {
			if v.pushSession == nil {
				continue
			}

			if v.pushSession.IsFresh {
				if group.rtmpGopCache.MetadataEnsureWithSetDataFrame != nil {
					_ = v.pushSession.Write(group.rtmpGopCache.MetadataEnsureWithSetDataFrame)
				}
				if group.rtmpGopCache.VideoSeqHeader != nil {
					_ = v.pushSession.Write(group.rtmpGopCache.VideoSeqHeader)
				}
				if group.rtmpGopCache.AacSeqHeader != nil {
					_ = v.pushSession.Write(group.rtmpGopCache.AacSeqHeader)
				}
				for i := 0; i < group.rtmpGopCache.GetGopCount(); i++ {
					for _, item := range group.rtmpGopCache.GetGopDataAt(i) {
						_ = v.pushSession.Write(item)
					}
				}

				v.pushSession.IsFresh = false
			}

			_ = v.pushSession.Write(lazyRtmpChunkDivider.GetEnsureWithSdf())
		}
	}

	// # 广播。遍历所有 httpflv sub session，转发数据
	for session := range group.httpflvSubSessionSet {
		if session.IsFresh {
			if group.httpflvGopCache.MetadataEnsureWithoutSetDataFrame != nil {
				session.Write(group.httpflvGopCache.MetadataEnsureWithoutSetDataFrame)
			}
			if group.httpflvGopCache.VideoSeqHeader != nil {
				session.Write(group.httpflvGopCache.VideoSeqHeader)
			}
			if group.httpflvGopCache.AacSeqHeader != nil {
				session.Write(group.httpflvGopCache.AacSeqHeader)
			}
			gopCount := group.httpflvGopCache.GetGopCount()
			if gopCount > 0 {
				// GOP缓存中肯定包含了关键帧
				session.ShouldWaitVideoKeyFrame = false
			}
			for i := 0; i < gopCount; i++ {
				for _, item := range group.httpflvGopCache.GetGopDataAt(i) {
					session.Write(item)
				}
			}

			session.IsFresh = false
		}

		// 是否在等待关键帧
		if session.ShouldWaitVideoKeyFrame {
			if msg.IsVideoKeyNalu() {
				session.Write(lazyRtmpMsg2FlvTag.GetEnsureWithoutSdf())
				session.ShouldWaitVideoKeyFrame = false
			}
		} else {
			session.Write(lazyRtmpMsg2FlvTag.GetEnsureWithoutSdf())
		}
	}

	// # 录制flv文件
	if group.recordFlv != nil {
		if err := group.recordFlv.WriteRaw(lazyRtmpMsg2FlvTag.GetEnsureWithoutSdf()); err != nil {
			Log.Errorf("[%s] record flv write error. err=%+v", group.UniqueKey, err)
		}
	}

	// # 缓存关键信息，以及gop
	if group.config.RtmpConfig.Enable || group.config.RtmpConfig.RtmpsEnable {
		if !group.rtmpGopCache.Feed(msg, lazyRtmpChunkDivider.GetEnsureWithoutSdf()) {
			Log.Warnf("[%s] over frame number limit for a single gop in rtmp cache.", group.UniqueKey)
		}
		if msg.Header.MsgTypeId == base.RtmpTypeIdMetadata {
			group.rtmpGopCache.SetMetadata(lazyRtmpChunkDivider.GetEnsureWithSdf(), lazyRtmpChunkDivider.GetEnsureWithoutSdf())
		}
	}
	if group.config.HttpflvConfig.Enable {
		if !group.httpflvGopCache.Feed(msg, lazyRtmpMsg2FlvTag.GetEnsureWithoutSdf()) {
			Log.Warnf("[%s] over frame number limit for a single gop in http flv cache.", group.UniqueKey)
		}
		if msg.Header.MsgTypeId == base.RtmpTypeIdMetadata {
			// 注意，因为withSdf实际上用不上，而且我们也没实现，所以全部用without了
			group.httpflvGopCache.SetMetadata(lazyRtmpMsg2FlvTag.GetEnsureWithoutSdf(), lazyRtmpMsg2FlvTag.GetEnsureWithoutSdf())
		}
	}

	// # 记录stat
	if group.stat.AudioCodec == "" {
		if msg.Header.MsgTypeId == base.RtmpTypeIdAudio {
			switch msg.AudioCodecId() {
			case base.RtmpSoundFormatAac:
				if msg.IsAacSeqHeader() {
					group.stat.AudioCodec = base.AudioCodecAac
				}
			case base.RtmpSoundFormatG711U:
				group.stat.AudioCodec = base.AudioCodecG711U
			case base.RtmpSoundFormatG711A:
				group.stat.AudioCodec = base.AudioCodecG711A
			case base.RtmpSoundFormatOpus:
				group.stat.AudioCodec = base.AudioCodecOpus
			}
		}
	}
	if group.stat.VideoCodec == "" {
		if msg.IsAvcKeySeqHeader() {
			group.stat.VideoCodec = base.VideoCodecAvc
		}
		if msg.IsHevcKeySeqHeader() {
			group.stat.VideoCodec = base.VideoCodecHevc
		}
	}
	if group.stat.VideoHeight == 0 || group.stat.VideoWidth == 0 {
		if msg.IsAvcKeySeqHeader() {
			sps, _, err := avc.ParseSpsPpsFromSeqHeader(msg.Payload)
			if err == nil {
				var ctx avc.Context
				err = avc.ParseSps(sps, &ctx)
				if err == nil {
					group.stat.VideoHeight = int(ctx.Height)
					group.stat.VideoWidth = int(ctx.Width)
				}
			}
		}
		if msg.IsHevcKeySeqHeader() {
			var sps []byte
			var err error
			if msg.IsEnhanced() {
				_, sps, _, err = hevc.ParseVpsSpsPpsFromEnhancedSeqHeader(msg.Payload)
			} else {
				_, sps, _, err = hevc.ParseVpsSpsPpsFromSeqHeader(msg.Payload)
			}

			if err == nil {
				var ctx hevc.Context
				err = hevc.ParseSps(sps, &ctx)
				if err == nil {
					group.stat.VideoHeight = int(ctx.PicHeightInLumaSamples)
					group.stat.VideoWidth = int(ctx.PicWidthInLumaSamples)
				}
			}
		}
	}
	if msg.Header.MsgTypeId == base.RtmpTypeIdVideo {
		group.inVideoFpsRecords.Add(nazalog.Clock.Now().Unix(), 1)
	}
}

// ---------------------------------------------------------------------------------------------------------------------

func (group *Group) feedRtpPacket(pkt rtprtcp.RtpPacket) {
	// 如果配置项 OutWaitKeyFrameFlag 为false，则音频和视频都直接发送。（音频和视频都不等待视频关键帧，都不等待任何数据）
	if !group.config.RtspConfig.OutWaitKeyFrameFlag {
		for s := range group.rtspSubSessionSet {
			s.WriteRtpPacket(pkt)
		}
		return
	}

	var (
		boundary        bool // 是否是视频GOP起始位置
		boundaryChecked bool // 保证遍历sub session时，只在必要时检查0次或1次，减少性能开销
	)

	for s := range group.rtspSubSessionSet {
		// session的 ShouldWaitVideoKeyFrame 为false，那么可能有两种情况：
		// 1. 对输入流做智能检测时，判定为流内没有视频
		// 2. 该输出流已经发送过了GOP起始数据
		//
		// 这两种情况下，音频或视频数据都直接发送，不需要等了
		if !s.ShouldWaitVideoKeyFrame {
			s.WriteRtpPacket(pkt)
			continue
		}

		if !boundaryChecked {
			switch group.sdpCtx.GetVideoPayloadTypeBase() {
			case base.AvPacketPtAvc:
				boundary = rtprtcp.IsAvcBoundary(pkt)
			case base.AvPacketPtHevc:
				boundary = rtprtcp.IsHevcBoundary(pkt)
			default:
				// 注意，不是avc和hevc时，直接发送
				boundary = true
			}
			boundaryChecked = true
		}

		if boundary {
			s.WriteRtpPacket(pkt)
			s.ShouldWaitVideoKeyFrame = false
		}
	}
}

// ---------------------------------------------------------------------------------------------------------------------

func (group *Group) feedTsPackets(tsPackets []byte, frame *mpegts.Frame, boundary bool) {
	// 注意，hls的处理放在前面，让hls先判断是否打开新的fragment并flush audio
	if group.hlsMuxer != nil {
		group.hlsMuxer.FeedMpegts(tsPackets, frame, boundary)
	}

	// # 遍历 httpts sub session
	for session := range group.httptsSubSessionSet {
		if session.IsFresh {
			// ## 如果是新加入者

			// 发送头
			session.Write(group.patpmt)

			// 如果有缓存，发送缓存
			// 并且设置标志，后续都实时转发就行了
			gopCount := group.httptsGopCache.GetGopCount()
			for i := 0; i < gopCount; i++ {
				for _, item := range group.httptsGopCache.GetGopDataAt(i) {
					session.Write(item)
				}
			}
			if gopCount > 0 {
				session.ShouldWaitBoundary = false
			}

			// 新加入逻辑只用走一次
			session.IsFresh = false
		}

		// ## 转发本次数据
		if session.ShouldWaitBoundary {
			if boundary {
				session.Write(tsPackets)

				session.ShouldWaitBoundary = false
			} else {
				// 需要继续等
			}
		} else {
			session.Write(tsPackets)
		}
	} // for loop iterate httptsSubSessionSet

	if group.recordMpegts != nil {
		if err := group.recordMpegts.Write(tsPackets); err != nil {
			Log.Errorf("[%s] record mpegts write error. err=%+v", group.UniqueKey, err)
		}
	}

	group.httptsGopCache.Feed(tsPackets, boundary)
}

// ---------------------------------------------------------------------------------------------------------------------

func (group *Group) write2RtmpSubSessions(b []byte) {
	for session := range group.rtmpSubSessionSet {
		if session.IsFresh || session.ShouldWaitVideoKeyFrame {
			continue
		}
		_ = session.Write(b)
	}
}

func (group *Group) writev2RtmpSubSessions(bs net.Buffers) {
	for session := range group.rtmpSubSessionSet {
		if session.IsFresh || session.ShouldWaitVideoKeyFrame {
			continue
		}
		_ = session.Writev(bs)
	}
}

// ---------------------------------------------------------------------------------------------------------------------

func (group *Group) feedWaitRtspSubSessions() {
	for session := range group.rtspSubSessionSet {
		if session.Stage.Load() == rtsp.SubSessionStageReadDescribe {
			session.FeedSdp(*group.sdpCtx)
		}
	}
}
