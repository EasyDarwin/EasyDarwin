// Copyright 2020, Chef.  All rights reserved.
// https://github.com/q191201771/lal
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

package mpegts

import (
	"encoding/hex"
	"fmt"
	"github.com/q191201771/naza/pkg/nazabytes"
)

// Frame 帧数据，用于打包成mpegts格式的数据
type Frame struct {
	Pts uint64 // =(毫秒 * 90)
	Cts uint32
	Dts uint64
	Cc  uint8 // continuity_counter of TS Header

	// PID of PES Header
	// 音频 mpegts.PidAudio
	// 视频 mpegts.PidVideo
	Pid uint16

	// stream_id of PES Header
	// 音频 mpegts.StreamIdAudio
	// 视频 mpegts.StreamIdVideo
	Sid uint8

	// 音频 全部为false
	// 视频 关键帧为true，非关键帧为false
	Key bool

	// 音频AAC 格式为2字节ADTS头加raw frame
	// 视频AVC 格式为Annexb
	Raw []byte
}

// Pack annexb格式的流转换为mpegts流
//
// 注意，内部会增加 Frame.Cc 的值.
//
// @return: 内存块为独立申请，调度结束后，内部不再持有
func (frame *Frame) Pack() []byte {
	bufLen := len(frame.Raw) * 2 // 预分配一块足够大的内存
	if bufLen < 1024 {
		bufLen = 1024
	}
	// TODO(chef): perf 复用这块buffer
	buf := make([]byte, bufLen)

	lpos := 0              // 当前输入帧的处理位置
	rpos := len(frame.Raw) // 当前输入帧大小
	first := true          // 是否为帧的首个packet的标准
	packetPosAtBuf := 0    // 当前输出packet相对于整个输出内存块的位置

	for lpos != rpos {

		// TODO(chef): CHEFNOTICEME 正常来说，预分配的内存应该是足够用了，我们加个扩容逻辑保证绝对正确性，并且加个日志观察一段时间
		if packetPosAtBuf+188 > len(buf) {
			Log.Warnf("buffer too short. frame size=%d, buf=%d, packetPosAtBuf=%d", len(frame.Raw), len(buf), packetPosAtBuf)
			newBuf := make([]byte, packetPosAtBuf+188)
			copy(newBuf, buf)
			buf = newBuf
		}

		packet := buf[packetPosAtBuf : packetPosAtBuf+188] // 当前输出packet
		wpos := 0                                          // 当前输出packet的写入位置
		packetPosAtBuf += 188

		frame.Cc++

		// 每个packet都需要添加TS Header
		// -----TS Header----------------
		// sync_byte
		// transport_error_indicator    0
		// payload_unit_start_indicator
		// transport_priority           0
		// PID
		// transport_scrambling_control 0
		// adaptation_field_control
		// continuity_counter
		// ------------------------------
		packet[0] = syncByte // sync_byte
		packet[1] = 0x0
		if first {
			packet[1] = 0x40 // payload_unit_start_indicator
		}
		packet[1] |= uint8((frame.Pid >> 8) & 0x1F) //PID高5位
		packet[2] = uint8(frame.Pid & 0xFF)         //PID低8位

		// adaptation_field_control 先设置成无Adaptation
		// continuity_counter
		packet[3] = 0x10 | (frame.Cc & 0x0f)
		wpos += 4

		if first {
			if frame.Key {
				// 关键帧的首个packet需要添加Adaptation
				// -----Adaptation-----------------------
				// adaptation_field_length
				// discontinuity_indicator              0
				// random_access_indicator              1
				// elementary_stream_priority_indicator 0
				// PCR_flag                             1
				// OPCR_flag                            0
				// splicing_point_flag                  0
				// transport_private_data_flag          0
				// adaptation_field_extension_flag      0
				// program_clock_reference_base
				// reserved
				// program_clock_reference_extension
				// --------------------------------------
				packet[3] |= 0x20 // adaptation_field_control 设置Adaptation
				packet[4] = 7     // adaptation_field_length
				packet[5] = 0x50  // random_access_indicator + PCR_flag

				pcr := uint64(0)
				if frame.Dts > delay {
					pcr = frame.Dts - delay
				}
				packPcr(packet[6:], pcr) // using 6 byte

				wpos += 8
			}

			// 帧的首个packet需要添加PES Header
			// -----PES Header------------
			// packet_start_code_prefix
			// stream_id
			// PES_packet_length
			// '10'
			// PES_scrambling_control    0
			// PES_priority              0
			// data_alignment_indicator  0
			// copyright                 0
			// original_or_copy          0
			// PTS_DTS_flags
			// ESCR_flag                 0
			// ES_rate_flag              0
			// DSM_trick_mode_flag       0
			// additional_copy_info_flag 0
			// PES_CRC_flag              0
			// PES_extension_flag        0
			// PES_header_data_length
			// ---------------------------
			packet[wpos] = 0x00        // packet_start_code_prefix 24-bits
			packet[wpos+1] = 0x00      //
			packet[wpos+2] = 0x01      //
			packet[wpos+3] = frame.Sid // stream_id
			wpos += 4

			// 计算PES Header中一些字段的值
			// PTS相关
			headerSize := uint8(5)
			flags := uint8(0x80)
			// DTS相关
			if frame.Dts != frame.Pts {
				headerSize += 5
				flags |= 0x40
			}

			pesSize := rpos + int(headerSize) + 3 // PES Header剩余3字节 + PTS/PTS长度 + 整个帧的长度
			if pesSize > 0xFFFF {
				pesSize = 0
			}

			packet[wpos] = uint8(pesSize >> 8)     // PES_packet_length
			packet[wpos+1] = uint8(pesSize & 0xFF) //
			packet[wpos+2] = 0x80                  // 除了reserve的'10'，其他字段都是0
			packet[wpos+3] = flags                 // PTS/DTS flag
			packet[wpos+4] = headerSize            // PES_header_data_length: PTS+DTS数据长度
			wpos += 5

			// 写入PTS的值
			packPts(packet[wpos:], flags>>6, frame.Pts+delay)
			wpos += 5
			// 写入DTS的值
			if frame.Pts != frame.Dts {
				packPts(packet[wpos:], 1, frame.Dts+delay)
				wpos += 5
			}

			first = false
		}

		// 把帧的内容切割放入packet中
		bodySize := 188 - wpos // 当前TS packet，可写入大小
		inSize := rpos - lpos  // 整个帧剩余待打包大小

		if bodySize <= inSize {
			// 当前packet写不完这个帧，或者刚好够写完
			copy(packet[wpos:], frame.Raw[lpos:lpos+inSize])
			lpos += bodySize
		} else {
			// 当前packet可以写完这个帧，并且还有空闲空间
			// 此时，真实数据挪最后，中间用0xFF填充到Adaptation中
			// 注意，此时有两种情况
			// 1. 原本有Adaptation
			// 2. 原本没有Adaptation

			stuffSize := bodySize - inSize // 当前TS packet的剩余空闲空间

			if packet[3]&0x20 != 0 {
				// has Adaptation

				base := int(4 + packet[4]) // TS Header + Adaptation
				if wpos > base {
					// 比如有PES Header

					copy(packet[base+stuffSize:], packet[base:wpos])
				}
				wpos = base + stuffSize

				packet[4] += uint8(stuffSize) // adaptation_field_length
				for i := 0; i < stuffSize; i++ {
					packet[base+i] = 0xFF
				}
			} else {
				// no Adaptation

				packet[3] |= 0x20

				base := 4
				if wpos > base {
					copy(packet[base+stuffSize:], packet[base:wpos])
				}
				wpos += stuffSize

				packet[4] = uint8(stuffSize - 1) // adaptation_field_length
				if stuffSize >= 2 {
					// TODO chef 这里是参考nginx rtmp module的实现，为什么这个字节写0而不是0xFF
					packet[5] = 0
					for i := 0; i < stuffSize-2; i++ {
						packet[6+i] = 0xFF
					}
				}
			}

			// 真实数据放在packet尾部
			copy(packet[wpos:], frame.Raw[lpos:lpos+inSize])
			lpos = rpos
		}
	}

	return buf[:packetPosAtBuf]
}

func (frame *Frame) DebugString() string {
	return fmt.Sprintf("sid=%d, dts=%d, pts=%d, cc=%d, key=%t, len=%d, payload=%s",
		frame.Sid, frame.Dts, frame.Pts, frame.Cc, frame.Key, len(frame.Raw), hex.Dump(nazabytes.Prefix(frame.Raw, 8)))
}

// ----- private -------------------------------------------------------------------------------------------------------

func packPcr(out []byte, pcr uint64) {
	out[0] = uint8(pcr >> 25)
	out[1] = uint8(pcr >> 17)
	out[2] = uint8(pcr >> 9)
	out[3] = uint8(pcr >> 1)
	out[4] = uint8(pcr<<7) | 0x7e
	out[5] = 0

	//pcrLow := pcr % 300
	//pcrHigh := pcr / 300
	//out[0] = uint8(pcrHigh >> 25)
	//out[1] = uint8(pcrHigh >> 17)
	//out[2] = uint8(pcrHigh >> 9)
	//out[3] = uint8(pcrHigh >> 1)
	//out[4] = uint8(pcrHigh<<7) | uint8(pcrLow>>8) | 0x7e
	//out[5] = uint8(pcrLow)
}

// 注意，除PTS外，DTS也使用这个函数打包
func packPts(out []byte, fb uint8, pts uint64) {
	var val uint64
	out[0] = (fb << 4) | (uint8(pts>>30) & 0x07) | 1

	val = (((pts >> 15) & 0x7FFF) << 1) | 1
	out[1] = uint8(val >> 8)
	out[2] = uint8(val)

	val = ((pts & 0x7FFF) << 1) | 1
	out[3] = uint8(val >> 8)
	out[4] = uint8(val)
}
