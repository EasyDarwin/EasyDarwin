// Copyright 2021, Chef.  All rights reserved.
// https://github.com/q191201771/lal
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

package remux

import (
	"math"

	"github.com/q191201771/lal/pkg/base"
	"github.com/q191201771/lal/pkg/rtmp"
)

const (
	dummyAudioFilterStageAnalysis = 1
	dummyAudioFilterStageNormal   = 2
	dummyAudioFilterStageDummy    = 3
)

type DummyAudioFilter struct {
	uk          string
	waitAudioMs int
	onPop       rtmp.OnReadRtmpAvMsg

	stage           int
	earlyStageQueue []base.RtmpMsg
	firstVideoTs    uint32
	prevAudioTs     uint32

	audioCount int
}

// NewDummyAudioFilter 检测输入的rtmp流中是否有音频，如果有，则原样返回；如果没有，则制造静音音频数据叠加在rtmp流里面
//
// @param waitAudioMs 等待音频数据时间，如果超出这个时间还没有接收到音频数据，则开始制造静音数据
// @param onPop       注意，所有回调都发生在输入函数调用中
func NewDummyAudioFilter(uk string, waitAudioMs int, onPop rtmp.OnReadRtmpAvMsg) *DummyAudioFilter {
	return &DummyAudioFilter{
		uk:           uk,
		waitAudioMs:  waitAudioMs,
		onPop:        onPop,
		stage:        dummyAudioFilterStageAnalysis,
		firstVideoTs: math.MaxUint32,
		prevAudioTs:  math.MaxUint32,
	}
}

func (filter *DummyAudioFilter) OnReadRtmpAvMsg(msg base.RtmpMsg) {
	filter.Feed(msg)
}

func (filter *DummyAudioFilter) Feed(msg base.RtmpMsg) {
	switch filter.stage {
	case dummyAudioFilterStageAnalysis:
		filter.handleAnalysisStage(msg)
	case dummyAudioFilterStageNormal:
		filter.handleNormalStage(msg)
	case dummyAudioFilterStageDummy:
		filter.handleDummyStage(msg)
	}
}

// 初始阶段，分析是否存在音频
func (filter *DummyAudioFilter) handleAnalysisStage(msg base.RtmpMsg) {
	switch msg.Header.MsgTypeId {
	case base.RtmpTypeIdMetadata:
		// metadata直接入队列
		filter.cache(msg)
	case base.RtmpTypeIdAudio:
		// 原始流中存在音频，将所有缓存数据出队列，进入normal stage
		for i := range filter.earlyStageQueue {
			filter.onPopProxy(filter.earlyStageQueue[i])
		}
		filter.onPopProxy(msg)

		filter.stage = dummyAudioFilterStageNormal
	case base.RtmpTypeIdVideo:
		// 分析视频数据累计时长是否达到阈值

		// 注意，为了避免seq header的时间戳和视频帧不是线性的（0或其他特殊的值）我们直接入队列并跳过
		if msg.IsVideoKeySeqHeader() {
			filter.cache(msg)
			return
		}

		// 记录首个视频帧的时间戳
		if filter.firstVideoTs == math.MaxUint32 {
			filter.cache(msg)
			filter.firstVideoTs = msg.Header.TimestampAbs
			return
		}

		// 没有达到阈值
		if msg.Header.TimestampAbs-filter.firstVideoTs < uint32(filter.waitAudioMs) {
			filter.cache(msg)
			return
		}

		// 达到阈值
		Log.Debugf("[%s] start make dummy audio.", filter.uk)
		filter.stage = dummyAudioFilterStageDummy
		for i := range filter.earlyStageQueue {
			filter.handleDummyStage(filter.earlyStageQueue[i])
		}
		filter.clearCache()
		filter.handleDummyStage(msg)
	}
}

// 原始流中存在音频
func (filter *DummyAudioFilter) handleNormalStage(msg base.RtmpMsg) {
	filter.onPopProxy(msg)
}

// 原始流中不存在音频
func (filter *DummyAudioFilter) handleDummyStage(msg base.RtmpMsg) {
	if msg.Header.MsgTypeId == base.RtmpTypeIdAudio {
		// 由于我们已经开始制造静音包了，静音包的编码参数可能会和实际音频参数不一致，所以我们只能过滤掉原始的音频数据了
		Log.Warnf("[%s] recv audio but we are making dummy audio.", filter.uk)
		return
	}

	if msg.Header.MsgTypeId == base.RtmpTypeIdMetadata {
		filter.onPopProxy(msg)
		return
	}

	if msg.IsVideoKeySeqHeader() {
		// TODO(chef): 这里的时间戳可以考虑减1，但是注意处理一些边界条件
		ats := msg.Header.TimestampAbs
		amsg := filter.makeAudioSeqHeader(ats)
		filter.onPopProxy(amsg)
		filter.onPopProxy(msg)
		return
	}

	if filter.prevAudioTs == math.MaxUint32 {
		ats := msg.Header.TimestampAbs
		amsg := filter.makeOneAudio(ats)
		filter.onPopProxy(amsg)
		filter.onPopProxy(msg)
		filter.prevAudioTs = ats
	} else {
		for {
			ats := filter.prevAudioTs + filter.calcAudioDurationMs()
			if ats > msg.Header.TimestampAbs {
				break
			}
			amsg := filter.makeOneAudio(ats)
			filter.onPopProxy(amsg)
			filter.prevAudioTs = ats
		}
		filter.onPopProxy(msg)
	}
}

func (filter *DummyAudioFilter) cache(msg base.RtmpMsg) {
	filter.earlyStageQueue = append(filter.earlyStageQueue, msg.Clone())
}

func (filter *DummyAudioFilter) clearCache() {
	filter.earlyStageQueue = nil
}

func (filter *DummyAudioFilter) onPopProxy(msg base.RtmpMsg) {
	if filter.onPop != nil {
		filter.onPop(msg)
	}
}

func (filter *DummyAudioFilter) makeAudioSeqHeader(ts uint32) base.RtmpMsg {
	// aac (LC), 48000 Hz, stereo, fltp
	return base.RtmpMsg{
		Header: base.RtmpHeader{
			Csid:         rtmp.CsidAudio,
			MsgLen:       4,
			MsgTypeId:    base.RtmpTypeIdAudio,
			MsgStreamId:  rtmp.Msid1,
			TimestampAbs: ts,
		},
		Payload: []byte{0xaf, 0x00, 0x11, 0x90},
	}
}

func (filter *DummyAudioFilter) makeOneAudio(ts uint32) base.RtmpMsg {
	filter.audioCount++
	return base.RtmpMsg{
		Header: base.RtmpHeader{
			Csid:         rtmp.CsidAudio,
			MsgLen:       8,
			MsgTypeId:    base.RtmpTypeIdAudio,
			MsgStreamId:  rtmp.Msid1,
			TimestampAbs: ts,
		},
		// 注意，前面2字节是seq header头部信息，后面6个字节是AAC静音包
		Payload: []byte{0xaf, 0x01, 0x21, 0x10, 0x04, 0x60, 0x8c, 0x1c},
	}
}

func (filter *DummyAudioFilter) calcAudioDurationMs() uint32 {
	v := filter.audioCount % 3
	if v == 1 || v == 2 {
		return 21
	}
	return 22
}
