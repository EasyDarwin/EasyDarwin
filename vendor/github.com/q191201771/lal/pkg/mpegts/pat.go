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

// Pat ---------------------------------------------------------------------------------------------------
// Program association section
// <iso13818-1.pdf> <2.4.4.3> <page 61/174>
// table_id                 [8b] *
// section_syntax_indicator [1b]
// '0'                      [1b]
// reserved                 [2b]
// section_length           [12b] **
// transport_stream_id      [16b] **
// reserved                 [2b]
// version_number           [5b]
// current_next_indicator   [1b]  *
// section_number           [8b]  *
// last_section_number      [8b]  *
// -----loop-----
// program_number           [16b] **
// reserved                 [3b]
// program_map_PID          [13b] ** if program_number == 0 then network_PID else then program_map_PID
// --------------
// CRC_32                   [32b] ****
// ---------------------------------------------------------------------------------------------------
type Pat struct {
	tid   uint8
	ssi   uint8
	sl    uint16
	tsi   uint16
	vn    uint8
	cni   uint8
	sn    uint8
	lsn   uint8
	ppes  []PatProgramElement
	crc32 uint32
}

type PatProgramElement struct {
	pn    uint16
	pmpid uint16
}

func ParsePat(b []byte) (pat Pat) {
	// TODO chef: 检查长度
	br := nazabits.NewBitReader(b)
	pat.tid, _ = br.ReadBits8(8)
	pat.ssi, _ = br.ReadBits8(1)
	_, _ = br.ReadBits8(3)
	pat.sl, _ = br.ReadBits16(12)
	pat.tsi, _ = br.ReadBits16(16)
	_, _ = br.ReadBits8(2)
	pat.vn, _ = br.ReadBits8(5)
	pat.cni, _ = br.ReadBits8(1)
	pat.sn, _ = br.ReadBits8(8)
	pat.lsn, _ = br.ReadBits8(8)

	length := pat.sl - 9

	for i := uint16(0); i < length; i += 4 {
		var ppe PatProgramElement
		ppe.pn, _ = br.ReadBits16(16)
		_, _ = br.ReadBits8(3)
		// TODO chef if pn == 0
		ppe.pmpid, _ = br.ReadBits16(13)
		pat.ppes = append(pat.ppes, ppe)
	}
	pat.crc32, _ = br.ReadBits32(32)
	return
}

func (pat *Pat) SearchPid(pid uint16) bool {
	for _, ppe := range pat.ppes {
		if pid == ppe.pmpid {
			return true
		}
	}
	return false
}

func PackPat() []byte {
	ts := make([]byte, 188)

	tsheader := []byte{0x47, 0x40, 0x00, 0x10}
	copy(ts, tsheader)

	psi := NewPsi()
	psi.sectionData.header.tableId = TsPsiIdPas
	psi.sectionData.header.sectionSyntaxIndicator = 1
	psi.sectionData.section.tableIdExtension = 1
	psi.sectionData.section.currentNextIndicator = 1
	psi.sectionData.patData.pes = append(psi.sectionData.patData.pes, PatProgramElement{
		pn:    1,
		pmpid: PidPmt,
	})

	psilen, psiData := psi.Pack()
	copy(ts[4:], psiData)

	stuffinglen := 188 - 4 - psilen
	for i := 0; i < stuffinglen; i++ {
		ts[4+psilen+i] = 0xff
	}

	return ts
}
