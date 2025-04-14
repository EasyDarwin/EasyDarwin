package mpegps

//单元来源于https://github.com/yapingcat/gomedia
import (
	"errors"
	"github.com/q191201771/lal/pkg/avc"
	"github.com/q191201771/lal/pkg/hevc"
	"github.com/q191201771/naza/pkg/nazalog"
)

type psStream struct {
	sid       uint8
	cid       PsStreamType
	pts       uint64
	dts       uint64
	streamBuf []byte
}

func newPsStream(sid uint8, cid PsStreamType) *psStream {
	return &psStream{
		sid:       sid,
		cid:       cid,
		streamBuf: make([]byte, 0, 4096),
	}
}
func (p *psStream) setCid(cid PsStreamType) {
	p.cid = cid
}

func (p *psStream) clearBuf() {
	p.streamBuf = p.streamBuf[:0]
}

type PsDemuxer struct {
	streamMap map[uint8]*psStream
	pkg       *PsPacket
	mpeg1     bool
	cache     []byte
	OnFrame   func(frame []byte, cid PsStreamType, pts uint64, dts uint64)
	//解ps包过程中，解码回调psm，system header，pes包等
	//decodeResult 解码ps包时的产生的错误
	//这个回调主要用于debug，查看是否ps包存在问题
	OnPacket func(pkg Display, decodeResult error)

	verifyBuf []byte

	log nazalog.Logger
}

func NewPsDemuxer() *PsDemuxer {
	psDemuxer := &PsDemuxer{
		streamMap: make(map[uint8]*psStream),
		pkg:       new(PsPacket),
		cache:     make([]byte, 0, 256),
		OnFrame:   nil,
		OnPacket:  nil,
	}
	return psDemuxer
}

func (psDemuxer *PsDemuxer) Input(data []byte) error {
	var bs *BitStream
	if len(psDemuxer.cache) > 0 {
		psDemuxer.cache = append(psDemuxer.cache, data...)
		bs = NewBitStream(psDemuxer.cache)
	} else {
		bs = NewBitStream(data)
	}

	saveReseved := func() {
		tmpcache := make([]byte, bs.RemainBytes())
		copy(tmpcache, bs.RemainData())
		psDemuxer.cache = tmpcache
	}

	var ret error = nil
	for !bs.EOS() {
		if mpegerr, ok := ret.(Error); ok {
			if mpegerr.NeedMore() {
				saveReseved()
			}
			break
		}
		if bs.RemainBits() < 32 {
			ret = errNeedMore
			saveReseved()
			break
		}
		prefix_code := bs.NextBits(32)
		switch prefix_code {
		case 0x000001BA: //pack header
			if psDemuxer.pkg.Header == nil {
				psDemuxer.pkg.Header = new(PsPackHeader)
			}
			ret = psDemuxer.pkg.Header.Decode(bs)
			psDemuxer.mpeg1 = psDemuxer.pkg.Header.IsMpeg1
			if psDemuxer.OnPacket != nil {
				psDemuxer.OnPacket(psDemuxer.pkg.Header, ret)
			}
		case 0x000001BB: //system header
			if psDemuxer.pkg.Header == nil {
				return errors.New("PsDemuxer.pkg.Header must not be nil")
			}
			if psDemuxer.pkg.System == nil {
				psDemuxer.pkg.System = new(SystemHeader)
			}
			ret = psDemuxer.pkg.System.Decode(bs)
			if psDemuxer.OnPacket != nil {
				psDemuxer.OnPacket(psDemuxer.pkg.System, ret)
			}
		case 0x000001BC: //program stream map
			if psDemuxer.pkg.Psm == nil {
				psDemuxer.pkg.Psm = new(ProgramStreamMap)
			}
			if ret = psDemuxer.pkg.Psm.Decode(bs); ret == nil {
				for _, streaminfo := range psDemuxer.pkg.Psm.StreamMap {
					if _, found := psDemuxer.streamMap[streaminfo.ElementaryStreamId]; !found {
						stream := newPsStream(streaminfo.ElementaryStreamId, PsStreamType(streaminfo.StreamType))
						psDemuxer.streamMap[stream.sid] = stream
					} else {
						stream := psDemuxer.streamMap[streaminfo.ElementaryStreamId]
						stream.setCid(PsStreamType(streaminfo.StreamType))
					}
				}
			}
			if psDemuxer.OnPacket != nil {
				psDemuxer.OnPacket(psDemuxer.pkg.Psm, ret)
			}
		case 0x000001BD, 0x000001BE, 0x000001BF, 0x000001F0, 0x000001F1,
			0x000001F2, 0x000001F3, 0x000001F4, 0x000001F5, 0x000001F6,
			0x000001F7, 0x000001F8, 0x000001F9, 0x000001FA, 0x000001FB:
			if psDemuxer.pkg.CommPes == nil {
				psDemuxer.pkg.CommPes = new(CommonPesPacket)
			}
			ret = psDemuxer.pkg.CommPes.Decode(bs)
		case 0x000001FF: //program stream directory
			if psDemuxer.pkg.Psd == nil {
				psDemuxer.pkg.Psd = new(ProgramStreamDirectory)
			}
			ret = psDemuxer.pkg.Psd.Decode(bs)
		case 0x000001B9: //MPEG_program_end_code
			continue
		default:
			if prefix_code&0xFFFFFFE0 == 0x000001C0 || prefix_code&0xFFFFFFE0 == 0x000001E0 {
				if psDemuxer.pkg.Pes == nil {
					psDemuxer.pkg.Pes = NewPesPacket()
				}
				if psDemuxer.mpeg1 {
					ret = psDemuxer.pkg.Pes.DecodeMpeg1(bs)
				} else {
					ret = psDemuxer.pkg.Pes.Decode(bs)
				}
				if psDemuxer.OnPacket != nil {
					psDemuxer.OnPacket(psDemuxer.pkg.Pes, ret)
				}
				if ret == nil {
					if stream, found := psDemuxer.streamMap[psDemuxer.pkg.Pes.StreamId]; found {
						if psDemuxer.mpeg1 && stream.cid == PsStreamUnknow {
							psDemuxer.guessCodecid(stream)
						}
						psDemuxer.demuxPespacket(stream, psDemuxer.pkg.Pes)
					} else {
						if psDemuxer.mpeg1 {
							stream := newPsStream(psDemuxer.pkg.Pes.StreamId, PsStreamUnknow)
							psDemuxer.streamMap[stream.sid] = stream
							stream.streamBuf = append(stream.streamBuf, psDemuxer.pkg.Pes.PesPayload...)
							stream.pts = psDemuxer.pkg.Pes.Pts
							stream.dts = psDemuxer.pkg.Pes.Dts
						} else if psDemuxer.pkg.Pes.StreamId == uint8(PesStreamVideo) {
							if len(psDemuxer.verifyBuf) > 256 {
								psDemuxer.verifyBuf = psDemuxer.verifyBuf[:0]
							}
							psDemuxer.verifyBuf = append(psDemuxer.verifyBuf, psDemuxer.pkg.Pes.PesPayload...)
							if h26x, err := mpegH26xVerify(psDemuxer.verifyBuf); err == nil {
								switch h26x {
								case CodecUnknown:
								case CodecH264:
									streamH264 := newPsStream(uint8(PesStreamVideo), PsStreamH264)
									psDemuxer.streamMap[streamH264.sid] = streamH264
									psDemuxer.demuxPespacket(streamH264, psDemuxer.pkg.Pes)
								case CodecH265:
									streamH265 := newPsStream(uint8(PesStreamVideo), PsStreamH265)
									psDemuxer.streamMap[streamH265.sid] = streamH265
									psDemuxer.demuxPespacket(streamH265, psDemuxer.pkg.Pes)
								}
							}
						} else if psDemuxer.pkg.Pes.StreamId == uint8(PesStreamAudio) {
							if _, found = psDemuxer.streamMap[uint8(PesStreamVideo)]; found {
								psStreamType := audioVerify(psDemuxer.pkg.Pes.PesPayload)
								streamAudio := newPsStream(uint8(PesStreamAudio), psStreamType)
								psDemuxer.streamMap[streamAudio.sid] = streamAudio
								psDemuxer.demuxPespacket(streamAudio, psDemuxer.pkg.Pes)
							}
						}
					}
				}
			} else {
				bs.SkipBits(8)
			}
		}
	}

	if ret == nil && len(psDemuxer.cache) > 0 {
		psDemuxer.cache = nil
	}

	return ret
}

func (psDemuxer *PsDemuxer) Flush() {
	for _, stream := range psDemuxer.streamMap {
		if len(stream.streamBuf) == 0 {
			continue
		}
		if psDemuxer.OnFrame != nil {
			psDemuxer.OnFrame(stream.streamBuf, stream.cid, stream.pts/90, stream.dts/90)
		}
	}
}

func (psDemuxer *PsDemuxer) guessCodecid(stream *psStream) {
	if stream.sid&0xE0 == uint8(PesStreamAudio) {
		psStreamType := audioVerify(psDemuxer.pkg.Pes.PesPayload)
		stream.cid = psStreamType
	} else if stream.sid&0xE0 == uint8(PesStreamVideo) {
		if h26x, err := mpegH26xVerify(stream.streamBuf); err == nil {
			switch h26x {
			case CodecUnknown:
			case CodecH264:
				stream.cid = PsStreamH264
			case CodecH265:
				stream.cid = PsStreamH265
			}
		}
	}
}

func (psDemuxer *PsDemuxer) demuxPespacket(stream *psStream, pes *PesPacket) error {
	switch stream.cid {
	case PsStreamAac, PsStreamG711A, PsStreamG711U:
		return psDemuxer.demuxAudio(stream, pes)
	case PsStreamH264, PsStreamH265:
		return psDemuxer.demuxH26x(stream, pes)
	case PsStreamUnknow:
		if stream.pts != pes.Pts {
			stream.streamBuf = nil
		}
		stream.streamBuf = append(stream.streamBuf, pes.PesPayload...)
		stream.pts = pes.Pts
		stream.dts = pes.Dts
	}
	return nil
}

func (psDemuxer *PsDemuxer) demuxAudio(stream *psStream, pes *PesPacket) error {
	if psDemuxer.OnFrame != nil {
		psDemuxer.OnFrame(pes.PesPayload, stream.cid, pes.Pts/90, pes.Dts/90)
	}
	return nil
}

func (psDemuxer *PsDemuxer) demuxH26x(stream *psStream, pes *PesPacket) error {
	if stream.pts == 0 {
		stream.streamBuf = append(stream.streamBuf, pes.PesPayload...)
		stream.pts = pes.Pts
		stream.dts = pes.Dts
	} else if stream.pts == pes.Pts || pes.Pts == 0 {
		stream.streamBuf = append(stream.streamBuf, pes.PesPayload...)
	} else {
		start, sc := FindStartCode(stream.streamBuf, 0)
		for start >= 0 && start < len(stream.streamBuf) {
			end, sc2 := FindStartCode(stream.streamBuf, start+int(sc))
			if end < 0 {
				end = len(stream.streamBuf)
			}
			if stream.cid == PsStreamH264 {
				naluType := H264NaluType(stream.streamBuf[start:])
				if naluType != avc.NaluTypeAud {
					if psDemuxer.OnFrame != nil {
						psDemuxer.OnFrame(stream.streamBuf[start:end], stream.cid, stream.pts/90, stream.dts/90)
					}
				}
			} else if stream.cid == PsStreamH265 {
				naluType := H265NaluType(stream.streamBuf[start:])
				if naluType != hevc.NaluTypeAud {
					if psDemuxer.OnFrame != nil {
						psDemuxer.OnFrame(stream.streamBuf[start:end], stream.cid, stream.pts/90, stream.dts/90)
					}
				}
			}
			start = end
			sc = sc2
		}
		stream.streamBuf = nil
		stream.streamBuf = append(stream.streamBuf, pes.PesPayload...)
		stream.pts = pes.Pts
		stream.dts = pes.Dts
	}

	return nil
}
