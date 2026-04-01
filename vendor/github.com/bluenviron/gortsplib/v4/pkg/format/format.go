// Package format contains RTP format definitions, decoders and encoders.
package format

import (
	"strings"

	"github.com/pion/rtp"
)

func getCodecAndClock(rtpMap string) (string, string) {
	parts2 := strings.SplitN(rtpMap, "/", 2)
	if len(parts2) != 2 {
		return "", ""
	}

	return strings.ToLower(parts2[0]), parts2[1]
}

type unmarshalContext struct {
	mediaType   string
	payloadType uint8
	clock       string
	codec       string
	rtpMap      string
	fmtp        map[string]string
}

// Format is a media format.
// It defines the payload type of RTP packets and how to encode/decode them.
type Format interface {
	unmarshal(ctx *unmarshalContext) error

	// Codec returns the codec name.
	Codec() string

	// ClockRate returns the clock rate.
	ClockRate() int

	// PayloadType returns the payload type.
	PayloadType() uint8

	// RTPMap returns the rtpmap attribute.
	RTPMap() string

	// FMTP returns the fmtp attribute.
	FMTP() map[string]string

	// PTSEqualsDTS checks whether PTS is equal to DTS in RTP packets.
	PTSEqualsDTS(*rtp.Packet) bool
}

// Unmarshal decodes a format from a media description.
func Unmarshal(mediaType string, payloadType uint8, rtpMap string, fmtp map[string]string) (Format, error) {
	codec, clock := getCodecAndClock(rtpMap)

	format := func() Format {
		switch {
		/*
		* static payload types
		**/

		// video

		case payloadType == 32:
			return &MPEG1Video{}

		case payloadType == 26:
			return &MJPEG{}

		case payloadType == 33:
			return &MPEGTS{}

		// audio

		case payloadType == 14:
			return &MPEG1Audio{}

		case payloadType == 9:
			return &G722{}

		case payloadType == 0, payloadType == 8:
			return &G711{}

		case payloadType == 10, payloadType == 11:
			return &LPCM{}

		/*
		* dynamic payload types
		**/

		case payloadType >= 96 && payloadType <= 127:
			switch {
			// video

			case codec == "av1" && clock == "90000":
				return &AV1{}

			case codec == "vp9" && clock == "90000":
				return &VP9{}

			case codec == "vp8" && clock == "90000":
				return &VP8{}

			case codec == "h265" && clock == "90000":
				return &H265{}

			case codec == "h264" && clock == "90000":
				return &H264{}

			case codec == "mp4v-es" && clock == "90000":
				return &MPEG4Video{}

			// audio

			case codec == "opus", codec == "multiopus":
				return &Opus{}

			case codec == "vorbis":
				return &Vorbis{}

			case codec == "mpeg4-generic", codec == "mp4a-latm":
				return &MPEG4Audio{}

			case codec == "ac3":
				return &AC3{}

			case codec == "speex":
				return &Speex{}

			case (codec == "g726-16" ||
				codec == "g726-24" ||
				codec == "g726-32" ||
				codec == "g726-40" ||
				codec == "aal2-g726-16" ||
				codec == "aal2-g726-24" ||
				codec == "aal2-g726-32" ||
				codec == "aal2-g726-40") && clock == "8000":
				return &G726{}

			case codec == "pcma", codec == "pcmu":
				return &G711{}

			case codec == "l8", codec == "l16", codec == "l24":
				return &LPCM{}
			}
		}

		return &Generic{}
	}()

	err := format.unmarshal(&unmarshalContext{
		mediaType:   mediaType,
		payloadType: payloadType,
		clock:       clock,
		codec:       codec,
		rtpMap:      rtpMap,
		fmtp:        fmtp,
	})
	if err != nil {
		return nil, err
	}

	return format, nil
}
