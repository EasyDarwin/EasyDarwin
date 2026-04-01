// Copyright 2020, Chef.  All rights reserved.
// https://github.com/q191201771/lal
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

package base

// 文档见： https://pengrl.com/lal/#/HTTPAPI

// ----- request -------------------------------------------------------------------------------------------------------

const (
	PullRetryNumForever = -1 // 永远重试
	PullRetryNumNever   = 0  // 不重试

	AutoStopPullAfterNoOutMsNever       = -1
	AutoStopPullAfterNoOutMsImmediately = 0

	RtspModeTcp = 0
	RtspModeUdp = 1
)

type ApiCtrlStartRelayPullReq struct {
	Url                      string `json:"url"`
	StreamName               string `json:"stream_name"`
	PullTimeoutMs            int    `json:"pull_timeout_ms"`
	PullRetryNum             int    `json:"pull_retry_num"`
	AutoStopPullAfterNoOutMs int    `json:"auto_stop_pull_after_no_out_ms"`
	RtspMode                 int    `json:"rtsp_mode"`
	DebugDumpPacket          string `json:"debug_dump_packet"`
}

type ApiCtrlKickSessionReq struct {
	StreamName string `json:"stream_name"`
	SessionId  string `json:"session_id"`
}

type ApiCtrlStartRtpPubReq struct {
	StreamName      string `json:"stream_name"`
	Port            int    `json:"port"`
	TimeoutMs       int    `json:"timeout_ms"`
	IsTcpFlag       int    `json:"is_tcp_flag"`
	DebugDumpPacket string `json:"debug_dump_packet"`
}

// ----- response ------------------------------------------------------------------------------------------------------

const (
	ErrorCodeSucc = 0
	DespSucc      = "succ"

	ErrorCodePageNotFound = 404
	DespPageNotFound      = "page not found, check this document out: https://pengrl.com/lal/#/HTTPAPI"

	ErrorCodeGroupNotFound   = 1001
	DespGroupNotFound        = "group not found"
	ErrorCodeParamMissing    = 1002
	DespParamMissing         = "param missing"
	ErrorCodeSessionNotFound = 1003
	DespSessionNotFound      = "session not found"

	ErrorCodeStartRelayPullFail = 2001
	ErrorCodeListenUdpPortFail  = 2002
)

type ApiRespBasic struct {
	ErrorCode int    `json:"error_code"`
	Desp      string `json:"desp"`
}

var ApiNotFoundResp = ApiRespBasic{
	ErrorCode: ErrorCodePageNotFound,
	Desp:      DespPageNotFound,
}

type ApiStatLalInfoResp struct {
	ApiRespBasic
	Data LalInfo `json:"data"`
}

type ApiStatAllGroupResp struct {
	ApiRespBasic
	Data struct {
		Groups []StatGroup `json:"groups"`
	} `json:"data"`
}

type ApiStatGroupResp struct {
	ApiRespBasic
	Data *StatGroup `json:"data"`
}

type ApiCtrlStartRelayPullResp struct {
	ApiRespBasic
	Data struct {
		StreamName string `json:"stream_name"`
		SessionId  string `json:"session_id"`
	} `json:"data"`
}

type ApiCtrlStopRelayPullResp struct {
	ApiRespBasic
	Data struct {
		SessionId string `json:"session_id"`
	} `json:"data"`
}

type ApiCtrlKickSessionResp struct {
	ApiRespBasic
}

type ApiCtrlStartRtpPubResp struct {
	ApiRespBasic
	Data struct {
		StreamName string `json:"stream_name"`
		SessionId  string `json:"session_id"`
		Port       int    `json:"port"`
	} `json:"data"`
}
