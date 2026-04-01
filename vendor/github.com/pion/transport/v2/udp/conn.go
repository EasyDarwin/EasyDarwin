// SPDX-FileCopyrightText: 2023 The Pion community <https://pion.ly>
// SPDX-License-Identifier: MIT

// Package udp provides a connection-oriented listener over a UDP PacketConn
package udp

import (
	"context"
	"errors"
	"net"
	"sync"
	"sync/atomic"
	"time"

	"github.com/pion/logging"
	"github.com/pion/transport/v2/deadline"
	"github.com/pion/transport/v2/packetio"
	"golang.org/x/net/ipv4"
)

const (
	receiveMTU           = 8192
	sendMTU              = 1500
	defaultListenBacklog = 128 // same as Linux default
)

// Typed errors
var (
	ErrClosedListener      = errors.New("udp: listener closed")
	ErrListenQueueExceeded = errors.New("udp: listen queue exceeded")
	ErrInvalidBatchConfig  = errors.New("udp: invalid batch config")
)

// listener augments a connection-oriented Listener over a UDP PacketConn
type listener struct {
	pConn net.PacketConn

	readBatchSize int

	accepting    atomic.Value // bool
	acceptCh     chan *Conn
	doneCh       chan struct{}
	doneOnce     sync.Once
	acceptFilter func([]byte) bool

	connLock sync.Mutex
	conns    map[string]*Conn
	connWG   *sync.WaitGroup

	readWG   sync.WaitGroup
	errClose atomic.Value // error

	readDoneCh chan struct{}
	errRead    atomic.Value // error

	logger logging.LeveledLogger
}

// Accept waits for and returns the next connection to the listener.
func (l *listener) Accept() (net.Conn, error) {
	select {
	case c := <-l.acceptCh:
		l.connWG.Add(1)
		return c, nil

	case <-l.readDoneCh:
		err, _ := l.errRead.Load().(error)
		return nil, err

	case <-l.doneCh:
		return nil, ErrClosedListener
	}
}

// Close closes the listener.
// Any blocked Accept operations will be unblocked and return errors.
func (l *listener) Close() error {
	var err error
	l.doneOnce.Do(func() {
		l.accepting.Store(false)
		close(l.doneCh)

		l.connLock.Lock()
		// Close unaccepted connections
	lclose:
		for {
			select {
			case c := <-l.acceptCh:
				close(c.doneCh)
				delete(l.conns, c.rAddr.String())

			default:
				break lclose
			}
		}
		nConns := len(l.conns)
		l.connLock.Unlock()

		l.connWG.Done()

		if nConns == 0 {
			// Wait if this is the final connection
			l.readWG.Wait()
			if errClose, ok := l.errClose.Load().(error); ok {
				err = errClose
			}
		} else {
			err = nil
		}
	})

	return err
}

// Addr returns the listener's network address.
func (l *listener) Addr() net.Addr {
	return l.pConn.LocalAddr()
}

// BatchIOConfig indicates config to batch read/write packets,
// it will use ReadBatch/WriteBatch to improve throughput for UDP.
type BatchIOConfig struct {
	Enable bool
	// ReadBatchSize indicates the maximum number of packets to be read in one batch, a batch size less than 2 means
	// disable read batch.
	ReadBatchSize int
	// WriteBatchSize indicates the maximum number of packets to be written in one batch
	WriteBatchSize int
	// WriteBatchInterval indicates the maximum interval to wait before writing packets in one batch
	// small interval will reduce latency/jitter, but increase the io count.
	WriteBatchInterval time.Duration
}

// ListenConfig stores options for listening to an address.
type ListenConfig struct {
	// Backlog defines the maximum length of the queue of pending
	// connections. It is equivalent of the backlog argument of
	// POSIX listen function.
	// If a connection request arrives when the queue is full,
	// the request will be silently discarded, unlike TCP.
	// Set zero to use default value 128 which is same as Linux default.
	Backlog int

	// AcceptFilter determines whether the new conn should be made for
	// the incoming packet. If not set, any packet creates new conn.
	AcceptFilter func([]byte) bool

	// ReadBufferSize sets the size of the operating system's
	// receive buffer associated with the listener.
	ReadBufferSize int

	// WriteBufferSize sets the size of the operating system's
	// send buffer associated with the connection.
	WriteBufferSize int

	Batch BatchIOConfig

	LoggerFactory logging.LoggerFactory
}

// Listen creates a new listener based on the ListenConfig.
func (lc *ListenConfig) Listen(network string, laddr *net.UDPAddr) (net.Listener, error) {
	if lc.Backlog == 0 {
		lc.Backlog = defaultListenBacklog
	}

	if lc.Batch.Enable && (lc.Batch.WriteBatchSize <= 0 || lc.Batch.WriteBatchInterval <= 0) {
		return nil, ErrInvalidBatchConfig
	}

	conn, err := net.ListenUDP(network, laddr)
	if err != nil {
		return nil, err
	}

	if lc.ReadBufferSize > 0 {
		_ = conn.SetReadBuffer(lc.ReadBufferSize)
	}
	if lc.WriteBufferSize > 0 {
		_ = conn.SetWriteBuffer(lc.WriteBufferSize)
	}

	loggerFactory := lc.LoggerFactory
	if loggerFactory == nil {
		loggerFactory = logging.NewDefaultLoggerFactory()
	}

	logger := loggerFactory.NewLogger("transport")

	l := &listener{
		pConn:        conn,
		acceptCh:     make(chan *Conn, lc.Backlog),
		conns:        make(map[string]*Conn),
		doneCh:       make(chan struct{}),
		acceptFilter: lc.AcceptFilter,
		connWG:       &sync.WaitGroup{},
		readDoneCh:   make(chan struct{}),
		logger:       logger,
	}

	if lc.Batch.Enable {
		l.pConn = NewBatchConn(conn, lc.Batch.WriteBatchSize, lc.Batch.WriteBatchInterval)
		l.readBatchSize = lc.Batch.ReadBatchSize
	}

	l.accepting.Store(true)
	l.connWG.Add(1)
	l.readWG.Add(2) // wait readLoop and Close execution routine

	go l.readLoop()
	go func() {
		l.connWG.Wait()
		if err := l.pConn.Close(); err != nil {
			l.errClose.Store(err)
		}
		l.readWG.Done()
	}()

	return l, nil
}

// Listen creates a new listener using default ListenConfig.
func Listen(network string, laddr *net.UDPAddr) (net.Listener, error) {
	return (&ListenConfig{}).Listen(network, laddr)
}

// readLoop has to tasks:
//  1. Dispatching incoming packets to the correct Conn.
//     It can therefore not be ended until all Conns are closed.
//  2. Creating a new Conn when receiving from a new remote.
func (l *listener) readLoop() {
	defer l.readWG.Done()
	defer close(l.readDoneCh)

	if br, ok := l.pConn.(BatchReader); ok && l.readBatchSize > 1 {
		l.readBatch(br)
	} else {
		l.read()
	}
}

func (l *listener) readBatch(br BatchReader) {
	msgs := make([]ipv4.Message, l.readBatchSize)
	for i := range msgs {
		msg := &msgs[i]
		msg.Buffers = [][]byte{make([]byte, receiveMTU)}
		msg.OOB = make([]byte, 40)
	}
	for {
		n, err := br.ReadBatch(msgs, 0)
		if err != nil {
			l.errRead.Store(err)
			return
		}
		for i := 0; i < n; i++ {
			l.dispatchMsg(msgs[i].Addr, msgs[i].Buffers[0][:msgs[i].N])
		}
	}
}

func (l *listener) read() {
	buf := make([]byte, receiveMTU)
	for {
		n, raddr, err := l.pConn.ReadFrom(buf)
		if err != nil {
			l.errRead.Store(err)
			l.logger.Tracef("error reading from connection err=%v", err)
			return
		}
		l.dispatchMsg(raddr, buf[:n])
	}
}

func (l *listener) dispatchMsg(addr net.Addr, buf []byte) {
	conn, ok, err := l.getConn(addr, buf)
	if err != nil {
		return
	}
	if ok {
		_, err := conn.buffer.Write(buf)
		if err != nil {
			l.logger.Tracef("error dispatching message addr=%v err=%v", addr, err)
		}
	}
}

func (l *listener) getConn(raddr net.Addr, buf []byte) (*Conn, bool, error) {
	l.connLock.Lock()
	defer l.connLock.Unlock()
	conn, ok := l.conns[raddr.String()]
	if !ok {
		if isAccepting, ok := l.accepting.Load().(bool); !isAccepting || !ok {
			return nil, false, ErrClosedListener
		}
		if l.acceptFilter != nil {
			if !l.acceptFilter(buf) {
				return nil, false, nil
			}
		}
		conn = l.newConn(raddr)
		select {
		case l.acceptCh <- conn:
			l.conns[raddr.String()] = conn
		default:
			return nil, false, ErrListenQueueExceeded
		}
	}
	return conn, true, nil
}

// Conn augments a connection-oriented connection over a UDP PacketConn
type Conn struct {
	listener *listener

	rAddr net.Addr

	buffer *packetio.Buffer

	doneCh   chan struct{}
	doneOnce sync.Once

	writeDeadline *deadline.Deadline
}

func (l *listener) newConn(rAddr net.Addr) *Conn {
	return &Conn{
		listener:      l,
		rAddr:         rAddr,
		buffer:        packetio.NewBuffer(),
		doneCh:        make(chan struct{}),
		writeDeadline: deadline.New(),
	}
}

// Read reads from c into p
func (c *Conn) Read(p []byte) (int, error) {
	return c.buffer.Read(p)
}

// Write writes len(p) bytes from p to the DTLS connection
func (c *Conn) Write(p []byte) (n int, err error) {
	select {
	case <-c.writeDeadline.Done():
		return 0, context.DeadlineExceeded
	default:
	}
	return c.listener.pConn.WriteTo(p, c.rAddr)
}

// Close closes the conn and releases any Read calls
func (c *Conn) Close() error {
	var err error
	c.doneOnce.Do(func() {
		c.listener.connWG.Done()
		close(c.doneCh)
		c.listener.connLock.Lock()
		delete(c.listener.conns, c.rAddr.String())
		nConns := len(c.listener.conns)
		c.listener.connLock.Unlock()

		if isAccepting, ok := c.listener.accepting.Load().(bool); nConns == 0 && !isAccepting && ok {
			// Wait if this is the final connection
			c.listener.readWG.Wait()
			if errClose, ok := c.listener.errClose.Load().(error); ok {
				err = errClose
			}
		} else {
			err = nil
		}

		if errBuf := c.buffer.Close(); errBuf != nil && err == nil {
			err = errBuf
		}
	})

	return err
}

// LocalAddr implements net.Conn.LocalAddr
func (c *Conn) LocalAddr() net.Addr {
	return c.listener.pConn.LocalAddr()
}

// RemoteAddr implements net.Conn.RemoteAddr
func (c *Conn) RemoteAddr() net.Addr {
	return c.rAddr
}

// SetDeadline implements net.Conn.SetDeadline
func (c *Conn) SetDeadline(t time.Time) error {
	c.writeDeadline.Set(t)
	return c.SetReadDeadline(t)
}

// SetReadDeadline implements net.Conn.SetDeadline
func (c *Conn) SetReadDeadline(t time.Time) error {
	return c.buffer.SetReadDeadline(t)
}

// SetWriteDeadline implements net.Conn.SetDeadline
func (c *Conn) SetWriteDeadline(t time.Time) error {
	c.writeDeadline.Set(t)
	// Write deadline of underlying connection should not be changed
	// since the connection can be shared.
	return nil
}
