// Copyright 2020, Chef.  All rights reserved.
// https://github.com/q191201771/lal
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

package rtprtcp

// (70 * 365 + 17) * 24 * 60 * 60
const ntpOffset uint64 = 2208988800

// Ntp2UnixNano 将ntp时间戳转换为Unix时间戳，Unix时间戳单位是纳秒
func Ntp2UnixNano(v uint64) uint64 {
	msw := v >> 32
	lsw := v & 0xFFFFFFFF
	return (msw-ntpOffset)*1e9 + (lsw*1e9)>>32
}

// MswLsw2UnixNano 将ntp时间戳（高32位低32位分开的形式）转换为Unix时间戳
func MswLsw2UnixNano(msw, lsw uint64) uint64 {
	return Ntp2UnixNano(MswLsw2Ntp(msw, lsw))
}

// MswLsw2Ntp msw是ntp的高32位，lsw是ntp的低32位
func MswLsw2Ntp(msw, lsw uint64) uint64 {
	return (msw << 32) | lsw
}

//func UnixNano2Ntp(v uint64) uint64 {
//	msw := v / 1e9 + offset
//	lsw := ((v % 1e9) << 32) / 1e9
//	return (msw << 32) | lsw
//}
