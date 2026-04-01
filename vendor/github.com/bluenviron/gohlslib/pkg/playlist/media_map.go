package playlist

import (
	"fmt"

	"github.com/bluenviron/gohlslib/pkg/playlist/primitives"
)

// MediaMap is a EXT-X-MAP tag.
type MediaMap struct {
	// URI
	// required
	URI string

	// BYTERANGE
	ByteRangeLength *uint64
	ByteRangeStart  *uint64
}

func (t *MediaMap) unmarshal(v string) error {
	attrs, err := primitives.AttributesUnmarshal(v)
	if err != nil {
		return err
	}

	for key, val := range attrs {
		switch key {
		case "URI":
			t.URI = val

		case "BYTERANGE":
			length, start, err := primitives.ByteRangeUnmarshal(val)
			if err != nil {
				return err
			}

			t.ByteRangeLength = &length
			t.ByteRangeStart = start
		}
	}

	if t.URI == "" {
		return fmt.Errorf("URI not found")
	}

	return nil
}

func (t MediaMap) marshal() string {
	ret := "#EXT-X-MAP:URI=\"" + t.URI + "\""

	if t.ByteRangeLength != nil {
		ret += ",BYTERANGE=" + primitives.ByteRangeMarshal(*t.ByteRangeLength, t.ByteRangeStart) + ""
	}

	ret += "\n"

	return ret
}
