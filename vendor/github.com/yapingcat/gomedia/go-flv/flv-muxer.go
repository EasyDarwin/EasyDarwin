package flv

import (
    "bytes"
    "errors"

    "github.com/yapingcat/gomedia/go-codec"
)

func WriteAudioTag(data []byte, cid FLV_SOUND_FORMAT, sampleRate int, channelCount int, isSequenceHeader bool) []byte {
    var atag AudioTag
    atag.SoundFormat = uint8(cid)
    if cid == FLV_AAC {
        atag.SoundRate = uint8(FLV_SAMPLE_44000)
        atag.SoundSize = 1
        atag.SoundType = 1
    } else {
        switch sampleRate {
        case 5500:
            atag.SoundRate = uint8(FLV_SAMPLE_5500)
        case 11025:
            atag.SoundRate = uint8(FLV_SAMPLE_11000)
        case 22050:
            atag.SoundRate = uint8(FLV_SAMPLE_22000)
        case 44100:
            atag.SoundRate = uint8(FLV_SAMPLE_44000)
        default:
            atag.SoundRate = uint8(FLV_SAMPLE_44000)
        }
        atag.SoundSize = 1
        if channelCount > 1 {
            atag.SoundType = 1
        } else {
            atag.SoundType = 0
        }
    }

    if isSequenceHeader {
        atag.AACPacketType = 0
    } else {
        atag.AACPacketType = 1
    }
    tagData := atag.Encode()
    tagData = append(tagData, data...)
    return tagData
}

func WriteVideoTag(data []byte, isKey bool, cid FLV_VIDEO_CODEC_ID, cts int32, isSequenceHeader bool) []byte {
    var vtag VideoTag
    vtag.CodecId = uint8(cid)
    vtag.CompositionTime = cts
    if isKey {
        vtag.FrameType = uint8(KEY_FRAME)
    } else {
        vtag.FrameType = uint8(INTER_FRAME)
    }
    if isSequenceHeader {
        vtag.AVCPacketType = uint8(AVC_SEQUENCE_HEADER)
    } else {
        vtag.AVCPacketType = uint8(AVC_NALU)
    }
    tagData := vtag.Encode()
    tagData = append(tagData, data...)
    return tagData
}

type AVTagMuxer interface {
    Write(frames []byte, pts uint32, dts uint32) [][]byte
}

type AVCMuxer struct {
    spsset map[uint64][]byte
    ppsset map[uint64][]byte
    cache  []byte
    first  bool
}

func NewAVCMuxer() *AVCMuxer {
    return &AVCMuxer{
        spsset: make(map[uint64][]byte),
        ppsset: make(map[uint64][]byte),
        cache:  make([]byte, 0, 1024),
        first:  true,
    }
}

func (muxer *AVCMuxer) Write(frames []byte, pts uint32, dts uint32) [][]byte {
    var vcl bool = false
    var isKey bool = false
    codec.SplitFrameWithStartCode(frames, func(nalu []byte) bool {
        naltype := codec.H264NaluType(nalu)
        switch naltype {
        case codec.H264_NAL_SPS:
            spsid := codec.GetSPSIdWithStartCode(nalu)
            s, found := muxer.spsset[spsid]
            if !found || !bytes.Equal(s, nalu) {
                naluCopy := make([]byte, len(nalu))
                copy(naluCopy, nalu)
                muxer.spsset[spsid] = naluCopy
                muxer.cache = append(muxer.cache, codec.ConvertAnnexBToAVCC(nalu)...)
            }
        case codec.H264_NAL_PPS:
            ppsid := codec.GetPPSIdWithStartCode(nalu)
            muxer.ppsset[ppsid] = nalu
            s, found := muxer.ppsset[ppsid]
            if !found || !bytes.Equal(s, nalu) {
                naluCopy := make([]byte, len(nalu))
                copy(naluCopy, nalu)
                muxer.ppsset[ppsid] = naluCopy
                muxer.cache = append(muxer.cache, codec.ConvertAnnexBToAVCC(nalu)...)
            }
        default:
            if naltype <= codec.H264_NAL_I_SLICE {
                vcl = true
                if naltype == codec.H264_NAL_I_SLICE {
                    isKey = true
                }
            }
            muxer.cache = append(muxer.cache, codec.ConvertAnnexBToAVCC(nalu)...)
        }
        return true
    })
    var tags [][]byte
    if muxer.first && len(muxer.ppsset) > 0 && len(muxer.spsset) > 0 {
        spss := make([][]byte, len(muxer.spsset))
        idx := 0
        for _, sps := range muxer.spsset {
            spss[idx] = sps
            idx++
        }
        idx = 0
        ppss := make([][]byte, len(muxer.ppsset))
        for _, pps := range muxer.ppsset {
            ppss[idx] = pps
            idx++
        }
        extraData, _ := codec.CreateH264AVCCExtradata(spss, ppss)
        tags = append(tags, WriteVideoTag(extraData, true, FLV_AVC, 0, true))
        muxer.first = false
    }

    if vcl {
        tags = append(tags, WriteVideoTag(muxer.cache, isKey, FLV_AVC, int32(pts-dts), false))
        muxer.cache = muxer.cache[:0]
    }
    return tags
}

type HevcMuxer struct {
    hvcc  *codec.HEVCRecordConfiguration
    cache []byte
    first bool
}

func NewHevcMuxer() *HevcMuxer {
    return &HevcMuxer{
        hvcc:  codec.NewHEVCRecordConfiguration(),
        cache: make([]byte, 0, 1024),
        first: true,
    }
}

func (muxer *HevcMuxer) Write(frames []byte, pts uint32, dts uint32) [][]byte {
    var vcl bool = false
    var isKey bool = false
    codec.SplitFrameWithStartCode(frames, func(nalu []byte) bool {
        naltype := codec.H265NaluType(nalu)
        switch naltype {
        case codec.H265_NAL_SPS:
            muxer.hvcc.UpdateSPS(nalu)
            muxer.cache = append(muxer.cache, codec.ConvertAnnexBToAVCC(nalu)...)
        case codec.H265_NAL_PPS:
            muxer.hvcc.UpdatePPS(nalu)
            muxer.cache = append(muxer.cache, codec.ConvertAnnexBToAVCC(nalu)...)
        case codec.H265_NAL_VPS:
            muxer.hvcc.UpdateVPS(nalu)
            muxer.cache = append(muxer.cache, codec.ConvertAnnexBToAVCC(nalu)...)
        default:
            if naltype >= 16 && naltype <= 21 {
                isKey = true
            }
            vcl = codec.IsH265VCLNaluType(naltype)
            muxer.cache = append(muxer.cache, codec.ConvertAnnexBToAVCC(nalu)...)
        }
        return true
    })
    var tags [][]byte
    if muxer.first && len(muxer.hvcc.Arrays) > 0 {
        extraData, _ := muxer.hvcc.Encode()
        tags = append(tags, WriteVideoTag(extraData, true, FLV_HEVC, 0, true))
        muxer.first = false
    }
    if vcl {
        tags = append(tags, WriteVideoTag(muxer.cache, isKey, FLV_HEVC, int32(pts-dts), false))
        muxer.cache = muxer.cache[:0]
    }
    return tags
}

func CreateVideoMuxer(cid FLV_VIDEO_CODEC_ID) AVTagMuxer {
    if cid == FLV_AVC {
        return NewAVCMuxer()
    } else if cid == FLV_HEVC {
        return NewHevcMuxer()
    }
    return nil
}

type AACMuxer struct {
    updateSequence bool
}

func NewAACMuxer() *AACMuxer {
    return &AACMuxer{updateSequence: true}
}

func (muxer *AACMuxer) Write(frames []byte, pts uint32, dts uint32) [][]byte {
    var tags [][]byte
    codec.SplitAACFrame(frames, func(aac []byte) {
        hdr := codec.NewAdtsFrameHeader()
        hdr.Decode(aac)
        if muxer.updateSequence {
            asc, _ := codec.ConvertADTSToASC(aac)
            tags = append(tags, WriteAudioTag(asc.Encode(), FLV_AAC, 0, 0, true))
            muxer.updateSequence = false
        }
        tags = append(tags, WriteAudioTag(aac[7:], FLV_AAC, 0, 0, false))
    })
    return tags
}

type G711AMuxer struct {
    channelCount int
    sampleRate   int
}

func NewG711AMuxer(channelCount int, sampleRate int) *G711AMuxer {
    return &G711AMuxer{
        channelCount: channelCount,
        sampleRate:   sampleRate,
    }
}

func (muxer *G711AMuxer) Write(frames []byte, pts uint32, dts uint32) [][]byte {
    tags := make([][]byte, 1)
    tags[0] = WriteAudioTag(frames, FLV_G711A, muxer.sampleRate, muxer.channelCount, true)
    return tags
}

type G711UMuxer struct {
    channelCount int
    sampleRate   int
}

func NewG711UMuxer(channelCount int, sampleRate int) *G711UMuxer {
    return &G711UMuxer{
        channelCount: channelCount,
        sampleRate:   sampleRate,
    }
}

func (muxer *G711UMuxer) Write(frames []byte, pts uint32, dts uint32) [][]byte {
    tags := make([][]byte, 1)
    tags[0] = WriteAudioTag(frames, FLV_G711U, muxer.sampleRate, muxer.channelCount, true)
    return tags
}

type Mp3Muxer struct {
}

func (muxer *Mp3Muxer) Write(frames []byte, pts uint32, dts uint32) [][]byte {
    tags := make([][]byte, 1)
    codec.SplitMp3Frames(frames, func(head *codec.MP3FrameHead, frame []byte) {
        tags = append(tags, WriteAudioTag(frames, FLV_MP3, head.GetSampleRate(), head.GetChannelCount(), true))
    })
    return tags
}

func CreateAudioMuxer(cid FLV_SOUND_FORMAT) AVTagMuxer {
    if cid == FLV_AAC {
        return &AACMuxer{updateSequence: true}
    } else if cid == FLV_G711A {
        return NewG711AMuxer(1, 5500)
    } else if cid == FLV_G711U {
        return NewG711UMuxer(1, 5500)
    } else if cid == FLV_MP3 {
        return new(Mp3Muxer)
    } else {
        return nil
    }
}

type FlvMuxer struct {
    videoMuxer AVTagMuxer
    audioMuxer AVTagMuxer
}

func NewFlvMuxer(vid FLV_VIDEO_CODEC_ID, aid FLV_SOUND_FORMAT) *FlvMuxer {
    return &FlvMuxer{
        videoMuxer: CreateVideoMuxer(vid),
        audioMuxer: CreateAudioMuxer(aid),
    }
}

func (muxer *FlvMuxer) SetVideoCodeId(cid FLV_VIDEO_CODEC_ID) {
    muxer.videoMuxer = CreateVideoMuxer(cid)
}

func (muxer *FlvMuxer) SetAudioCodeId(cid FLV_SOUND_FORMAT) {
    muxer.audioMuxer = CreateAudioMuxer(cid)
}

func (muxer *FlvMuxer) WriteVideo(frames []byte, pts uint32, dts uint32) ([][]byte, error) {
    if muxer.videoMuxer == nil {
        return nil, errors.New("video Muxer is Nil")
    }
    return muxer.WriteFrames(VIDEO_TAG, frames, pts, dts)
}

func (muxer *FlvMuxer) WriteAudio(frames []byte, pts uint32, dts uint32) ([][]byte, error) {
    if muxer.audioMuxer == nil {
        return nil, errors.New("audio Muxer is Nil")
    }
    return muxer.WriteFrames(AUDIO_TAG, frames, pts, dts)
}

func (muxer *FlvMuxer) WriteFrames(frameType TagType, frames []byte, pts uint32, dts uint32) ([][]byte, error) {

    var ftag FlvTag
    var tags [][]byte
    if frameType == AUDIO_TAG {
        ftag.TagType = uint8(AUDIO_TAG)
        tags = muxer.audioMuxer.Write(frames, pts, dts)
    } else if frameType == VIDEO_TAG {
        ftag.TagType = uint8(VIDEO_TAG)
        tags = muxer.videoMuxer.Write(frames, pts, dts)
    } else {
        return nil, errors.New("unsupport Frame Type")
    }
    ftag.Timestamp = dts & 0x00FFFFFF
    ftag.TimestampExtended = uint8(dts >> 24 & 0xFF)

    tmptags := make([][]byte, 0, 1)
    for _, tag := range tags {
        ftag.DataSize = uint32(len(tag))
        vtag := ftag.Encode()
        vtag = append(vtag, tag...)
        tmptags = append(tmptags, vtag)
    }
    return tmptags, nil
}
