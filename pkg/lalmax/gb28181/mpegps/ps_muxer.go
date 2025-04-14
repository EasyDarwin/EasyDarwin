package mpegps

//单元来源于https://github.com/yapingcat/gomedia
import (
	"github.com/q191201771/lal/pkg/avc"
	"github.com/q191201771/lal/pkg/hevc"
)

type PsMuxer struct {
	system     *SystemHeader
	psm        *ProgramStreamMap
	OnPacket   func(pkg []byte, pts uint64)
	firstframe bool
}

func NewPsMuxer() *PsMuxer {
	muxer := new(PsMuxer)
	muxer.firstframe = true
	muxer.system = new(SystemHeader)
	muxer.system.RateBound = 26234
	muxer.psm = new(ProgramStreamMap)
	muxer.psm.CurrentNextIndicator = 1
	muxer.psm.ProgramStreamMapVersion = 1
	muxer.OnPacket = nil
	return muxer
}

func (muxer *PsMuxer) AddStream(cid PsStreamType) uint8 {
	if cid == PsStreamH265 || cid == PsStreamH264 {
		es := NewElementaryStream(uint8(PesStreamVideo) + muxer.system.VideoBound)
		es.PStdBufferBoundScale = 1
		es.PStdBufferSizeBound = 400
		muxer.system.Streams = append(muxer.system.Streams, es)
		muxer.system.VideoBound++
		muxer.psm.StreamMap = append(muxer.psm.StreamMap, NewElementaryStreamElem(uint8(cid), es.StreamId))
		muxer.psm.ProgramStreamMapVersion++
		return es.StreamId
	} else {
		es := NewElementaryStream(uint8(PesStreamAudio) + muxer.system.AudioBound)
		es.PStdBufferBoundScale = 0
		es.PStdBufferSizeBound = 32
		muxer.system.Streams = append(muxer.system.Streams, es)
		muxer.system.AudioBound++
		muxer.psm.StreamMap = append(muxer.psm.StreamMap, NewElementaryStreamElem(uint8(cid), es.StreamId))
		muxer.psm.ProgramStreamMapVersion++
		return es.StreamId
	}
}

func (muxer *PsMuxer) Write(sid uint8, frame []byte, pts uint64, dts uint64) error {
	var stream *ElementaryStreamElem = nil
	for _, es := range muxer.psm.StreamMap {
		if es.ElementaryStreamId == sid {
			stream = es
			break
		}
	}
	if stream == nil {
		return errNotFound
	}
	if len(frame) <= 0 {
		return nil
	}
	var withaud bool = false
	var idrFlag bool = false
	var first bool = true
	var vcl bool = false
	if stream.StreamType == uint8(PsStreamH264) || stream.StreamType == uint8(PsStreamH265) {
		SplitFrame(frame, func(nalu []byte) bool {
			if stream.StreamType == uint8(PsStreamH264) {
				naluType := avc.ParseNaluType(nalu[0])
				if naluType == avc.NaluTypeAud {
					withaud = true
					return false
				} else if naluType >= avc.NaluTypeSlice && naluType <= avc.NaluTypeIdrSlice {
					if naluType == avc.NaluTypeIdrSlice {
						idrFlag = true
					}
					vcl = true
					return false
				}
				return true
			} else {
				naluType := hevc.ParseNaluType(nalu[0])
				if naluType == hevc.NaluTypeAud {
					withaud = true
					return false
				} else if naluType >= hevc.NaluTypeSliceBlaWlp && naluType <= hevc.NaluTypeSliceRsvIrapVcl23 ||
					naluType >= hevc.NaluTypeSliceTrailN && naluType <= hevc.NaluTypeSliceRaslR {
					if naluType >= hevc.NaluTypeSliceBlaWlp && naluType <= hevc.NaluTypeSliceRsvIrapVcl23 {
						idrFlag = true
					}
					vcl = true
					return false
				}
				return true
			}
		})
	}

	dts = dts * 90
	pts = pts * 90
	bsw := NewBitStreamWriter(1024)
	var pack PsPackHeader
	pack.SystemClockReferenceBase = dts - 3600
	pack.SystemClockReferenceExtension = 0
	pack.ProgramMuxRate = 6106
	pack.Encode(bsw)
	if muxer.firstframe || idrFlag {
		muxer.system.Encode(bsw)
		muxer.psm.Encode(bsw)
		muxer.firstframe = false
	}
	pespkg := NewPesPacket()
	for len(frame) > 0 {
		peshdrlen := 13
		pespkg.StreamId = sid
		pespkg.PtsDtsFlags = 0x03
		pespkg.PesHeaderDataLength = 10
		pespkg.Pts = pts
		pespkg.Dts = dts
		if idrFlag {
			pespkg.DataAlignmentIndicator = 1
		}
		if first && !withaud && vcl {
			if stream.StreamType == uint8(PsStreamH264) {
				pespkg.PesPayload = append(pespkg.PesPayload, H264AudNalu...)
				peshdrlen += 6
			} else if stream.StreamType == uint8(PsStreamH265) {
				pespkg.PesPayload = append(pespkg.PesPayload, H265AudNalu...)
				peshdrlen += 7
			}
		}
		if peshdrlen+len(frame) >= 0xFFFF {
			pespkg.PesPacketLength = 0xFFFF
			pespkg.PesPayload = append(pespkg.PesPayload, frame[0:0xFFFF-peshdrlen]...)
			frame = frame[0xFFFF-peshdrlen:]
		} else {
			pespkg.PesPacketLength = uint16(peshdrlen + len(frame))
			pespkg.PesPayload = append(pespkg.PesPayload, frame[0:]...)
			frame = frame[:0]
		}
		pespkg.Encode(bsw)
		pespkg.PesPayload = pespkg.PesPayload[:0]
		if muxer.OnPacket != nil {
			muxer.OnPacket(bsw.Bits(), pts)
		}
		bsw.Reset()
		first = false
	}
	return nil
}
