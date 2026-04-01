// Copyright 2022, Chef.  All rights reserved.
// https://github.com/q191201771/lal
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

package gb28181

import (
	"bytes"
	"encoding/hex"

	"github.com/q191201771/lal/pkg/base"
	"github.com/q191201771/lal/pkg/h2645"
	"github.com/q191201771/lal/pkg/rtprtcp"

	"github.com/q191201771/naza/pkg/bele"
	"github.com/q191201771/naza/pkg/nazabits"
	"github.com/q191201771/naza/pkg/nazabytes"
	"github.com/q191201771/naza/pkg/nazalog"
)

// PsUnpacker 解析ps(Program Stream)流
type PsUnpacker struct {
	list     rtprtcp.RtpPacketList
	buf      *nazabytes.Buffer
	audioBuf []byte
	videoBuf []byte

	audioStreamType  uint8
	videoStreamType  uint8
	audioPayloadType base.AvPacketPt
	videoPayloadType base.AvPacketPt

	preAudioPts int64
	preVideoPts int64
	preAudioDts int64
	preVideoDts int64

	preAudioRtpts int64
	preVideoRtpts int64

	onAvPacket base.OnAvPacketFunc

	waitSpsFlag bool

	feedPacketCount     int
	feedBodyCount       int
	onAvPacketWrapCount int
	onAvPacketCount     int
}

func NewPsUnpacker() *PsUnpacker {
	p := &PsUnpacker{
		buf:           nazabytes.NewBuffer(psBufInitSize),
		preVideoPts:   -1,
		preAudioPts:   -1,
		preVideoRtpts: -1,
		preAudioRtpts: -1,
		waitSpsFlag:   true,
	}
	p.list.InitMaxSize(maxUnpackRtpListSize)

	return p
}

// WithOnAvPacket
//
// @param onAvPacket: 回调函数中 base.AvPacket 字段说明：
// PayloadType AvPacketPt 见 base.AvPacketPt。
// Timestamp   int64      dts，单位毫秒。
// Pts         int64      pts，单位毫秒。
// Payload     []byte
// 对于视频，h264和h265是AnnexB格式。
// 对于音频，AAC是前面携带adts的格式。
func (p *PsUnpacker) WithOnAvPacket(onAvPacket base.OnAvPacketFunc) *PsUnpacker {
	p.onAvPacket = onAvPacket
	return p
}

// FeedRtpPacket
//
// 注意，内部会处理丢包、乱序等问题
//
// @param b: rtp包，注意，包含rtp包头部分，内部不持有该内存块
func (p *PsUnpacker) FeedRtpPacket(b []byte) error {
	// TODO(chef): [opt] 当前遇到的场景都是，音频和视频共用一个ssrc，并且音频和视频的seq是打在一起的，是否存在两个ssrc的情况？ 202209

	p.feedPacketCount++

	//defer func() {
	//	nazalog.Debugf("<<<<<<<<<< PsUnpacker. list=%s", p.list.DebugString())
	//}()

	ipkt, err := rtprtcp.ParseRtpPacket(b)
	if err != nil {
		nazalog.Errorf("PsUnpacker ParseRtpPacket failed. b=%s, err=%+v",
			hex.Dump(nazabytes.Prefix(b, 64)), err)
		return err
	}

	//nazalog.Debugf(">>>>>>>>>> PsUnpacker FeedRtpPacket. h=%+v, len=%d, body=%s",
	//	ipkt.Header, len(ipkt.Raw), hex.Dump(nazabytes.Prefix(ipkt.Raw[12:], 8)))

	var isStartPositionFn = func(pkt rtprtcp.RtpPacket) bool {
		body := pkt.Body()
		return len(body) > 4 && bytes.Compare(body[0:3], []byte{0, 0, 1}) == 0
	}

	// 处理丢包、乱序、重复

	// 过期了直接丢掉
	if p.list.IsStale(ipkt.Header.Seq) {
		//nazalog.Debugf("PsUnpacker NOTICE stale, drop. %d", ipkt.Header.Seq)
		return ErrGb28181
	}
	// 插入队列
	//nazalog.Debugf("PsUnpacker FeedRtpPacket insert. %d", ipkt.Header.Seq)
	p.list.Insert(ipkt)
	for {
		// 循环判断头部是否是顺序的

		if p.list.IsFirstSequential() {
			// 如果头一个是顺序的，取出来，喂入解析器

			opkt := p.list.PopFirst()
			p.list.SetDoneSeq(opkt.Header.Seq)
			//nazalog.Debugf("PsUnpacker FeedRtpBody. %d", opkt.Header.Seq)
			errFeedRtpBody := p.FeedRtpBody(opkt.Body(), opkt.Header.Timestamp)
			if errFeedRtpBody != nil {
				p.list.Reset()
			}
		} else {
			// 不是顺序的，如果还没达到容器阈值，就先缓存在容器中，直接退出了
			// 注意，如果队列为空，也会走到这，然后通过!full退出

			if !p.list.Full() {
				//nazalog.Debugf("PsUnpacker exit check !full.")
				break
			}

			// 如果达到容器阈值了，就丢弃一部分
			// 丢弃哪些呢？
			// 先丢第一个，因为满了至少要丢一个了。
			//
			// 再丢弃连续的，直到下一个可解析帧位置
			// 因为不连续的话，没法判断和正在丢弃的是否同一帧的，可以给个机会看后续是否能收到
			prev := p.list.PopFirst()
			//nazalog.Debugf("PsUnpacker NOTICE drop. %d", prev.Header.Seq)

			for p.list.Size > 0 {
				curr := p.list.PeekFirst()
				if rtprtcp.SubSeq(curr.Header.Seq, prev.Header.Seq) != 1 {
					//nazalog.Debugf("PsUnpacker exit drop !sequential.")
					break
				}

				if isStartPositionFn(curr) {
					//nazalog.Debugf("PsUnpacker exit drop start.")

					// 注意，这里需要设置done seq，确保这个seq在以后的判断中可被使用
					p.list.SetDoneSeq(curr.Header.Seq - 1)
					break
				}

				prev = p.list.PopFirst()
				//nazalog.Debugf("PsUnpacker NOTICE drop. %d", prev.Header.Seq)
			}

			// 注意，缓存的数据也需要清除
			p.buf.Reset()
			p.audioBuf = nil
			p.videoBuf = nil
		}
	}

	return nil
}

// FeedRtpBody 注意，传入的数据应该是连续的，属于完整帧的
func (p *PsUnpacker) FeedRtpBody(rtpBody []byte, rtpts uint32) error {
	p.feedBodyCount++

	//nazalog.Debugf("> FeedRtpBody. len=%d, prev buf=%d", len(rtpBody), p.buf.Len())
	p.buf.Write(rtpBody)
	// ISO/IEC iso13818-1
	//
	// 2.5 Program Stream bitstream requirements
	//
	// TODO(chef): [fix] 有些没做有效长度判断
	for p.buf.Len() != 0 {
		rb := p.buf.Bytes()
		i := 0
		code := bele.BeUint32(rb[i:])
		i += 4

		var consumed int
		switch code {
		case psPackStartCodePackHeader:
			//nazalog.Debugf("----------pack header----------")
			consumed = parsePackHeader(rb, i)
		case psPackStartCodeSystemHeader:
			//nazalog.Debugf("----------system header----------")
			// 2.5.3.5 System header
			// Table 2-32 - Program Stream system header
			//
			consumed = parsePackStreamBody(rb, i)
		case psPackStartCodeProgramStreamMap:
			//nazalog.Debugf("----------program stream map----------")
			consumed = p.parsePsm(rb, i)
		case psPackStartCodeAudioStream:
			//nazalog.Debugf("----------audio stream----------")
			consumed = p.parseAvStream(int(code), rtpts, rb, i)
		case psPackStartCodeVideoStream:
			//nazalog.Debugf("----------video stream----------")
			consumed = p.parseAvStream(int(code), rtpts, rb, i)
		case psPackStartCodePackEnd:
			//nazalog.Errorf("----------skip----------. %s", hex.Dump(nazabytes.Prefix(rb[i-4:], 32)))
			consumed = 0
		case psPackStartCodeHikStream:
			consumed = parsePackStreamBody(rb, i)
			//nazalog.Debugf("----------hik stream----------consumed=%d", consumed)
		case psPackStartCodePesPrivate2:
			fallthrough
		case psPackStartCodePesEcm:
			fallthrough
		case psPackStartCodePesEmm:
			fallthrough
		case psPackStartCodePesPadding:
			fallthrough
		case psPackStartCodePesPsd:
			consumed = parsePackStreamBody(rb, i)
		default:
			// TODO(chef): [opt] 所有code都处理后，不符合格式的code可以考虑重置unpacker，重新处理新喂入的数据 202207
			nazalog.Errorf("unknown ps code, reset all cache buffer. %d,%d,%d, %s",
				p.buf.Len(), len(p.audioBuf), len(p.videoBuf), hex.Dump(nazabytes.Prefix(rb[i-4:], 32)))
			p.buf.Reset()
			p.audioBuf = nil
			p.videoBuf = nil
			return base.ErrGb28181
		}

		if consumed < 0 {
			// 消费失败并不一定是数据有问题，可能是数据不完整需要等待下一个rtp包
			if code != psPackStartCodeVideoStream && code != psPackStartCodeHikStream {
				nazalog.Warnf("consumed failed. code=%d, len=(%d,%d), i=%d, buf=%s",
					code, len(rtpBody), len(rb), i, hex.Dump(nazabytes.Prefix(rb[i-4:], 128)))
			}
			return nil
		}
		p.buf.Skip(i + consumed)
		//nazalog.Debugf("skip. %d", i+consumed)
	}
	return nil
}

func (p *PsUnpacker) Dispose() {
	nazalog.Debugf("PsUnpacker Dispose. (%d, %d, %d, %d, %d)",
		p.feedPacketCount, p.feedBodyCount, p.list.Size, p.onAvPacketWrapCount, p.onAvPacketCount)
}

func (p *PsUnpacker) parsePsm(rb []byte, index int) int {
	// 2.5.4 Program Stream map
	// Table 2-35 - Program Stream map
	//

	i := index

	if len(rb[i:]) < 6 {
		return -1
	}

	// skip program_stream_map_length
	// skip current_next_indicator
	// skip reserved
	// skip program_stream_map_version
	// skip reverved
	// skip marked_bit
	i += 4 // 2 + 1 + 1

	// program_stream_info_length
	l := int(bele.BeUint16(rb[i:]))
	i += 2

	if len(rb[i:]) < l+2 {
		return -1
	}

	i += l

	// elementary_stream_map_length
	esml := int(bele.BeUint16(rb[i:]))
	//nazalog.Debugf("l=%d, esml=%d", l, esml)
	i += 2

	if len(rb[i:]) < esml+4 {
		return -1
	}

	for esml > 0 {
		streamType := rb[i]
		i += 1
		streamId := rb[i]
		i += 1
		// elementary_stream_info_length
		if streamId >= 0xe0 && streamId <= 0xef {
			p.videoStreamType = streamType

			switch p.videoStreamType {
			case StreamTypeH264:
				p.videoPayloadType = base.AvPacketPtAvc
			case StreamTypeH265:
				p.videoPayloadType = base.AvPacketPtHevc
			default:
				p.videoPayloadType = base.AvPacketPtUnknown
			}
		} else if streamId >= 0xc0 && streamId <= 0xdf {
			p.audioStreamType = streamType

			switch p.audioStreamType {
			case StreamTypeAAC:
				p.audioPayloadType = base.AvPacketPtAac
			case StreamTypeG711A:
				p.audioPayloadType = base.AvPacketPtG711A
			case StreamTypeG711U:
				p.audioPayloadType = base.AvPacketPtG711U
			default:
				p.audioPayloadType = base.AvPacketPtUnknown
			}
		}
		esil := int(bele.BeUint16(rb[i:]))
		//nazalog.Debugf("streamType=%d, streamId=%d, esil=%d", streamType, streamId, esil)
		i += 2 + esil
		esml = esml - 4 - esil
	}
	// skip
	i += 4

	return i - index
}

func (p *PsUnpacker) parseAvStream(code int, rtpts uint32, rb []byte, index int) int {
	i := index

	// 注意，由于length是两字节，所以存在一个帧分成多个pes包的情况
	length := int(bele.BeUint16(rb[i:]))
	if length == 65535 {
		nazalog.Warnf("CHEFGREPME length=%d", length)
	}
	i += 2

	//nazalog.Debugf("parseAvStream. code=%d, expected=%d, actual=%d", code, length, len(rb)-i)

	if len(rb)-i < length {
		return -1
	}

	ptsDtsFlag := rb[i+1] >> 6
	phdl := int(rb[i+2]) // pes header data length
	i += 3

	var pts int64 = -1
	var dts int64 = -1
	j := 0
	if ptsDtsFlag&0x2 != 0 {
		_, pts = readPts(rb[i:])
		j += 5
	}
	if ptsDtsFlag&0x1 != 0 {
		_, dts = readPts(rb[i+j:])
	} else {
		dts = pts
	}

	i += phdl

	//nazalog.Debugf("parseAvStream. code=%d, length=%d, pts=%d, dts=%d", code, length, pts, dts)

	if code == psPackStartCodeAudioStream {
		// 注意，处理音频的逻辑和处理视频的类似，参考处理视频的注释
		if p.audioStreamType == StreamTypeAAC || p.audioStreamType == StreamTypeG711A || p.audioStreamType == StreamTypeG711U {
			//nazalog.Debugf("audio code=%d, length=%d, ptsDtsFlag=%d, phdl=%d, pts=%d, dts=%d,type=%d", code, length, ptsDtsFlag, phdl, pts, dts, p.audioStreamType)
			if pts == -1 {
				if p.preAudioPts == -1 {
					if p.preAudioRtpts == -1 {
						// noop
					} else {
						if p.preAudioRtpts != int64(rtpts) {
							p.onAvPacketWrap(&base.AvPacket{
								PayloadType: p.audioPayloadType,
								Timestamp:   p.preAudioDts / 90,
								Pts:         p.preAudioPts / 90,
								Payload:     p.audioBuf,
							})
							// TODO(chef): [perf] 复用内存块 202209
							p.audioBuf = nil
						} else {
							// noop
						}
					}
				} else {
					pts = p.preAudioPts
				}
			} else {
				if pts != p.preAudioPts && p.preAudioPts >= 0 {
					p.onAvPacketWrap(&base.AvPacket{
						PayloadType: p.audioPayloadType,
						Timestamp:   p.preAudioDts / 90,
						Pts:         p.preAudioPts / 90,
						Payload:     p.audioBuf,
					})
					p.audioBuf = nil
				} else {
					// noop
				}
			}
			p.audioBuf = append(p.audioBuf, rb[i:i+length-3-phdl]...)
			p.preAudioRtpts = int64(rtpts)
			p.preAudioPts = pts
			p.preAudioDts = dts
		} else {
			nazalog.Warnf("unknown audio stream type. ast=%d", p.audioStreamType)
		}
	} else if code == psPackStartCodeVideoStream {
		// 判断出当前pes是否是新的帧，然后将缓存中的帧回调给上层

		if pts == -1 {
			// 当前pes包没有pts

			if p.preVideoPts == -1 {
				if p.preVideoRtpts == -1 {
					// 整个流的第一帧，啥也不干
				} else {
					// 整个流没有pts字段，退化成使用rtpts

					if p.preVideoRtpts != int64(rtpts) {
						// 使用rtp的时间戳回调，但是，并不用rtp的时间戳更新pts
						p.iterateNaluByStartCode(code, p.preVideoRtpts, p.preVideoRtpts)
						p.videoBuf = nil
					} else {
						// 同一帧，啥也不干
					}
				}
			} else {
				// 当前pes包没有pts，而前一个有
				// 这种情况我们认为是同一帧，啥也不干
				pts = p.preVideoPts
			}
		} else {
			// 当前pes包有pts

			if pts != p.preVideoPts && p.preVideoPts >= 0 {
				// 当前pes包是新的帧，将缓存中的帧回调给上层

				p.iterateNaluByStartCode(code, p.preVideoPts, p.preVideoPts)
				p.videoBuf = nil
			} else {
				// 两种情况：
				// 1. pts != prev && prev == -1 也即第一帧
				// 2. pts == prev && prev != -1 也即同一帧（前后两个pes包的时间戳相同）
				// 这两种情况，都啥也不干
			}
		}

		// 注意，是处理完之前的数据后，再将当前pes包存入缓存中
		p.videoBuf = append(p.videoBuf, rb[i:i+length-3-phdl]...)
		p.preVideoRtpts = int64(rtpts)
		p.preVideoPts = pts
		p.preVideoDts = dts
	}

	return 2 + length
}

// parsePackHeader 注意，`rb[index:]`为待解析的内存块
func parsePackHeader(rb []byte, index int) int {
	// 2.5.3.3 Pack layer of Program Stream
	// Table 2-33 - Program Stream pack header

	i := index
	// TODO(chef): 这里按MPEG-2处理，还需要处理MPEG-1 202206

	// skip system clock reference(SCR)
	// skip PES program mux rate
	i += 6 + 3
	if len(rb) <= i {
		nazalog.Warnf("[p] expected=%d, actual=%d", i, len(rb))
		return -1
	}

	// skip stuffing
	l := int(rb[i] & 0x7)
	i += 1 + l

	return i - index
}

func parsePackStreamBody(rb []byte, index int) int {
	i := index

	if len(rb) < i+2 {
		nazalog.Debugf("needed=%d, actual=%d", i+2, len(rb))
		return -1
	}
	l := int(bele.BeUint16(rb[i:]))
	i += 2 + l
	if len(rb) < i {
		nazalog.Debugf("needed=%d, actual=%d", i, len(rb))
		return -1
	}

	return i - index
}

// iterateNaluByStartCode 通过nal start code分隔缓存数据，将nal回调给上层
func (p *PsUnpacker) iterateNaluByStartCode(code int, pts, dts int64) {
	leading, preLeading, startPos := 0, 0, 0
	startPos, preLeading = h2645.IterateNaluStartCode(p.videoBuf, 0)
	if startPos < 0 {
		nazalog.Errorf("CHEFNOTICEME %s", hex.Dump(nazabytes.Prefix(p.videoBuf, 32)))
		return
	}

	nextPos := startPos
	nalu := p.videoBuf[:0]
	for startPos >= 0 {
		nextPos, leading = h2645.IterateNaluStartCode(p.videoBuf, startPos+preLeading)
		// 找到下一个，则取两个start code之间的内容
		// 找不到下一个，则取startcode到末尾的内容
		// 不管是否找到下一个，都回调
		if nextPos >= 0 {
			nalu = p.videoBuf[startPos:nextPos]
		} else {
			nalu = p.videoBuf[startPos:]
		}
		startPos = nextPos

		p.onAvPacketWrap(&base.AvPacket{
			PayloadType: p.videoPayloadType,
			Timestamp:   dts / 90,
			Pts:         pts / 90,
			Payload:     nalu,
		})

		if nextPos >= 0 {
			preLeading = leading
		}
	}
}

func (p *PsUnpacker) onAvPacketWrap(packet *base.AvPacket) {
	p.onAvPacketWrapCount++
	//nazalog.Debugf("PsUnpacker > onAvPacketWrap. packet=%s", packet.DebugString())
	if packet.IsVideo() {
		typ := h2645.ParseNaluType(packet.PayloadType == base.AvPacketPtAvc, packet.Payload[4])
		//nazalog.Debugf("PsUnpacker onAvPacketWrap. type=%d", typ)
		// TODO(chef): [opt] 等待sps等信息再开始回调，这个逻辑不完整简化了 202209
		if p.waitSpsFlag {
			if packet.PayloadType == base.AvPacketPtAvc {
				if typ == h2645.H264NaluTypeSps || typ == h2645.H264NaluTypePps {
					p.waitSpsFlag = false
				} else {
					//nazalog.Debugf("PsUnpacker onAvPacketWrap. drop. %d", typ)
					return
				}
			} else if packet.PayloadType == base.AvPacketPtHevc {
				if typ == h2645.H265NaluTypeVps || typ == h2645.H265NaluTypeSps || typ == h2645.H265NaluTypePps {
					p.waitSpsFlag = false
				} else {
					//nazalog.Debugf("PsUnpacker onAvPacketWrap. drop. %d", typ)
					return
				}
			}
		}
	}
	if p.onAvPacket != nil {
		p.onAvPacketCount++
		//nazalog.Debugf("PsUnpacker > onAvPacket. packet=%s", packet.DebugString())
		p.onAvPacket(packet)
	}
}

// ---------------------------------------------------------------------------------------------------------------------

// TODO(chef): [refactor] 以下代码拷贝来自package mpegts，重复了

// Pes -----------------------------------------------------------
// <iso13818-1.pdf>
// <2.4.3.6 PES packet> <page 49/174>
// <Table E.1 - PES packet header example> <page 142/174>
// <F.0.2 PES packet> <page 144/174>
// packet_start_code_prefix  [24b] *** always 0x00, 0x00, 0x01
// stream_id                 [8b]  *
// PES_packet_length         [16b] **
// '10'                      [2b]
// PES_scrambling_control    [2b]
// PES_priority              [1b]
// data_alignment_indicator  [1b]
// copyright                 [1b]
// original_or_copy          [1b]  *
// PTS_DTS_flags             [2b]
// ESCR_flag                 [1b]
// ES_rate_flag              [1b]
// DSM_trick_mode_flag       [1b]
// additional_copy_info_flag [1b]
// PES_CRC_flag              [1b]
// PES_extension_flag        [1b]  *
// PES_header_data_length    [8b]  *
// -----------------------------------------------------------
type Pes struct {
	pscp       uint32
	sid        uint8
	ppl        uint16
	pad1       uint8
	ptsDtsFlag uint8
	pad2       uint8
	phdl       uint8
	pts        int64
	dts        int64
}

func ParsePes(b []byte) (pes Pes, length int) {
	br := nazabits.NewBitReader(b)
	//pes.pscp, _ = br.ReadBits32(24)
	//pes.sid, _ = br.ReadBits8(8)
	//pes.ppl, _ = br.ReadBits16(16)

	pes.pad1, _ = br.ReadBits8(8)
	pes.ptsDtsFlag, _ = br.ReadBits8(2)
	pes.pad2, _ = br.ReadBits8(6)
	pes.phdl, _ = br.ReadBits8(8)

	_, _ = br.ReadBytes(uint(pes.phdl))
	length = 9 + int(pes.phdl)

	// 处理得不是特别标准
	if pes.ptsDtsFlag&0x2 != 0 {
		_, pes.pts = readPts(b[9:])
	}
	if pes.ptsDtsFlag&0x1 != 0 {
		_, pes.dts = readPts(b[14:])
	} else {
		pes.dts = pes.pts
	}
	//pes.pts = (pes.pts - delay) / 90
	//pes.dts = (pes.dts - delay) / 90

	return
}

// read pts or dts
func readPts(b []byte) (fb uint8, pts int64) {
	fb = b[0] >> 4
	pts |= int64((b[0]>>1)&0x07) << 30
	pts |= (int64(b[1])<<8 | int64(b[2])) >> 1 << 15
	pts |= (int64(b[3])<<8 | int64(b[4])) >> 1
	return
}
