// Copyright 2022, Chef.  All rights reserved.
// https://github.com/q191201771/lal
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

package logic

import (
	"github.com/q191201771/lal/pkg/base"
	"github.com/q191201771/naza/pkg/nazalog"
)

var Log = nazalog.GetGlobalLogger()

var (

	// 所有session超时管理整理如下：
	//
	// (1.) 第一种方式，是上层判断
	//
	// (1.1.) CheckSessionAliveIntervalSec
	// - rtmp pub, rtsp pub,
	// - rtmp pull, rtsp pull,
	// - rtmp sub, rtsp sub, httpflv sub, httpts sub,
	// - rtmp push,
	//
	// (1.2.) HTTP-API参数 ApiCtrlStartRtpPubReq.TimeoutMs
	// - ps pub,
	//
	// (1.3.) 无
	// - customize pub,
	// - hls sub,
	//
	// (2.) 第二种方式，是session自身提供的超时功能：
	//
	// (2.x.) rtmp pub, rtmp sub: 底层naza connection，并且设置了超时 rtmp.serverSessionReadAvTimeoutMs rtmp.serverSessionWriteAvTimeoutMs
	// (2.x.) rtsp pub, rtsp sub: cmd以及tcp模式时底层naza connection，但是没有设置超时(udp使用 nazanet.UdpConnection),
	// (2.x.) rtmp pull, rtsp pull: HTTP-API参数 ApiCtrlStartRelayPullReq.PullTimeoutMs 静态回源时 StaticRelayPullTimeoutMs
	// (2.x.) httpflv sub, httpts sub:  httpflv.SubSessionWriteTimeoutMs , httpts.SubSessionWriteTimeoutMs
	// (2.x.) rtmp push: RelayPushTimeoutMs, RelayPushWriteAvTimeoutMs,
	// (2.x.) 无: ps pub, customize pub,
	// (2.x.) hls sub: 配置文件中配置项 sub_session_timeout_ms
	//
	// (3.) client类型session默认超时：
	// - rtmp push: rtmp.PushSessionOption.PushTimeoutMs WriteAvTimeoutMs
	// - rtsp push: rtsp.PushSessionOption.PushTimeoutMs
	// - rtmp pull: rtmp.PullSessionOption.PullTimeoutMs ReadAvTimeoutMs
	// - rtsp pull: rtsp.PullSessionOption.PullTimeoutMs
	// - httpflv pull: httpflv.PullSessionOption.PullTimeoutMs ReadTimeoutMs

	// CheckSessionAliveIntervalSec
	//
	// 检查session是否有数据传输的时间间隔，该间隔内没有数据传输的session将被关闭。
	//
	// 对于输入型session，检查一定时间内，是否没有收到数据。
	//
	// 对于输出型session，检查一定时间内，是否没有发送数据。
	// 注意，socket阻塞无法发送和上层没有向该session喂入数据都算没有发送数据。
	//
	CheckSessionAliveIntervalSec uint32 = 10

	RelayPushTimeoutMs        = 10000
	RelayPushWriteAvTimeoutMs = 10000

	StaticRelayPullTimeoutMs = 10000

	DefaultApiCtrlStartRtpPubReqTimeoutMs        = 60000
	DefaultApiCtrlStartRelayPullReqPullTimeoutMs = 10000
)

// 注意，这是配置文件中静态回源的配置值，不是HTTP-API的默认值
const (
	staticRelayPullRetryNum                 = base.PullRetryNumForever
	staticRelayPullAutoStopPullAfterNoOutMs = base.AutoStopPullAfterNoOutMsImmediately
)

var (
	// calcSessionStatIntervalSec 计算所有session收发码率的时间间隔
	//
	calcSessionStatIntervalSec uint32 = 5
)

const (
	defaultHlsCalcSessionStatIntervalSec uint32 = 10
)
