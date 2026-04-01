// Copyright 2019, Chef.  All rights reserved.
// https://github.com/q191201771/lal
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

package base

import (
	"net"
	"strings"

	"github.com/q191201771/naza/pkg/connection"
)

type BasicHttpSubSession struct {
	BasicHttpSubSessionOption

	suffix      string
	conn        connection.Connection
	sessionStat BasicSessionStat
}

type BasicHttpSubSessionOption struct {
	Conn          net.Conn
	ConnModOption connection.ModOption
	SessionType   SessionType
	UrlCtx        UrlContext
	IsWebSocket   bool
	WebSocketKey  string
}

func NewBasicHttpSubSession(option BasicHttpSubSessionOption) *BasicHttpSubSession {
	s := &BasicHttpSubSession{
		BasicHttpSubSessionOption: option,
		conn:                      connection.New(option.Conn, option.ConnModOption),
		sessionStat:               NewBasicSessionStat(option.SessionType, option.Conn.RemoteAddr().String()),
	}
	return s
}

// ---------------------------------------------------------------------------------------------------------------------
// IServerSessionLifecycle interface
// ---------------------------------------------------------------------------------------------------------------------

func (session *BasicHttpSubSession) RunLoop() error {
	buf := make([]byte, 128)
	_, err := session.conn.Read(buf)
	return err
}

func (session *BasicHttpSubSession) Dispose() error {
	return session.conn.Close()
}

// ---------------------------------------------------------------------------------------------------------------------

func (session *BasicHttpSubSession) WriteHttpResponseHeader(b []byte) {
	if session.IsWebSocket {
		session.write(UpdateWebSocketHeader(session.WebSocketKey, ""))
	} else {
		session.write(b)
	}
}

func (session *BasicHttpSubSession) Write(b []byte) {
	if session.IsWebSocket {
		wsHeader := WsHeader{
			Fin:           true,
			Rsv1:          false,
			Rsv2:          false,
			Rsv3:          false,
			Opcode:        Wso_Binary,
			PayloadLength: uint64(len(b)),
			Masked:        false,
		}
		session.write(MakeWsFrameHeader(wsHeader))
	}
	session.write(b)
}

// ---------------------------------------------------------------------------------------------------------------------
// IObject interface
// ---------------------------------------------------------------------------------------------------------------------

func (session *BasicHttpSubSession) UniqueKey() string {
	return session.sessionStat.UniqueKey()
}

// ---------------------------------------------------------------------------------------------------------------------
// ISessionUrlContext interface
// ---------------------------------------------------------------------------------------------------------------------

func (session *BasicHttpSubSession) Url() string {
	return session.UrlCtx.Url
}

func (session *BasicHttpSubSession) AppName() string {
	return session.UrlCtx.PathWithoutLastItem
}

func (session *BasicHttpSubSession) StreamName() string {
	var suffix string
	switch session.SessionType {
	case SessionTypeFlvSub:
		suffix = ".flv"
	case SessionTypeTsSub:
		suffix = ".ts"
	default:
		Log.Warnf("[%s] acquire stream name but protocol unknown.", session.UniqueKey())
	}
	return strings.TrimSuffix(session.UrlCtx.LastItemOfPath, suffix)
}

func (session *BasicHttpSubSession) RawQuery() string {
	return session.UrlCtx.RawQuery
}

// ----- ISessionStat --------------------------------------------------------------------------------------------------

func (session *BasicHttpSubSession) GetStat() StatSession {
	return session.sessionStat.GetStat()
}

func (session *BasicHttpSubSession) UpdateStat(intervalSec uint32) {
	session.sessionStat.UpdateStatWitchConn(session.conn, intervalSec)
}

func (session *BasicHttpSubSession) IsAlive() (readAlive, writeAlive bool) {
	return session.sessionStat.IsAliveWitchConn(session.conn)
}

// ---------------------------------------------------------------------------------------------------------------------

func (session *BasicHttpSubSession) write(b []byte) {
	// TODO(chef) handle write error
	_, _ = session.conn.Write(b)
}
