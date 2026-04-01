// Copyright 2020, Chef.  All rights reserved.
// https://github.com/q191201771/lal
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

package sdp

import (
	"strconv"
	"strings"

	"github.com/q191201771/lal/pkg/base"
	"github.com/q191201771/naza/pkg/nazaerrors"
)

type RawContext struct {
	MediaDescList []MediaDesc
}

type MediaDesc struct {
	M         M
	ARtpMap   ARtpMap
	AFmtPBase *AFmtPBase
	AControl  AControl
}

type M struct {
	Media string
	PT    int // 暂时只支持m只有一个pt值的情况
}

type ARtpMap struct {
	PayloadType        int
	EncodingName       string
	ClockRate          int
	EncodingParameters string
}

type AFmtPBase struct {
	Format     int               // same as PayloadType
	Parameters map[string]string // name -> value
}

type AControl struct {
	Value string
}

// ParseSdp2RawContext 例子见单元测试
func ParseSdp2RawContext(b []byte) (RawContext, error) {
	lines := strings.Split(string(b), "\r\n")
	ctx, err := parseSdp2RawContext(lines)
	if err == nil {
		return ctx, nil
	}

	// TestCase13，再尝试抢救一下
	var newlines []string
	i := 0
	for i < len(lines) {
		if strings.HasPrefix(lines[i], "a=fmtp") {
			newline := lines[i]
			j := i + 1
			// TODO(chef): 如果换行的数据刚好是`m=`或`a=`开头呢？
			for ; j < len(lines); j++ {
				if !strings.HasPrefix(lines[j], "m=") &&
					!strings.HasPrefix(lines[j], "a=") {
					newline += lines[j]
				} else {
					break
				}
			}
			newlines = append(newlines, newline)
			i = j
		} else {
			newlines = append(newlines, lines[i])
			i++
		}
	}
	return parseSdp2RawContext(newlines)
}

func ParseM(s string) (ret M, err error) {
	ss := strings.TrimPrefix(s, "m=")
	items := strings.Split(ss, " ")
	if len(items) < 1 {
		return ret, nazaerrors.Wrap(base.ErrSdp)
	}
	ret.Media = items[0]
	if len(items) > 3 {
		ret.PT, _ = strconv.Atoi(items[3])
	}
	return
}

// ParseARtpMap 例子见单元测试
func ParseARtpMap(s string) (ret ARtpMap, err error) {
	// rfc 3640 3.3.1.  General
	// rfc 3640 3.3.6.  High Bit-rate AAC
	//
	// a=rtpmap:<payload type> <encoding name>/<clock rate>[/<encoding parameters>]
	//

	items := strings.SplitN(s, ":", 2)
	if len(items) != 2 {
		err = nazaerrors.Wrap(base.ErrSdp)
		return
	}
	items = strings.SplitN(items[1], " ", 2)
	if len(items) != 2 {
		err = nazaerrors.Wrap(base.ErrSdp)
		return
	}
	ret.PayloadType, err = strconv.Atoi(items[0])
	if err != nil {
		return
	}
	items = strings.SplitN(items[1], "/", 3)
	switch len(items) {
	case 3:
		ret.EncodingParameters = items[2]
		fallthrough
	case 2:
		ret.EncodingName = items[0]
		ret.ClockRate, err = strconv.Atoi(items[1])
		if err != nil {
			return
		}
	default:
		err = nazaerrors.Wrap(base.ErrSdp)
	}
	return
}

// ParseAFmtPBase 例子见单元测试
func ParseAFmtPBase(s string) (ret AFmtPBase, err error) {
	// rfc 3640 4.4.1.  The a=fmtp Keyword
	//
	// a=fmtp:<format> <parameter name>=<value>[; <parameter name>=<value>]
	//

	ret.Parameters = make(map[string]string)

	items := strings.SplitN(s, ":", 2)
	if len(items) != 2 {
		err = nazaerrors.Wrap(base.ErrSdp)
		return
	}

	items = strings.SplitN(items[1], " ", 2)
	if len(items) != 2 {
		err = nazaerrors.Wrap(base.ErrSdp)
		return
	}

	ret.Format, err = strconv.Atoi(items[0])
	if err != nil {
		return
	}

	// 见TestCase11
	items[1] = strings.TrimLeft(items[1], ";")
	// 见TestCase12
	items[1] = strings.TrimRight(items[1], ";")

	items = strings.Split(items[1], ";")
	for _, pp := range items {
		pp = strings.TrimSpace(pp)
		kv := strings.SplitN(pp, "=", 2)
		if len(kv) != 2 {
			err = nazaerrors.Wrap(base.ErrSdp)
			return
		}
		ret.Parameters[kv[0]] = kv[1]
	}

	return
}

func ParseAControl(s string) (ret AControl, err error) {
	if !strings.HasPrefix(s, "a=control:") {
		err = nazaerrors.Wrap(base.ErrSdp)
		return
	}
	ret.Value = strings.TrimPrefix(s, "a=control:")
	return
}

// ---------------------------------------------------------------------------------------------------------------------

func parseSdp2RawContext(lines []string) (RawContext, error) {
	var (
		sdpCtx RawContext
		md     *MediaDesc
	)

	for _, line := range lines {
		if strings.HasPrefix(line, "m=") {
			m, err := ParseM(line)
			if err != nil {
				return sdpCtx, err
			}
			if md != nil {
				sdpCtx.MediaDescList = append(sdpCtx.MediaDescList, *md)
			}
			md = &MediaDesc{
				M: m,
			}
		}
		if strings.HasPrefix(line, "a=rtpmap") {
			aRtpMap, err := ParseARtpMap(line)
			if err != nil {
				return sdpCtx, err
			}
			if md == nil {
				continue
			}
			md.ARtpMap = aRtpMap
		}
		if strings.HasPrefix(line, "a=fmtp") {
			aFmtPBase, err := ParseAFmtPBase(line)
			if err != nil {
				return sdpCtx, err
			}
			if md == nil {
				continue
			}
			md.AFmtPBase = &aFmtPBase
		}
		if strings.HasPrefix(line, "a=control") {
			aControl, err := ParseAControl(line)
			if err != nil {
				return sdpCtx, err
			}
			if md == nil {
				continue
			}
			md.AControl = aControl
		}
	}
	if md != nil {
		sdpCtx.MediaDescList = append(sdpCtx.MediaDescList, *md)
	}

	return sdpCtx, nil
}
