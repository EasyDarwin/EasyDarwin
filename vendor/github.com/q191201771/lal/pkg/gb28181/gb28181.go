// Copyright 2022, Chef.  All rights reserved.
// https://github.com/q191201771/lal
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

package gb28181

import (
	"errors"
	"github.com/q191201771/naza/pkg/nazalog"
	"github.com/q191201771/naza/pkg/nazanet"
)

// TODO(chef): [feat] http api stop_rtp_pub 202207
// TODO(chef): [feat] http api /api/stat/all_rtp_pub，不过这个可以用已有的all_group代替 202207
// TODO(chef): [perf] 优化ps解析，内存块 202207
// TODO(chef): [opt] avpkt转rtmp时，可能需要接一个缓存队列 202208

var (
	Log = nazalog.GetGlobalLogger()
)

// ErrGb28181 TODO(chef): [refactor] move to pkg base 202207
var ErrGb28181 = errors.New("lal.gb28181: fxxk")

// TODO(chef): [opt] 除了队列长度，还可以通过时长控制 202209
var maxUnpackRtpListSize = 1024

var (
	defaultPubSessionPortMin = uint16(30000) // 注意，udp和tcp都使用这个端口范围
	defaultPubSessionPortMax = uint16(60000)
)

var defaultUdpConnPoll *nazanet.AvailUdpConnPool

func init() {
	defaultUdpConnPoll = nazanet.NewAvailUdpConnPool(defaultPubSessionPortMin, defaultPubSessionPortMax)
}
