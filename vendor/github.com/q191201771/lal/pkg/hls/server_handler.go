// Copyright 2020, Chef.  All rights reserved.
// https://github.com/q191201771/lal
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

package hls

import (
	"bytes"
	"errors"
	"fmt"
	"net/http"
	"net/url"
	"strings"
	"sync"
	"time"

	"github.com/q191201771/lal/pkg/base"
)

type IHlsServerHandlerObserver interface {
	OnNewHlsSubSession(session *SubSession) error
	OnDelHlsSubSession(session *SubSession)
}

type ServerHandler struct {
	outPath           string
	observer          IHlsServerHandlerObserver
	urlPattern        string
	sessionMap        map[string]*SubSession
	mutex             sync.Mutex
	subSessionTimeout time.Duration
	subSessionHashKey string
}

func NewServerHandler(outPath, urlPattern, subSessionHashKey string, subSessionTimeoutMs int, observer IHlsServerHandlerObserver) *ServerHandler {
	if strings.HasPrefix(urlPattern, "/") {
		urlPattern = urlPattern[1:]
	}
	sh := &ServerHandler{
		outPath:           outPath,
		observer:          observer,
		urlPattern:        urlPattern,
		sessionMap:        make(map[string]*SubSession),
		subSessionTimeout: time.Duration(subSessionTimeoutMs) * time.Millisecond,
		subSessionHashKey: subSessionHashKey,
	}
	go sh.runLoop()
	return sh
}

func (s *ServerHandler) ServeHTTP(resp http.ResponseWriter, req *http.Request) {
	urlCtx, err := base.ParseUrl(base.ParseHttpRequest(req), 80)
	if err != nil {
		Log.Errorf("parse url. err=%+v", err)
		return
	}

	s.ServeHTTPWithUrlCtx(resp, req, urlCtx)
}

func (s *ServerHandler) ServeHTTPWithUrlCtx(resp http.ResponseWriter, req *http.Request, urlCtx base.UrlContext) {
	//Log.Debugf("%+v", req)

	var sessionIdHash string
	var err error

	urlObj, _ := url.Parse(urlCtx.Url)

	// TODO chef:
	// - check appname in URI path

	filename := urlCtx.LastItemOfPath
	filetype := urlCtx.GetFileType()

	// 如果开启了hls sub session功能
	if s.isSubSessionModeEnable() {
		sessionIdHash = urlObj.Query().Get("session_id")
		if filetype == "ts" && sessionIdHash != "" {
			// 注意，为了增强容错性，不管是session_id字段无效，还是session_id为空，我们都依然返回ts文件内容给播放端
			if sessionIdHash != "" {
				err = s.keepSessionAlive(sessionIdHash)
				if err != nil {
					Log.Warnf("keepSessionAlive failed. session=%s, err=%+v", sessionIdHash, err)
				}
			} else {
				// noop
			}
		} else if filetype == "m3u8" {
			if sessionIdHash != "" {
				err = s.keepSessionAlive(sessionIdHash)
				if err != nil {
					Log.Warnf("keepSessionAlive failed. session=%s, err=%+v", sessionIdHash, err)
				}
			} else {
				// m3u8请求时，session_id不存在，创建session对象，并让m3u8跳转到携带session_id的url请求

				session, err := s.createSubSession(req, urlCtx)
				if err != nil {
					resp.WriteHeader(http.StatusNotFound)
					return
				}

				query := urlObj.Query()
				query.Set("session_id", session.sessionIdHash)
				redirectUrl := urlObj.Path + "?" + query.Encode()
				resp.Header().Add("Cache-Control", "no-cache")
				base.AddCorsHeaders2HlsIfNeeded(resp)
				http.Redirect(resp, req, redirectUrl, http.StatusFound)
				return
			}
		}
	}

	ri := PathStrategy.GetRequestInfo(urlCtx, s.outPath)
	//Log.Debugf("%+v", ri)

	if filename == "" || (filetype != "m3u8" && filetype != "ts") || ri.StreamName == "" || ri.FileNameWithPath == "" {
		err = errors.New(fmt.Sprintf("invalid hls request. url=%+v, request=%+v", urlCtx, ri))
		Log.Warnf(err.Error())
		resp.WriteHeader(http.StatusFound)
		return
	}

	content, _err := ReadFile(ri.FileNameWithPath)
	if _err != nil {
		err = errors.New(fmt.Sprintf("read hls file failed. request=%+v, err=%+v", ri, _err))
		Log.Warnf(err.Error())
		resp.WriteHeader(http.StatusNotFound)
		return
	}

	switch filetype {
	case "m3u8":
		resp.Header().Add("Content-Type", "application/x-mpegurl")
		resp.Header().Add("Server", base.LalHlsM3u8Server)
		// 给ts文件都携带上session_id字段
		if sessionIdHash != "" {
			content = bytes.ReplaceAll(content, []byte(".ts"), []byte(".ts?session_id="+sessionIdHash))
		}
	case "ts":
		resp.Header().Add("Content-Type", "video/mp2t")
		resp.Header().Add("Server", base.LalHlsTsServer)
	}
	resp.Header().Add("Cache-Control", "no-cache")
	base.AddCorsHeaders2HlsIfNeeded(resp)

	if sessionIdHash != "" {
		session := s.getSubSession(sessionIdHash)
		if session != nil {
			session.AddWroteBytesSum(uint64(len(content)))
		}
	}

	_, _ = resp.Write(content)
	return
}

// getSubSession 获取 SubSession，如果不存在，返回nil
func (s *ServerHandler) getSubSession(sessionIdHash string) *SubSession {
	s.mutex.Lock()
	defer s.mutex.Unlock()
	return s.sessionMap[sessionIdHash]
}

func (s *ServerHandler) createSubSession(req *http.Request, urlCtx base.UrlContext) (*SubSession, error) {
	s.mutex.Lock()
	defer s.mutex.Unlock()
	session := NewSubSession(req, urlCtx, s.urlPattern, s.subSessionHashKey, s.subSessionTimeout)
	s.sessionMap[session.sessionIdHash] = session
	if err := s.observer.OnNewHlsSubSession(session); err != nil {
		delete(s.sessionMap, session.sessionIdHash)
		return nil, err
	}
	return session, nil
}

// keepSessionAlive 标记延长session存活时间，如果session不存在，返回 base.ErrHlsSessionNotFound
func (s *ServerHandler) keepSessionAlive(sessionIdHash string) error {
	s.mutex.Lock()
	defer s.mutex.Unlock()
	session := s.sessionMap[sessionIdHash]
	if session == nil {
		return base.ErrHlsSessionNotFound
	}
	session.KeepAlive()
	return nil
}

func (s *ServerHandler) clearExpireSession() {
	s.mutex.Lock()
	defer s.mutex.Unlock()
	for sessionIdHash, session := range s.sessionMap {
		if session.IsExpired() {
			delete(s.sessionMap, sessionIdHash)
			s.observer.OnDelHlsSubSession(session)
		}
	}
}

func (s *ServerHandler) isSubSessionModeEnable() bool {
	return s.subSessionHashKey != ""
}

func (s *ServerHandler) runLoop() {
	// TODO(chef): [refactor] 也许可以弄到group中管理超时，和其他协议的session管理方式保持一致 202211
	ticker := time.NewTicker(1 * time.Second)
	for range ticker.C {
		s.clearExpireSession()
	}
}

// m3u8文件用这个也行
//resp.Header().Add("Content-Type", "application/vnd.apple.mpegurl")
