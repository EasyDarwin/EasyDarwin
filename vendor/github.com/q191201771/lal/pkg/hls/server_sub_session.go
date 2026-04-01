// Copyright 2022, Chef.  All rights reserved.
// https://github.com/q191201771/lal
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

package hls

import (
	"net/http"
	"strings"
	"time"

	"github.com/q191201771/lal/pkg/base"
	"github.com/q191201771/naza/pkg/connection"
	"github.com/q191201771/naza/pkg/nazamd5"
)

type SubSession struct {
	LastRequestTime time.Time
	urlCtx          base.UrlContext
	hlsUrlPattern   string
	appName         string
	timeout         time.Duration
	sessionIdHash   string // Because session.UniqueKey() too easy to guess so that we need to hash it with a key to prevent client guess session id

	stat     base.StatSession
	prevStat connection.Stat
	currStat connection.StatAtomic
}

func (s *SubSession) UniqueKey() string {
	return s.stat.SessionId
}

func NewSubSession(req *http.Request, urlCtx base.UrlContext, hlsUrlPattern, sessionHashKey string, timeout time.Duration) *SubSession {
	if strings.HasPrefix(hlsUrlPattern, "/") {
		hlsUrlPattern = hlsUrlPattern[1:]
	}
	session := &SubSession{
		LastRequestTime: time.Now(),
		urlCtx:          urlCtx,
		hlsUrlPattern:   hlsUrlPattern,
		timeout:         timeout,
	}
	// stat
	session.stat.SessionId = base.GenUkHlsSubSession()
	session.stat.Protocol = base.SessionProtocolHlsStr
	session.stat.BaseType = base.SessionBaseTypeSubStr
	session.stat.RemoteAddr = req.RemoteAddr
	session.stat.StartTime = time.Now().String()

	// TODO(chef): [refactor] 也许后续可以弄短点，比如前8位或16位 202211
	session.sessionIdHash = nazamd5.Md5([]byte(session.stat.SessionId + sessionHashKey))
	return session
}

func (s *SubSession) Url() string {
	return s.urlCtx.Url
}

func (s *SubSession) AppName() string {
	if s.appName == "" {
		s.appName = GetAppNameFromUrlCtx(s.urlCtx, s.hlsUrlPattern)
	}
	return s.appName
}

func (s *SubSession) StreamName() string {
	return GetStreamNameFromUrlCtx(s.urlCtx)
}

func (s *SubSession) RawQuery() string {
	return s.urlCtx.RawQuery
}

func (s *SubSession) UpdateStat(intervalSec uint32) {
	wroteBytesSum := s.currStat.WroteBytesSum.Load()
	wDiff := wroteBytesSum - s.prevStat.WroteBytesSum
	s.stat.WriteBitrateKbits = int(wDiff * 8 / 1024 / uint64(intervalSec))
	s.stat.BitrateKbits = s.stat.WriteBitrateKbits
	s.prevStat.WroteBytesSum = wroteBytesSum
	return
}

func (s *SubSession) AddWroteBytesSum(wbs uint64) {
	s.currStat.WroteBytesSum.Add(wbs)
}

func (s *SubSession) GetStat() base.StatSession {
	s.stat.WroteBytesSum = s.currStat.WroteBytesSum.Load()
	return s.stat
}

func (s *SubSession) IsAlive() (readAlive, writeAlive bool) {
	if !s.IsExpired() {
		return true, true
	}
	return false, false
}

func (s *SubSession) IsExpired() bool {
	return s.LastRequestTime.Add(s.timeout).Before(time.Now())
}

func (s *SubSession) KeepAlive() {
	s.LastRequestTime = time.Now()
}

func GetAppNameFromUrlCtx(urlCtx base.UrlContext, hlsUrlPattern string) string {
	if hlsUrlPattern == "" {
		return urlCtx.PathWithoutLastItem
	} else {
		urlParts := strings.Split(urlCtx.PathWithoutLastItem, hlsUrlPattern)
		if len(urlParts) > 1 {
			return urlParts[1]
		}
	}
	return ""
}

func GetStreamNameFromUrlCtx(urlCtx base.UrlContext) string {
	filename := urlCtx.LastItemOfPath
	filetype := urlCtx.GetFileType()
	if filetype == "m3u8" && (filename == playlistM3u8FileName || filename == recordM3u8FileName) {
		uriItems := strings.Split(urlCtx.Path, "/")
		if l := len(uriItems); l >= 2 {
			return uriItems[len(uriItems)-2]
		}
	}
	fileNameWithoutType := urlCtx.GetFilenameWithoutType()
	return fileNameWithoutType
}
