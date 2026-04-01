// Copyright 2020, Chef.  All rights reserved.
// https://github.com/q191201771/lal
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

package sdp

import (
	"encoding/base64"
	"encoding/hex"
	"strings"

	"github.com/q191201771/naza/pkg/nazaerrors"

	"github.com/q191201771/lal/pkg/base"
)

func ParseAsc(a *AFmtPBase) ([]byte, error) {
	// AAC的这个地方除了97，还遇到过104的，暂时不要这个判断了
	//if a.Format != base.RtpPacketTypeAac {
	//	return nil, nazaerrors.Wrap(base.ErrSdp)
	//}

	v, ok := a.Parameters["config"]
	if !ok {
		return nil, nazaerrors.Wrap(base.ErrSdp)
	}
	if len(v) < 4 || (len(v)%2) != 0 {
		return nil, nazaerrors.Wrap(base.ErrSdp)
	}
	return hex.DecodeString(v)
}

func ParseVpsSpsPps(a *AFmtPBase) (vps, sps, pps []byte, err error) {
	v, ok := a.Parameters["sprop-vps"]
	if !ok {
		return nil, nil, nil, nazaerrors.Wrap(base.ErrSdp)
	}
	if vps, err = base64.StdEncoding.DecodeString(v); err != nil {
		return nil, nil, nil, err
	}

	v, ok = a.Parameters["sprop-sps"]
	if !ok {
		return nil, nil, nil, nazaerrors.Wrap(base.ErrSdp)
	}
	if sps, err = base64.StdEncoding.DecodeString(v); err != nil {
		return nil, nil, nil, err
	}

	v, ok = a.Parameters["sprop-pps"]
	if !ok {
		return nil, nil, nil, nazaerrors.Wrap(base.ErrSdp)
	}
	if pps, err = base64.StdEncoding.DecodeString(v); err != nil {
		return nil, nil, nil, err
	}

	return
}

// ParseSpsPps
//
// 解析AVC/H264的sps，pps
// 例子见单元测试
func ParseSpsPps(a *AFmtPBase) (sps, pps []byte, err error) {
	v, ok := a.Parameters["sprop-parameter-sets"]
	if !ok {
		return nil, nil, nazaerrors.Wrap(base.ErrSdp)
	}

	items := strings.SplitN(v, ",", 2)
	if len(items) != 2 {
		return nil, nil, nazaerrors.Wrap(base.ErrSdp)
	}

	sps, err = base64.StdEncoding.DecodeString(items[0])
	if err != nil {
		return nil, nil, nazaerrors.Wrap(base.ErrSdp)
	}

	pps, err = base64.StdEncoding.DecodeString(items[1])
	return
}
