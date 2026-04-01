// Copyright 2023, Chef.  All rights reserved.
// https://github.com/q191201771/lal
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

package mpegts

import (
	"github.com/q191201771/naza/pkg/bele"
	"github.com/q191201771/naza/pkg/nazabits"
)

// PsiId
const (
	TsPsiIdPas            = 0x00 // program_association_section
	TsPsiIdCas            = 0x01 // conditional_access_section (CA_section)
	TsPsiIdPms            = 0x02 // TS_program_map_section
	TsPsiIdDs             = 0x03 // TS_description_section
	TsPsiIdSds            = 0x04 // ISO_IEC_14496_scene_description_section
	TsPsiIdOds            = 0x05 // ISO_IEC_14496_object_descriptor_section
	TsPsiIdIso138181Start = 0x06 // ITU-T Rec. H.222.0 | ISO/IEC 13818-1 reserved
	TsPsiIdIso138181End   = 0x37
	TsPsiIdIso138186Start = 0x38 // Defined in ISO/IEC 13818-6
	TsPsiIdIso138186End   = 0x3F
	TsPsiIdUserStart      = 0x40 // User private
	TsPsiIdUserEnd        = 0xFE
	TsPsiIdForbidden      = 0xFF // forbidden
)

const (
	DescriptorTagAC3                        = 0x6a
	DescriptorTagAVCVideo                   = 0x28
	DescriptorTagComponent                  = 0x50
	DescriptorTagContent                    = 0x54
	DescriptorTagDataStreamAlignment        = 0x6
	DescriptorTagEnhancedAC3                = 0x7a
	DescriptorTagExtendedEvent              = 0x4e
	DescriptorTagExtension                  = 0x7f
	DescriptorTagISO639LanguageAndAudioType = 0xa
	DescriptorTagLocalTimeOffset            = 0x58
	DescriptorTagMaximumBitrate             = 0xe
	DescriptorTagNetworkName                = 0x40
	DescriptorTagParentalRating             = 0x55
	DescriptorTagPrivateDataIndicator       = 0xf
	DescriptorTagPrivateDataSpecifier       = 0x5f
	DescriptorTagRegistration               = 0x5
	DescriptorTagService                    = 0x48
	DescriptorTagShortEvent                 = 0x4d
	DescriptorTagStreamIdentifier           = 0x52
	DescriptorTagSubtitling                 = 0x59
	DescriptorTagTeletext                   = 0x56
	DescriptorTagVBIData                    = 0x45
	DescriptorTagVBITeletext                = 0x46
)

const (
	opusIdentifier = 0x4f707573 // Opus
)

type PsiSection struct {
	pointerFileld uint8
	sectionData   PsiSectionData
}

type PsiSectionData struct {
	header  PsiTableHeader
	section PsiTableSyntaxSection
	patData PatSpecificData
	pmtData PmtSpecificData
}

type PsiTableHeader struct {
	tableId                uint8
	sectionSyntaxIndicator uint8
	sectionLength          uint16
}

type PsiTableSyntaxSection struct {
	tableIdExtension     uint16
	versionNumber        uint8
	currentNextIndicator uint8
	sectionNumber        uint8
	lastSectionNumber    uint8
	tableData            []byte
	crc32                uint32
}

type PatSpecificData struct {
	pes []PatProgramElement
}

type PmtSpecificData struct {
	pcrPid            uint16
	programInfoLength uint16
	pes               []PmtProgramElement
}

func NewPsi() *PsiSection {
	return &PsiSection{
		pointerFileld: 0x00,
	}
}

func (psi *PsiSection) Pack() (int, []byte) {
	psiSection := make([]byte, 1+3+psi.calcPsiSectionLength())
	bw := nazabits.NewBitWriter(psiSection)

	bw.WriteBits8(8, psi.pointerFileld)
	psi.writePsiTableHeader(&bw)
	psi.writePsiTableSyntaxSection(&bw)

	crc := CalcCrc32(0xffffffff, psiSection[1:len(psiSection)-4])
	bele.LePutUint32(psiSection[4+psi.calcPsiSectionLength()-4:], crc)

	return int(1 + 3 + psi.calcPsiSectionLength()), psiSection
}

func (psi *PsiSection) writePsiTableHeader(bw *nazabits.BitWriter) {
	bw.WriteBits8(8, psi.sectionData.header.tableId)
	bw.WriteBit(psi.sectionData.header.sectionSyntaxIndicator)
	bw.WriteBit(0)
	bw.WriteBits8(2, 0xff)

	psi.sectionData.header.sectionLength = psi.calcPsiSectionLength()
	bw.WriteBits16(12, psi.sectionData.header.sectionLength)

	return
}

func (psi *PsiSection) writePsiTableSyntaxSection(bw *nazabits.BitWriter) {
	psi.writePsiTableSyntaxSectionHeader(bw)
	psi.writePsiTableSyntaxSectionData(bw)

	return
}

func (psi *PsiSection) writePsiTableSyntaxSectionHeader(bw *nazabits.BitWriter) {
	bw.WriteBits16(16, psi.sectionData.section.tableIdExtension)
	bw.WriteBits8(2, 0xff)
	bw.WriteBits8(5, psi.sectionData.section.versionNumber)
	bw.WriteBit(psi.sectionData.section.currentNextIndicator)
	bw.WriteBits8(8, psi.sectionData.section.sectionNumber)
	bw.WriteBits8(8, psi.sectionData.section.lastSectionNumber)
	return
}

func (psi *PsiSection) writePsiTableSyntaxSectionData(bw *nazabits.BitWriter) {
	switch psi.sectionData.header.tableId {
	case TsPsiIdPas:
		psi.writePatSection(bw)
	case TsPsiIdPms:
		psi.writePmtSection(bw)
	}

	return
}

func (psi *PsiSection) calcPsiSectionLength() (length uint16) {
	if psi.sectionData.header.tableId == TsPsiIdPas || psi.sectionData.header.tableId == TsPsiIdPms {
		// Table ID extension(16 bits)+Reserved bits(2 bits)+Version number(5 bits)+Current next Indicator(1 bit)+Section number(8 bits)+Last section number(8 bits)
		length += 5
	}

	switch psi.sectionData.header.tableId {
	case TsPsiIdPas:
		length += psi.calaPatSectionLength()
	case TsPsiIdPms:
		length += psi.calaPmtSectionLength()
	}

	length += 4 //crc32

	return
}

func (psi *PsiSection) calaPatSectionLength() (length uint16) {
	length = uint16(4 * len(psi.sectionData.patData.pes))
	return
}

func (psi *PsiSection) calaPmtSectionLength() (length uint16) {
	// Reserved bits(3 bits)+PCR PID(13 bits)+Reserved bits(4 bits)+Program info length(12 bits)
	length = 4

	for _, pe := range psi.sectionData.pmtData.pes {
		length += 5

		if len(pe.Descriptors) > 0 {
			length += psi.calcDescriptorsLength(pe.Descriptors)
		}
	}

	return
}

func (psi *PsiSection) calcDescriptorsLength(ds []Descriptor) uint16 {
	length := uint16(0)
	for _, d := range ds {
		length += 2 // tag and length
		length += uint16(psi.calcDescriptorLength(d))
	}
	return length
}

func (psi *PsiSection) calcDescriptorLength(d Descriptor) uint8 {
	if d.Length == 0 {
		return 0
	}

	switch d.Tag {
	case DescriptorTagRegistration:
		return psi.calcDescriptorRegistrationLength(d.Registration)
	case DescriptorTagExtension:
		return psi.calcDescriptorExtensionLength(d.Extension)
	}

	return 0
}

func (psi *PsiSection) calcDescriptorRegistrationLength(d DescriptorRegistration) uint8 {
	return uint8(4 + len(d.AdditionalIdentificationInfo))
}

func (psi *PsiSection) calcDescriptorExtensionLength(d DescriptorExtension) uint8 {
	// tag
	ret := 1
	if d.Unknown != nil {
		ret += len(d.Unknown)
	}

	return uint8(ret)
}

func (psi *PsiSection) writePatSection(bw *nazabits.BitWriter) {
	for _, pe := range psi.sectionData.patData.pes {
		bw.WriteBits16(16, pe.pn)
		bw.WriteBits8(3, 0xff)
		bw.WriteBits16(13, pe.pmpid)
	}

	return
}

func (psi *PsiSection) writePmtSection(bw *nazabits.BitWriter) {
	bw.WriteBits8(3, 0xff)
	bw.WriteBits16(13, psi.sectionData.pmtData.pcrPid)
	bw.WriteBits8(4, 0xff)
	bw.WriteBits16(12, psi.sectionData.pmtData.programInfoLength)

	for _, pe := range psi.sectionData.pmtData.pes {
		bw.WriteBits8(8, pe.StreamType)
		bw.WriteBits8(3, 0xff)
		bw.WriteBits16(13, pe.Pid)
		psi.writeDescriptorsWithLength(bw, pe.Descriptors)
	}
	return
}

func (psi *PsiSection) writeDescriptorsWithLength(bw *nazabits.BitWriter, dps []Descriptor) {
	bw.WriteBits8(4, 0xff)

	infolen := psi.calcDescriptorsLength(dps)
	bw.WriteBits16(12, infolen)

	for _, dp := range dps {
		psi.writeDescriptor(bw, dp)
	}
}

func (psi *PsiSection) writeDescriptor(bw *nazabits.BitWriter, d Descriptor) {
	length := psi.calcDescriptorLength(d)

	bw.WriteBits8(8, d.Tag)
	bw.WriteBits8(8, length)

	switch d.Tag {
	case DescriptorTagRegistration:
		psi.writeDescriptorRegistration(bw, d.Registration)
	case DescriptorTagExtension:
		psi.writeDescriptorExtension(bw, d.Extension)
	}
}

func (psi *PsiSection) writeDescriptorRegistration(bw *nazabits.BitWriter, d DescriptorRegistration) {
	bw.WriteBits16(16, uint16((d.FormatIdentifier>>16)&0xFFFF))
	bw.WriteBits16(16, uint16(d.FormatIdentifier&0xFFFF))

	if len(d.AdditionalIdentificationInfo) > 0 {
		for _, b := range d.AdditionalIdentificationInfo {
			bw.WriteBits8(8, b)
		}
	}
}

func (psi *PsiSection) writeDescriptorExtension(bw *nazabits.BitWriter, d DescriptorExtension) {
	bw.WriteBits8(8, d.Tag)

	if len(d.Unknown) > 0 {
		for _, b := range d.Unknown {
			bw.WriteBits8(8, b)
		}
	}
}

type Descriptor struct {
	Length       uint8
	Tag          uint8
	Registration DescriptorRegistration
	Extension    DescriptorExtension
}

type DescriptorRegistration struct {
	AdditionalIdentificationInfo []byte
	FormatIdentifier             uint32
}

type DescriptorExtension struct {
	SupplementaryAudio DescriptorExtensionSupplementaryAudio
	Tag                uint8
	Unknown            []byte
}

type DescriptorExtensionSupplementaryAudio struct {
	EditorialClassification uint8
	HasLanguageCode         bool
	LanguageCode            []byte
	MixType                 bool
	PrivateData             []byte
}
