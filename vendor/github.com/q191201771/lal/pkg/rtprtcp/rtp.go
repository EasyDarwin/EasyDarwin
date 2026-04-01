// Copyright 2020, Chef.  All rights reserved.
// https://github.com/q191201771/lal
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

package rtprtcp

// h264的格式：
//
// rfc3984 5.2.  Common Structure of the RTP Payload Format
// Table 1.  Summary of NAL unit types and their payload structures
//
// Type   Packet    Type name                        Section
// ---------------------------------------------------------
// 0      undefined                                    -
// 1-23   NAL unit  Single NAL unit packet per H.264   5.6
// 24     STAP-A    Single-time aggregation packet     5.7.1
// 25     STAP-B    Single-time aggregation packet     5.7.1
// 26     MTAP16    Multi-time aggregation packet      5.7.2
// 27     MTAP24    Multi-time aggregation packet      5.7.2
// 28     FU-A      Fragmentation unit                 5.8
// 29     FU-B      Fragmentation unit                 5.8
// 30-31  undefined                                    -
//
// h265的格式：
//
// rfc7798
// 4.4.2.  Aggregation Packets (APs)
// 4.4.3.  Fragmentation Units

const (
	NaluTypeAvcSingleMax = 23
	NaluTypeAvcStapa     = 24 // one packet, multiple nals
	NaluTypeAvcFua       = 28

	NaluTypeHevcAp  = 48
	NaluTypeHevcFua = 49
)

// CompareSeq 比较序号的值，内部处理序号翻转问题，见单元测试中的例子
//
// @return
//   - 0 a和b相等
//   - 1 a大于b
//   - -1 a小于b
func CompareSeq(a, b uint16) int {
	if a == b {
		return 0
	}
	if a > b {
		if a-b < 32768 {
			return 1
		}

		return -1
	}

	// must be a < b
	if b-a < 32768 {
		return -1
	}

	return 1
}

// SubSeq a减b的值，内部处理序号翻转问题，如果a小于b，则返回负值，见单元测试中的例子
func SubSeq(a, b uint16) int {
	if a == b {
		return 0
	}

	if a > b {
		d := a - b
		if d < 16384 {
			return int(d)
		}
		return int(d) - 65536
	}

	// must be a < b
	d := b - a
	if d < 16384 {
		return -int(d)
	}

	return 65536 - int(d)
}
