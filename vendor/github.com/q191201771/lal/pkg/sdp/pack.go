// Copyright 2021, Chef.  All rights reserved.
// https://github.com/q191201771/lal
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

package sdp

import (
	"encoding/base64"
	"encoding/hex"
	"fmt"
	"strings"

	"github.com/q191201771/lal/pkg/base"
)

type VideoInfo struct {
	VideoPt       base.AvPacketPt
	Vps, Sps, Pps []byte
}

type AudioInfo struct {
	AudioPt           base.AvPacketPt
	SamplingFrequency int
	Asc               []byte
}

func Pack(videoInfo VideoInfo, audioInfo AudioInfo) (ctx LogicContext, err error) {
	// 组装SDP头部
	sdpStr := fmt.Sprintf(`v=0
o=- 0 0 IN IP4 127.0.0.1
s=No Name
c=IN IP4 127.0.0.1
t=0 0
a=tool:%s
`, base.LalPackSdp)

	// 组装视频SDP信息
	streamid := 0
	videoSdpStr := buildVideoSdpInfo(videoInfo, streamid)
	if videoSdpStr != "" {
		sdpStr += videoSdpStr
		streamid++
	}

	// 组装音频SDP信息
	audioSdpStr := buildAudioSdpInfo(audioInfo, streamid)
	if audioSdpStr != "" {
		sdpStr += audioSdpStr
	}

	if videoSdpStr == "" && audioSdpStr == "" {
		return ctx, fmt.Errorf("invalid video and audio info, sdp:%s", sdpStr)
	}

	raw := []byte(strings.ReplaceAll(sdpStr, "\n", "\r\n"))
	ctx, err = ParseSdp2LogicContext(raw)
	return
}

func buildVideoSdpInfo(videoInfo VideoInfo, streamid int) string {
	if videoInfo.VideoPt == base.AvPacketPtAvc {
		if videoInfo.Sps == nil || videoInfo.Pps == nil {
			return ""
		}

		tmpl := `m=video 0 RTP/AVP %d
a=rtpmap:96 H264/90000
a=fmtp:96 packetization-mode=1; sprop-parameter-sets=%s,%s; profile-level-id=640016
a=control:streamid=%d
`
		return fmt.Sprintf(tmpl, base.AvPacketPtAvc, base64.StdEncoding.EncodeToString(videoInfo.Sps), base64.StdEncoding.EncodeToString(videoInfo.Pps), streamid)
	} else if videoInfo.VideoPt == base.AvPacketPtHevc {
		if videoInfo.Sps == nil || videoInfo.Pps == nil || videoInfo.Vps == nil {
			return ""
		}

		tmpl := `m=video 0 RTP/AVP %d
a=rtpmap:98 H265/90000
a=fmtp:98 profile-id=1;sprop-sps=%s;sprop-pps=%s;sprop-vps=%s
a=control:streamid=%d
`
		return fmt.Sprintf(tmpl, base.AvPacketPtHevc, base64.StdEncoding.EncodeToString(videoInfo.Sps), base64.StdEncoding.EncodeToString(videoInfo.Pps), base64.StdEncoding.EncodeToString(videoInfo.Vps), streamid)
	}

	return ""
}

func buildAudioSdpInfo(audioInfo AudioInfo, streamid int) string {
	if audioInfo.AudioPt == base.AvPacketPtAac {
		if audioInfo.Asc == nil {
			return ""
		}

		tmpl := `m=audio 0 RTP/AVP %d
b=AS:128
a=rtpmap:%d MPEG4-GENERIC/%d/2
a=fmtp:%d profile-level-id=1;mode=AAC-hbr;sizelength=13;indexlength=3;indexdeltalength=3; config=%s
a=control:streamid=%d
`
		return fmt.Sprintf(tmpl, base.AvPacketPtAac, base.AvPacketPtAac, audioInfo.SamplingFrequency, base.AvPacketPtAac, hex.EncodeToString(audioInfo.Asc), streamid)
	} else if audioInfo.AudioPt == base.AvPacketPtG711A {
		tmpl := `m=audio 0 RTP/AVP %d
a=rtpmap:%d PCMA/%d
a=control:streamid=%d
`
		return fmt.Sprintf(tmpl, base.AvPacketPtG711A, base.AvPacketPtG711A, audioInfo.SamplingFrequency, streamid)
	} else if audioInfo.AudioPt == base.AvPacketPtG711U {
		tmpl := `m=audio 0 RTP/AVP %d
a=rtpmap:%d PCMU/%d
a=control:streamid=%d
`
		return fmt.Sprintf(tmpl, base.AvPacketPtG711U, base.AvPacketPtG711U, audioInfo.SamplingFrequency, streamid)
	} else if audioInfo.AudioPt == base.AvPacketPtOpus {
		tmpl := `m=audio 0 RTP/AVP %d
a=rtpmap:%d opus/48000/2
a=control:streamid=%d
`
		return fmt.Sprintf(tmpl, base.AvPacketPtOpus, base.AvPacketPtOpus, streamid)
	}

	return ""
}
