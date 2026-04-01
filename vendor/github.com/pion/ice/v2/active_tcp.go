// SPDX-FileCopyrightText: 2023 The Pion community <https://pion.ly>
// SPDX-License-Identifier: MIT

package ice

import (
	"context"
	"io"
	"net"
	"sync/atomic"
	"time"

	"github.com/pion/logging"
	"github.com/pion/transport/v2/packetio"
)

type activeTCPConn struct {
	readBuffer, writeBuffer *packetio.Buffer
	localAddr, remoteAddr   atomic.Value
	closed                  int32
}

func newActiveTCPConn(ctx context.Context, localAddress, remoteAddress string, log logging.LeveledLogger) (a *activeTCPConn) {
	a = &activeTCPConn{
		readBuffer:  packetio.NewBuffer(),
		writeBuffer: packetio.NewBuffer(),
	}

	laddr, err := getTCPAddrOnInterface(localAddress)
	if err != nil {
		atomic.StoreInt32(&a.closed, 1)
		log.Infof("Failed to dial TCP address %s: %v", remoteAddress, err)
		return
	}
	a.localAddr.Store(laddr)

	go func() {
		defer func() {
			atomic.StoreInt32(&a.closed, 1)
		}()

		dialer := &net.Dialer{
			LocalAddr: laddr,
		}
		conn, err := dialer.DialContext(ctx, "tcp", remoteAddress)
		if err != nil {
			log.Infof("Failed to dial TCP address %s: %v", remoteAddress, err)
			return
		}

		a.remoteAddr.Store(conn.RemoteAddr())

		go func() {
			buff := make([]byte, receiveMTU)

			for atomic.LoadInt32(&a.closed) == 0 {
				n, err := readStreamingPacket(conn, buff)
				if err != nil {
					log.Infof("Failed to read streaming packet: %s", err)
					break
				}

				if _, err := a.readBuffer.Write(buff[:n]); err != nil {
					log.Infof("Failed to write to buffer: %s", err)
					break
				}
			}
		}()

		buff := make([]byte, receiveMTU)

		for atomic.LoadInt32(&a.closed) == 0 {
			n, err := a.writeBuffer.Read(buff)
			if err != nil {
				log.Infof("Failed to read from buffer: %s", err)
				break
			}

			if _, err = writeStreamingPacket(conn, buff[:n]); err != nil {
				log.Infof("Failed to write streaming packet: %s", err)
				break
			}
		}

		if err := conn.Close(); err != nil {
			log.Infof("Failed to close connection: %s", err)
		}
	}()

	return a
}

func (a *activeTCPConn) ReadFrom(buff []byte) (n int, srcAddr net.Addr, err error) {
	if atomic.LoadInt32(&a.closed) == 1 {
		return 0, nil, io.ErrClosedPipe
	}

	srcAddr = a.RemoteAddr()
	n, err = a.readBuffer.Read(buff)
	return
}

func (a *activeTCPConn) WriteTo(buff []byte, _ net.Addr) (n int, err error) {
	if atomic.LoadInt32(&a.closed) == 1 {
		return 0, io.ErrClosedPipe
	}

	return a.writeBuffer.Write(buff)
}

func (a *activeTCPConn) Close() error {
	atomic.StoreInt32(&a.closed, 1)
	_ = a.readBuffer.Close()
	_ = a.writeBuffer.Close()
	return nil
}

func (a *activeTCPConn) LocalAddr() net.Addr {
	if v, ok := a.localAddr.Load().(*net.TCPAddr); ok {
		return v
	}

	return &net.TCPAddr{}
}

func (a *activeTCPConn) RemoteAddr() net.Addr {
	if v, ok := a.remoteAddr.Load().(*net.TCPAddr); ok {
		return v
	}

	return &net.TCPAddr{}
}

func (a *activeTCPConn) SetDeadline(time.Time) error      { return io.EOF }
func (a *activeTCPConn) SetReadDeadline(time.Time) error  { return io.EOF }
func (a *activeTCPConn) SetWriteDeadline(time.Time) error { return io.EOF }

func getTCPAddrOnInterface(address string) (*net.TCPAddr, error) {
	addr, err := net.ResolveTCPAddr("tcp", address)
	if err != nil {
		return nil, err
	}

	l, err := net.ListenTCP("tcp", addr)
	if err != nil {
		return nil, err
	}
	defer func() {
		_ = l.Close()
	}()

	tcpAddr, ok := l.Addr().(*net.TCPAddr)
	if !ok {
		return nil, errInvalidAddress
	}

	return tcpAddr, nil
}
