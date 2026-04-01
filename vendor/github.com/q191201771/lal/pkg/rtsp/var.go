// Copyright 2022, Chef.  All rights reserved.
// https://github.com/q191201771/lal
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

package rtsp

import "github.com/q191201771/naza/pkg/nazalog"

var Log = nazalog.GetGlobalLogger()

// BaseInSessionTimestampFilterFlag 控制输入 BaseInSession 的音视频数据是否开启时间戳过滤器，也即经过 AvPacketQueue 处理
var BaseInSessionTimestampFilterFlag = true
var TimestampFilterHandleRotateFlag = true
