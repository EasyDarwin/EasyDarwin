// SPDX-FileCopyrightText: 2023 The Pion community <https://pion.ly>
// SPDX-License-Identifier: MIT

package udp

import (
	"io"
	"net"
	"runtime"
	"sync"
	"sync/atomic"
	"time"

	"golang.org/x/net/ipv4"
	"golang.org/x/net/ipv6"
)

// BatchWriter represents conn can write messages in batch
type BatchWriter interface {
	WriteBatch(ms []ipv4.Message, flags int) (int, error)
}

// BatchReader represents conn can read messages in batch
type BatchReader interface {
	ReadBatch(msg []ipv4.Message, flags int) (int, error)
}

// BatchPacketConn represents conn can read/write messages in batch
type BatchPacketConn interface {
	BatchWriter
	BatchReader
	io.Closer
}

// BatchConn uses ipv4/v6.NewPacketConn to wrap a net.PacketConn to write/read messages in batch,
// only available in linux. In other platform, it will use single Write/Read as same as net.Conn.
type BatchConn struct {
	net.PacketConn

	batchConn BatchPacketConn

	batchWriteMutex    sync.Mutex
	batchWriteMessages []ipv4.Message
	batchWritePos      int
	batchWriteLast     time.Time

	batchWriteSize     int
	batchWriteInterval time.Duration

	closed int32
}

// NewBatchConn creates a *BatchConn from net.PacketConn with batch configs.
func NewBatchConn(conn net.PacketConn, batchWriteSize int, batchWriteInterval time.Duration) *BatchConn {
	bc := &BatchConn{
		PacketConn:         conn,
		batchWriteLast:     time.Now(),
		batchWriteInterval: batchWriteInterval,
		batchWriteSize:     batchWriteSize,
		batchWriteMessages: make([]ipv4.Message, batchWriteSize),
	}
	for i := range bc.batchWriteMessages {
		bc.batchWriteMessages[i].Buffers = [][]byte{make([]byte, sendMTU)}
	}

	// batch write only supports linux
	if runtime.GOOS == "linux" {
		if pc4 := ipv4.NewPacketConn(conn); pc4 != nil {
			bc.batchConn = pc4
		} else if pc6 := ipv6.NewPacketConn(conn); pc6 != nil {
			bc.batchConn = pc6
		}
	}

	if bc.batchConn != nil {
		go func() {
			writeTicker := time.NewTicker(batchWriteInterval / 2)
			defer writeTicker.Stop()
			for atomic.LoadInt32(&bc.closed) != 1 {
				<-writeTicker.C
				bc.batchWriteMutex.Lock()
				if bc.batchWritePos > 0 && time.Since(bc.batchWriteLast) >= bc.batchWriteInterval {
					_ = bc.flush()
				}
				bc.batchWriteMutex.Unlock()
			}
		}()
	}

	return bc
}

// Close batchConn and the underlying PacketConn
func (c *BatchConn) Close() error {
	atomic.StoreInt32(&c.closed, 1)
	c.batchWriteMutex.Lock()
	if c.batchWritePos > 0 {
		_ = c.flush()
	}
	c.batchWriteMutex.Unlock()
	if c.batchConn != nil {
		return c.batchConn.Close()
	}
	return c.PacketConn.Close()
}

// WriteTo write message to an UDPAddr, addr should be nil if it is a connected socket.
func (c *BatchConn) WriteTo(b []byte, addr net.Addr) (int, error) {
	if c.batchConn == nil {
		return c.PacketConn.WriteTo(b, addr)
	}
	return c.enqueueMessage(b, addr)
}

func (c *BatchConn) enqueueMessage(buf []byte, raddr net.Addr) (int, error) {
	var err error
	c.batchWriteMutex.Lock()
	defer c.batchWriteMutex.Unlock()

	msg := &c.batchWriteMessages[c.batchWritePos]
	// reset buffers
	msg.Buffers = msg.Buffers[:1]
	msg.Buffers[0] = msg.Buffers[0][:cap(msg.Buffers[0])]

	c.batchWritePos++
	if raddr != nil {
		msg.Addr = raddr
	}
	if n := copy(msg.Buffers[0], buf); n < len(buf) {
		extraBuffer := make([]byte, len(buf)-n)
		copy(extraBuffer, buf[n:])
		msg.Buffers = append(msg.Buffers, extraBuffer)
	} else {
		msg.Buffers[0] = msg.Buffers[0][:n]
	}
	if c.batchWritePos == c.batchWriteSize {
		err = c.flush()
	}
	return len(buf), err
}

// ReadBatch reads messages in batch, return length of message readed and error.
func (c *BatchConn) ReadBatch(msgs []ipv4.Message, flags int) (int, error) {
	if c.batchConn == nil {
		n, addr, err := c.PacketConn.ReadFrom(msgs[0].Buffers[0])
		if err == nil {
			msgs[0].N = n
			msgs[0].Addr = addr
			return 1, nil
		}
		return 0, err
	}
	return c.batchConn.ReadBatch(msgs, flags)
}

func (c *BatchConn) flush() error {
	var writeErr error
	var txN int
	for txN < c.batchWritePos {
		n, err := c.batchConn.WriteBatch(c.batchWriteMessages[txN:c.batchWritePos], 0)
		if err != nil {
			writeErr = err
			break
		}
		txN += n
	}
	c.batchWritePos = 0
	c.batchWriteLast = time.Now()
	return writeErr
}
