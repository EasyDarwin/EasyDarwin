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
	if end-offset < 1 {
		return nil
	}

	return info
}

func (rtp *RTPInfo) IsKeyframeStart() bool {
	var realNALU uint8
	payloadHeader := rtp.Payload[0] //https://tools.ietf.org/html/rfc6184#section-5.2
	NaluType := uint8(payloadHeader & 0x1F)

	switch {
	case NaluType <= 23:
		realNALU = rtp.Payload[0]
		//log.Printf("Single NAL:%d", rtp.NaluType)
	case NaluType == 28 || NaluType == 29:
		realNALU = rtp.Payload[1]
		if realNALU&0x80 != 0 {
			//log.Printf("FU NAL Begin :%d", rtp.NaluType)
		} else {
			return false
		}
		if realNALU&0x40 != 0 {
			//log.Printf("FU NAL End :%d", rtp.NaluType)
		}
	}
	if realNALU&0x1F == 0x05 {
		return true
	}
	if realNALU&0x1F == 0x07 { // maybe sps pps header + key frame?
		if len(rtp.Payload) < 200 { // consider sps pps header only.
			return false
		}
		return true
	}
	return false
}

func (rtp *RTPInfo) IsKeyframeStartH265() bool {
	if len(rtp.Payload) >= 3 {
		firstByte := rtp.Payload[0]
		headerType := (firstByte >> 1) & 0x3f
		var frameType uint8
		if headerType == 49 { //Fragmentation Units

			FUHeader := rtp.Payload[2]
			/*
			   +---------------+
			   |0|1|2|3|4|5|6|7|
			   +-+-+-+-+-+-+-+-+
			   |S|E|  FuType   |
			   +---------------+
			*/
			rtpStart := (FUHeader & 0x80) != 0
			if !rtpStart {
				if (FUHeader & 0x40) != 0 {
					//log.Printf("FU frame end")
				}
				return false
			} else {
				//log.Printf("FU frame start")
			}
			frameType = FUHeader & 0x3f
		} else if headerType == 48 { //Aggregation Packets

		} else if headerType == 50 { //PACI Packets

		} else { // Single NALU
			/*
							+---------------+---------------+
				            |0|1|2|3|4|5|6|7|0|1|2|3|4|5|6|7|
				            +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
				            |F|   Type    |  LayerId  | TID |
				            +-------------+-----------------+
			*/
			frameType = firstByte & 0x7e
		}
		if frameType >= 16 && frameType <= 21 {
			return true
		}
		if frameType == 32 {
			// vps sps pps...
			if len(rtp.Payload) < 200 { // consider sps pps header only.
				return false
			}
			return true
		}
	}
	return false
}
