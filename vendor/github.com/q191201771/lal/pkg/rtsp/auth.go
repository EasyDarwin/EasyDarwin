// Copyright 2020, Chef.  All rights reserved.
// https://github.com/q191201771/lal
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

package rtsp

import (
	"crypto/rand"
	"encoding/base64"
	"fmt"
	"strings"

	"github.com/q191201771/lal/pkg/base"

	"github.com/q191201771/naza/pkg/nazamd5"
)

// TODO chef: 考虑部分内容移入naza中

const (
	AuthTypeDigest = "Digest"
	AuthTypeBasic  = "Basic"
	AuthAlgorithm  = "MD5"
)

type Auth struct {
	Username string
	Password string

	Typ       string
	Realm     string
	Nonce     string
	Algorithm string
	Uri       string
	Response  string
	Opaque    string // 暂时没用
	Stale     string // 暂时没用
}

// ParseAuthorization 解析字段，server side使用
func (a *Auth) ParseAuthorization(authStr string) (err error) {
	switch {
	case strings.HasPrefix(authStr, "Basic "):
		a.Typ = AuthTypeDigest
		authBase64Str := strings.TrimPrefix(authStr, "Basic ")

		authInfo, err := base64.StdEncoding.DecodeString(authBase64Str)
		if err != nil {
			return err
		}

		tmp := strings.Split(string(authInfo), ":")
		if len(tmp) != 2 {
			return fmt.Errorf("invalid Authorization:%s", authStr)
		}

		a.Username, a.Password = tmp[0], tmp[1]

	case strings.HasPrefix(authStr, "Digest "):
		a.Typ = AuthTypeDigest

		authDigestStr := strings.TrimPrefix(authStr, "Digest ")
		a.Username = a.getV(authDigestStr, `username="`)
		a.Realm = a.getV(authDigestStr, `realm="`)
		a.Nonce = a.getV(authDigestStr, `nonce="`)
		a.Uri = a.getV(authDigestStr, `uri="`)
		a.Algorithm = a.getV(authDigestStr, `algorithm="`)
		a.Response = a.getV(authDigestStr, `response="`)
		a.Opaque = a.getV(authDigestStr, `opaque="`)
		a.Stale = a.getV(authDigestStr, `stale="`)
	}

	return nil
}

// FeedWwwAuthenticate 使用第一轮回复，client side使用
func (a *Auth) FeedWwwAuthenticate(auths []string, username, password string) {
	a.Username = username
	a.Password = password
	//目前只处理第一个
	var s string
	if len(auths) > 0 {
		s = auths[0]
	} else {
		return
	}
	s = strings.TrimPrefix(s, HeaderWwwAuthenticate)
	s = strings.TrimSpace(s)
	if strings.HasPrefix(s, AuthTypeBasic) {
		a.Typ = AuthTypeBasic
		return
	}
	if !strings.HasPrefix(s, AuthTypeDigest) {
		Log.Warnf("FeedWwwAuthenticate type invalid. v=%s", s)
		return
	}

	a.Typ = AuthTypeDigest
	a.Realm = a.getV(s, `realm="`)
	a.Nonce = a.getV(s, `nonce="`)
	a.Algorithm = a.getV(s, `algorithm="`)

	if a.Realm == "" {
		Log.Warnf("FeedWwwAuthenticate realm invalid. v=%s", s)
	}
	if a.Nonce == "" {
		Log.Warnf("FeedWwwAuthenticate realm invalid. v=%s", s)
	}
	if a.Algorithm == "" {
		a.Algorithm = AuthAlgorithm
		Log.Warnf("FeedWwwAuthenticate algorithm not found fallback to %s. v=%s", AuthAlgorithm, s)
	}
	if a.Algorithm != AuthAlgorithm {
		Log.Warnf("FeedWwwAuthenticate algorithm invalid, only support MD5. v=%s", s)
	}
}

// MakeAuthorization 生成第二轮请求，client side使用
//
// 如果没有调用`FeedWwwAuthenticate`初始化过，则直接返回空字符串
func (a *Auth) MakeAuthorization(method, uri string) string {
	if a.Username == "" {
		return ""
	}
	switch a.Typ {
	case AuthTypeBasic:
		base1 := base64.StdEncoding.EncodeToString([]byte(fmt.Sprintf(`%s:%s`, a.Username, a.Password)))
		return fmt.Sprintf(`%s %s`, a.Typ, base1)
	case AuthTypeDigest:
		ha1 := nazamd5.Md5([]byte(fmt.Sprintf("%s:%s:%s", a.Username, a.Realm, a.Password)))
		ha2 := nazamd5.Md5([]byte(fmt.Sprintf("%s:%s", method, uri)))
		response := nazamd5.Md5([]byte(fmt.Sprintf("%s:%s:%s", ha1, a.Nonce, ha2)))
		return fmt.Sprintf(`%s username="%s", realm="%s", nonce="%s", uri="%s", response="%s", algorithm="%s"`, a.Typ, a.Username, a.Realm, a.Nonce, uri, response, a.Algorithm)
	}

	return ""
}

// MakeAuthenticate 生成第一轮的回复，server side使用
func (a *Auth) MakeAuthenticate(method string) string {
	switch method {
	case AuthTypeBasic:
		return fmt.Sprintf("%s realm=\"%s\"", method, base.LalRtspRealm)
	case AuthTypeDigest:
		return fmt.Sprintf("%s realm=\"%s\", nonce=\"%s\"", method, base.LalRtspRealm, a.nonce())
	}
	return ""
}

// CheckAuthorization 验证第二轮请求，server side使用
func (a *Auth) CheckAuthorization(method, username, password string) bool {
	switch a.Typ {
	case AuthTypeBasic:
		if username == a.Username && password == a.Password {
			return true
		}
	case AuthTypeDigest:
		// The "response" field is computed as:
		// md5(md5(<username>:<realm>:<password>):<nonce>:md5(<cmd>:<url>))

		ha1 := nazamd5.Md5([]byte(fmt.Sprintf("%s:%s:%s", username, a.Realm, password)))
		ha2 := nazamd5.Md5([]byte(fmt.Sprintf("%s:%s", method, a.Uri)))
		response := nazamd5.Md5([]byte(fmt.Sprintf("%s:%s:%s", ha1, a.Nonce, ha2)))
		if a.Response == response {
			return true
		}
	}
	return false
}

// ---------------------------------------------------------------------------------------------------------------------

func (a *Auth) getV(s string, pre string) string {
	b := strings.Index(s, pre)
	if b == -1 {
		return ""
	}
	e := strings.Index(s[b+len(pre):], `"`)
	if e == -1 {
		return ""
	}
	return s[b+len(pre) : b+len(pre)+e]
}

func (a *Auth) nonce() string {
	k := make([]byte, 32)
	for bytes := 0; bytes < len(k); {
		n, _ := rand.Read(k[bytes:])
		bytes += n
	}

	return nazamd5.Md5(k)
}
