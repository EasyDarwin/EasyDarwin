// Copyright 2019, Chef.  All rights reserved.
// https://github.com/q191201771/lal
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

package rtmp

import "github.com/q191201771/naza/pkg/nazalog"

// TODO chef 一些更专业的配置项，暂时只在该源码文件中配置，不提供外部配置接口

var (
	Log = nazalog.GetGlobalLogger()
)

var (
	readBufSize                   = 4096  // server session connection读缓冲的大小
	wChanSize                     = 1024  // server session 发送数据时，channel 的大小
	serverSessionReadAvTimeoutMs  = 10000 // server pub session，读音视频数据超时
	serverSessionWriteAvTimeoutMs = 10000 // server sub session，写音视频数据超时
	//writeBufSize                  = 4096  // 注意，因为lal server在group中使用writev做merge合并发送，这个废弃不需要了

	// LocalChunkSize
	//
	// 本端（包括Server Session和Client Session）设置的chunk size，本端发送数据时切割chunk包时使用
	// （对端发送数据时的chunk size由对端决定，和本变量没有关系）
	//
	// 注意，这个值不应该设置的太小，原因有两方面：
	// 1. 性能与带宽
	//    切割的chunk包过多，会消耗更多的CPU资源（包括本地和远端），另外还可能增加传输时的chunk header带宽消耗
	// 2. 兼容性
	//    理论上，信令也要参考chunk size切割成chunk包，而对端使用chunk包合成message的实现不一定标准。
	//    我就遇到过这样的case，对端认为rtmp握手后的几个信令，每个信令都只使用一个chunk。
	//    假如我们将一条信令切割成多个chunk，对端可能就解析错误了，这属于对端实现的问题。
	//    但为了更好的兼容性，我们不要将chunk size设置的太小。
	//
	LocalChunkSize = 4096

	// 发送 base.RtmpTypeIdWinAckSize 信令中的阈值，目前 ServerSession 在使用
	windowAcknowledgementSize = 5000000

	peerBandwidth = 5000000
)

// 接收rtmp数据时，msg的初始内存块大小
// 注意，该值只影响性能，不影响功能（大小不够会自动扩容）
const initMsgLen = 4096
