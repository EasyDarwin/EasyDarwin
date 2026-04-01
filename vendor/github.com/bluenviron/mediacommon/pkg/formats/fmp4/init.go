package fmp4

import (
	"fmt"
	"io"

	"github.com/abema/go-mp4"

	"github.com/bluenviron/mediacommon/pkg/codecs/av1"
	"github.com/bluenviron/mediacommon/pkg/codecs/h265"
	"github.com/bluenviron/mediacommon/pkg/codecs/mpeg4audio"
)

// Specification: ISO 14496-1, Table 5
const (
	objectTypeIndicationVisualISO14496part2    = 0x20
	objectTypeIndicationAudioISO14496part3     = 0x40
	objectTypeIndicationVisualISO1318part2Main = 0x61
	objectTypeIndicationAudioISO11172part3     = 0x6B
	objectTypeIndicationVisualISO10918part1    = 0x6C
)

// Specification: ISO 14496-1, Table 6
const (
	streamTypeVisualStream = 0x04
	streamTypeAudioStream  = 0x05
)

func av1FindSequenceHeader(bs []byte) ([]byte, error) {
	tu, err := av1.BitstreamUnmarshal(bs, true)
	if err != nil {
		return nil, err
	}

	for _, obu := range tu {
		var h av1.OBUHeader
		err := h.Unmarshal(obu)
		if err != nil {
			return nil, err
		}

		if h.Type == av1.OBUTypeSequenceHeader {
			return obu, nil
		}
	}

	return nil, fmt.Errorf("sequence header not found")
}

func h265FindParams(params []mp4.HEVCNaluArray) ([]byte, []byte, []byte, error) {
	var vps []byte
	var sps []byte
	var pps []byte

	for _, arr := range params {
		switch h265.NALUType(arr.NaluType) {
		case h265.NALUType_VPS_NUT, h265.NALUType_SPS_NUT, h265.NALUType_PPS_NUT:
			if arr.NumNalus != 1 {
				return nil, nil, nil, fmt.Errorf("multiple VPS/SPS/PPS are not supported")
			}
		}

		switch h265.NALUType(arr.NaluType) {
		case h265.NALUType_VPS_NUT:
			vps = arr.Nalus[0].NALUnit

		case h265.NALUType_SPS_NUT:
			sps = arr.Nalus[0].NALUnit

		case h265.NALUType_PPS_NUT:
			pps = arr.Nalus[0].NALUnit
		}
	}

	if vps == nil {
		return nil, nil, nil, fmt.Errorf("VPS not provided")
	}

	if sps == nil {
		return nil, nil, nil, fmt.Errorf("SPS not provided")
	}

	if pps == nil {
		return nil, nil, nil, fmt.Errorf("PPS not provided")
	}

	return vps, sps, pps, nil
}

func h264FindParams(avcc *mp4.AVCDecoderConfiguration) ([]byte, []byte, error) {
	if len(avcc.SequenceParameterSets) > 1 {
		return nil, nil, fmt.Errorf("multiple SPS are not supported")
	}

	var sps []byte
	if len(avcc.SequenceParameterSets) == 1 {
		sps = avcc.SequenceParameterSets[0].NALUnit
	}

	if len(avcc.PictureParameterSets) > 1 {
		return nil, nil, fmt.Errorf("multiple PPS are not supported")
	}

	var pps []byte
	if len(avcc.PictureParameterSets) == 1 {
		pps = avcc.PictureParameterSets[0].NALUnit
	}

	return sps, pps, nil
}

func esdsFindDecoderConf(descriptors []mp4.Descriptor) *mp4.DecoderConfigDescriptor {
	for _, desc := range descriptors {
		if desc.Tag == mp4.DecoderConfigDescrTag {
			return desc.DecoderConfigDescriptor
		}
	}
	return nil
}

func esdsFindDecoderSpecificInfo(descriptors []mp4.Descriptor) []byte {
	for _, desc := range descriptors {
		if desc.Tag == mp4.DecSpecificInfoTag {
			return desc.Data
		}
	}
	return nil
}

// Init is a fMP4 initialization block.
type Init struct {
	Tracks []*InitTrack
}

// Unmarshal decodes a fMP4 initialization block.
func (i *Init) Unmarshal(r io.ReadSeeker) error {
	type readState int

	const (
		waitingTrak readState = iota
		waitingTkhd
		waitingMdhd
		waitingCodec
		waitingAv1C
		waitingVpcC
		waitingHvcC
		waitingAvcC
		waitingVideoEsds
		waitingAudioEsds
		waitingDOps
		waitingDac3
		waitingPcmC
	)

	state := waitingTrak
	var curTrack *InitTrack
	var width int
	var height int
	var sampleRate int
	var channelCount int

	_, err := mp4.ReadBoxStructure(r, func(h *mp4.ReadHandle) (interface{}, error) {
		if !h.BoxInfo.IsSupportedType() {
			if state != waitingTrak {
				i.Tracks = i.Tracks[:len(i.Tracks)-1]
				state = waitingTrak
			}
		} else {
			switch h.BoxInfo.Type.String() {
			case "moov":
				return h.Expand()

			case "trak":
				if state != waitingTrak {
					return nil, fmt.Errorf("unexpected box '%v'", h.BoxInfo.Type)
				}

				curTrack = &InitTrack{}
				i.Tracks = append(i.Tracks, curTrack)
				state = waitingTkhd
				return h.Expand()

			case "tkhd":
				if state != waitingTkhd {
					return nil, fmt.Errorf("unexpected box '%v'", h.BoxInfo.Type)
				}

				box, _, err := h.ReadPayload()
				if err != nil {
					return nil, err
				}
				tkhd := box.(*mp4.Tkhd)

				curTrack.ID = int(tkhd.TrackID)
				state = waitingMdhd

			case "mdia":
				return h.Expand()

			case "mdhd":
				if state != waitingMdhd {
					return nil, fmt.Errorf("unexpected box '%v'", h.BoxInfo.Type)
				}

				box, _, err := h.ReadPayload()
				if err != nil {
					return nil, err
				}
				mdhd := box.(*mp4.Mdhd)

				curTrack.TimeScale = mdhd.Timescale
				state = waitingCodec

			case "minf", "stbl", "stsd":
				return h.Expand()

			case "avc1":
				if state != waitingCodec {
					return nil, fmt.Errorf("unexpected box '%v'", h.BoxInfo.Type)
				}
				state = waitingAvcC
				return h.Expand()

			case "avcC":
				if state != waitingAvcC {
					return nil, fmt.Errorf("unexpected box '%v'", h.BoxInfo.Type)
				}

				box, _, err := h.ReadPayload()
				if err != nil {
					return nil, err
				}
				avcc := box.(*mp4.AVCDecoderConfiguration)

				sps, pps, err := h264FindParams(avcc)
				if err != nil {
					return nil, err
				}

				curTrack.Codec = &CodecH264{
					SPS: sps,
					PPS: pps,
				}
				state = waitingTrak

			case "vp09":
				if state != waitingCodec {
					return nil, fmt.Errorf("unexpected box '%v'", h.BoxInfo.Type)
				}

				box, _, err := h.ReadPayload()
				if err != nil {
					return nil, err
				}
				vp09 := box.(*mp4.VisualSampleEntry)

				width = int(vp09.Width)
				height = int(vp09.Height)
				state = waitingVpcC
				return h.Expand()

			case "vpcC":
				if state != waitingVpcC {
					return nil, fmt.Errorf("unexpected box '%v'", h.BoxInfo.Type)
				}

				box, _, err := h.ReadPayload()
				if err != nil {
					return nil, err
				}
				vpcc := box.(*mp4.VpcC)

				curTrack.Codec = &CodecVP9{
					Width:             width,
					Height:            height,
					Profile:           vpcc.Profile,
					BitDepth:          vpcc.BitDepth,
					ChromaSubsampling: vpcc.ChromaSubsampling,
					ColorRange:        vpcc.VideoFullRangeFlag != 0,
				}
				state = waitingTrak

			case "vp08": // VP8, not supported yet
				return nil, nil

			case "hev1", "hvc1":
				if state != waitingCodec {
					return nil, fmt.Errorf("unexpected box '%v'", h.BoxInfo.Type)
				}
				state = waitingHvcC
				return h.Expand()

			case "hvcC":
				if state != waitingHvcC {
					return nil, fmt.Errorf("unexpected box '%v'", h.BoxInfo.Type)
				}

				box, _, err := h.ReadPayload()
				if err != nil {
					return nil, err
				}
				hvcc := box.(*mp4.HvcC)

				vps, sps, pps, err := h265FindParams(hvcc.NaluArrays)
				if err != nil {
					return nil, err
				}

				curTrack.Codec = &CodecH265{
					VPS: vps,
					SPS: sps,
					PPS: pps,
				}
				state = waitingTrak

			case "av01":
				if state != waitingCodec {
					return nil, fmt.Errorf("unexpected box '%v'", h.BoxInfo.Type)
				}
				state = waitingAv1C
				return h.Expand()

			case "av1C":
				if state != waitingAv1C {
					return nil, fmt.Errorf("unexpected box '%v'", h.BoxInfo.Type)
				}

				box, _, err := h.ReadPayload()
				if err != nil {
					return nil, err
				}
				av1c := box.(*mp4.Av1C)

				sequenceHeader, err := av1FindSequenceHeader(av1c.ConfigOBUs)
				if err != nil {
					return nil, err
				}

				curTrack.Codec = &CodecAV1{
					SequenceHeader: sequenceHeader,
				}
				state = waitingTrak

			case "Opus":
				if state != waitingCodec {
					return nil, fmt.Errorf("unexpected box '%v'", h.BoxInfo.Type)
				}
				state = waitingDOps
				return h.Expand()

			case "dOps":
				if state != waitingDOps {
					return nil, fmt.Errorf("unexpected box '%v'", h.BoxInfo.Type)
				}

				box, _, err := h.ReadPayload()
				if err != nil {
					return nil, err
				}
				dops := box.(*mp4.DOps)

				curTrack.Codec = &CodecOpus{
					ChannelCount: int(dops.OutputChannelCount),
				}
				state = waitingTrak

			case "mp4v":
				if state != waitingCodec {
					return nil, fmt.Errorf("unexpected box '%v'", h.BoxInfo.Type)
				}

				box, _, err := h.ReadPayload()
				if err != nil {
					return nil, err
				}
				mp4v := box.(*mp4.VisualSampleEntry)

				width = int(mp4v.Width)
				height = int(mp4v.Height)
				state = waitingVideoEsds
				return h.Expand()

			case "mp4a":
				if state != waitingCodec {
					return nil, fmt.Errorf("unexpected box '%v'", h.BoxInfo.Type)
				}

				box, _, err := h.ReadPayload()
				if err != nil {
					return nil, err
				}
				mp4a := box.(*mp4.AudioSampleEntry)

				sampleRate = int(mp4a.SampleRate / 65536)
				channelCount = int(mp4a.ChannelCount)
				state = waitingAudioEsds
				return h.Expand()

			case "esds":
				box, _, err := h.ReadPayload()
				if err != nil {
					return nil, err
				}
				esds := box.(*mp4.Esds)

				conf := esdsFindDecoderConf(esds.Descriptors)
				if conf == nil {
					return nil, fmt.Errorf("unable to find decoder config")
				}

				switch state {
				case waitingVideoEsds:
					switch conf.ObjectTypeIndication {
					case objectTypeIndicationVisualISO14496part2:
						spec := esdsFindDecoderSpecificInfo(esds.Descriptors)
						if spec == nil {
							return nil, fmt.Errorf("unable to find decoder specific info")
						}

						curTrack.Codec = &CodecMPEG4Video{
							Config: spec,
						}

					case objectTypeIndicationVisualISO1318part2Main:
						spec := esdsFindDecoderSpecificInfo(esds.Descriptors)
						if spec == nil {
							return nil, fmt.Errorf("unable to find decoder specific info")
						}

						curTrack.Codec = &CodecMPEG1Video{
							Config: spec,
						}

					case objectTypeIndicationVisualISO10918part1:
						curTrack.Codec = &CodecMJPEG{
							Width:  width,
							Height: height,
						}

					default:
						return nil, fmt.Errorf("unsupported object type indication: 0x%.2x", conf.ObjectTypeIndication)
					}

					state = waitingTrak

				case waitingAudioEsds:
					switch conf.ObjectTypeIndication {
					case objectTypeIndicationAudioISO14496part3:
						spec := esdsFindDecoderSpecificInfo(esds.Descriptors)
						if spec == nil {
							return nil, fmt.Errorf("unable to find decoder specific info")
						}

						var c mpeg4audio.Config
						err := c.Unmarshal(spec)
						if err != nil {
							return nil, fmt.Errorf("invalid MPEG-4 Audio configuration: %w", err)
						}

						curTrack.Codec = &CodecMPEG4Audio{
							Config: c,
						}

					case objectTypeIndicationAudioISO11172part3:
						curTrack.Codec = &CodecMPEG1Audio{
							SampleRate:   sampleRate,
							ChannelCount: channelCount,
						}

					default:
						return nil, fmt.Errorf("unsupported object type indication: 0x%.2x", conf.ObjectTypeIndication)
					}

				default:
					return nil, fmt.Errorf("unexpected box '%v'", h.BoxInfo.Type)
				}

				state = waitingTrak

			case "ac-3":
				if state != waitingCodec {
					return nil, fmt.Errorf("unexpected box '%v'", h.BoxInfo.Type)
				}

				box, _, err := h.ReadPayload()
				if err != nil {
					return nil, err
				}
				ac3 := box.(*mp4.AudioSampleEntry)

				sampleRate = int(ac3.SampleRate / 65536)
				channelCount = int(ac3.ChannelCount)
				state = waitingDac3
				return h.Expand()

			case "dac3":
				if state != waitingDac3 {
					return nil, fmt.Errorf("unexpected box '%v'", h.BoxInfo.Type)
				}

				box, _, err := h.ReadPayload()
				if err != nil {
					return nil, err
				}
				dac3 := box.(*mp4.Dac3)

				curTrack.Codec = &CodecAC3{
					SampleRate:   sampleRate,
					ChannelCount: channelCount,
					Fscod:        dac3.Fscod,
					Bsid:         dac3.Bsid,
					Bsmod:        dac3.Bsmod,
					Acmod:        dac3.Acmod,
					LfeOn:        dac3.LfeOn != 0,
					BitRateCode:  dac3.BitRateCode,
				}
				state = waitingTrak

			case "ipcm":
				if state != waitingCodec {
					return nil, fmt.Errorf("unexpected box '%v'", h.BoxInfo.Type)
				}

				box, _, err := h.ReadPayload()
				if err != nil {
					return nil, err
				}
				ac3 := box.(*mp4.AudioSampleEntry)

				sampleRate = int(ac3.SampleRate / 65536)
				channelCount = int(ac3.ChannelCount)
				state = waitingPcmC
				return h.Expand()

			case "pcmC":
				if state != waitingPcmC {
					return nil, fmt.Errorf("unexpected box '%v'", h.BoxInfo.Type)
				}

				box, _, err := h.ReadPayload()
				if err != nil {
					return nil, err
				}
				pcmc := box.(*mp4.PcmC)

				curTrack.Codec = &CodecLPCM{
					LittleEndian: (pcmc.FormatFlags & 0x01) != 0,
					BitDepth:     int(pcmc.PCMSampleSize),
					SampleRate:   sampleRate,
					ChannelCount: channelCount,
				}
				state = waitingTrak
			}
		}

		return nil, nil
	})
	if err != nil {
		return err
	}

	if state != waitingTrak {
		return fmt.Errorf("parse error")
	}

	if len(i.Tracks) == 0 {
		return fmt.Errorf("no tracks found")
	}

	return nil
}

// Marshal encodes a fMP4 initialization file.
func (i *Init) Marshal(w io.WriteSeeker) error {
	/*
		|ftyp|
		|moov|
		|    |mvhd|
		|    |trak|
		|    |trak|
		|    |....|
		|    |mvex|
		|    |    |trex|
		|    |    |trex|
		|    |    |....|
	*/

	mw := newMP4Writer(w)

	_, err := mw.writeBox(&mp4.Ftyp{ // <ftyp/>
		MajorBrand:   [4]byte{'m', 'p', '4', '2'},
		MinorVersion: 1,
		CompatibleBrands: []mp4.CompatibleBrandElem{
			{CompatibleBrand: [4]byte{'m', 'p', '4', '1'}},
			{CompatibleBrand: [4]byte{'m', 'p', '4', '2'}},
			{CompatibleBrand: [4]byte{'i', 's', 'o', 'm'}},
			{CompatibleBrand: [4]byte{'h', 'l', 's', 'f'}},
		},
	})
	if err != nil {
		return err
	}

	_, err = mw.writeBoxStart(&mp4.Moov{}) // <moov>
	if err != nil {
		return err
	}

	_, err = mw.writeBox(&mp4.Mvhd{ // <mvhd/>
		Timescale:   1000,
		Rate:        65536,
		Volume:      256,
		Matrix:      [9]int32{0x00010000, 0, 0, 0, 0x00010000, 0, 0, 0, 0x40000000},
		NextTrackID: 4294967295,
	})
	if err != nil {
		return err
	}

	for _, track := range i.Tracks {
		err = track.marshal(mw)
		if err != nil {
			return err
		}
	}

	_, err = mw.writeBoxStart(&mp4.Mvex{}) // <mvex>
	if err != nil {
		return err
	}

	for _, track := range i.Tracks {
		_, err = mw.writeBox(&mp4.Trex{ // <trex/>
			TrackID:                       uint32(track.ID),
			DefaultSampleDescriptionIndex: 1,
		})
		if err != nil {
			return err
		}
	}

	err = mw.writeBoxEnd() // </mvex>
	if err != nil {
		return err
	}

	err = mw.writeBoxEnd() // </moov>
	if err != nil {
		return err
	}

	return nil
}
