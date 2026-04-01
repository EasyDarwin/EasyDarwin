// Copyright 2019, Chef.  All rights reserved.
// https://github.com/q191201771/lal
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

package rtmp

const (
	CsidAmf   = 5
	CsidAudio = 6
	CsidVideo = 7

	csidProtocolControl = 2
	csidOverConnection  = 3
	csidOverStream      = 5

	//minCsid = 2
	//maxCsid = 65599
)

const (
	tidClientConnect      = 1
	tidClientCreateStream = 2
	tidClientPlay         = 3
	tidClientPublish      = 3
)

// basic header 3 | message header 11 | extended ts 4
const maxHeaderSize = 18

// rtmp头中3字节时间戳的最大值
const maxTimestampInMessageHeader uint32 = 0xFFFFFF

const defaultChunkSize = 128 // 未收到对端设置chunk size时的默认值

const (
	//MSID0 = 0 // 所有除 publish、play、onStatus 之外的信令

	Msid1 = 1 // publish、play、onStatus 以及 音视频数据
)

const ackSeqMax = 0xf0000000

// ---------------------------------------------------------------------------------------------------------------------
// ### rtmp connect message
//
// #### ffmpeg pub
// app:      live
// type:     nonprivate
// flashVer: FMLE/3.0 (compatible; Lavf57.83.100)
// tcUrl:    rtmp://127.0.0.1:19350/live
//
// #### ffplay sub & vlc sub
// app:           live
// flashVer:      LNX 9,0,124,2
// tcUrl:         rtmp://127.0.0.1:19350/live
// fpad:          false
// capabilities:  15
// audioCodecs:   4071
// videoCodecs:   252
// videoFunction: 1
