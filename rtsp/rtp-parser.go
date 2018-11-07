package rtsp

import (
	"encoding/binary"
)

const (
	RTP_FIXED_HEADER_LENGTH = 12
)

type RTPInfo struct {
	Version        int
	Padding        bool
	Extension      bool
	CSRCCnt        int
	Marker         bool
	PayloadType    int
	SequenceNumber int
	Timestamp      int
	SSRC           int
	Payload        []byte
}

func ParseRTP(rtpBytes []byte) *RTPInfo {
	if len(rtpBytes) < RTP_FIXED_HEADER_LENGTH {
		return nil
	}
	firstByte := rtpBytes[0]
	secondByte := rtpBytes[1]
	info := &RTPInfo{
		Version:   int(firstByte >> 6),
		Padding:   (firstByte>>5)&1 == 1,
		Extension: (firstByte>>4)&1 == 1,
		CSRCCnt:   int(firstByte & 0x0f),

		Marker:         secondByte>>7 == 1,
		PayloadType:    int(secondByte & 0x7f),
		SequenceNumber: int(binary.BigEndian.Uint16(rtpBytes[2:])),
		Timestamp:      int(binary.BigEndian.Uint32(rtpBytes[4:])),
		SSRC:           int(binary.BigEndian.Uint32(rtpBytes[8:])),
	}
	offset := RTP_FIXED_HEADER_LENGTH
	end := len(rtpBytes)
	if end-offset >= 4*info.CSRCCnt {
		offset += 4 * info.CSRCCnt
	}
	if info.Extension && end-offset >= 4 {
		extLen := 4 * int(binary.BigEndian.Uint16(rtpBytes[offset+2:]))
		offset += 4
		if end-offset >= extLen {
			offset += extLen
		}
	}
	if info.Padding && end-offset > 0 {
		paddingLen := int(rtpBytes[end-1])
		if end-offset >= paddingLen {
			end -= paddingLen
		}
	}
	info.Payload = rtpBytes[offset:end]
	return info
}

func (rtp *RTPInfo) IsKeyframeStart() bool {
	if len(rtp.Payload) >= 2 && rtp.Payload[0] == 0x7c && (rtp.Payload[1] == 0x87 || rtp.Payload[1] == 0x85) {
		return true
	}
	return false
}

func (rtp *RTPInfo) IsKeyframeStartH265() bool {
	if len(rtp.Payload) >= 3 {
		firstByte := rtp.Payload[0]
		headerType := (firstByte >> 1) & 0x3f
		if headerType == 49 {
			frametByte := rtp.Payload[2]
			frameType := frametByte & 0x3f
			rtpStart := (frametByte & 0x80) >> 7
			if rtpStart == 1 && (frameType == 19 || frameType == 20 || frameType == 21 || frameType == 32 || frameType == 33 || frameType == 34) {
				return true
			}
		}
	}
	return false
}
