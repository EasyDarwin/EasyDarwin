// Copyright 2019, Chef.  All rights reserved.
// https://github.com/q191201771/lal
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

package httpflv

import (
	"net"

	"github.com/q191201771/lal/pkg/base"

	"github.com/q191201771/naza/pkg/connection"
)

var flvHttpResponseHeader []byte

type SubSession struct {
	core                    *base.BasicHttpSubSession
	IsFresh                 bool
	ShouldWaitVideoKeyFrame bool
}

func NewSubSession(conn net.Conn, urlCtx base.UrlContext, isWebSocket bool, websocketKey string) *SubSession {
	uk := base.GenUkFlvSubSession()
	s := &SubSession{
		core: base.NewBasicHttpSubSession(base.BasicHttpSubSessionOption{
			Conn: conn,
			ConnModOption: func(option *connection.Option) {
				option.WriteChanSize = SubSessionWriteChanSize
				option.WriteTimeoutMs = SubSessionWriteTimeoutMs
			},
			SessionType:  base.SessionTypeFlvSub,
			UrlCtx:       urlCtx,
			IsWebSocket:  isWebSocket,
			WebSocketKey: websocketKey,
		}),
		IsFresh:                 true,
		ShouldWaitVideoKeyFrame: true,
	}
	Log.Infof("[%s] lifecycle new httpflv SubSession. session=%p, remote addr=%s", uk, s, conn.RemoteAddr().String())
	return s
}

// ---------------------------------------------------------------------------------------------------------------------
// IServerSessionLifecycle interface
// ---------------------------------------------------------------------------------------------------------------------

func (session *SubSession) RunLoop() error {
	return session.core.RunLoop()
}

func (session *SubSession) Dispose() error {
	Log.Infof("[%s] lifecycle dispose httpflv SubSession.", session.core.UniqueKey())
	return session.core.Dispose()
}

// ---------------------------------------------------------------------------------------------------------------------

func (session *SubSession) WriteHttpResponseHeader() {
	Log.Debugf("[%s] > W http response header.", session.core.UniqueKey())
	session.core.WriteHttpResponseHeader(flvHttpResponseHeader)
}

func (session *SubSession) WriteFlvHeader() {
	Log.Debugf("[%s] > W http flv header.", session.core.UniqueKey())
	session.core.Write(FlvHeader)
}

func (session *SubSession) WriteTag(tag *Tag) {
	session.core.Write(tag.Raw)
}

func (session *SubSession) Write(b []byte) {
	session.core.Write(b)
}

// ---------------------------------------------------------------------------------------------------------------------
// IObject interface
// ---------------------------------------------------------------------------------------------------------------------

func (session *SubSession) UniqueKey() string {
	return session.core.UniqueKey()
}

// ---------------------------------------------------------------------------------------------------------------------
// ISessionUrlContext interface
// ---------------------------------------------------------------------------------------------------------------------

func (session *SubSession) Url() string {
	return session.core.Url()
}

func (session *SubSession) AppName() string {
	return session.core.AppName()
}

func (session *SubSession) StreamName() string {
	return session.core.StreamName()
}

func (session *SubSession) RawQuery() string {
	return session.core.RawQuery()
}

// ----- ISessionStat --------------------------------------------------------------------------------------------------

func (session *SubSession) UpdateStat(intervalSec uint32) {
	session.core.UpdateStat(intervalSec)
}

func (session *SubSession) GetStat() base.StatSession {
	return session.core.GetStat()
}

func (session *SubSession) IsAlive() (readAlive, writeAlive bool) {
	return session.core.IsAlive()
}

// ---------------------------------------------------------------------------------------------------------------------

func init() {
	flvHttpResponseHeaderStr := "HTTP/1.1 200 OK\r\n" +
		"Server: " + base.LalHttpflvSubSessionServer + "\r\n" +
		"Cache-Control: no-cache\r\n" +
		"Content-Type: video/x-flv\r\n" +
		"Connection: close\r\n" +
		"Expires: -1\r\n" +
		"Pragma: no-cache\r\n" +
		base.CorsHeaders +
		"\r\n"

	flvHttpResponseHeader = []byte(flvHttpResponseHeaderStr)
}
