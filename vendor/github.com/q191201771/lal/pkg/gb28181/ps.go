// Copyright 2022, Chef.  All rights reserved.
// https://github.com/q191201771/lal
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

package gb28181

const (
	psPackStartCodePackHeader       = 0x01ba
	psPackStartCodeSystemHeader     = 0x01bb
	psPackStartCodeProgramStreamMap = 0x01bc
	psPackStartCodeAudioStream      = 0x01c0
	psPackStartCodeVideoStream      = 0x01e0

	// TODO(chef): 0x01f2, 0x01f8
	psPackStartCodePesPrivate2 = 0x01bf // padding_stream_2
	psPackStartCodePesEcm      = 0x01f0 // ECM_stream
	psPackStartCodePesEmm      = 0x01f1 // EMM_stream
	psPackStartCodePesPadding  = 0x01be // padding_stream
	psPackStartCodePackEnd     = 0x01b9

	psPackStartCodeHikStream = 0x01bd
	psPackStartCodePesPsd    = 0x01ff // program_stream_directory

)

const (
	StreamTypeH264    uint8 = 0x1b
	StreamTypeH265          = 0x24
	StreamTypeAAC           = 0x0f
	StreamTypeG711A         = 0x90 //PCMA
	StreamTypeG711U         = 0x91 //PCMU
	StreamTypeG7221         = 0x92
	StreamTypeG7231         = 0x93
	StreamTypeG729          = 0x99
	StreamTypeUnknown       = -1
)

const psBufInitSize = 4096

const (
	PsHeaderlen     int = 14
	SysHeaderlen    int = 18
	SysMapHeaderLen int = 24
	PesHeaderLen    int = 19
)

const (
	MaxPesLen = 0xFFFF
)

const (
	StreamIdVideo = 0xe0
	StreamIdAudio = 0xc0
)

const adtsMinLen = 7
