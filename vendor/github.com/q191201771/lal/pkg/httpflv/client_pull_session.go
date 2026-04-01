// Copyright 2019, Chef.  All rights reserved.
// https://github.com/q191201771/lal
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

package httpflv

import (
	"context"
	"crypto/tls"
	"fmt"
	"net"
	"net/http"
	"sync"
	"time"

	"github.com/q191201771/lal/pkg/base"

	"github.com/q191201771/naza/pkg/nazahttp"

	"github.com/q191201771/naza/pkg/connection"
)

type PullSessionOption struct {
	// 从调用Pull函数，到接收音视频数据的前一步，也即发送完HTTP请求的超时时间
	// 如果为0，则没有超时时间
	PullTimeoutMs int

	ReadTimeoutMs int // 接收数据超时，单位毫秒，如果为0，则不设置超时
}

var defaultPullSessionOption = PullSessionOption{
	PullTimeoutMs: 10000,
	ReadTimeoutMs: 0,
}

type PullSession struct {
	option PullSessionOption // const after ctor

	conn        connection.Connection
	sessionStat base.BasicSessionStat

	urlCtx base.UrlContext

	disposeOnce sync.Once
}

type ModPullSessionOption func(option *PullSessionOption)

func NewPullSession(modOptions ...ModPullSessionOption) *PullSession {
	option := defaultPullSessionOption
	for _, fn := range modOptions {
		fn(&option)
	}

	s := &PullSession{
		option:      option,
		sessionStat: base.NewBasicSessionStat(base.SessionTypeFlvPull, ""),
	}
	Log.Infof("[%s] lifecycle new httpflv PullSession. session=%p", s.UniqueKey(), s)
	return s
}

// OnReadFlvTag @param tag: 底层保证回调上来的Raw数据长度是完整的（但是不会分析Raw内部的编码数据）
type OnReadFlvTag func(tag Tag)

// Pull 阻塞直到和对端完成拉流前，握手部分的工作，或者发生错误。
//
// 注意，握手指的是发送完HTTP Request，不包含接收任何数据，因为有的httpflv服务端，如果流不存在不会发送任何内容，此时我们也应该认为是握手完成了。
//
// @param rawUrl 支持如下两种格式（当然，关键点是对端支持）：
//  1. `http://{domain}/{app_name}/{stream_name}.flv`
//  2. `http://{ip}/{domain}/{app_name}/{stream_name}.flv`
//
// @param onReadFlvTag 读取到 flv tag 数据时回调。回调结束后，PullSession 不会再使用这块 <tag> 数据。
func (session *PullSession) Pull(rawUrl string, onReadFlvTag OnReadFlvTag) error {
	Log.Debugf("[%s] pull. url=%s", session.UniqueKey(), rawUrl)

	var (
		ctx    context.Context
		cancel context.CancelFunc
	)
	if session.option.PullTimeoutMs == 0 {
		ctx, cancel = context.WithCancel(context.Background())
	} else {
		ctx, cancel = context.WithTimeout(context.Background(), time.Duration(session.option.PullTimeoutMs)*time.Millisecond)
	}
	defer cancel()
	return session.pullContext(ctx, rawUrl, onReadFlvTag)
}

// ---------------------------------------------------------------------------------------------------------------------
// IClientSessionLifecycle interface
// ---------------------------------------------------------------------------------------------------------------------

// Dispose 文档请参考： IClientSessionLifecycle interface
func (session *PullSession) Dispose() error {
	return session.dispose(nil)
}

// WaitChan 文档请参考： IClientSessionLifecycle interface
func (session *PullSession) WaitChan() <-chan error {
	return session.conn.Done()
}

// ---------------------------------------------------------------------------------------------------------------------

// Url 文档请参考： interface ISessionUrlContext
func (session *PullSession) Url() string {
	return session.urlCtx.Url
}

// AppName 文档请参考： interface ISessionUrlContext
func (session *PullSession) AppName() string {
	return session.urlCtx.PathWithoutLastItem
}

// StreamName 文档请参考： interface ISessionUrlContext
func (session *PullSession) StreamName() string {
	return session.urlCtx.LastItemOfPath
}

// RawQuery 文档请参考： interface ISessionUrlContext
func (session *PullSession) RawQuery() string {
	return session.urlCtx.RawQuery
}

// UniqueKey 文档请参考： interface IObject
func (session *PullSession) UniqueKey() string {
	return session.sessionStat.UniqueKey()
}

// ----- ISessionStat --------------------------------------------------------------------------------------------------

// UpdateStat 文档请参考： interface ISessionStat
func (session *PullSession) UpdateStat(intervalSec uint32) {
	session.sessionStat.UpdateStatWitchConn(session.conn, intervalSec)
}

// GetStat 文档请参考： interface ISessionStat
func (session *PullSession) GetStat() base.StatSession {
	return session.sessionStat.GetStatWithConn(session.conn)
}

// IsAlive 文档请参考： interface ISessionStat
func (session *PullSession) IsAlive() (readAlive, writeAlive bool) {
	return session.sessionStat.IsAliveWitchConn(session.conn)
}

// ---------------------------------------------------------------------------------------------------------------------

func (session *PullSession) pullContext(ctx context.Context, rawUrl string, onReadFlvTag OnReadFlvTag) error {
	errChan := make(chan error, 1)
	url := rawUrl

	// 异步握手
	go func() {
		for {
			if err := session.connect(url); err != nil {
				errChan <- err
				return
			}
			if err := session.writeHttpRequest(); err != nil {
				errChan <- err
				return
			}

			statusCode, headers, err := session.readHttpRespHeader()
			if err != nil {
				errChan <- err
				return
			}

			// 处理跳转
			if statusCode == "301" || statusCode == "302" {
				url = headers.Get("Location")
				if url == "" {
					Log.Warnf("[%s] redirect but Location not found. headers=%+v", session.UniqueKey(), headers)
					errChan <- nil
					return
				}

				_ = session.conn.Close()
				Log.Debugf("[%s] redirect to %s", session.UniqueKey(), url)
				continue
			}

			errChan <- nil
			return
		}
	}()

	// 等待握手结果，或者超时通知
	select {
	case <-ctx.Done():
		// 注意，如果超时，可能连接已经建立了，要dispose避免泄漏
		_ = session.dispose(nil)
		return ctx.Err()
	case err := <-errChan:
		// 握手消息，不为nil则握手失败
		if err != nil {
			_ = session.dispose(err)
			return err
		}
	}

	// 握手成功，开启收数据协程
	go session.runReadLoop(onReadFlvTag)
	return nil
}

func (session *PullSession) connect(rawUrl string) (err error) {
	// TODO(chef): refactor 可以考虑抽象出一个http client，负责http拉流的建连、https、302等功能

	session.urlCtx, err = base.ParseHttpflvUrl(rawUrl)
	if err != nil {
		return
	}

	session.sessionStat.SetRemoteAddr(session.urlCtx.HostWithPort)

	Log.Debugf("[%s] > tcp connect. %s", session.UniqueKey(), session.urlCtx.HostWithPort)

	var conn net.Conn
	if session.urlCtx.Scheme == "https" {
		conf := &tls.Config{
			InsecureSkipVerify: true,
		}
		conn, err = tls.Dial("tcp", session.urlCtx.HostWithPort, conf)
	} else {
		conn, err = net.Dial("tcp", session.urlCtx.HostWithPort)
	}

	if err != nil {
		return err
	}

	Log.Debugf("[%s] tcp connect succ. remote=%s", session.UniqueKey(), conn.RemoteAddr().String())

	session.conn = connection.New(conn, func(option *connection.Option) {
		option.ReadBufSize = readBufSize
		option.WriteTimeoutMs = session.option.ReadTimeoutMs // TODO chef: 为什么是 Read 赋值给 Write
		option.ReadTimeoutMs = session.option.ReadTimeoutMs
	})
	return nil
}

func (session *PullSession) writeHttpRequest() error {
	// # 发送 http GET 请求
	Log.Debugf("[%s] > W http request. GET %s", session.UniqueKey(), session.urlCtx.PathWithRawQuery)
	req := fmt.Sprintf("GET %s HTTP/1.0\r\nUser-Agent: %s\r\nAccept: */*\r\nRange: byte=0-\r\nConnection: close\r\nHost: %s\r\nIcy-MetaData: 1\r\n\r\n",
		session.urlCtx.PathWithRawQuery, base.LalHttpflvPullSessionUa, session.urlCtx.StdHost)
	_, err := session.conn.Write([]byte(req))
	return err
}

func (session *PullSession) readHttpRespHeader() (statusCode string, headers http.Header, err error) {
	var statusLine string
	if statusLine, headers, err = nazahttp.ReadHttpHeader(session.conn); err != nil {
		return
	}
	_, statusCode, _, err = nazahttp.ParseHttpStatusLine(statusLine)
	if err != nil {
		return
	}

	Log.Debugf("[%s] < R http response header. statusLine=%s", session.UniqueKey(), statusLine)
	return
}

func (session *PullSession) readFlvHeader() ([]byte, error) {
	flvHeader := make([]byte, flvHeaderSize)
	_, err := session.conn.ReadAtLeast(flvHeader, flvHeaderSize)
	if err != nil {
		return flvHeader, err
	}
	Log.Debugf("[%s] < R http flv header.", session.UniqueKey())

	// TODO chef: check flv header's value
	return flvHeader, nil
}

func (session *PullSession) readTag() (Tag, error) {
	return ReadTag(session.conn)
}

func (session *PullSession) runReadLoop(onReadFlvTag OnReadFlvTag) {
	var err error
	defer func() {
		_ = session.dispose(err)
	}()

	if _, err = session.readFlvHeader(); err != nil {
		return
	}

	for {
		var tag Tag
		tag, err = session.readTag()
		if err != nil {
			return
		}
		onReadFlvTag(tag)
	}
}

func (session *PullSession) dispose(err error) error {
	var retErr error
	session.disposeOnce.Do(func() {
		Log.Infof("[%s] lifecycle dispose httpflv PullSession. err=%+v", session.UniqueKey(), err)
		if session.conn == nil {
			retErr = base.ErrSessionNotStarted
			return
		}
		retErr = session.conn.Close()
	})
	return retErr
}
