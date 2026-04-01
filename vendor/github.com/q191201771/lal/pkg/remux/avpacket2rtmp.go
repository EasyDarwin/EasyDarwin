// Copyright 2021, Chef.  All rights reserved.
// https://github.com/q191201771/lal
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

package remux

import (
	"github.com/q191201771/lal/pkg/aac"
	"github.com/q191201771/lal/pkg/avc"
	"github.com/q191201771/lal/pkg/base"
	"github.com/q191201771/lal/pkg/hevc"
	"github.com/q191201771/lal/pkg/rtmp"
	"github.com/q191201771/lal/pkg/rtprtcp"
	"github.com/q191201771/lal/pkg/sdp"
	"github.com/q191201771/naza/pkg/bele"
)

// AvPacket2RtmpRemuxer AvPacket转换为RTMP
//
// 目前AvPacket来自:
//
// - RTSP:       sdp以及rtp的合帧包
// - gb28181 ps: rtp的合帧包
// - customize:  业务方通过接口向lalserver输入的流
// - ffmpeg:     ffmpeg编码后的数据
// - 理论上也支持webrtc，后续接入webrtc时再验证
type AvPacket2RtmpRemuxer struct {
	option    base.AvPacketStreamOption
	onRtmpMsg rtmp.OnReadRtmpAvMsg

	hasEmittedMetadata bool
	audioType          base.AvPacketPt
	videoType          base.AvPacketPt

	vps []byte // 从AvPacket数据中获取
	sps []byte
	pps []byte

	hasAdts2Asc bool
}

func NewAvPacket2RtmpRemuxer() *AvPacket2RtmpRemuxer {
	return &AvPacket2RtmpRemuxer{
		option:    base.DefaultApsOption,
		audioType: base.AvPacketPtUnknown,
		videoType: base.AvPacketPtUnknown,
	}
}

// WithOption
//
// TODO(chef): [refactor] 返回*AvPacket2RtmpRemuxer 202208
func (r *AvPacket2RtmpRemuxer) WithOption(modOption func(option *base.AvPacketStreamOption)) {
	modOption(&r.option)

	// TODO(chef): [log] 打印所有option 202301
}

func (r *AvPacket2RtmpRemuxer) WithOnRtmpMsg(onRtmpMsg rtmp.OnReadRtmpAvMsg) *AvPacket2RtmpRemuxer {
	r.onRtmpMsg = onRtmpMsg
	return r
}

// ---------------------------------------------------------------------------------------------------------------------

// OnRtpPacket OnSdp OnAvPacket
//
// 实现RTSP回调数据的接口 rtsp.IBaseInSessionObserver ，使得接入时方便些
func (r *AvPacket2RtmpRemuxer) OnRtpPacket(pkt rtprtcp.RtpPacket) {
	// noop
}
func (r *AvPacket2RtmpRemuxer) OnSdp(sdpCtx sdp.LogicContext) {
	r.InitWithAvConfig(sdpCtx.Asc, sdpCtx.Vps, sdpCtx.Sps, sdpCtx.Pps)
}
func (r *AvPacket2RtmpRemuxer) OnAvPacket(pkt base.AvPacket) {
	r.FeedAvPacket(pkt)
}

// ---------------------------------------------------------------------------------------------------------------------

// InitWithAvConfig rtsp场景下，有时sps、pps等信息只包含在sdp中，有时包含在rtp包中，
// 这里提供输入sdp的sps、pps等信息的机会，如果没有，可以不调用
//
// 内部不持有输入参数的内存块
func (r *AvPacket2RtmpRemuxer) InitWithAvConfig(asc, vps, sps, pps []byte) {
	var err error
	var bVsh []byte
	var bAsh []byte

	if asc != nil {
		r.audioType = base.AvPacketPtAac
	}
	if sps != nil && pps != nil {
		if vps != nil {
			r.videoType = base.AvPacketPtHevc
		} else {
			r.videoType = base.AvPacketPtAvc
		}
	}

	if r.audioType == base.AvPacketPtUnknown && r.videoType == base.AvPacketPtUnknown {
		Log.Warn("neither audio nor video exist")
		return
	}

	if r.audioType != base.AvPacketPtUnknown {
		bAsh, err = aac.MakeAudioDataSeqHeaderWithAsc(asc)
		if err != nil {
			Log.Errorf("build aac seq header failed. err=%+v", err)
			return
		}
	}
	if r.videoType != base.AvPacketPtUnknown {
		if r.videoType == base.AvPacketPtHevc {
			bVsh, err = hevc.BuildSeqHeaderFromVpsSpsPps(vps, sps, pps)
			if err != nil {
				Log.Errorf("build hevc seq header failed. err=%+v", err)
				return
			}
		} else {
			bVsh, err = avc.BuildSeqHeaderFromSpsPps(sps, pps)
			if err != nil {
				Log.Errorf("build avc seq header failed. err=%+v", err)
				return
			}
		}
	}

	if r.audioType != base.AvPacketPtUnknown {
		r.emitRtmpAvMsg(true, bAsh, 0)
	}

	if r.videoType != base.AvPacketPtUnknown {
		r.emitRtmpAvMsg(false, bVsh, 0)
	}
}

// FeedAvPacket
//
// 输入 base.AvPacket 数据
//
// @param pkt:
//   - 如果是aac，格式是裸数据或带adts头，具体取决于前面的配置。
//   - 如果是h264，格式是avcc或Annexb，具体取决于前面的配置。
//     内部不持有该内存块。
func (r *AvPacket2RtmpRemuxer) FeedAvPacket(pkt base.AvPacket) {
	switch pkt.PayloadType {
	case base.AvPacketPtAvc:
		fallthrough
	case base.AvPacketPtHevc:
		var nals [][]byte
		var err error
		if r.option.VideoFormat == base.AvPacketStreamVideoFormatAvcc {
			nals, err = avc.SplitNaluAvcc(pkt.Payload)
		} else {
			nals, err = avc.SplitNaluAnnexb(pkt.Payload)
		}
		if err != nil {
			Log.Errorf("iterate nalu failed. err=%+v", err)
			return
		}

		pos := 5
		maxLength := len(pkt.Payload) + pos + len(nals)
		payload := make([]byte, maxLength)

		for _, nal := range nals {
			if pkt.PayloadType == base.AvPacketPtAvc {
				t := avc.ParseNaluType(nal[0])
				if t == avc.NaluTypeAud {
					continue
				} else if t == avc.NaluTypeSps || t == avc.NaluTypePps {
					// 如果有sps，pps，先把它们抽离出来进行缓存
					if t == avc.NaluTypeSps {
						r.setSps(nal)
					} else {
						r.setPps(nal)
					}

					// 注意，由于sps空值时，可能是nil也可能是[0:0]，所以这里不用nil做判断，而用len
					if len(r.sps) > 0 && len(r.pps) > 0 {
						// 凑齐了，发送video seq header
						//
						// TODO(chef): 是否应该判断sps、pps是连续的，比如rtp seq的关系，或者timestamp是相等的

						bVsh, err := avc.BuildSeqHeaderFromSpsPps(r.sps, r.pps)
						if err != nil {
							Log.Errorf("build avc seq header failed. err=%+v", err)
							continue
						}
						r.emitRtmpAvMsg(false, bVsh, pkt.Timestamp)
						//if !AvPacket2RtmpRemuxerAddSpsPps2KeyFrameFlag {
						//	r.clearVideoSeqHeader()
						//}
						r.clearVideoSeqHeader()
					}
				} else {
					// 重组实际数据

					if t == avc.NaluTypeIdrSlice {
						//if AvPacket2RtmpRemuxerAddSpsPps2KeyFrameFlag {
						//	// 关键帧 组合sps vps与数据帧
						//	nal = append(append(avc.BuildSpsPps2Annexb(r.sps, r.pps)[4:], hevc.NaluStartCode4...), nal...)
						//	// 考虑feed时 无sps 与pps数据
						//	if len(pkt.Payload) < len(nal) {
						//		maxLength = len(nal) + pos
						//		payload = make([]byte, maxLength)
						//	}
						//}
						payload[0] = base.RtmpAvcKeyFrame
					} else {
						payload[0] = base.RtmpAvcInterFrame
					}
					payload[1] = base.RtmpAvcPacketTypeNalu
					bele.BePutUint32(payload[pos:], uint32(len(nal)))
					pos += 4
					copy(payload[pos:], nal)
					pos += len(nal)
				}
			} else if pkt.PayloadType == base.AvPacketPtHevc {
				t := hevc.ParseNaluType(nal[0])
				if t == hevc.NaluTypeAud {
					continue
				} else if t == hevc.NaluTypeVps || t == hevc.NaluTypeSps || t == hevc.NaluTypePps {
					if t == hevc.NaluTypeVps {
						r.setVps(nal)
					} else if t == hevc.NaluTypeSps {
						r.setSps(nal)
					} else {
						r.setPps(nal)
					}
					if len(r.vps) > 0 && len(r.sps) > 0 && len(r.pps) > 0 {
						bVsh, err := hevc.BuildSeqHeaderFromVpsSpsPps(r.vps, r.sps, r.pps)
						if err != nil {
							Log.Errorf("build hevc seq header failed. err=%+v", err)
							continue
						}
						r.emitRtmpAvMsg(false, bVsh, pkt.Timestamp)
						//if !AvPacket2RtmpRemuxerAddSpsPps2KeyFrameFlag {
						//	r.clearVideoSeqHeader()
						//}
						r.clearVideoSeqHeader()
					}
				} else {
					if hevc.IsIrapNalu(t) {
						//if AvPacket2RtmpRemuxerAddSpsPps2KeyFrameFlag {
						//	// 关键帧 组合vps sps pps与数据帧
						//	annexb, err := hevc.BuildVpsSpsPps2Annexb(r.vps, r.sps, r.pps)
						//	if err != nil {
						//		Log.Errorf("build hevc vps sps pps data failed. err=%+v", err)
						//	}
						//	nal = append(append(annexb[4:], hevc.NaluStartCode4...), nal...)
						//	// 考虑feed时 无sps 与pps数据
						//	if len(pkt.Payload) < len(nal) {
						//		maxLength = len(nal) + pos
						//		payload = make([]byte, maxLength)
						//	}
						//}
						payload[0] = base.RtmpHevcKeyFrame
					} else {
						payload[0] = base.RtmpHevcInterFrame
					}
					payload[1] = base.RtmpHevcPacketTypeNalu
					bele.BePutUint32(payload[pos:], uint32(len(nal)))
					pos += 4
					copy(payload[pos:], nal)
					pos += len(nal)
				}
			}
		}

		// 有实际数据
		if pos > 5 {
			r.emitRtmpAvMsg(false, payload[:pos], pkt.Timestamp)
		}

	case base.AvPacketPtAac:
		if r.option.AudioFormat == base.AvPacketStreamAudioFormatRawAac {
			length := len(pkt.Payload) + 2
			payload := make([]byte, length)
			// TODO(chef) 处理此处的魔数0xAF
			payload[0] = 0xAF
			payload[1] = base.RtmpAacPacketTypeRaw
			copy(payload[2:], pkt.Payload)
			r.emitRtmpAvMsg(true, payload, pkt.Timestamp)
		} else if r.option.AudioFormat == base.AvPacketStreamAudioFormatAdtsAac {
			if !r.hasAdts2Asc {
				adts, err := aac.MakeAudioDataSeqHeaderWithAdtsHeader(pkt.Payload)
				if err != nil {
					Log.Errorf("%+v", err)
				}

				r.emitRtmpAvMsg(true, adts, pkt.Timestamp)

				r.hasAdts2Asc = true
			}

			length := len(pkt.Payload) - 5 // -7+2
			if length < 7 {
				return
			}
			payload := make([]byte, length)
			payload[0] = 0xAF
			payload[1] = base.RtmpAacPacketTypeRaw
			copy(payload[2:], pkt.Payload[7:])
			r.emitRtmpAvMsg(true, payload, pkt.Timestamp)
		}

	case base.AvPacketPtG711A:
		length := len(pkt.Payload) + 1
		payload := make([]byte, length)
		// ffmpeg是固定值
		payload[0] = 0x72
		copy(payload[1:], pkt.Payload)
		r.emitRtmpAvMsg(true, payload, pkt.Timestamp)

	case base.AvPacketPtG711U:
		length := len(pkt.Payload) + 1
		payload := make([]byte, length)
		// ffmpeg是固定值
		payload[0] = 0x82
		copy(payload[1:], pkt.Payload)
		r.emitRtmpAvMsg(true, payload, pkt.Timestamp)

	case base.AvPacketPtOpus:
		length := len(pkt.Payload) + 1
		payload := make([]byte, length)
		// codecid=13, 44kHz、16bits、Stereo
		payload[0] = 0xdf
		copy(payload[1:], pkt.Payload)
		r.emitRtmpAvMsg(true, payload, pkt.Timestamp)

	default:
		Log.Warnf("unsupported packet. type=%d", pkt.PayloadType)
	}
}

// ---------------------------------------------------------------------------------------------------------------------

func (r *AvPacket2RtmpRemuxer) emitRtmpAvMsg(isAudio bool, payload []byte, timestamp int64) {
	if !r.hasEmittedMetadata {
		// TODO(chef): 此处简化了从sps中获取宽高写入metadata的逻辑
		audiocodecid := -1
		videocodecid := -1
		if r.audioType == base.AvPacketPtAac {
			audiocodecid = int(base.RtmpSoundFormatAac)
		}
		switch r.videoType {
		case base.AvPacketPtAvc:
			videocodecid = int(base.RtmpCodecIdAvc)
		case base.AvPacketPtHevc:
			videocodecid = int(base.RtmpCodecIdHevc)
		}
		bMetadata, err := rtmp.BuildMetadata(-1, -1, audiocodecid, videocodecid)
		if err != nil {
			Log.Errorf("build metadata failed. err=%+v", err)
			return
		}
		r.onRtmpMsg(base.RtmpMsg{
			Header: base.RtmpHeader{
				Csid:         rtmp.CsidAmf,
				MsgLen:       uint32(len(bMetadata)),
				MsgTypeId:    base.RtmpTypeIdMetadata,
				MsgStreamId:  rtmp.Msid1,
				TimestampAbs: 0,
			},
			Payload: bMetadata,
		})
		r.hasEmittedMetadata = true
	}

	var msg base.RtmpMsg
	msg.Header.MsgStreamId = rtmp.Msid1

	if isAudio {
		msg.Header.Csid = rtmp.CsidAudio
		msg.Header.MsgTypeId = base.RtmpTypeIdAudio
	} else {
		msg.Header.Csid = rtmp.CsidVideo
		msg.Header.MsgTypeId = base.RtmpTypeIdVideo
	}

	msg.Header.MsgLen = uint32(len(payload))
	msg.Header.TimestampAbs = uint32(timestamp)
	msg.Payload = payload

	r.onRtmpMsg(msg)
}

func (r *AvPacket2RtmpRemuxer) setVps(b []byte) {
	r.vps = r.vps[0:0]
	r.vps = append(r.vps, b...)
}

func (r *AvPacket2RtmpRemuxer) setSps(b []byte) {
	r.sps = r.sps[0:0]
	r.sps = append(r.sps, b...)
}

func (r *AvPacket2RtmpRemuxer) setPps(b []byte) {
	r.pps = r.pps[0:0]
	r.pps = append(r.pps, b...)
}

func (r *AvPacket2RtmpRemuxer) clearVideoSeqHeader() {
	r.vps = r.vps[0:0]
	r.sps = r.sps[0:0]
	r.pps = r.pps[0:0]
}
