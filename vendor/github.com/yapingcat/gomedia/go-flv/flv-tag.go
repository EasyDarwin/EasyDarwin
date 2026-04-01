package flv

import "errors"

const FLVTAG_SIZE uint32 = 11

//  FLV Tag
//  ------------------------------------------------------------------------
//  Field                   type                     Comment
//  ------------------------------------------------------------------------
//  Reserved                UB[2]              Reserved for FMS, should be 0
//
//  Filter                  UB[1]              Indicates if packets are filtered.
//                                               0 = No pre-processing required.
//                                               1 = Pre-processing (such as decryption) of the packet is required before it can be rendered.
//                                               Shall be 0 in unencrypted files, and 1 for encrypted tags. See Annex F. FLV Encryption for the use of filters.
//
//  TagType                 UB[5]              Type of contents in this tag. The following types are defined:
//                                                8 = audio
//                                                9 = video
//                                                18 = script data
//
//  DataSize                UI24               Length of the message. Number of bytes after StreamID to end of tag (Equal to length of the tag – 11)

//  Timestamp               UI24               Time in milliseconds at which the data in this tag applies. This value is relative to the first tag in the FLV file,
//                                             which always has a timestamp of 0.
//
//  TimestampExtended       UI8                Extension of the Timestamp field to form a SI32 value. This
//                                             field represents the upper 8 bits, while the previous
//                                             Timestamp field represents the lower 24 bits of the time in milliseconds.
//
//  StreamID                UI24               Always 0
//  ------------------------------------------------------------------------

type FlvTag struct {
    TagType           uint8
    DataSize          uint32
    Timestamp         uint32
    TimestampExtended uint8
    StreamID          uint32
}

func (ftag FlvTag) Encode() []byte {
    tag := make([]byte, 11)
    tag[0] = ftag.TagType
    PutUint24(tag[1:], ftag.DataSize)
    PutUint24(tag[4:], ftag.Timestamp)
    tag[7] = ftag.TimestampExtended
    PutUint24(tag[8:], ftag.StreamID)
    return tag
}

func (ftag *FlvTag) Decode(data []byte) {
    ftag.TagType = data[0] & 0x1F
    ftag.DataSize = GetUint24(data[1:])
    ftag.Timestamp = GetUint24(data[4:])
    ftag.TimestampExtended = data[7]
    ftag.StreamID = GetUint24(data[8:])
}

//  Video Tag
//  VideoTagHeader
//  ------------------------------------------------------------------------
//  Field                   type                     Comment
//  ------------------------------------------------------------------------
//  Frame Type              UB[4]              Type of video frame. The following values are defined:
//                                             1 = key frame (for AVC, a seekable frame)
//                                             2 = inter frame (for AVC, a non-seekable frame)
//                                             3 = disposable inter frame (H.263 only)
//                                             4 = generated key frame (reserved for server use only)
//                                             5 = video info/command frame
//
//  CodecID                 UB[4]              Codec Identifier. The following values are defined:
//                                             2 = Sorenson H.263
//                                             3 = Screen video
//                                             4 = On2 VP6
//                                             5 = On2 VP6 with alpha channel 6 = Screen video version 2
//                                             7 = AVC
//
//  AVCPacketType          IF CodecID == 7     The following values are defined:
//                              UI8               0 = AVC sequence header
//                                                1 = AVC NALU
//                                                2 = AVC end of sequence (lower level NALU sequence ender is not required or supported)
//
//  CompositionTime        IF CodecID == 7     IF AVCPacketType == 1
//                              SI24                    Composition time offset
//                                               ELSE
//                                                    0
//  ------------------------------------------------------------------------
type VideoTag struct {
    FrameType       uint8
    CodecId         uint8
    AVCPacketType   uint8
    CompositionTime int32
}

func (vtag VideoTag) Encode() (tag []byte) {
    if vtag.CodecId == uint8(FLV_AVC) || vtag.CodecId == uint8(FLV_HEVC) {
        tag = make([]byte, 5)
        tag[1] = vtag.AVCPacketType
        PutUint24(tag[2:], uint32(vtag.CompositionTime))
    } else {
        tag = make([]byte, 1)
    }
    tag[0] = (vtag.FrameType << 4) | (vtag.CodecId & 0x0F)
    return
}

// 外部已经确保len(data) >= 5
func (vtag *VideoTag) Decode(data []byte) {
    isExHeader := data[0] & 0x80
    if isExHeader != 0 {
        // enhanced flv
        vtag.FrameType = (data[0] >> 4) & 0x07
        vtag.AVCPacketType = data[0] & 0x0F

        // TODO av1和VP9
        if data[1] == 'h' && data[2] == 'v' && data[3] == 'c' && data[4] == '1' {
            // hevc
            vtag.CodecId = uint8(FLV_HEVC)

            if vtag.AVCPacketType == PacketTypeCodedFrames {
                vtag.CompositionTime = int32(GetUint24(data[5:]))
            }
        }
    } else {
        vtag.FrameType = data[0] >> 4
        vtag.CodecId = data[0] & 0x0F
        if vtag.CodecId == uint8(FLV_AVC) || vtag.CodecId == uint8(FLV_HEVC) {
            vtag.AVCPacketType = data[1]
            vtag.CompositionTime = int32(GetUint24(data[2:]))
        }
    }
}

//  Audio Tag
//  AudioTagHeader
//  ------------------------------------------------------------------------
//  Field                   type                        Comment
//  ------------------------------------------------------------------------
//  SoundFormat             UB[4]                       Format of SoundData. The following values are defined: 0 = Linear PCM, platform endian
//                                                         1 = ADPCM
//                                                         2 = MP3
//                                                         3 = Linear PCM, little endian 4 = Nellymoser 16 kHz mono 5 = Nellymoser 8 kHz mono 6 = Nellymoser
//                                                         7 = G.711 A-law logarithmic PCM
//                                                         8 = G.711 mu-law logarithmic PCM
//                                                         9 = reserved
//                                                         10 = AAC
//                                                         11 = Speex
//                                                         14 = MP3 8 kHz
//                                                         15 = Device-specific sound
//                                                         Formats 7, 8, 14, and 15 are reserved.
//                                                         AAC is supported in Flash Player 9,0,115,0 and higher. Speex is supported in Flash Player 10 and higher.
//
//
//  SoundRate               UB[2]                       Sampling rate. The following values are defined:
//                                                         0 = 5.5 kHz
//                                                         1 = 11 kHz
//                                                         2 = 22 kHz
//                                                         3 = 44 kHz
//
//  SoundSize               UB[1]                       Size of each audio sample. This parameter only pertains to uncompressed formats. Compressed formats always decode to 16 bits internally.
//                                                          0 = 8-bit samples
//                                                          1 = 16-bit samples
//
//  SoundType               UB[1]                       Mono or stereo sound
//                                                        0 = Mono sound
//                                                        1 = Stereo sound
//
//  AACPacketType          IF SoundFormat == 10         The following values are defined:
//                             UI8                        0 = AAC sequence header
//                                                        1 = AAC raw
//
//  ------------------------------------------------------------------------

type AudioTag struct {
    SoundFormat   uint8
    SoundRate     uint8
    SoundSize     uint8
    SoundType     uint8
    AACPacketType uint8
}

func (atag AudioTag) Encode() (tag []byte) {
    if atag.SoundFormat == 10 {
        tag = make([]byte, 2)
        tag[1] = atag.AACPacketType
    } else {
        tag = make([]byte, 1)
    }
    tag[0] = atag.SoundFormat<<4 | atag.SoundRate<<2 | atag.SoundSize<<1 | atag.SoundType
    return
}

func (atag *AudioTag) Decode(data []byte) error {
    if len(data) < 1 {
        return errors.New("audio tag header size < 1 ")
    }
    atag.SoundFormat = data[0] >> 4
    atag.SoundRate = (data[0] >> 2) & 0x03
    atag.SoundSize = (data[0] >> 1) & 0x01
    atag.SoundType = data[0] & 0x01
    if atag.SoundFormat == 10 {
        if len(data) < 2 {
            return errors.New("aac audio tag header size < 2")
        }
        atag.AACPacketType = data[1]
    }
    return nil
}
