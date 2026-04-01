package playlist

import (
	"fmt"
	"strconv"
	"time"

	"github.com/bluenviron/gohlslib/pkg/playlist/primitives"
)

// MediaPart is a EXT-X-PART tag.
type MediaPart struct {
	// DURATION
	// required
	Duration time.Duration

	// URI
	// required
	URI string

	// INDEPENDENT
	Independent bool

	// BYTERANGE
	ByteRangeLength *uint64
	ByteRangeStart  *uint64

	// GAP
	Gap bool
}

func (p *MediaPart) unmarshal(v string) error {
	attrs, err := primitives.AttributesUnmarshal(v)
	if err != nil {
		return err
	}

	for key, val := range attrs {
		switch key {
		case "DURATION":
			tmp, err := primitives.DurationUnmarshal(val)
			if err != nil {
				return err
			}
			p.Duration = tmp

		case "URI":
			p.URI = val

		case "INDEPENDENT":
			p.Independent = (val == "YES")

		case "BYTERANGE":
			length, start, err := primitives.ByteRangeUnmarshal(val)
			if err != nil {
				return err
			}
			p.ByteRangeLength = &length
			p.ByteRangeStart = start

		case "GAP":
			p.Gap = true
		}
	}

	if p.Duration == 0 {
		return fmt.Errorf("DURATION missing")
	}

	if p.URI == "" {
		return fmt.Errorf("URI missing")
	}

	return nil
}

func (p MediaPart) marshal() string {
	ret := "#EXT-X-PART:DURATION=" + strconv.FormatFloat(p.Duration.Seconds(), 'f', 5, 64) +
		",URI=\"" + p.URI + "\""

	if p.Independent {
		ret += ",INDEPENDENT=YES"
	}

	if p.ByteRangeLength != nil {
		ret += ",BYTERANGE=" + primitives.ByteRangeMarshal(*p.ByteRangeLength, p.ByteRangeStart) + ""
	}

	if p.Gap {
		ret += ",GAP=YES"
	}

	ret += "\n"
	return ret
}
