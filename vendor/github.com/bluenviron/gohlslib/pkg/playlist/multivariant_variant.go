package playlist

import (
	"fmt"
	"strconv"
	"strings"

	"github.com/bluenviron/gohlslib/pkg/playlist/primitives"
)

// MultivariantVariant is a EXT-X-STREAM-INF tag.
type MultivariantVariant struct {
	// BANDWIDTH
	// required
	Bandwidth int

	// CODECS
	// required
	Codecs []string

	// URL
	// required
	URI string

	// AVERAGE-BANDWIDTH
	AverageBandwidth *int

	// RESOLUTION
	Resolution string

	// FRAME-RATE
	FrameRate *float64

	// VIDEO
	Video string

	// AUDIO
	Audio string

	// SUBTITLES
	Subtitles string

	// CLOSED-CAPTIONS
	ClosedCaptions string
}

func (v *MultivariantVariant) unmarshal(va string) error {
	lines := strings.Split(va, "\n")

	attrs, err := primitives.AttributesUnmarshal(lines[0])
	if err != nil {
		return err
	}

	for key, val := range attrs {
		switch key {
		case "BANDWIDTH":
			tmp, err := strconv.ParseUint(val, 10, 31)
			if err != nil {
				return err
			}
			v.Bandwidth = int(tmp)

		case "AVERAGE-BANDWIDTH":
			tmp, err := strconv.ParseUint(val, 10, 31)
			if err != nil {
				return err
			}
			tmp2 := int(tmp)
			v.AverageBandwidth = &tmp2

		case "CODECS":
			v.Codecs = strings.Split(val, ",")

		case "RESOLUTION":
			v.Resolution = val

		case "FRAME-RATE":
			tmp, err := strconv.ParseFloat(val, 64)
			if err != nil {
				return err
			}
			v.FrameRate = &tmp

		case "VIDEO":
			v.Video = val

		case "AUDIO":
			v.Audio = val

		case "SUBTITLES":
			v.Subtitles = val

		case "CLOSED-CAPTIONS":
			v.ClosedCaptions = val
		}
	}

	if len(lines[1]) == 0 || lines[1][0] == '#' {
		return fmt.Errorf("invalid URI: %s", lines[1])
	}
	v.URI = lines[1]

	return nil
}

func (v MultivariantVariant) marshal() string {
	ret := "#EXT-X-STREAM-INF:BANDWIDTH=" + strconv.FormatInt(int64(v.Bandwidth), 10)

	if v.AverageBandwidth != nil {
		ret += ",AVERAGE-BANDWIDTH=" + strconv.FormatInt(int64(*v.AverageBandwidth), 10)
	}

	ret += ",CODECS=\"" + strings.Join(v.Codecs, ",") + "\""

	if v.Resolution != "" {
		ret += ",RESOLUTION=" + v.Resolution
	}

	if v.FrameRate != nil {
		ret += ",FRAME-RATE=" + strconv.FormatFloat(*v.FrameRate, 'f', 3, 64)
	}

	if v.Video != "" {
		ret += ",VIDEO=\"" + v.Video + "\""
	}

	if v.Audio != "" {
		ret += ",AUDIO=\"" + v.Audio + "\""
	}

	if v.Subtitles != "" {
		ret += ",SUBTITLES=\"" + v.Subtitles + "\""
	}

	if v.ClosedCaptions != "" {
		ret += ",CLOSED-CAPTIONS=\"" + v.ClosedCaptions + "\""
	}

	ret += "\n" + v.URI + "\n"

	return ret
}
