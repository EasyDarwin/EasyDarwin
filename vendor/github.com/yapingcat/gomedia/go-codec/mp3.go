package codec

import (
    "bytes"
    "errors"
    "fmt"
)

//mp3 file format
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |          IDV3V2               |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |        audio frame            |
// +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
// |          IDV3V1               |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

//IDV3V2 head

// char Header[3];  /*必须为"ID3"否则认为标签不存在*/
// char Ver;        /*版本号 ID3V2.3 就记录 3*/
// char Revision;  /*副版本号此版本记录为 0*/
// char Flag;       /*存放标志的字节，这个版本只定义了三位，稍后详细解说*/
// char Size[4];   /*标签大小，包括标签头的 10 个字节和所有的标签帧的大小*/

//标签大小
// 一共四个字节，但每个字节只用 7 位，最高位不使用恒为 0。所以格式如下
// 0xxxxxxx 0xxxxxxx 0xxxxxxx 0xxxxxxx
// 计算大小时要将 0 去掉，得到一个 28 位的二进制数，就是标签大小,计算公式如下：
// int total_size;
// total_size = (Size[0]&0x7F)*0x200000 + (Size[1]&0x7F)*0x400 + (Size[2]&0x7F)*0x80 + +(Size[3]&0x7F)

//ID3V1

//ID3V1存放在 MP3 文件的末尾，128个字节
// typedef struct tagID3V1
// {
//     char Header[3]; /*标签头必须是"TAG"否则认为没有标签*/
//     char Title[30]; /*标题*/
//     char Artist[30]; /*作者*/
//     char Album[30]; /*专集*/
//     char Year[4]; /*出品年代*/
//     char Comment[28]; /*备注*/
//     char reserve; /*保留*/
//     char track;; /*音轨*/
//     char Genre; /*类型*/
// }ID3V1,*pID3V1;

// mp3 frame
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |     FRAMEHEADER(4 byte)   |    CRC(free)(0 or 2 byte)  |  MAIN_DATA 长度由帧头计算 |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

//
//  Frame head
//  ----------------------------------------------------------------------------------------------------
//  Field                length(bits)  Position(bits)      Description
//  ----------------------------------------------------------------------------------------------------
//  Syncwork                11            (31-21)          Frame sync (all bits set)
//
//  Version                  2            (20-19)          MPEG Audio version
//                                                         00 - MPEG Version 2.5
//                                                         01 - reserved
//                                                         10 - MPEG Version 2
//                                                         11 - MPEG Version 1
//
//
//  Layer                    2             (18-17)         Layer description
//                                                         00 - reserved
//                                                         01 - Layer III
//                                                         10 - Layer II
//                                                         11 - Layer I
//
//  Protection               1              16             Protection bit
//                                                         0 - Protected by CRC (16bit crc follows header)
//                                                         1 - Not protected
//
//  Bitrate index            4              15-12          bits V1,L1 V1,L2 V1,L3 V2,L1 V2,L2 V2,L3
//                                                         0000 free  free  free  free  free  free
//                                                         0001 32    32    32    32    32    8(8)
//                                                         0010 64    48    40    64    48    16(16)
//                                                         0011 96    56    48    96    56    24(24)
//                                                         0100 128   64    56   128    64    32(32)
//                                                         0101 160   80    64   160    80    64(40)
//                                                         0110 192   96    80   192    96    80(48)
//                                                         0111 224   112   96   224   112    56(56)
//                                                         1000 256   128  112   256   128    64(64)
//                                                         1001 288   160  128   288   160    128(80)
//                                                         1010 320   192  160   320   192    160(96)
//                                                         1011 352   224  192   352   224    112(112)
//                                                         1100 384   256  224   384   256    128(128)
//                                                         1101 416   320  256   416   320    256(144)
//                                                         1110 448   384  320   448   384    320(160)
//                                                         1111 bad   bad  bad   bad   bad    bad

//                                                         NOTES: All values are in kbps
//                                                         V1 - MPEG Version 1
//                                                         V2 - MPEG Version 2 and Version 2.5
//                                                         L1 - Layer I
//                                                         L2 - Layer II
//                                                         L3 - Layer III
//                                                         "free" means variable bitrate.
//                                                         "bad" means that this is not an allowed value
//
//  SampleRate Index         2             11-10           Sampling rate frequency index (values are in Hz)
//                                                         bits MPEG1   MPEG2   MPEG2.5
//                                                         00   44100   22050   11025
//                                                         01   48000   24000   12000
//                                                         10   32000   16000   8000
//                                                         11   reserv. reserv. reserv.
//  Padding bit              1               9             Padding bit
//                                                         0 - frame is not padded
//                                                         1 - frame is padded with one extra bit

//  Private bit              1               8             Private bit (unknown purpose)

//  mode                     2               7-6           Channel Mode
//                                                         00 - Stereo
//                                                         01 - Joint stereo (Stereo)
//                                                         10 - Dual channel (Stereo)
//                                                         11 - Single channel (Mono)
//
//
//  Mode extension           2               5-4           value | Intensity stereo |MS stereo
//                                                         00       off              off
//                                                         01       on               off
//                                                         10      off                on
//                                                         11       on               This
//
//  Copyright                1                3            Copyright
//                                                         0 - Audio is not copyrighted
//                                                         1 - Audio is copyrighted
//
//  Original                 1                2            0 - Copy of original media
//                                                         1 - Original media
//
//  Emphasis                 2               1-0           00 - none
//                                                         01 - 50/15 ms
//                                                         10 - reserved
//                                                         11 - CCIT J.17
//  ------------------------------------------------------------------------------------------------

//ffmpeg mpegaudiotabs.h ff_mpa_bitrate_tab
var BitRateTable [2][3][16]int = [2][3][16]int{
    {
        {0, 32, 64, 96, 128, 160, 192, 224, 256, 288, 320, 352, 384, 416, 448, -1},
        {0, 32, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 380, -1},
        {0, 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, -1},
    },
    {
        {0, 32, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 384, -1},
        {0, 8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160, -1},
        {0, 8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160, -1},
    },
}

var SampleRateTable [3][4]int = [3][4]int{
    {44100, 48000, 32000, 0},
    {22050, 24000, 16000, 0},
    {11025, 12000, 8000, 0},
}

const (
    VERSION_RESERVED = 0
    VERSION_MPEG_1   = 1
    VERSION_MPEG_2   = 2
    VERSION_MPEG_2_5 = 3
)

const (
    LAYER_RESERVED = 0
    LAYER_1        = 1
    LAYER_2        = 2
    LAYER_3        = 3
)

type ID3V2 struct {
    Ver      uint8
    Revision uint8
    Flag     uint8
    Size     uint32
}

type MP3FrameHead struct {
    Version         uint8
    Layer           uint8
    Protecttion     uint8
    BitrateIndex    uint8
    SampleRateIndex uint8
    Padding         uint8
    Private         uint8
    Mode            uint8
    ModeExtension   uint8
    Copyright       uint8
    Original        uint8
    Emphasis        uint8
    SampleSize      int
    FrameSize       int
}

func DecodeMp3Head(data []byte) (*MP3FrameHead, error) {
    if len(data) == 0 {
        return nil, errors.New("empty mp3 frame")
    }
    bs := NewBitStream(data)
    syncWord := bs.GetBits(11)
    if syncWord != 0x7FF {
        return nil, errors.New("mp3 frame must start with 0xFFE")
    }

    head := &MP3FrameHead{}
    head.Version = uint8(bs.GetBits(2))
    switch head.Version {
    case 0x00:
        head.Version = VERSION_MPEG_2_5
    case 0x01:
        head.Version = VERSION_RESERVED
    case 0x02:
        head.Version = VERSION_MPEG_2
    case 0x03:
        head.Version = VERSION_MPEG_1
    }

    head.Layer = uint8(bs.GetBits(2))

    switch head.Layer {
    case 0x00:
        head.Layer = LAYER_RESERVED
    case 0x01:
        head.Layer = LAYER_3
    case 0x02:
        head.Layer = LAYER_2
    case 0x03:
        head.Layer = LAYER_1
    }

    head.Protecttion = bs.GetBit()
    head.BitrateIndex = uint8(bs.GetBits(4))
    head.SampleRateIndex = uint8(bs.GetBits(2))
    head.Padding = bs.GetBit()
    head.Private = bs.GetBit()
    head.Mode = uint8(bs.GetBits(2))
    head.ModeExtension = uint8(bs.GetBits(2))
    head.Copyright = bs.GetBit()
    head.Original = bs.GetBit()
    head.Emphasis = uint8(bs.GetBits(2))

    if head.Layer == LAYER_1 {
        head.SampleSize = 384
    } else if head.Layer == LAYER_2 {
        head.SampleSize = 1152
    } else {
        if head.Version == VERSION_MPEG_1 {
            head.SampleSize = 1152
        } else {
            head.SampleSize = 576
        }
    }

    br := head.GetBitRate()
    head.FrameSize = head.SampleSize / 8 * br / head.GetSampleRate()
    //layer 1 has 4 bytes padding,other has one byte
    if head.Layer == LAYER_1 {
        head.FrameSize += int(head.Padding) * 4
    } else {
        head.FrameSize += int(head.Padding)
    }
    return head, nil
}

func (mp3 *MP3FrameHead) GetChannelCount() int {
    if mp3.Mode == 0x11 {
        return 1
    } else {
        return 2
    }
}

func (mp3 *MP3FrameHead) GetBitRate() int {
    var i int = 0
    if mp3.Version == VERSION_MPEG_2 || mp3.Version == VERSION_MPEG_2_5 {
        i = 1
    }
    return BitRateTable[i][mp3.Layer-1][mp3.BitrateIndex] * 1000
}

func (mp3 *MP3FrameHead) GetSampleRate() int {
    if mp3.Version == LAYER_RESERVED {
        return 0
    }
    return SampleRateTable[mp3.Version-1][mp3.SampleRateIndex]
}

func SplitMp3Frames(data []byte, onFrame func(head *MP3FrameHead, frame []byte)) error {
    for len(data) > 0 {
        if bytes.HasPrefix(data, []byte{'I', 'D', '3'}) {
            if len(data) < 10 {
                return errors.New("ID3V2 tag head must has 10 bytes")
            }
            fmt.Println("Get ID3 tag")
            var size uint32 = uint32(data[7])
            size = size<<7 | uint32(data[8])
            size = size<<7 | uint32(data[9])
            data = data[10+size:]
            fmt.Println("tag size ", size)
        } else if bytes.HasPrefix(data, []byte{'T', 'A', 'G'}) {
            if len(data) < 128 {
                return errors.New("ID3V1 must has 128 bytes")
            }
            data = data[128:]
        } else {
            head, err := DecodeMp3Head(data)
            if err != nil {
                fmt.Println(err)
                return err
            }
            if onFrame != nil {
                onFrame(head, data[:head.FrameSize])
            }
            data = data[head.FrameSize:]
        }
    }
    return nil
}
