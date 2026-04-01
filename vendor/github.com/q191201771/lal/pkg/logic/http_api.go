// Copyright 2020, Chef.  All rights reserved.
// https://github.com/q191201771/lal
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

package logic

import (
	_ "embed"
	"encoding/json"
	"html/template"
	"io"
	"net"
	"net/http"
	"strings"

	"github.com/q191201771/naza/pkg/nazajson"

	"github.com/q191201771/naza/pkg/nazahttp"

	"github.com/q191201771/lal/pkg/base"
)

//go:embed http_an__lal.html
var webUITpl string

type HttpApiServer struct {
	addr string
	sm   *ServerManager

	ln net.Listener
}

func NewHttpApiServer(addr string, sm *ServerManager) *HttpApiServer {
	return &HttpApiServer{
		addr: addr,
		sm:   sm,
	}
}

func (h *HttpApiServer) Listen() (err error) {
	if h.ln, err = net.Listen("tcp", h.addr); err != nil {
		return
	}
	Log.Infof("start http-api server listen. addr=%s", h.addr)
	return
}

func (h *HttpApiServer) RunLoop() error {
	mux := http.NewServeMux()

	mux.HandleFunc("/lal.html", h.webUIHandler)

	mux.HandleFunc("/api/stat/group", h.statGroupHandler)
	mux.HandleFunc("/api/stat/all_group", h.statAllGroupHandler)
	mux.HandleFunc("/api/stat/lal_info", h.statLalInfoHandler)

	mux.HandleFunc("/api/ctrl/start_relay_pull", h.ctrlStartRelayPullHandler)
	mux.HandleFunc("/api/ctrl/stop_relay_pull", h.ctrlStopRelayPullHandler)
	mux.HandleFunc("/api/ctrl/kick_session", h.ctrlKickSessionHandler)
	mux.HandleFunc("/api/ctrl/start_rtp_pub", h.ctrlStartRtpPubHandler)
	// 所有没有注册路由的走下面这个处理函数
	mux.HandleFunc("/", h.notFoundHandler)

	var srv http.Server
	srv.Handler = mux
	return srv.Serve(h.ln)
}

// TODO chef: dispose

// ---------------------------------------------------------------------------------------------------------------------

func (h *HttpApiServer) statLalInfoHandler(w http.ResponseWriter, req *http.Request) {
	var v base.ApiStatLalInfoResp
	v.ErrorCode = base.ErrorCodeSucc
	v.Desp = base.DespSucc
	v.Data = h.sm.StatLalInfo()
	feedback(v, w)
}

func (h *HttpApiServer) statAllGroupHandler(w http.ResponseWriter, req *http.Request) {
	var v base.ApiStatAllGroupResp
	v.ErrorCode = base.ErrorCodeSucc
	v.Desp = base.DespSucc
	v.Data.Groups = h.sm.StatAllGroup()
	feedback(v, w)
}

func (h *HttpApiServer) statGroupHandler(w http.ResponseWriter, req *http.Request) {
	var v base.ApiStatGroupResp

	q := req.URL.Query()
	streamName := q.Get("stream_name")
	if streamName == "" {
		v.ErrorCode = base.ErrorCodeParamMissing
		v.Desp = base.DespParamMissing
		feedback(v, w)
		return
	}

	v.Data = h.sm.StatGroup(streamName)
	if v.Data == nil {
		v.ErrorCode = base.ErrorCodeGroupNotFound
		v.Desp = base.DespGroupNotFound
		feedback(v, w)
		return
	}

	v.ErrorCode = base.ErrorCodeSucc
	v.Desp = base.DespSucc
	feedback(v, w)
}

// ---------------------------------------------------------------------------------------------------------------------

func (h *HttpApiServer) ctrlStartRelayPullHandler(w http.ResponseWriter, req *http.Request) {
	var v base.ApiCtrlStartRelayPullResp
	var info base.ApiCtrlStartRelayPullReq

	j, err := unmarshalRequestJsonBody(req, &info, "url")
	if err != nil {
		Log.Warnf("http api start pull error. err=%+v", err)
		v.ErrorCode = base.ErrorCodeParamMissing
		v.Desp = base.DespParamMissing
		feedback(v, w)
		return
	}

	if !j.Exist("pull_timeout_ms") {
		info.PullTimeoutMs = DefaultApiCtrlStartRelayPullReqPullTimeoutMs
	}
	if !j.Exist("pull_retry_num") {
		info.PullRetryNum = base.PullRetryNumNever
	}
	if !j.Exist("auto_stop_pull_after_no_out_ms") {
		info.AutoStopPullAfterNoOutMs = base.AutoStopPullAfterNoOutMsNever
	}
	if !j.Exist("rtsp_mode") {
		info.RtspMode = base.RtspModeTcp
	}

	Log.Infof("http api start pull. req info=%+v", info)

	resp := h.sm.CtrlStartRelayPull(info)
	feedback(resp, w)
}

func (h *HttpApiServer) ctrlStopRelayPullHandler(w http.ResponseWriter, req *http.Request) {
	var v base.ApiCtrlStopRelayPullResp

	q := req.URL.Query()
	streamName := q.Get("stream_name")
	if streamName == "" {
		v.ErrorCode = base.ErrorCodeParamMissing
		v.Desp = base.DespParamMissing
		feedback(v, w)
		return
	}

	Log.Infof("http api stop pull. stream_name=%s", streamName)

	resp := h.sm.CtrlStopRelayPull(streamName)
	feedback(resp, w)
}

func (h *HttpApiServer) ctrlKickSessionHandler(w http.ResponseWriter, req *http.Request) {
	var v base.ApiCtrlKickSessionResp
	var info base.ApiCtrlKickSessionReq

	_, err := unmarshalRequestJsonBody(req, &info, "stream_name", "session_id")
	if err != nil {
		Log.Warnf("http api kick session error. err=%+v", err)
		v.ErrorCode = base.ErrorCodeParamMissing
		v.Desp = base.DespParamMissing
		feedback(v, w)
		return
	}

	Log.Infof("http api kick session. req info=%+v", info)

	resp := h.sm.CtrlKickSession(info)
	feedback(resp, w)
}

func (h *HttpApiServer) ctrlStartRtpPubHandler(w http.ResponseWriter, req *http.Request) {
	var v base.ApiCtrlStartRtpPubResp
	var info base.ApiCtrlStartRtpPubReq

	j, err := unmarshalRequestJsonBody(req, &info, "stream_name")
	if err != nil {
		Log.Warnf("http api start rtp pub error. err=%+v", err)
		v.ErrorCode = base.ErrorCodeParamMissing
		v.Desp = base.DespParamMissing
		feedback(v, w)
		return
	}

	if !j.Exist("timeout_ms") {
		info.TimeoutMs = DefaultApiCtrlStartRtpPubReqTimeoutMs
	}
	// 不存在时默认0值的，不需要手动写了
	//if !j.Exist("port") {
	//	info.Port = 0
	//}
	//if !j.Exist("is_tcp_flag") {
	//	info.IsTcpFlag = 0
	//}

	Log.Infof("http api start rtp pub. req info=%+v", info)

	resp := h.sm.CtrlStartRtpPub(info)
	feedback(resp, w)
}

func (h *HttpApiServer) webUIHandler(w http.ResponseWriter, req *http.Request) {
	t, err := template.New("webUI").Parse(webUITpl)
	if err != nil {
		Log.Errorf("invaild html template: %v", err)
		return
	}

	lalInfo := h.sm.StatLalInfo()
	data := map[string]interface{}{
		"ServerID":      lalInfo.ServerId,
		"LalVersion":    lalInfo.LalVersion,
		"ApiVersion":    lalInfo.ApiVersion,
		"NotifyVersion": lalInfo.NotifyVersion,
		"WebUiVersion":  lalInfo.WebUiVersion,
		"StartTime":     lalInfo.StartTime,
	}
	for _, item := range strings.Split(lalInfo.BinInfo, ". ") {
		if index := strings.Index(item, "="); index != -1 {
			k := item[:index]
			v := strings.TrimPrefix(strings.TrimSuffix(item[index:], "."), "=")
			data[k] = v
		}
	}
	t.Execute(w, data)
}

func (h *HttpApiServer) notFoundHandler(w http.ResponseWriter, req *http.Request) {
	Log.Warnf("invalid http-api request. uri=%s, raddr=%s", req.RequestURI, req.RemoteAddr)
	//w.WriteHeader(http.StatusNotFound)
	feedback(base.ApiNotFoundResp, w)
}

// ---------------------------------------------------------------------------------------------------------------------

func feedback(v interface{}, w http.ResponseWriter) {
	resp, _ := json.Marshal(v)
	w.Header().Add("Server", base.LalHttpApiServer)
	base.AddCorsHeaders(w)
	w.Header().Set("Content-Type", "application/json")
	_, _ = w.Write(resp)
}

// unmarshalRequestJsonBody
//
// TODO(chef): [refactor] 搬到naza中 202205
func unmarshalRequestJsonBody(r *http.Request, info interface{}, keyFieldList ...string) (nazajson.Json, error) {
	body, err := io.ReadAll(r.Body)
	if err != nil {
		return nazajson.Json{}, err
	}

	j, err := nazajson.New(body)
	if err != nil {
		return j, err
	}
	for _, kf := range keyFieldList {
		if !j.Exist(kf) {
			return j, nazahttp.ErrParamMissing
		}
	}

	return j, json.Unmarshal(body, info)
}
