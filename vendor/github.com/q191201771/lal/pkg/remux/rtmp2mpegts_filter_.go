// Copyright 2021, Chef.  All rights reserved.
// https://github.com/q191201771/lal
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

package remux

import (
	"github.com/q191201771/lal/pkg/base"
	"github.com/q191201771/lal/pkg/mpegts"
)

// rtmp2MpegtsFilter
//
// 缓存流起始的一些数据，判断流中是否存在音频、视频，以及编码格式，生成正确的mpegts PatPmt头信息
//
// 一旦判断结束，该队列变成直进直出，不再有实际缓存
//
// TODO(chef): [refactor] 文件名改为 rtmp2mpegts_filter__probe.go 202304
type rtmp2MpegtsFilter struct {
	maxMsgSize int
	data       []base.RtmpMsg
	observer   iRtmp2MpegtsFilterObserver

	audioCodecId int
	videoCodecId int
	done         bool
}

type iRtmp2MpegtsFilterObserver interface {
	// OnPatPmt
	//
	// 该回调一定发生在数据回调之前
	// 只会返回两种格式，h264和h265
	//
	// TODO(chef): [opt] 当没有视频时，不应该返回h264的格式
	// TODO(chef) 这里可以考虑换成只通知drain，由上层完成FragmentHeader的组装逻辑
	//
	onPatPmt(b []byte)

	onPop(msg base.RtmpMsg)
}

// NewRtmp2MpegtsFilter
//
// @param maxMsgSize: 最大缓存多少个包
func newRtmp2MpegtsFilter(maxMsgSize int, observer iRtmp2MpegtsFilterObserver) *rtmp2MpegtsFilter {
	return &rtmp2MpegtsFilter{
		maxMsgSize:   maxMsgSize,
		data:         make([]base.RtmpMsg, maxMsgSize)[0:0],
		observer:     observer,
		audioCodecId: -1,
		videoCodecId: -1,
		done:         false,
	}
}

// Push
//
// @param msg: 函数调用结束后，内部不持有该内存块
func (q *rtmp2MpegtsFilter) Push(msg base.RtmpMsg) {
	if q.done {
		q.observer.onPop(msg)
		return
	}

	q.data = append(q.data, msg.Clone())

	switch msg.Header.MsgTypeId {
	case base.RtmpTypeIdAudio:
		q.audioCodecId = int(msg.Payload[0] >> 4)
	case base.RtmpTypeIdVideo:
		q.videoCodecId = int(msg.VideoCodecId())
	}

	if q.videoCodecId != -1 && q.audioCodecId != -1 {
		q.drain()
		return
	}

	if len(q.data) >= q.maxMsgSize {
		q.drain()
		return
	}
}

// ---------------------------------------------------------------------------------------------------------------------

func (q *rtmp2MpegtsFilter) drain() {
	patpmt := mpegts.PackPat()
	patpmt = append(patpmt, mpegts.PackPmt(q.videoCodecId, q.audioCodecId)...)
	q.observer.onPatPmt(patpmt)

	for i := range q.data {
		q.observer.onPop(q.data[i])
	}

	q.data = nil

	q.done = true
}
