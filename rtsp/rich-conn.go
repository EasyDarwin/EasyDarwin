package rtsp

import (
	"net"
	"time"
)

type RichConn struct {
	net.Conn
	timeout time.Duration
}

func (conn *RichConn) Read(b []byte) (n int, err error) {
	if conn.timeout > 0 {
		conn.Conn.SetReadDeadline(time.Now().Add(conn.timeout))
	}
	return conn.Conn.Read(b)
}

func (conn *RichConn) Write(b []byte) (n int, err error) {
	if conn.timeout > 0 {
		conn.Conn.SetWriteDeadline(time.Now().Add(conn.timeout))
	}
	return conn.Conn.Write(b)
}
