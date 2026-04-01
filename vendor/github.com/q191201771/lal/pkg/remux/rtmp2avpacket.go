// Copyright 2022, Chef.  All rights reserved.
// https://github.com/q191201771/lal
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

package remux

import (
	"github.com/q191201771/lal/pkg/base"
	"github.com/q191201771/lal/pkg/h2645"
)

// TODO(chef): 该文件处于开发阶段，请不要直接使用
// TODO(chef): 支持音频 202206

// Rtmp2AvPacketRemuxer
//
// 用途：
// - 将rtmp流中的视频转换成ffmpeg可解码的格式
type Rtmp2AvPacketRemuxer struct {
	option     Rtmp2AvPacketRemuxerOption
	onAvPacket func(pkt base.AvPacket, arg interface{})

	spspps []byte // annexb格式
}

type Rtmp2AvPacketRemuxerOption struct {
	// TODO(chef): impl me 202206
	TryInPlaceFlag bool // 尝试在原有内存上直接修改
	EraseSeiFlag   bool
}

var defaultRtmp2AvPacketRemuxerOption = Rtmp2AvPacketRemuxerOption{
	TryInPlaceFlag: false,
	EraseSeiFlag:   true,
}

func NewRtmp2AvPacketRemuxer() *Rtmp2AvPacketRemuxer {
	return &Rtmp2AvPacketRemuxer{
		option:     defaultRtmp2AvPacketRemuxerOption,
		onAvPacket: defaultOnAvPacket,
	}
}

func (r *Rtmp2AvPacketRemuxer) WithOption(modOption func(option *Rtmp2AvPacketRemuxerOption)) *Rtmp2AvPacketRemuxer {
	modOption(&r.option)
	return r
}

// WithOnAvPacket
//
// @param onAvPacket: pkt 内存由内部新申请，回调后内部不再使用
func (r *Rtmp2AvPacketRemuxer) WithOnAvPacket(onAvPacket func(pkt base.AvPacket, arg interface{})) *Rtmp2AvPacketRemuxer {
	r.onAvPacket = onAvPacket
	return r
}

func (r *Rtmp2AvPacketRemuxer) FeedRtmpMsg(msg base.RtmpMsg, arg interface{}) error {
	switch msg.Header.MsgTypeId {
	case base.RtmpTypeIdVideo:
		return r.feedVideo(msg, arg)
	}
	return nil
}

// ---------------------------------------------------------------------------------------------------------------------

func (r *Rtmp2AvPacketRemuxer) feedVideo(msg base.RtmpMsg, arg interface{}) error {
	if len(msg.Payload) <= 5 {
		return nil
	}

	isH264 := msg.VideoCodecId() == base.RtmpCodecIdAvc

	var err error
	if msg.IsVideoKeySeqHeader() {
		r.spspps, err = h2645.SeqHeader2Annexb(isH264, msg.Payload)
		return err
	}

	var out []byte
	var vps, sps, pps []byte
	appendSpsppsFlag := false
	err = h2645.IterateNaluAvcc(msg.Payload[5:], func(nal []byte) {
		nalType := h2645.ParseNaluType(isH264, nal[0])

		if isH264 {
			if nalType == h2645.H264NaluTypeSps {
				sps = nal
			} else if nalType == h2645.H264NaluTypePps {
				pps = nal
				if len(sps) != 0 && len(pps) != 0 {
					r.spspps = r.spspps[0:0]
					r.spspps = append(r.spspps, h2645.NaluStartCode4...)
					r.spspps = append(r.spspps, sps...)
					r.spspps = append(r.spspps, h2645.NaluStartCode4...)
					r.spspps = append(r.spspps, pps...)
				}
			} else if nalType == h2645.H264NaluTypeIdrSlice {
				if !appendSpsppsFlag {
					out = append(out, r.spspps...)
					appendSpsppsFlag = true
				}

				out = append(out, h2645.NaluStartCode4...)
				out = append(out, nal...)
			} else if nalType == h2645.H264NaluTypeSei {
				if r.option.EraseSeiFlag {
					// noop
				} else {
					out = append(out, h2645.NaluStartCode4...)
					out = append(out, nal...)
				}
			} else {
				out = append(out, h2645.NaluStartCode4...)
				out = append(out, nal...)
			}
		} else {
			if nalType == h2645.H265NaluTypeVps {
				vps = nal
			} else if nalType == h2645.H265NaluTypeSps {
				sps = nal
			} else if nalType == h2645.H265NaluTypePps {
				pps = nal
				if len(vps) != 0 && len(sps) != 0 && len(pps) != 0 {
					r.spspps = r.spspps[0:0]
					r.spspps = append(r.spspps, h2645.NaluStartCode4...)
					r.spspps = append(r.spspps, vps...)
					r.spspps = append(r.spspps, h2645.NaluStartCode4...)
					r.spspps = append(r.spspps, sps...)
					r.spspps = append(r.spspps, h2645.NaluStartCode4...)
					r.spspps = append(r.spspps, pps...)
				}
			} else if h2645.H265IsIrapNalu(nalType) {
				if !appendSpsppsFlag {
					out = append(out, r.spspps...)
					appendSpsppsFlag = true
				}

				out = append(out, h2645.NaluStartCode4...)
				out = append(out, nal...)
			} else if nalType == h2645.H265NaluTypeSei {
				if r.option.EraseSeiFlag {
					// noop
				} else {
					out = append(out, h2645.NaluStartCode4...)
					out = append(out, nal...)
				}
			} else {
				out = append(out, h2645.NaluStartCode4...)
				out = append(out, nal...)
			}
		}
	})

	if len(out) > 0 {
		pkt := base.AvPacket{
			Timestamp: int64(msg.Header.TimestampAbs),
			Pts:       int64(msg.Pts()),
			Payload:   out,
		}
		if isH264 {
			pkt.PayloadType = base.AvPacketPtAvc
		} else {
			pkt.PayloadType = base.AvPacketPtHevc
		}
		r.onAvPacket(pkt, arg)
	}

	return err
}

// ---------------------------------------------------------------------------------------------------------------------

func defaultOnAvPacket(pkt base.AvPacket, arg interface{}) {
	// noop
}
