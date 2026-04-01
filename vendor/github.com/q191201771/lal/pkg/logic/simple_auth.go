// Copyright 2022, Chef.  All rights reserved.
// https://github.com/q191201771/lal
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

package logic

import (
	"net/url"
	"strings"

	"github.com/q191201771/lal/pkg/base"
	"github.com/q191201771/naza/pkg/nazamd5"
)

func SimpleAuthCalcSecret(key string, streamName string) string {
	return nazamd5.Md5([]byte(key + streamName))
}

// ---------------------------------------------------------------------------------------------------------------------

// TODO(chef): [refactor] 结合 NotifyHandler 整理

const secretName = "lal_secret"

type SimpleAuthCtx struct {
	config SimpleAuthConfig
}

func NewSimpleAuthCtx(config SimpleAuthConfig) *SimpleAuthCtx {
	return &SimpleAuthCtx{
		config: config,
	}
}

func (s *SimpleAuthCtx) OnPubStart(info base.PubStartInfo) error {
	if s.config.PubRtmpEnable && info.Protocol == base.SessionProtocolRtmpStr ||
		s.config.PubRtspEnable && info.Protocol == base.SessionProtocolRtspStr {
		return s.check(info.StreamName, info.UrlParam)
	}
	return nil
}

func (s *SimpleAuthCtx) OnSubStart(info base.SubStartInfo) error {
	if (s.config.SubRtmpEnable && info.Protocol == base.SessionProtocolRtmpStr) ||
		(s.config.SubHttpflvEnable && info.Protocol == base.SessionProtocolFlvStr) ||
		(s.config.SubHttptsEnable && info.Protocol == base.SessionProtocolTsStr) ||
		(s.config.SubRtspEnable && info.Protocol == base.SessionProtocolRtspStr) {
		return s.check(info.StreamName, info.UrlParam)
	}
	return nil
}

func (s *SimpleAuthCtx) OnHls(streamName string, urlParam string) error {
	if s.config.HlsM3u8Enable {
		return s.check(streamName, urlParam)
	}
	return nil
}

func (s *SimpleAuthCtx) check(streamName string, urlParam string) error {
	q, err := url.ParseQuery(urlParam)
	if err != nil {
		return err
	}
	v := q.Get(secretName)
	if v == "" {
		return base.ErrSimpleAuthParamNotFound
	}
	v = strings.ToLower(v)

	// 注意，只有DangerousLalSecret配置了值，才验证参数是否和DangerousLalSecret相等
	if len(s.config.DangerousLalSecret) != 0 && v == s.config.DangerousLalSecret {
		return nil
	}

	se := SimpleAuthCalcSecret(s.config.Key, streamName)
	if v == se {
		return nil
	}

	Log.Warnf("[%p] SimpleAuthCtx::check failed, se=%s, key=%s, streamName=%s, v=%s, urlParam=%s",
		s, se, s.config.Key, streamName, v, urlParam)
	return base.ErrSimpleAuthFailed
}
