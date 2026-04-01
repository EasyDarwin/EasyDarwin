// Copyright 2022, Chef.  All rights reserved.
// https://github.com/q191201771/lal
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

package gb28181

import (
	"fmt"
	"github.com/q191201771/lal/pkg/base"
	"github.com/q191201771/naza/pkg/bele"
	"github.com/q191201771/naza/pkg/nazabytes"
	"github.com/q191201771/naza/pkg/nazalog"
	"github.com/q191201771/naza/pkg/nazanet"
	"io"
	"net"
	"sync"
)

type OnReadPacket func(b []byte)

type PubSession struct {
	unpacker *PsUnpacker

	streamName string

	hookOnReadPacket OnReadPacket

	isTcpFlag bool

	disposeOnce sync.Once
	udpConn     *nazanet.UdpConnection
	listener    net.Listener
	tcpConn     net.Conn
	sessionStat base.BasicSessionStat
}

func NewPubSession() *PubSession {
	return &PubSession{
		unpacker:    NewPsUnpacker(),
		sessionStat: base.NewBasicSessionStat(base.SessionTypePsPub, ""),
	}
}

// WithOnAvPacket 设置音视频的回调。
//
//	@param onAvPacket: 见 PsUnpacker.WithOnAvPacket 的注释
func (session *PubSession) WithOnAvPacket(onAvPacket base.OnAvPacketFunc) *PubSession {
	session.unpacker.WithOnAvPacket(onAvPacket)
	return session
}

func (session *PubSession) WithStreamName(streamName string) *PubSession {
	session.streamName = streamName
	return session
}

// WithHookReadPacket
//
// 将接收的数据返回给上层。
// 注意，底层的解析逻辑依然走。
// 可以用这个方式来截取数据进行调试。
func (session *PubSession) WithHookReadPacket(fn OnReadPacket) *PubSession {
	session.hookOnReadPacket = fn
	return session
}

// Listen 非阻塞函数
//
// 注意，当`port`参数为0时，内部会自动选择一个可用端口监听，并通过返回值返回该端口
func (session *PubSession) Listen(port int, isTcpFlag bool) (int, error) {
	session.isTcpFlag = isTcpFlag

	if isTcpFlag {
		return session.listenTcp(port)
	}
	return session.listenUdp(port)
}

// RunLoop 阻塞函数
func (session *PubSession) RunLoop() error {
	if session.isTcpFlag {
		return session.runLoopTcp()
	}
	return session.runLoopUdp()
}

// ----- IServerSessionLifecycle ---------------------------------------------------------------------------------------

func (session *PubSession) Dispose() error {
	return session.dispose(nil)
}

// ----- ISessionUrlContext --------------------------------------------------------------------------------------------

func (session *PubSession) Url() string {
	Log.Warnf("[%s] PubSession.Url() is not implemented", session.UniqueKey())
	return "invalid"
}

func (session *PubSession) AppName() string {
	Log.Warnf("[%s] PubSession.AppName() is not implemented", session.UniqueKey())
	return "invalid"
}

func (session *PubSession) StreamName() string {
	// 如果stream name没有设置，则使用session的unique key作为stream name
	if session.streamName == "" {
		return session.UniqueKey()
	}
	return session.streamName
}

func (session *PubSession) RawQuery() string {
	Log.Warnf("[%s] PubSession.RawQuery() is not implemented", session.UniqueKey())
	return "invalid"
}

// ----- IObject -------------------------------------------------------------------------------------------------------

func (session *PubSession) UniqueKey() string {
	return session.sessionStat.UniqueKey()
}

// ----- ISessionStat --------------------------------------------------------------------------------------------------

func (session *PubSession) UpdateStat(intervalSec uint32) {
	session.sessionStat.UpdateStat(intervalSec)
}

func (session *PubSession) GetStat() base.StatSession {
	return session.sessionStat.GetStat()
}

func (session *PubSession) IsAlive() (readAlive, writeAlive bool) {
	return session.sessionStat.IsAlive()
}

// ---------------------------------------------------------------------------------------------------------------------

func (session *PubSession) listenUdp(port int) (int, error) {
	var err error
	var uconn *net.UDPConn
	var addr string

	if port == 0 {
		uconn, _, err = defaultUdpConnPoll.Acquire()
		if err != nil {
			return -1, err
		}

		port = uconn.LocalAddr().(*net.UDPAddr).Port
	} else {
		addr = fmt.Sprintf(":%d", port)
	}

	session.udpConn, err = nazanet.NewUdpConnection(func(option *nazanet.UdpConnectionOption) {
		option.LAddr = addr
		option.Conn = uconn
	})
	return port, err
}

func (session *PubSession) listenTcp(port int) (int, error) {
	var err error

	// TODO(chef): [refactor] 考虑挪到naza中，udp在naza中有类似的实现 202209
	if port == 0 {
		for i := defaultPubSessionPortMin; i < defaultPubSessionPortMax; i++ {
			addr := fmt.Sprintf(":%d", i)
			if session.listener, err = net.Listen("tcp", addr); err == nil {
				return int(i), nil
			}
		}
		return 0, err
	}

	addr := fmt.Sprintf(":%d", port)
	session.listener, err = net.Listen("tcp", addr)
	return port, err
}

func (session *PubSession) runLoopUdp() error {
	err := session.udpConn.RunLoop(func(b []byte, raddr *net.UDPAddr, err error) bool {
		if len(b) == 0 && err != nil {
			return false
		}

		session.feedPacket(b)
		return true
	})
	return err
}

func (session *PubSession) runLoopTcp() error {
	for {
		conn, err := session.listener.Accept()
		if err != nil {
			nazalog.Debugf("[%s] stop accept. err=%+v", session.UniqueKey(), err)
			return err
		}

		if session.tcpConn != nil {
			nazalog.Warnf("[%s] tcp conn already exist, close the prev. err=%+v", session.UniqueKey(), err)
			session.tcpConn.Close()
			// TODO(chef): [fix] reset unpack 202209
		}

		session.tcpConn = conn

		go func() {
			lb := make([]byte, 2)
			buf := nazabytes.NewBuffer(1500) // 初始1500，如果不够会扩容
			for {
				if _, rErr := io.ReadFull(conn, lb); rErr != nil {
					nazalog.Debugf("[%s] read failed. err=%+v", session.UniqueKey(), rErr)
					break
				}
				length := int(bele.BeUint16(lb))
				b := buf.ReserveBytes(length)
				if _, rErr := io.ReadFull(conn, b); rErr != nil {
					nazalog.Debugf("[%s] read failed. err=%+v", session.UniqueKey(), rErr)
					break
				}

				session.feedPacket(b)
			}
		}()
	}
}

func (session *PubSession) feedPacket(b []byte) {
	if session.hookOnReadPacket != nil {
		session.hookOnReadPacket(b)
	}

	session.sessionStat.AddReadBytes(len(b))
	session.unpacker.FeedRtpPacket(b)
}

func (session *PubSession) dispose(err error) error {
	var retErr error
	session.disposeOnce.Do(func() {
		Log.Infof("[%s] lifecycle dispose gb28181 PubSession. err=%+v", session.UniqueKey(), err)
		if session.isTcpFlag {
			if session.tcpConn == nil {
				retErr = base.ErrSessionNotStarted
				return
			}
			retErr = session.tcpConn.Close()
		} else {
			if session.udpConn == nil {
				retErr = base.ErrSessionNotStarted
				return
			}
			retErr = session.udpConn.Dispose()
		}

		session.unpacker.Dispose()
	})
	return retErr
}
