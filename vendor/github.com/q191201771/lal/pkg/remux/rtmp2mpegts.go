// Copyright 2020, Chef.  All rights reserved.
// https://github.com/q191201771/lal
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

package remux

import (
	"encoding/hex"

	"github.com/q191201771/lal/pkg/aac"
	"github.com/q191201771/lal/pkg/avc"
	"github.com/q191201771/lal/pkg/base"
	"github.com/q191201771/lal/pkg/hevc"
	"github.com/q191201771/lal/pkg/mpegts"
	"github.com/q191201771/naza/pkg/nazabytes"
	"github.com/q191201771/naza/pkg/nazalog"
)

const (
	initialVideoOutBufferSize          = 1024 * 1024
	calcFragmentHeaderQueueSize        = 16
	maxAudioCacheDelayByAudio   uint64 = 150 * 90 // 单位（毫秒*90）
	maxAudioCacheDelayByVideo   uint64 = 300 * 90 // 单位（毫秒*90）
)

type IRtmp2MpegtsRemuxerObserver interface {
	// OnPatPmt
	//
	// @param b: const只读内存块，上层可以持有，但是不允许修改
	//
	OnPatPmt(b []byte)

	// OnTsPackets
	//
	// @param tsPackets:
	//  - mpegts数据，有一个或多个188字节的ts数据组成
	//  - 回调结束后，remux.Rtmp2MpegtsRemuxer 不再使用这块内存块
	//
	// @param frame: 各字段含义见 mpegts.Frame 结构体定义
	//
	// @param boundary: 是否到达边界处，根据该字段，上层可判断诸如：
	//  - hls是否允许开启新的ts文件切片
	//  - 新的ts播放流从该位置开始播放
	//
	OnTsPackets(tsPackets []byte, frame *mpegts.Frame, boundary bool)
}

// Rtmp2MpegtsRemuxer 输入rtmp流，输出mpegts流
type Rtmp2MpegtsRemuxer struct {
	uk string

	observer        IRtmp2MpegtsRemuxerObserver
	filter          *rtmp2MpegtsFilter
	videoOut        []byte // Annexb
	spspps          []byte // Annexb 也可能是vps+sps+pps
	ascCtx          *aac.AscContext
	audioCc         uint8
	videoCc         uint8
	timestampFilter Rtmp2MpegtsTimestampFilter

	// audioCacheFrames: 缓存音频packet数据，注意，可能包含多个音频packet
	//
	// audioCacheFirstFramePts: audioCacheFrames中第一个音频packet的时间戳
	//
	// 缓存（也即合并）多个packet并让他们使用第一个packet的时间戳，目的是为了还原单packet的精度。
	//
	// 比如，aac 48000的每帧数据时长为21.333，
	// 由于rtmp的时间戳是整数（单位毫秒），所以它使用如下方式：
	//  duration: 21, 21, 22
	// timestamp: 0, 21, 42, 64
	// 其中duration的第一个和第二个都少了0.33，第三个多了0.66，然后循环
	// timestamp中0和64是对的，21和42是有误差的，然后循环。也即3个packet，timestamp恢复严格意义上的正确。
	// 这个用整数类型毫秒单位没法避免。
	//
	// 当 * 90换算成mpegts的时间戳时，duration的小数部分的误差还是存在，
	// 这将导致部分hls播放器（比如macOS的QuickTime）会出现声音效果不好的情况。
	// 所以我们优化为将多个packet合并，使得原来每个packet之间出现异常的情况，缓解为缓存与缓存间可能出现异常的情况
	//
	// TODO(chef): 可按ffmpeg中av_rescale_delta的手段进行优化 202206
	//
	// 其大致思想是，当duration在一定范围内，使用timestamp*90；当duration超过一定范围，则直接用采样率换算得到的duration。
	// 也即，迁移中方法是21或者22 * 90，后一种直接是21.33*90=1920
	// int64_t av_rescale_delta(AVRational in_tb, int64_t in_ts,  AVRational fs_tb, int duration, int64_t *last, AVRational out_tb)
	// in_tb    1000
	// in_ts    dts
	// fs_tb    48000
	// duration 1024
	// last
	// out_tb   90000
	//
	// 但是ffmpeg的做法也有问题，就是每个packet都转ts，padding会比较多，导致ts码流变大
	//
	//TODO chef: rename to DTS
	//
	audioCacheFrames        []byte
	audioCacheFirstFramePts uint64

	opened bool
}

func NewRtmp2MpegtsRemuxer(observer IRtmp2MpegtsRemuxerObserver) *Rtmp2MpegtsRemuxer {
	uk := base.GenUkRtmp2MpegtsRemuxer()
	r := &Rtmp2MpegtsRemuxer{
		uk:       uk,
		observer: observer,
	}
	r.audioCacheFrames = nil
	r.videoOut = make([]byte, initialVideoOutBufferSize)
	r.videoOut = r.videoOut[0:0]
	r.filter = newRtmp2MpegtsFilter(calcFragmentHeaderQueueSize, r)
	r.timestampFilter.Init(uk)

	nazalog.Debugf("[%s] NewRtmp2MpegtsRemuxer", r.uk)

	return r
}

// FeedRtmpMessage
//
// @param msg: msg.Payload 调用结束后，函数内部不会持有这块内存
func (s *Rtmp2MpegtsRemuxer) FeedRtmpMessage(msg base.RtmpMsg) {
	s.filter.Push(msg)
}

func (s *Rtmp2MpegtsRemuxer) Dispose() {
	s.FlushAudio()
}

// ---------------------------------------------------------------------------------------------------------------------

// FlushAudio
//
// 吐出音频数据的三种情况：
// 1. 收到音频或视频时，音频缓存队列已达到一定长度（内部判断）
// 2. 打开一个新的TS文件切片时
// 3. 输入流关闭时
func (s *Rtmp2MpegtsRemuxer) FlushAudio() {
	if s.audioCacheEmpty() {
		return
	}

	var frame mpegts.Frame
	frame.Cc = s.audioCc
	frame.Dts = s.audioCacheFirstFramePts
	frame.Cts = 0
	frame.Pts = s.audioCacheFirstFramePts
	frame.Key = false
	frame.Raw = s.audioCacheFrames
	frame.Pid = mpegts.PidAudio
	frame.Sid = mpegts.StreamIdAudio

	// 注意，在回调前设置为空，因为回调中有可能再次调用FlushAudio
	s.resetAudioCache()

	s.onFrame(&frame)
	// 回调结束后更新cc
	s.audioCc = frame.Cc
}

func (s *Rtmp2MpegtsRemuxer) UniqueKey() string {
	return s.uk
}

// ----- implement of iRtmp2MpegtsFilterObserver ----------------------------------------------------------------------------------------------------------------

// onPatPmt onPop
//
// 实现 iRtmp2MpegtsFilterObserver
func (s *Rtmp2MpegtsRemuxer) onPatPmt(b []byte) {
	s.observer.OnPatPmt(b)
}

func (s *Rtmp2MpegtsRemuxer) onPop(msg base.RtmpMsg) {
	switch msg.Header.MsgTypeId {
	case base.RtmpTypeIdAudio:
		if msg.AudioCodecId() != base.RtmpSoundFormatAac && msg.AudioCodecId() != base.RtmpSoundFormatOpus {
			return
		}
		s.feedAudio(msg)
	case base.RtmpTypeIdVideo:
		s.feedVideo(msg)
	}
}

// ---------------------------------------------------------------------------------------------------------------------

func (s *Rtmp2MpegtsRemuxer) feedVideo(msg base.RtmpMsg) {
	if len(msg.Payload) <= 5 {
		Log.Warnf("[%s] rtmp msg too short, ignore. header=%+v, payload=%s", s.uk, msg.Header, hex.Dump(msg.Payload))
		return
	}

	codecId := msg.VideoCodecId()
	if codecId != base.RtmpCodecIdAvc && codecId != base.RtmpCodecIdHevc {
		return
	}

	// 将数据转换成Annexb

	// 如果是seq header sps pps，缓存住，然后直接返回
	var err error
	if msg.IsAvcKeySeqHeader() {
		if s.spspps, err = avc.SpsPpsSeqHeader2Annexb(msg.Payload); err != nil {
			Log.Errorf("[%s] cache spspps failed. err=%+v", s.uk, err)
		}
		return
	} else if msg.IsHevcKeySeqHeader() {
		if msg.IsEnhanced() {
			if s.spspps, err = hevc.VpsSpsPpsEnhancedSeqHeader2Annexb(msg.Payload); err != nil {
				Log.Errorf("[%s] cache vpsspspps failed. err=%+v", s.uk, err)
			}
		} else {
			if s.spspps, err = hevc.VpsSpsPpsSeqHeader2Annexb(msg.Payload); err != nil {
				Log.Errorf("[%s] cache vpsspspps failed. err=%+v", s.uk, err)
			}
		}

		return
	}

	audSent := false
	spsppsSent := false
	s.resetVideoOutBuffer()

	// msg中可能有多个NALU，逐个获取
	var nals [][]byte
	if codecId == base.RtmpCodecIdHevc && msg.IsEnchanedHevcNalu() {
		index := msg.GetEnchanedHevcNaluIndex()
		nals, err = avc.SplitNaluAvcc(msg.Payload[index:])
	} else {
		nals, err = avc.SplitNaluAvcc(msg.Payload[5:])
	}
	if err != nil {
		Log.Errorf("[%s] iterate nalu failed. err=%+v, header=%+v, payload=%s", err, s.uk, msg.Header, hex.Dump(nazabytes.Prefix(msg.Payload, 32)))
		return
	}

	var vps, sps, pps []byte
	for _, nal := range nals {
		var nalType uint8
		switch codecId {
		case base.RtmpCodecIdAvc:
			nalType = avc.ParseNaluType(nal[0])
		case base.RtmpCodecIdHevc:
			nalType = hevc.ParseNaluType(nal[0])
		}

		// 处理 sps pps aud
		//
		// aud 过滤掉，我们有自己的添加aud的逻辑
		//
		// sps pps
		// 注意，有的流，seq header中的sps和pps是错误的，需要从nals里获取sps pps并更新
		// 见 https://github.com/q191201771/lal/issues/143
		//
		// TODO(chef): rtmp转其他类型的模块也存在这个问题，应该抽象出一个统一处理的地方
		//
		if codecId == base.RtmpCodecIdAvc {
			if nalType == avc.NaluTypeAud {
				continue
			} else if nalType == avc.NaluTypeSps {
				sps = nal
				continue
			} else if nalType == avc.NaluTypePps {
				pps = nal
				if len(sps) != 0 && len(pps) != 0 {
					s.spspps = s.spspps[0:0]
					s.spspps = append(s.spspps, avc.NaluStartCode4...)
					s.spspps = append(s.spspps, sps...)
					s.spspps = append(s.spspps, avc.NaluStartCode4...)
					s.spspps = append(s.spspps, pps...)
				}
				continue
			}
		} else if codecId == base.RtmpCodecIdHevc {
			if nalType == hevc.NaluTypeSei || nalType == hevc.NaluTypeSeiSuffix {
				// TODO(chef): [opt] 考虑缓存下来，放到关键帧前面 202208
				continue
			}
			if nalType == hevc.NaluTypeAud {
				continue
			} else if nalType == hevc.NaluTypeVps {
				vps = nal
				continue
			} else if nalType == hevc.NaluTypeSps {
				sps = nal
				continue
			} else if nalType == hevc.NaluTypePps {
				pps = nal
				if len(vps) != 0 && len(sps) != 0 && len(pps) != 0 {
					s.spspps = s.spspps[0:0]
					s.spspps = append(s.spspps, avc.NaluStartCode4...)
					s.spspps = append(s.spspps, vps...)
					s.spspps = append(s.spspps, avc.NaluStartCode4...)
					s.spspps = append(s.spspps, sps...)
					s.spspps = append(s.spspps, avc.NaluStartCode4...)
					s.spspps = append(s.spspps, pps...)
				}
				continue
			}
		}

		// tag中的首个nalu前面写入aud
		if !audSent {
			// 注意，因为前面已经过滤了sps pps aud的信息，所以这里可以认为都是需要用aud分隔的，不需要单独判断了
			//if codecId == base.RtmpCodecIdAvc && (nalType == avc.NaluTypeSei || nalType == avc.NaluTypeIdrSlice || nalType == avc.NaluTypeSlice) {
			switch codecId {
			case base.RtmpCodecIdAvc:
				s.videoOut = append(s.videoOut, avc.AudNalu...)
			case base.RtmpCodecIdHevc:
				s.videoOut = append(s.videoOut, hevc.AudNalu...)
			}
			audSent = true
		}

		// 关键帧前追加sps pps
		if codecId == base.RtmpCodecIdAvc {
			// h264的逻辑，一个tag中，多个连续的关键帧只追加一个，不连续则每个关键帧前都追加。为什么要这样处理
			switch nalType {
			case avc.NaluTypeIdrSlice:
				if !spsppsSent {
					if s.videoOut, err = s.appendSpsPps(s.videoOut); err != nil {
						Log.Warnf("[%s] append spspps by not exist.", s.uk)
						return
					}
				}
				spsppsSent = true
			case avc.NaluTypeSlice:
				// 这里只有P帧，没有SEI。为什么要这样处理
				spsppsSent = false
			}
		} else {
			// TODO(chef): [refactor] avc和hevc可以考虑再抽象一层高层的包，使得更上层代码简洁一些
			if hevc.IsIrapNalu(nalType) {
				if !spsppsSent {
					if s.videoOut, err = s.appendSpsPps(s.videoOut); err != nil {
						Log.Warnf("[%s] append spspps by not exist.", s.uk)
						return
					}
				}
				spsppsSent = true
			} else {
				// 这里简化了，只要不是关键帧，就刷新标志
				spsppsSent = false
			}
		}

		// 如果写入了aud或spspps，则用start code3，否则start code4。为什么要这样处理
		// 这里不知为什么要区分写入两种类型的start code
		if len(s.videoOut) == 0 {
			s.videoOut = append(s.videoOut, avc.NaluStartCode4...)
		} else {
			s.videoOut = append(s.videoOut, avc.NaluStartCode3...)
		}

		s.videoOut = append(s.videoOut, nal...)
	} // for loop

	if len(s.videoOut) == 0 {
		// 比如只有SEI nal或者非seq header的msg只有sps, pps, vps
		return
	}

	dts := uint64(msg.Header.TimestampAbs) * 90

	if !s.audioCacheEmpty() && s.audioCacheFirstFramePts+maxAudioCacheDelayByVideo < dts {
		s.FlushAudio()
	}

	var frame mpegts.Frame
	frame.Cc = s.videoCc
	frame.Dts = dts
	frame.Cts = msg.Cts()
	frame.Pts = frame.Dts + 90*uint64(frame.Cts)
	frame.Key = msg.IsVideoKeyNalu()
	frame.Raw = s.videoOut
	frame.Pid = mpegts.PidVideo
	frame.Sid = mpegts.StreamIdVideo

	s.onFrame(&frame)
	s.videoCc = frame.Cc
}

func (s *Rtmp2MpegtsRemuxer) feedAudio(msg base.RtmpMsg) {
	if len(msg.Payload) <= 2 {
		Log.Warnf("[%s] rtmp msg too short, ignore. header=%+v, payload=%s", s.uk, msg.Header, hex.Dump(msg.Payload))
		return
	}

	//Log.Debugf("[%s] hls: feedAudio. dts=%d len=%d", s.uk, msg.Header.TimestampAbs, len(msg.Payload))

	if msg.AudioCodecId() == base.RtmpSoundFormatAac {
		if msg.Payload[1] == base.RtmpAacPacketTypeSeqHeader {
			if err := s.cacheAacSeqHeader(msg); err != nil {
				Log.Errorf("[%s] cache aac seq header failed. err=%+v", s.uk, err)
			}
			return
		}

		if !s.audioSeqHeaderCached() {
			Log.Warnf("[%s] feed audio message but aac seq header not exist.", s.uk)
			return
		}
	}

	pts := uint64(msg.Header.TimestampAbs) * 90
	if msg.AudioCodecId() == base.RtmpSoundFormatAac {
		if !s.audioCacheEmpty() && s.audioCacheFirstFramePts+maxAudioCacheDelayByAudio < pts {
			s.FlushAudio()
		}

		if s.audioCacheEmpty() {
			s.audioCacheFirstFramePts = pts
		}

		adtsHeader := s.ascCtx.PackAdtsHeader(int(msg.Header.MsgLen - 2))
		s.audioCacheFrames = append(s.audioCacheFrames, adtsHeader...)
		s.audioCacheFrames = append(s.audioCacheFrames, msg.Payload[2:]...)
	} else {
		s.audioCacheFirstFramePts = pts
		s.audioCacheFrames = append(s.audioCacheFrames, msg.Payload[1:]...)
		s.FlushAudio()
	}
}

func (s *Rtmp2MpegtsRemuxer) cacheAacSeqHeader(msg base.RtmpMsg) error {
	var err error
	s.ascCtx, err = aac.NewAscContext(msg.Payload[2:])
	return err
}

func (s *Rtmp2MpegtsRemuxer) audioSeqHeaderCached() bool {
	return s.ascCtx != nil
}

func (s *Rtmp2MpegtsRemuxer) appendSpsPps(out []byte) ([]byte, error) {
	if s.spspps == nil {
		return out, base.ErrHls
	}

	out = append(out, s.spspps...)
	return out, nil
}

func (s *Rtmp2MpegtsRemuxer) videoSeqHeaderCached() bool {
	return len(s.spspps) != 0
}

func (s *Rtmp2MpegtsRemuxer) audioCacheEmpty() bool {
	return len(s.audioCacheFrames) == 0
}

func (s *Rtmp2MpegtsRemuxer) resetAudioCache() {
	s.audioCacheFrames = s.audioCacheFrames[0:0]
}

func (s *Rtmp2MpegtsRemuxer) resetVideoOutBuffer() {
	s.videoOut = s.videoOut[0:0]
}

func (s *Rtmp2MpegtsRemuxer) onFrame(frame *mpegts.Frame) {
	//Log.Debugf("in frame=%s", frame.DebugString())
	s.timestampFilter.Do(frame)
	//Log.Debugf("ou frame=%s", frame.DebugString())

	var boundary bool

	if frame.Sid == mpegts.StreamIdAudio {
		// 为了考虑没有视频的情况也能切片，所以这里判断spspps为空时，也建议生成fragment
		boundary = !s.videoSeqHeaderCached()
	} else {
		// 收到视频，可能触发建立fragment的条件是：
		// 关键帧数据 &&
		// (
		//  (没有收到过音频seq header) || 说明 只有视频
		//  (收到过音频seq header && fragment没有打开) || 说明 音视频都有，且都已ready
		//  (收到过音频seq header && fragment已经打开 && 音频缓存数据不为空) 说明 为什么音频缓存需不为空？
		// )
		boundary = frame.Key && (!s.audioSeqHeaderCached() || !s.opened || !s.audioCacheEmpty())
	}

	if boundary {
		s.opened = true
	}

	packets := frame.Pack()

	//nazalog.Debugf("> OnTsPackets. frame=%s, boundary=%v, packets=%d", frame.DebugString(), boundary, len(packets))
	s.observer.OnTsPackets(packets, frame, boundary)
}
