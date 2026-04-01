// Copyright 2021, Chef.  All rights reserved.
// https://github.com/q191201771/lal
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

package rtsp

import (
	"bufio"
	"context"
	"fmt"
	"net"
	"strings"
	"sync"
	"time"

	"github.com/q191201771/naza/pkg/nazaerrors"

	"github.com/q191201771/lal/pkg/base"
	"github.com/q191201771/lal/pkg/rtprtcp"
	"github.com/q191201771/lal/pkg/sdp"
	"github.com/q191201771/naza/pkg/connection"
	"github.com/q191201771/naza/pkg/nazahttp"
	"github.com/q191201771/naza/pkg/nazanet"
)

type ClientCommandSessionType int

const (
	readBufSize                 = 256
	writeGetParameterIntervalMs = 10000
)

const (
	CcstPullSession ClientCommandSessionType = iota
	CcstPushSession
)

type ClientCommandSessionOption struct {
	DoTimeoutMs int
	OverTcp     bool
}

var defaultClientCommandSessionOption = ClientCommandSessionOption{
	DoTimeoutMs: 10000,
	OverTcp:     false,
}

type IClientCommandSessionObserver interface {
	OnConnectResult()

	// OnDescribeResponse only for PullSession
	OnDescribeResponse(sdpCtx sdp.LogicContext)

	OnSetupWithConn(uri string, rtpConn, rtcpConn *nazanet.UdpConnection)
	OnSetupWithChannel(uri string, rtpChannel, rtcpChannel int)
	OnSetupResult()

	OnInterleavedPacket(packet []byte, channel int)
}

// ClientCommandSession Push和Pull共用，封装了客户端底层信令信令部分。
// 业务方应该使用PushSession和PullSession，而不是直接使用ClientCommandSession，除非你确定要这么做。
type ClientCommandSession struct {
	uniqueKey string
	t         ClientCommandSessionType
	observer  IClientCommandSessionObserver
	option    ClientCommandSessionOption

	rawUrl string
	urlCtx base.UrlContext
	conn   connection.Connection

	cseq                        int
	methodGetParameterSupported bool
	auth                        Auth

	sdpCtx sdp.LogicContext

	sessionId string
	channel   int

	disposeOnce sync.Once
}

type ModClientCommandSessionOption func(option *ClientCommandSessionOption)

func NewClientCommandSession(t ClientCommandSessionType, uniqueKey string, observer IClientCommandSessionObserver, modOptions ...ModClientCommandSessionOption) *ClientCommandSession {
	option := defaultClientCommandSessionOption
	for _, fn := range modOptions {
		fn(&option)
	}
	s := &ClientCommandSession{
		t:         t,
		uniqueKey: uniqueKey,
		observer:  observer,
		option:    option,
	}
	Log.Infof("[%s] lifecycle new rtsp ClientCommandSession. session=%p", uniqueKey, s)
	return s
}

// InitWithSdp only for PushSession
func (session *ClientCommandSession) InitWithSdp(sdpCtx sdp.LogicContext) {
	session.sdpCtx = sdpCtx
}

func (session *ClientCommandSession) Do(rawUrl string) error {
	var (
		ctx    context.Context
		cancel context.CancelFunc
	)
	if session.option.DoTimeoutMs == 0 {
		ctx, cancel = context.WithCancel(context.Background())
	} else {
		ctx, cancel = context.WithTimeout(context.Background(), time.Duration(session.option.DoTimeoutMs)*time.Millisecond)
	}
	defer cancel()
	return session.doContext(ctx, rawUrl)
}

// ---------------------------------------------------------------------------------------------------------------------
// IClientSessionLifecycle interface
// ---------------------------------------------------------------------------------------------------------------------

// Dispose 文档请参考： IClientSessionLifecycle interface
func (session *ClientCommandSession) Dispose() error {
	return session.dispose(nil)
}

// WaitChan 文档请参考： IClientSessionLifecycle interface
func (session *ClientCommandSession) WaitChan() <-chan error {
	return session.conn.Done()
}

// ---------------------------------------------------------------------------------------------------------------------

func (session *ClientCommandSession) WriteInterleavedPacket(packet []byte, channel int) error {
	if session.conn == nil {
		return base.ErrSessionNotStarted
	}
	_, err := session.conn.Write(packInterleaved(channel, packet))
	return err
}

func (session *ClientCommandSession) RemoteAddr() string {
	if session.conn == nil {
		return ""
	}
	return session.conn.RemoteAddr().String()
}

func (session *ClientCommandSession) Url() string {
	return session.urlCtx.Url
}

func (session *ClientCommandSession) AppName() string {
	return session.urlCtx.PathWithoutLastItem
}

func (session *ClientCommandSession) StreamName() string {
	return session.urlCtx.LastItemOfPath
}

func (session *ClientCommandSession) RawQuery() string {
	return session.urlCtx.RawQuery
}

func (session *ClientCommandSession) UniqueKey() string {
	return session.uniqueKey
}

func (session *ClientCommandSession) doContext(ctx context.Context, rawUrl string) error {
	errChan := make(chan error, 1)

	go func() {
		if err := session.connect(rawUrl); err != nil {
			errChan <- err
			return
		}

		if err := session.writeOptions(); err != nil {
			errChan <- err
			return
		}

		switch session.t {
		case CcstPullSession:
			if err := session.writeDescribe(); err != nil {
				errChan <- err
				return
			}

			if err := session.writeSetup(); err != nil {
				errChan <- err
				return
			}
			session.observer.OnSetupResult()

			if err := session.writePlay(); err != nil {
				errChan <- err
				return
			}
		case CcstPushSession:
			if err := session.writeAnnounce(); err != nil {
				errChan <- err
				return
			}

			if err := session.writeSetup(); err != nil {
				errChan <- err
				return
			}
			session.observer.OnSetupResult()

			if err := session.writeRecord(); err != nil {
				errChan <- err
				return
			}
		}

		errChan <- nil
	}()

	select {
	case <-ctx.Done():
		_ = session.dispose(ctx.Err())
		return ctx.Err()
	case err := <-errChan:
		if err != nil {
			_ = session.dispose(err)
			return err
		}
	}

	go session.runReadLoop()
	return nil
}

func (session *ClientCommandSession) runReadLoop() {
	var loopErr error
	defer func() {
		_ = session.dispose(loopErr)
	}()

	if !session.methodGetParameterSupported {
		// TCP模式，需要收取数据进行处理
		if session.option.OverTcp {
			var r = bufio.NewReader(session.conn)
			for {
				isInterleaved, packet, channel, err := readInterleaved(r)
				if err != nil {
					loopErr = err
					return
				}
				if isInterleaved {
					session.observer.OnInterleavedPacket(packet, int(channel))
				}
			}
		}

		// not over tcp
		// 接收TCP对端关闭FIN信号
		dummy := make([]byte, 1)
		_, err := session.conn.Read(dummy)
		loopErr = err
		return
	}

	// 对端支持get_parameter，需要定时向对端发送get_parameter进行保活

	Log.Debugf("[%s] start get_parameter timer.", session.uniqueKey)
	var r = bufio.NewReader(session.conn)
	t := time.NewTicker(writeGetParameterIntervalMs * time.Millisecond)
	defer t.Stop()

	if session.option.OverTcp {
		for {
			select {
			case <-t.C:
				session.cseq++
				if err := session.writeCmd(MethodGetParameter, session.urlCtx.RawUrlWithoutUserInfo, nil, ""); err != nil {
					loopErr = err
					return
				}
			default:
				// noop
			}

			isInterleaved, packet, channel, err := readInterleaved(r)
			if err != nil {
				loopErr = err
				return
			}
			if isInterleaved {
				session.observer.OnInterleavedPacket(packet, int(channel))
			} else {
				if _, err := nazahttp.ReadHttpResponseMessage(r); err != nil {
					loopErr = err
					return
				}
			}
		}
	}

	// not over tcp
	for {
		select {
		case <-t.C:
			session.cseq++
			if _, err := session.writeCmdReadResp(MethodGetParameter, session.urlCtx.RawUrlWithoutUserInfo, nil, ""); err != nil {
				loopErr = err
				return
			}
		}

	}
}

func (session *ClientCommandSession) connect(rawUrl string) (err error) {
	session.rawUrl = rawUrl

	session.urlCtx, err = base.ParseRtspUrl(rawUrl)
	if err != nil {
		return err
	}

	Log.Debugf("[%s] > tcp connect.", session.uniqueKey)

	// # 建立连接
	conn, err := net.Dial("tcp", session.urlCtx.HostWithPort)
	if err != nil {
		return err
	}
	session.conn = connection.New(conn, func(option *connection.Option) {
		option.ReadBufSize = readBufSize
	})
	Log.Debugf("[%s] < tcp connect. laddr=%s, raddr=%s", session.uniqueKey, conn.LocalAddr().String(), conn.RemoteAddr().String())

	session.observer.OnConnectResult()
	return nil
}
func (session *ClientCommandSession) writeOptions() error {
	ctx, err := session.writeCmdReadResp(MethodOptions, session.urlCtx.RawUrlWithoutUserInfo, nil, "")
	if err != nil {
		return err
	}

	method := ctx.Headers.Get(HeaderPublic)

	if method == "" {
		return nil
	}
	if strings.Contains(method, MethodGetParameter) {
		session.methodGetParameterSupported = true
	}
	return nil
}

func (session *ClientCommandSession) writeDescribe() error {
	headers := map[string]string{
		HeaderAccept: HeaderAcceptApplicationSdp,
	}
	ctx, err := session.writeCmdReadResp(MethodDescribe, session.urlCtx.RawUrlWithoutUserInfo, headers, "")
	if err != nil {
		return err
	}

	sdpCtx, err := sdp.ParseSdp2LogicContext(ctx.Body)
	if err != nil {
		return err
	}
	session.sdpCtx = sdpCtx
	session.observer.OnDescribeResponse(session.sdpCtx)
	return nil
}

func (session *ClientCommandSession) writeAnnounce() error {
	headers := map[string]string{
		HeaderAccept: HeaderAcceptApplicationSdp,
	}
	_, err := session.writeCmdReadResp(MethodAnnounce, session.urlCtx.RawUrlWithoutUserInfo, headers, string(session.sdpCtx.RawSdp))
	return err
}

func (session *ClientCommandSession) writeSetup() error {
	setup := func(setupUri string) error {
		if session.option.OverTcp {
			if err := session.writeOneSetupTcp(setupUri); err != nil {
				// 461情况下尝试切换UDP重试
				if err == base.ErrRtspUnsupportedTransport {
					if err := session.writeOneSetup(setupUri); err != nil {
						return err
					}

					session.option.OverTcp = false
				} else {
					return err
				}
			}
		} else {
			if err := session.writeOneSetup(setupUri); err != nil {
				// 461情况尝试切换TCP重试
				if err == base.ErrRtspUnsupportedTransport {
					if err = session.writeOneSetupTcp(setupUri); err != nil {
						return err
					}

					session.option.OverTcp = true
				} else {
					return err
				}
			}
		}

		return nil
	}

	if session.sdpCtx.HasVideoAControl() {
		uri := session.sdpCtx.MakeVideoSetupUri(session.urlCtx.RawUrlWithoutUserInfo)
		if err := setup(uri); err != nil {
			return err
		}
	}
	// can't else if
	if session.sdpCtx.HasAudioAControl() {
		uri := session.sdpCtx.MakeAudioSetupUri(session.urlCtx.RawUrlWithoutUserInfo)
		if err := setup(uri); err != nil {
			return err
		}
	}
	return nil
}

func (session *ClientCommandSession) writeOneSetup(setupUri string) error {
	rtpC, lRtpPort, rtcpC, lRtcpPort, err := availUdpConnPool.Acquire2()
	if err != nil {
		return err
	}

	var htv string
	switch session.t {
	case CcstPushSession:
		htv = fmt.Sprintf(HeaderTransportClientRecordTmpl, lRtpPort, lRtcpPort)
	case CcstPullSession:
		htv = fmt.Sprintf(HeaderTransportClientPlayTmpl, lRtpPort, lRtcpPort)
	}
	headers := map[string]string{
		HeaderTransport: htv,
	}
	ctx, err := session.writeCmdReadResp(MethodSetup, setupUri, headers, "")
	if err != nil {
		return err
	}

	if ctx.StatusCode == "461" {
		// 切换transport尝试继续
		err = base.ErrRtspUnsupportedTransport
		return err
	}

	session.sessionId = strings.Split(ctx.Headers.Get(HeaderSession), ";")[0]

	rRtpPort, rRtcpPort, err := parseServerPort(ctx.Headers.Get(HeaderTransport))
	var rtpRAddr, rtcpRAddr string
	if err != nil {
		// 增强兼容性逻辑
		// 有用户反馈，存在对端不返回server_port的情况，对端是easydrawin
		// 其实在pull的情况下，没有对端端口也可以，因为不发数据，或者需要发送时，使用接收时获取到的对端地址即可

		Log.Warnf("[%s] init conn, parseServerPort failed. lRtpPort=%d, lRtcpPort=%d",
			session.uniqueKey, lRtpPort, lRtcpPort)

		switch session.t {
		case CcstPullSession:
			// noop
		case CcstPushSession:
			fallthrough
		default:
			return err
		}
	} else {
		rtpRAddr = net.JoinHostPort(session.urlCtx.Host, fmt.Sprintf("%d", rRtpPort))
		rtcpRAddr = net.JoinHostPort(session.urlCtx.Host, fmt.Sprintf("%d", rRtcpPort))
	}

	Log.Debugf("[%s] init conn. lRtpPort=%d, lRtcpPort=%d, rRtpPort=%d, rRtcpPort=%d",
		session.uniqueKey, lRtpPort, lRtcpPort, rRtpPort, rRtcpPort)

	rtpConn, err := nazanet.NewUdpConnection(func(option *nazanet.UdpConnectionOption) {
		option.Conn = rtpC
		option.RAddr = rtpRAddr
		option.MaxReadPacketSize = rtprtcp.MaxRtpRtcpPacketSize
	})
	if err != nil {
		return err
	}

	rtcpConn, err := nazanet.NewUdpConnection(func(option *nazanet.UdpConnectionOption) {
		option.Conn = rtcpC
		option.RAddr = rtcpRAddr
		option.MaxReadPacketSize = rtprtcp.MaxRtpRtcpPacketSize
	})
	if err != nil {
		return err
	}

	session.observer.OnSetupWithConn(setupUri, rtpConn, rtcpConn)
	return nil
}

func (session *ClientCommandSession) writeOneSetupTcp(setupUri string) error {
	rtpChannel := session.channel
	rtcpChannel := session.channel + 1
	session.channel += 2

	var htv string
	switch session.t {
	case CcstPushSession:
		htv = fmt.Sprintf(HeaderTransportClientRecordTcpTmpl, rtpChannel, rtcpChannel)
	case CcstPullSession:
		htv = fmt.Sprintf(HeaderTransportClientPlayTcpTmpl, rtpChannel, rtcpChannel)
	}
	headers := map[string]string{
		HeaderTransport: htv,
	}
	ctx, err := session.writeCmdReadResp(MethodSetup, setupUri, headers, "")
	if err != nil {
		return err
	}

	if ctx.StatusCode == "461" {
		err = base.ErrRtspUnsupportedTransport
		return err
	}

	session.sessionId = strings.Split(ctx.Headers.Get(HeaderSession), ";")[0]

	// TODO chef: 这里没有解析回传的channel id了，因为我假定了它和request中的是一致的
	session.observer.OnSetupWithChannel(setupUri, rtpChannel, rtcpChannel)
	return nil
}

func (session *ClientCommandSession) writePlay() error {
	headers := map[string]string{
		HeaderRange: HeaderRangeDefault,
	}
	_, err := session.writeCmdReadResp(MethodPlay, session.urlCtx.RawUrlWithoutUserInfo, headers, "")
	return err
}

func (session *ClientCommandSession) writeRecord() error {
	headers := map[string]string{
		HeaderRange: HeaderRangeDefault,
	}
	_, err := session.writeCmdReadResp(MethodRecord, session.urlCtx.RawUrlWithoutUserInfo, headers, "")
	return err
}

func (session *ClientCommandSession) writeCmd(method, uri string, headers map[string]string, body string) error {
	session.cseq++
	if headers == nil {
		headers = make(map[string]string)
	}
	headers[HeaderCSeq] = fmt.Sprintf("%d", session.cseq)
	headers[HeaderUserAgent] = base.LalRtspPullSessionUa
	if body != "" {
		headers[HeaderContentLength] = fmt.Sprintf("%d", len(body))
	}

	// 鉴权时固定用RawUrlWithoutUserInfo
	auth := session.auth.MakeAuthorization(method, session.urlCtx.RawUrlWithoutUserInfo)
	if auth != "" {
		headers[HeaderAuthorization] = auth
	}

	if session.sessionId != "" {
		headers[HeaderSession] = session.sessionId
	}

	req := PackRequest(method, uri, headers, body)
	//Log.Debugf("[%s] > write %s.", session.uniqueKey, method)
	Log.Debugf("[%s] > write %s. req=%s", session.uniqueKey, method, req)
	_, err := session.conn.Write([]byte(req))
	return err
}

// @param headers 可以为nil
// @param body 可以为空
func (session *ClientCommandSession) writeCmdReadResp(method, uri string, headers map[string]string, body string) (ctx nazahttp.HttpRespMsgCtx, err error) {
	for i := 0; i < 2; i++ {
		if err = session.writeCmd(method, uri, headers, body); err != nil {
			return
		}

		ctx, err = nazahttp.ReadHttpResponseMessage(session.conn)
		if err != nil {
			return
		}
		Log.Debugf("[%s] < read response. version=%s, code=%s, reason=%s, headers=%+v, body=%s",
			session.uniqueKey, ctx.Version, ctx.StatusCode, ctx.Reason, ctx.Headers, string(ctx.Body))

		if ctx.StatusCode != "401" {
			return
		}

		session.auth.FeedWwwAuthenticate(ctx.Headers.Values(HeaderWwwAuthenticate), session.urlCtx.Username, session.urlCtx.Password)
	}

	err = nazaerrors.Wrap(base.ErrRtsp)
	return
}

func (session *ClientCommandSession) dispose(err error) error {
	var retErr error
	session.disposeOnce.Do(func() {
		Log.Infof("[%s] lifecycle dispose rtsp ClientCommandSession. session=%p, err=%+v", session.uniqueKey, session, err)
		if session.conn == nil {
			retErr = base.ErrSessionNotStarted
			return
		}
		retErr = session.conn.Close()
	})
	return retErr
}
