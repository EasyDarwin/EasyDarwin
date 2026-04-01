// Copyright 2020, Chef.  All rights reserved.
// https://github.com/q191201771/naza
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

package nazanet

import (
	"net"
	"time"
)

// TODO(chef): [opt] 增加函数，可以返回内部的本地地址 202208

// OnReadUdpPacket
//
// @return 上层回调返回false，则关闭UdpConnection
type OnReadUdpPacket func(b []byte, raddr *net.UDPAddr, err error) bool

type UdpConnectionOption struct {
	// 两种初始化方式：
	// 方式一：直接传入外部创建好的连接对象供内部使用
	Conn *net.UDPConn
	// 方式二：填入地址，内部创建连接对象
	// LAddr: 本地bind地址，如果设置为空，则自动选择可用端口
	//        比如作为客户端时，如果不想特别指定本地端口，可以设置为空
	//
	// RAddr: 对端地址，决定 UdpConnection.Write 函数的发送行为。
	// 注意，对端地址需显式填写IP。
	// 注意，即使使用方式一，也可以设置Rddr。
	// 作用: 比如作为客户端时，对端地址通常只有一个，在构造函数中指定，后续就不用每次发送都指定。
	//
	// 发送有两个函数 UdpConnection.Write 和 UdpConnection.Write2Addr：
	// UdpConnection.Write2Addr 在自身函数参数中显示指定对端地址，跟此处的 RAddr 不相关。
	// UdpConnection.Write 自身函数参数不包含对端地址，当此处的 RAddr 存在时，使用此处的地址，否则，使用读取到数据时的对端地址。
	// 注意，如果此处 RAddr 没有设置，且没有读取过数据，那么使用 UdpConnection.Write 将直接返回错误。
	LAddr string
	RAddr string

	MaxReadPacketSize int  // 读取数据时，内存块大小
	AllocEachRead     bool // 使用Read Loop时，是否每次读取都申请新的内存块，如果为false，则复用一块内存块
}

var defaultOption = UdpConnectionOption{
	MaxReadPacketSize: 1500,
	AllocEachRead:     true,
}

// UdpConnection xxx
type UdpConnection struct {
	option          UdpConnectionOption
	raddrFromOption *net.UDPAddr // 创建时通过外部设置的对端地址
	raddrFromRead   *net.UDPAddr // 接收数据时的对端地址
}

type ModUdpConnectionOption func(option *UdpConnectionOption)

func NewUdpConnection(modOptions ...ModUdpConnectionOption) (*UdpConnection, error) {
	var err error

	c := &UdpConnection{}
	c.option = defaultOption
	for _, fn := range modOptions {
		fn(&c.option)
	}
	if c.option.RAddr != "" {
		if c.raddrFromOption, err = net.ResolveUDPAddr(udpNetwork, c.option.RAddr); err != nil {
			return nil, err
		}
	}
	if c.option.Conn != nil {
		return c, nil
	}

	if c.option.Conn, err = listenUdpWithAddr(c.option.LAddr); err != nil {
		return nil, err
	}
	return c, err
}

func (c *UdpConnection) SetReadBuffer(bufSize int) error {
	err := c.option.Conn.SetReadBuffer(bufSize)
	if err != nil {
		c.Dispose()
	}
	return err
}
func (c *UdpConnection) SetWriteBuffer(bufSize int) error {
	err := c.option.Conn.SetWriteBuffer(bufSize)
	if err != nil {
		c.Dispose()
	}
	return err
}

// RunLoop 阻塞直至Read发生错误或上层回调函数返回false
//
// @return error: 如果外部调用Dispose，会返回error
//
// 注意，回调存在err!=nil(*net.OpError, Err={error | poll.errNetClosing} use of closed network connection), len==0的情况
func (c *UdpConnection) RunLoop(onRead OnReadUdpPacket) error {
	var b []byte
	if !c.option.AllocEachRead {
		b = make([]byte, c.option.MaxReadPacketSize)
	}
	for {
		if c.option.AllocEachRead {
			b = make([]byte, c.option.MaxReadPacketSize)
		}
		var n int
		var err error
		n, c.raddrFromRead, err = c.option.Conn.ReadFromUDP(b)
		if keepRunning := onRead(b[:n], c.raddrFromRead, err); !keepRunning {
			if err == nil {
				return c.Dispose()
			}
		}
		if err != nil {
			return err
		}
	}
}

// ReadWithTimeout 直接读取数据，不使用RunLoop
func (c *UdpConnection) ReadWithTimeout(timeoutMs int) ([]byte, *net.UDPAddr, error) {
	if timeoutMs > 0 {
		if err := c.option.Conn.SetReadDeadline(time.Now().Add(time.Duration(timeoutMs) * time.Millisecond)); err != nil {
			return nil, nil, err
		}
	}
	b := make([]byte, c.option.MaxReadPacketSize)
	n, raddr, err := c.option.Conn.ReadFromUDP(b)
	if err != nil {
		return nil, nil, err
	}
	return b[:n], raddr, nil
}

func (c *UdpConnection) Write(b []byte) error {
	if c.raddrFromOption != nil {
		_, err := c.option.Conn.WriteToUDP(b, c.raddrFromOption)
		return err
	}
	if c.raddrFromRead != nil {
		_, err := c.option.Conn.WriteToUDP(b, c.raddrFromRead)
		return err
	}

	return ErrNazaNet
}

func (c *UdpConnection) Write2Addr(b []byte, ruaddr *net.UDPAddr) error {
	_, err := c.option.Conn.WriteToUDP(b, ruaddr)
	return err
}

func (c *UdpConnection) Dispose() error {
	return c.option.Conn.Close()
}
