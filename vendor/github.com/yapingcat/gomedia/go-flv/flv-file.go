package flv

import (
    "encoding/binary"
    "errors"
    "io"

    "github.com/yapingcat/gomedia/go-codec"
)

//  FLV File
//  TheFLVheader
//  An FLV file shall begin with the FLV header:
//  -------------------------------------------------------------------------------------------------------------
//  FLV header
//  Field                       Type                            Comment
//  -------------------------------------------------------------------------------------------------------------
//  Signature                   UI8                            Signature byte always 'F' (0x46)
//
//  Signature                   UI8                            Signature byte always 'L' (0x4C)
//
//  Signature                   UI8                         Signature byte always 'V' (0x56)
//
//  Version                     UI8                         File version (for example, 0x01 for FLV version 1)
//
//  TypeFlagsReserved           UB[5]                       Shall be 0
//
//  TypeFlagsAudio              UB[1]                       1 = Audio tags are present
//
//  TypeFlagsReserved           UB[1]                       Shall be 0
//
//  TypeFlagsVideo              UB[1]                       1 = Video tags are present
//
//  DataOffset                  UI32                        The length of this header in bytes
//  -------------------------------------------------------------------------------------------------------------
//
//  TheFLVFileBody
//  After the FLV header, the remainder of an FLV file shall consist of alternating back-pointers and tags.
//  They interleave as shown in the following table:
//  -------------------------------------------------------------------------------------------------------------
//  FLV File Body
//  Field                       Type                        Comment
//  -------------------------------------------------------------------------------------------------------------
//  PreviousTagSize0            UI32                        Always 0
//  Tag1                        FLVTAG                      First tag
//  PreviousTagSize1            UI32                        Size of previous tag, including its header, in bytes. For FLV version 1,
//                                                          this value is 11 plus the DataSize of the previous tag
//  Tag2                        FLVTAG                      Second tag
//
//  ....
//
//  PreviousTagSizeN-1          UI32                        Size of second-to-last tag, including its header, in bytes.
//  ---------------------------------------------------------------------------------------------------------------

type FLV_PARSER_STATE int

const (
    FLV_PARSER_INIT FLV_PARSER_STATE = iota + 1
    FLV_PARSER_FILE_HEAD
    FLV_PARSER_TAG_SIZE
    FLV_PARSER_FLV_TAG
    FLV_PARSER_DETECT_VIDEO
    FLV_PARSER_DETECT_AUDIO
    FLV_PARSER_VIDEO_TAG
    FLV_PARSER_AUDIO_TAG
    FLV_PARSER_SCRIPT_TAG
)

type FlvReader struct {
    cache        []byte
    state        FLV_PARSER_STATE
    videoDemuxer VideoTagDemuxer
    audioDemuxer AudioTagDemuxer
    flvTag       FlvTag
    OnFrame      func(cid codec.CodecID, frame []byte, pts uint32, dts uint32)
}

func CreateFlvReader() *FlvReader {
    flvFile := &FlvReader{
        OnFrame:      nil,
        state:        FLV_PARSER_INIT,
        videoDemuxer: nil,
        audioDemuxer: nil,
        cache:        make([]byte, 0, 4096),
    }
    return flvFile
}

func (f *FlvReader) Input(data []byte) (err error) {
    var buf []byte
    if len(f.cache) > 0 {
        f.cache = append(f.cache, data...)
        buf = f.cache
    } else {
        buf = data
    }

    for len(buf) > 0 {
        switch f.state {
        case FLV_PARSER_INIT:
            f.state = FLV_PARSER_FILE_HEAD
        case FLV_PARSER_FILE_HEAD:
            if len(buf) < 9 {
                goto end
            }
            if err = f.readFlvHeader(buf[:9]); err != nil {
                goto end
            }
            buf = buf[9:]
            f.state = FLV_PARSER_TAG_SIZE
        case FLV_PARSER_TAG_SIZE:
            if len(buf) < 4 {
                goto end
            }
            buf = buf[4:]
            f.state = FLV_PARSER_FLV_TAG
        case FLV_PARSER_FLV_TAG:
            if len(buf) < 11 {
                goto end
            }
            f.flvTag.Decode(buf)
            buf = buf[11:]
            if f.flvTag.TagType == uint8(VIDEO_TAG) {
                if f.videoDemuxer == nil {
                    f.state = FLV_PARSER_DETECT_VIDEO
                } else {
                    f.state = FLV_PARSER_VIDEO_TAG
                }
            } else if f.flvTag.TagType == uint8(AUDIO_TAG) {
                if f.audioDemuxer == nil {
                    f.state = FLV_PARSER_DETECT_AUDIO
                } else {
                    f.state = FLV_PARSER_AUDIO_TAG
                }
            } else {
                //TODO MateData tag
                f.state = FLV_PARSER_SCRIPT_TAG
            }
        case FLV_PARSER_DETECT_VIDEO:
            if err = f.createVideoTagDemuxer(FLV_VIDEO_CODEC_ID(buf[0] & 0x0F)); err != nil {
                goto end
            }
            f.state = FLV_PARSER_VIDEO_TAG
        case FLV_PARSER_DETECT_AUDIO:
            if err = f.createAudioTagDemuxer(FLV_SOUND_FORMAT((buf[0] >> 4) & 0x0F)); err != nil {
                goto end
            }
            f.state = FLV_PARSER_AUDIO_TAG
        case FLV_PARSER_VIDEO_TAG:
            if f.flvTag.DataSize > uint32(len(buf)) {
                goto end
            }
            err = f.videoDemuxer.Decode(buf[:f.flvTag.DataSize])
            if err != nil {
                return err
            }
            buf = buf[f.flvTag.DataSize:]
            f.state = FLV_PARSER_TAG_SIZE
        case FLV_PARSER_AUDIO_TAG:
            if f.flvTag.DataSize > uint32(len(buf)) {
                goto end
            }
            err = f.audioDemuxer.Decode(buf[:f.flvTag.DataSize])
            if err != nil {
                return err
            }
            buf = buf[f.flvTag.DataSize:]
            f.state = FLV_PARSER_TAG_SIZE
        case FLV_PARSER_SCRIPT_TAG:
            if f.flvTag.DataSize > uint32(len(buf)) {
                goto end
            }
            buf = buf[f.flvTag.DataSize:]
            f.state = FLV_PARSER_TAG_SIZE
        default:
            panic("unkown state")
        }
    }

end:

    if err != nil {
        return
    }

    if len(f.cache) > 0 {
        if len(buf) > 0 {
            f.cache = buf
        } else {
            f.cache = f.cache[:0]
        }
    } else {
        if len(buf) > 0 {
            f.cache = append(f.cache, buf...)
        }
    }

    return nil
}

func (f *FlvReader) createVideoTagDemuxer(cid FLV_VIDEO_CODEC_ID) error {
    switch cid {
    case FLV_AVC:
        f.videoDemuxer = NewAVCTagDemuxer()
    case FLV_HEVC:
        f.videoDemuxer = NewHevcTagDemuxer()
    default:
        return errors.New("unsupport video codec id")
    }
    f.videoDemuxer.OnFrame(func(codecid codec.CodecID, frame []byte, cts int) {
        dts := uint32(f.flvTag.TimestampExtended)<<24 | f.flvTag.Timestamp
        pts := dts + uint32(cts)
        f.OnFrame(codecid, frame, pts, dts)
    })
    return nil
}

func (f *FlvReader) createAudioTagDemuxer(formats FLV_SOUND_FORMAT) error {
    switch formats {
    case FLV_G711A, FLV_G711U, FLV_MP3:
        f.audioDemuxer = NewG711Demuxer(formats)
    case FLV_AAC:
        f.audioDemuxer = NewAACTagDemuxer()
    default:
        return errors.New("unsupport audio codec id")
    }
    f.audioDemuxer.OnFrame(func(codecid codec.CodecID, frame []byte) {
        dts := uint32(f.flvTag.TimestampExtended)<<24 | f.flvTag.Timestamp
        pts := dts
        f.OnFrame(codecid, frame, pts, dts)
    })
    return nil
}

func (f *FlvReader) readFlvHeader(hdr []byte) error {
    if hdr[0] != 'F' || hdr[1] != 'L' || hdr[2] != 'V' {
        return errors.New("this file Is Not FLV File")
    }
    return nil
}

type FlvWriter struct {
    writer io.Writer
    muxer  *FlvMuxer
}

func CreateFlvWriter(writer io.Writer) *FlvWriter {
    flvFile := &FlvWriter{
        writer: writer,
        muxer:  new(FlvMuxer),
    }
    return flvFile
}

func (f *FlvWriter) WriteFlvHeader() (err error) {

    var flvhdr [9]byte
    flvhdr[0] = 'F'
    flvhdr[1] = 'L'
    flvhdr[2] = 'V'
    flvhdr[3] = 0x01
    flvhdr[4] = 0x05
    flvhdr[5] = 0
    flvhdr[6] = 0
    flvhdr[7] = 0
    flvhdr[8] = 9

    if _, err = f.writer.Write(flvhdr[:9]); err != nil {
        return
    }
    var previousTagSize0 [4]byte
    previousTagSize0[0] = 0
    previousTagSize0[1] = 0
    previousTagSize0[2] = 0
    previousTagSize0[3] = 0
    if _, err = f.writer.Write(previousTagSize0[:4]); err != nil {
        return
    }
    return
}

//adts aac frame
func (f *FlvWriter) WriteAAC(data []byte, pts uint32, dts uint32) error {
    if f.muxer.audioMuxer == nil {
        f.muxer.SetAudioCodeId(FLV_AAC)
    } else {
        if _, ok := f.muxer.audioMuxer.(*AACMuxer); !ok {
            panic("audio codec change")
        }
    }
    return f.writeAudio(data, pts, dts)
}

func (f *FlvWriter) WriteG711A(data []byte, pts uint32, dts uint32) error {
    if f.muxer.audioMuxer == nil {
        f.muxer.SetAudioCodeId(FLV_G711A)
    } else {
        if _, ok := f.muxer.audioMuxer.(*G711AMuxer); !ok {
            panic("audio codec change")
        }
    }
    return f.writeAudio(data, pts, dts)
}

func (f *FlvWriter) WriteG711U(data []byte, pts uint32, dts uint32) error {
    if f.muxer.audioMuxer == nil {
        f.muxer.SetAudioCodeId(FLV_G711U)
    } else {
        if _, ok := f.muxer.audioMuxer.(*G711UMuxer); !ok {
            panic("audio codec change")
        }
    }
    return f.writeAudio(data, pts, dts)
}

func (f *FlvWriter) WriteMp3(data []byte, pts uint32, dts uint32) error {
    if f.muxer.audioMuxer == nil {
        f.muxer.SetAudioCodeId(FLV_MP3)
    } else {
        if _, ok := f.muxer.audioMuxer.(*Mp3Muxer); !ok {
            panic("audio codec change")
        }
    }
    return f.writeAudio(data, pts, dts)
}

func (f *FlvWriter) writeAudio(data []byte, pts uint32, dts uint32) error {

    if tags, err := f.muxer.WriteAudio(data, pts, dts); err != nil {
        return err
    } else {
        for _, tag := range tags {
            if _, err := f.writer.Write(tag); err != nil {
                return err
            }
            if err := f.writePreviousTagSize(uint32(len(tag))); err != nil {
                return err
            }
        }
    }
    return nil
}

//H264 Frame with startcode 0x0000001
func (f *FlvWriter) WriteH264(data []byte, pts uint32, dts uint32) error {
    if f.muxer.videoMuxer == nil {
        f.muxer.SetVideoCodeId(FLV_AVC)
    } else {
        if _, ok := f.muxer.videoMuxer.(*AVCMuxer); !ok {
            panic("video codec change")
        }
    }

    return f.writeVideo(data, pts, dts)
}

func (f *FlvWriter) WriteH265(data []byte, pts uint32, dts uint32) error {
    if f.muxer.videoMuxer == nil {
        f.muxer.SetVideoCodeId(FLV_HEVC)
    } else {
        if _, ok := f.muxer.videoMuxer.(*HevcMuxer); !ok {
            panic("video codec change")
        }
    }
    return f.writeVideo(data, pts, dts)
}

func (f *FlvWriter) writeVideo(data []byte, pts uint32, dts uint32) error {
    if tags, err := f.muxer.WriteVideo(data, pts, dts); err != nil {
        return err
    } else {
        for _, tag := range tags {
            if _, err := f.writer.Write(tag); err != nil {
                return err
            }
            if err := f.writePreviousTagSize(uint32(len(tag))); err != nil {
                return err
            }
        }
    }
    return nil
}

func (f *FlvWriter) writePreviousTagSize(preTagSize uint32) error {
    tagsize := make([]byte, 4)
    binary.BigEndian.PutUint32(tagsize, preTagSize)
    if _, err := f.writer.Write(tagsize); err != nil {
        return err
    }
    return nil
}
