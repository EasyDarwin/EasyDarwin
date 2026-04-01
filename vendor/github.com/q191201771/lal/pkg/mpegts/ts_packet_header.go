// Copyright 2020, Chef.  All rights reserved.
// https://github.com/q191201771/lal
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

package mpegts

import (
	"github.com/q191201771/naza/pkg/nazabits"
)

// TsPacketHeader ------------------------------------------------
// <iso13818-1.pdf> <2.4.3.2> <page 36/174>
// sync_byte                    [8b]  * always 0x47
// transport_error_indicator    [1b]
// payload_unit_start_indicator [1b]
// transport_priority           [1b]
// PID                          [13b] **
// transport_scrambling_control [2b]
// adaptation_field_control     [2b]
// continuity_counter           [4b]  *
// ------------------------------------------------
type TsPacketHeader struct {
	Sync             uint8
	Err              uint8
	PayloadUnitStart uint8
	Prio             uint8
	Pid              uint16
	Scra             uint8
	Adaptation       uint8
	Cc               uint8
}

// TsPacketAdaptation ----------------------------------------------------------
// <iso13818-1.pdf> <Table 2-6> <page 40/174>
// adaptation_field_length              [8b] * 不包括自己这1字节
// discontinuity_indicator              [1b]
// random_access_indicator              [1b]
// elementary_stream_priority_indicator [1b]
// PCR_flag                             [1b]
// OPCR_flag                            [1b]
// splicing_point_flag                  [1b]
// transport_private_data_flag          [1b]
// adaptation_field_extension_flag      [1b] *
// -----if PCR_flag == 1-----
// program_clock_reference_base         [33b]
// reserved                             [6b]
// program_clock_reference_extension    [9b] ******
// ----------------------------------------------------------
type TsPacketAdaptation struct {
	Length uint8
}

// ParseTsPacketHeader 解析4字节TS Packet header
func ParseTsPacketHeader(b []byte) (h TsPacketHeader) {
	// TODO chef: 检查长度
	br := nazabits.NewBitReader(b)
	h.Sync, _ = br.ReadBits8(8)
	h.Err, _ = br.ReadBits8(1)
	h.PayloadUnitStart, _ = br.ReadBits8(1)
	h.Prio, _ = br.ReadBits8(1)
	h.Pid, _ = br.ReadBits16(13)
	h.Scra, _ = br.ReadBits8(2)
	h.Adaptation, _ = br.ReadBits8(2)
	h.Cc, _ = br.ReadBits8(4)
	return
}

// ParseTsPacketAdaptation TODO chef
func ParseTsPacketAdaptation(b []byte) (f TsPacketAdaptation) {
	br := nazabits.NewBitReader(b)
	f.Length, _ = br.ReadBits8(8)
	return
}
