// Copyright 2019, Chef.  All rights reserved.
// https://github.com/q191201771/naza
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

// Package connection
//
// 对 net.Conn 接口的二次封装，目的有两个：
// 1. 在流媒体传输这种特定的长连接场景下提供更方便、高性能的接口
// 2. 便于后续将TCPConn替换成其他传输协议
package connection

import (
	"bufio"
	"errors"
	"io"
	"net"
	"sync"
	"time"

	"github.com/q191201771/naza/pkg/unique"

	"github.com/q191201771/naza/pkg/nazaatomic"

	"github.com/q191201771/naza/pkg/nazalog"
)

var (
	ErrConnectionPanic = errors.New("naza.connection: using in a wrong way")
	ErrClosedAlready   = errors.New("naza.connection: connection closed already")
	ErrWriteChanFull   = errors.New("naza.connection: write channel full")
)

type Connection interface {
	// ----- net.Conn interface ----------------------------------------------------------------------------------------
	//
	// 注意，如果没有特别说明，函数的语义和 net.Conn 相同
	//

	// Read ...
	Read(b []byte) (n int, err error)

	// Write
	//
	// @return n 发送成功的大小
	//           注意，如果设置了 Option.WriteChanSize 做异步发送，那么`n`恒等于len(`b`)
	//
	Write(b []byte) (n int, err error)

	// Close 允许调用多次
	//
	Close() error

	LocalAddr() net.Addr
	RemoteAddr() net.Addr
	SetDeadline(t time.Time) error
	SetReadDeadline(t time.Time) error
	SetWriteDeadline(t time.Time) error

	// -----------------------------------------------------------------------------------------------------------------

	// Writev 发送多块不连续的内存块时使用
	//
	// 当有多块不连续的内存块需要发送时，调用 Writev 在某些平台性能会优于以下做法：
	// 1. 多次调用Write
	// 2. 将多块内存块拷贝拼接成一块内存块后调用Write
	// 原因是减少了系统调用以及内存拷贝（还有可能有内存管理）的开销
	//
	// 注意，如果需要发送的是一块连续的内存块，建议使用 Write 发送
	//
	Writev(b net.Buffers) (n int, err error)

	ReadAtLeast(buf []byte, min int) (n int, err error)
	ReadLine() (line []byte, isPrefix bool, err error) // 只有设置了ReadBufSize才可以使用这个方法

	// Flush
	//
	// 如果使用了bufio写缓冲，则将缓冲中的数据发送出去
	// 如果使用了channel异步发送，则阻塞等待，直到之前channel中的数据全部发送完毕
	//
	// 一般在Close前，想要将剩余数据发送完毕时调用
	//
	Flush() error

	// Done 阻塞直到连接关闭或发生错误
	//
	// 注意，向上层严格保证，消息发送后，后续Read，Write等调用都将失败
	//
	// 注意，向上层严格保证，消息只发送一次
	//
	// @return 返回nil则是本端主动调用Close关闭
	//
	Done() <-chan error

	// ModWriteChanSize Modxxx
	//
	// TODO chef: 这几个接口是否不提供
	// Mod类型函数不加锁，需要调用方保证不发生竞态调用
	//
	// ModWriteChanSize 只允许在初始化时为0的前提下调用
	ModWriteChanSize(n int)
	ModWriteBufSize(n int)
	ModReadTimeoutMs(n int)
	ModWriteTimeoutMs(n int)

	// GetStat 连接上读取和发送的字节总数。
	// 注意，如果是异步发送，发送字节统计的是调用底层write的值，而非上层调用Connection发送的值
	// 也即不包含Connection中的发送缓存部分，但是可能包含内核socket发送缓冲区的值。
	GetStat() Stat
}

type Stat struct {
	ReadBytesSum  uint64
	WroteBytesSum uint64
}

type StatAtomic struct {
	ReadBytesSum  nazaatomic.Uint64
	WroteBytesSum nazaatomic.Uint64
}

type WriteChanFullBehavior int

const (
	WriteChanFullBehaviorReturnError WriteChanFullBehavior = iota + 1
	WriteChanFullBehaviorBlock
)

type Option struct {
	// 如果不为0，则之后每次读/写使用bufio的缓冲
	ReadBufSize  int
	WriteBufSize int

	// 如果不为0，则之后每次读/写都带超时
	ReadTimeoutMs  int
	WriteTimeoutMs int

	// 如果不为0，则写使用channel将数据发送到后台协程中发送
	WriteChanSize int

	// 使用channel发送数据时，channel满了时Write函数的行为
	// WriteChanFullBehaviorReturnError 返回错误
	// WriteChanFullBehaviorBlock 阻塞直到向channel写入成功
	WriteChanFullBehavior WriteChanFullBehavior
}

// 没有配置的属性，将按如下配置
var defaultOption = Option{
	ReadBufSize:           0,
	WriteBufSize:          0,
	ReadTimeoutMs:         0,
	WriteTimeoutMs:        0,
	WriteChanSize:         0,
	WriteChanFullBehavior: WriteChanFullBehaviorReturnError,
}

type ModOption func(option *Option)

func New(conn net.Conn, modOptions ...ModOption) Connection {
	c := new(connection)
	c.uniqueKey = uniqueGen.GenUniqueKey()
	c.doneChan = make(chan error, 1)
	c.Conn = conn

	c.option = defaultOption

	for _, fn := range modOptions {
		fn(&c.option)
	}

	if c.option.ReadBufSize > 0 {
		c.r = bufio.NewReaderSize(conn, c.option.ReadBufSize)
	} else {
		c.r = conn
	}

	if c.option.WriteBufSize > 0 {
		c.w = bufio.NewWriterSize(conn, c.option.WriteBufSize)
	} else {
		c.w = conn
	}

	if c.option.WriteChanSize > 0 {
		c.wChan = make(chan wMsg, c.option.WriteChanSize)
		c.flushDoneChan = make(chan struct{}, 1)
		c.exitChan = make(chan struct{}, 1)
		go c.runWriteLoop()
	}

	nazalog.Debugf("[%s] lifecycle new connection. net.Conn=%p, naza.Connection=%p", c.uniqueKey, conn, c)
	return c
}

type wMsgType int

const (
	_ wMsgType = iota
	wMsgTypeWrite
	wMsgTypeWritev
	wMsgTypeFlush
)

type wMsg struct {
	t  wMsgType
	b  []byte
	bs net.Buffers
}

type connection struct {
	Conn          net.Conn
	r             io.Reader
	w             io.Writer
	option        Option
	uniqueKey     string
	wChan         chan wMsg
	flushDoneChan chan struct{}
	exitChan      chan struct{}
	doneChan      chan error
	closedFlag    nazaatomic.Bool
	closeOnce     sync.Once
	stat          StatAtomic
}

var uniqueGen *unique.SingleGenerator

func (c *connection) ModWriteChanSize(n int) {
	if c.option.WriteChanSize > 0 {
		panic(ErrConnectionPanic)
	}
	if n == 0 {
		return
	}

	c.option.WriteChanSize = n
	c.wChan = make(chan wMsg, n)
	c.flushDoneChan = make(chan struct{}, 1)
	c.exitChan = make(chan struct{}, 1)
	go c.runWriteLoop()
}

func (c *connection) ModWriteBufSize(n int) {
	if c.option.WriteBufSize > 0 {
		// 如果之前已经设置过写缓冲，直接 panic
		// 这里改成 flush 后替换成新缓冲也行，暂时没这个必要
		panic(ErrConnectionPanic)
	}
	c.option.WriteBufSize = n
	c.w = bufio.NewWriterSize(c.Conn, n)
}

func (c *connection) ModReadTimeoutMs(n int) {
	if c.option.ReadTimeoutMs > 0 {
		panic(ErrConnectionPanic)
	}
	c.option.ReadTimeoutMs = n
}

func (c *connection) ModWriteTimeoutMs(n int) {
	if c.option.WriteTimeoutMs > 0 {
		panic(ErrConnectionPanic)
	}
	c.option.WriteTimeoutMs = n
}

func (c *connection) ReadAtLeast(buf []byte, min int) (n int, err error) {
	if c.option.ReadTimeoutMs > 0 {
		err = c.SetReadDeadline(time.Now().Add(time.Duration(c.option.ReadTimeoutMs) * time.Millisecond))
		if err != nil {
			c.close(err)
			return 0, err
		}
	}
	n, err = io.ReadAtLeast(c.r, buf, min)
	if err != nil {
		c.close(err)
	}
	c.stat.ReadBytesSum.Add(uint64(n))
	return n, err
}

// TODO chef: 测试 bufio 设置的大小 < 换行符位置时的情况
func (c *connection) ReadLine() (line []byte, isPrefix bool, err error) {
	bufioReader, ok := c.r.(*bufio.Reader)
	if !ok {
		// 目前只有使用了 bufio.Reader 时才能执行 ReadLine 操作
		panic(ErrConnectionPanic)
	}
	if c.option.ReadTimeoutMs > 0 {
		err = c.SetReadDeadline(time.Now().Add(time.Duration(c.option.ReadTimeoutMs) * time.Millisecond))
		if err != nil {
			c.close(err)
			return nil, false, err
		}
	}
	line, isPrefix, err = bufioReader.ReadLine()
	if err != nil {
		c.close(err)
	}
	c.stat.ReadBytesSum.Add(uint64(len(line)))
	return line, isPrefix, err
}

func (c *connection) Read(b []byte) (n int, err error) {
	if c.option.ReadTimeoutMs > 0 {
		err = c.SetReadDeadline(time.Now().Add(time.Duration(c.option.ReadTimeoutMs) * time.Millisecond))
		if err != nil {
			c.close(err)
			return 0, err
		}
	}
	n, err = c.r.Read(b)
	if err != nil {
		c.close(err)
	}
	c.stat.ReadBytesSum.Add(uint64(n))
	return n, err
}

func (c *connection) Write(b []byte) (n int, err error) {
	if c.closedFlag.Load() {
		return 0, ErrClosedAlready
	}
	if c.option.WriteChanSize > 0 {
		switch c.option.WriteChanFullBehavior {
		case WriteChanFullBehaviorBlock:
			c.wChan <- wMsg{t: wMsgTypeWrite, b: b}
			return len(b), nil
		case WriteChanFullBehaviorReturnError:
			select {
			case c.wChan <- wMsg{t: wMsgTypeWrite, b: b}:
				return len(b), nil
			default:
				return 0, ErrWriteChanFull
			}
		}
	}
	return c.write(b)
}

func (c *connection) Writev(b net.Buffers) (n int, err error) {
	if c.closedFlag.Load() {
		return 0, ErrClosedAlready
	}
	if c.option.WriteChanSize > 0 {
		for _, v := range b {
			n += len(v)
		}
		switch c.option.WriteChanFullBehavior {
		case WriteChanFullBehaviorBlock:
			c.wChan <- wMsg{t: wMsgTypeWritev, bs: b}
			return n, nil
		case WriteChanFullBehaviorReturnError:
			select {
			case c.wChan <- wMsg{t: wMsgTypeWritev, bs: b}:
				return n, nil
			default:
				return 0, ErrWriteChanFull
			}
		}
	}
	return c.writev(b)
}

func (c *connection) Flush() error {
	if c.closedFlag.Load() {
		return ErrClosedAlready
	}
	if c.option.WriteChanSize > 0 {
		c.wChan <- wMsg{t: wMsgTypeFlush}
		<-c.flushDoneChan
		return nil
	}

	return c.flush()
}

func (c *connection) Close() error {
	nazalog.Debugf("[%s] Close.", c.uniqueKey)
	c.close(nil)
	return nil
}

func (c *connection) Done() <-chan error {
	return c.doneChan
}

func (c *connection) LocalAddr() net.Addr {
	return c.Conn.LocalAddr()
}

func (c *connection) RemoteAddr() net.Addr {
	return c.Conn.RemoteAddr()
}

func (c *connection) SetDeadline(t time.Time) error {
	err := c.Conn.SetDeadline(t)
	if err != nil {
		c.close(err)
	}
	return err
}

func (c *connection) SetReadDeadline(t time.Time) error {
	err := c.Conn.SetReadDeadline(t)
	if err != nil {
		c.close(err)
	}
	return err
}

func (c *connection) SetWriteDeadline(t time.Time) error {
	err := c.Conn.SetWriteDeadline(t)
	if err != nil {
		c.close(err)
	}
	return err
}

func (c *connection) GetStat() (s Stat) {
	s.ReadBytesSum = c.stat.ReadBytesSum.Load()
	s.WroteBytesSum = c.stat.WroteBytesSum.Load()
	return
}

func (c *connection) write(b []byte) (n int, err error) {
	if c.option.WriteTimeoutMs > 0 {
		err = c.SetWriteDeadline(time.Now().Add(time.Duration(c.option.WriteTimeoutMs) * time.Millisecond))
		if err != nil {
			c.close(err)
			return 0, err
		}
	}
	n, err = c.w.Write(b)
	if err != nil {
		c.close(err)
	}
	c.stat.WroteBytesSum.Add(uint64(n))
	return n, err
}

func (c *connection) writev(b net.Buffers) (n int, err error) {
	if c.option.WriteTimeoutMs > 0 {
		err = c.SetWriteDeadline(time.Now().Add(time.Duration(c.option.WriteTimeoutMs) * time.Millisecond))
		if err != nil {
			c.close(err)
			return 0, err
		}
	}
	var n64 int64
	n64, err = b.WriteTo(c.w)
	if err != nil {
		c.close(err)
	}
	n = int(n64)
	c.stat.WroteBytesSum.Add(uint64(n))
	return n, err
}

func (c *connection) runWriteLoop() {
	for {
		select {
		case <-c.exitChan:
			//nazalog.Debugf("[%s] recv exitChan and exit write loop", c.uniqueKey)
			return
		case msg := <-c.wChan:
			switch msg.t {
			case wMsgTypeWrite:
				if _, err := c.write(msg.b); err != nil {
					return
				}
			case wMsgTypeWritev:
				if _, err := c.writev(msg.bs); err != nil {
					return
				}
			case wMsgTypeFlush:
				if err := c.flush(); err != nil {
					c.flushDoneChan <- struct{}{}
					return
				}
				c.flushDoneChan <- struct{}{}
			}
		}
	}
}

func (c *connection) flush() error {
	w, ok := c.w.(*bufio.Writer)
	if ok {
		if c.option.WriteTimeoutMs > 0 {
			err := c.SetWriteDeadline(time.Now().Add(time.Duration(c.option.WriteTimeoutMs) * time.Millisecond))
			if err != nil {
				c.close(err)
				return err
			}
		}
		if err := w.Flush(); err != nil {
			c.close(err)
			return err
		}
	}
	return nil
}

func (c *connection) close(err error) {
	c.closeOnce.Do(func() {
		nazalog.Debugf("[%s] close once. err=%+v", c.uniqueKey, err)
		c.closedFlag.Store(true)
		if c.option.WriteChanSize > 0 {
			c.exitChan <- struct{}{}
		}

		// 注意，先Close后再发送消息，保证消息发送前，已经Close掉了
		_ = c.Conn.Close()
		c.doneChan <- err

		// 注意，如果使用了wChan，并不关闭它，避免竞态条件下connection继续使用它造成问题。让它随connection对象释放。
	})
}

func init() {
	uniqueGen = unique.NewSingleGenerator("NAZACONN")
}
