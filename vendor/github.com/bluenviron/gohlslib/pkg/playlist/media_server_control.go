package playlist

import (
	"strconv"
	"time"

	"github.com/bluenviron/gohlslib/pkg/playlist/primitives"
)

// MediaServerControl is a EXT-X-SERVER-CONTROL tag.
type MediaServerControl struct {
	// CAN-BLOCK-RELOAD
	CanBlockReload bool

	// PART-HOLD-BACK
	// The value is a decimal-floating-point number of seconds that
	// indicates the server-recommended minimum distance from the end of
	// the Playlist at which clients should begin to play or to which
	// they should seek when playing in Low-Latency Mode.  Its value MUST
	// be at least twice the Part Target Duration.  Its value SHOULD be
	// at least three times the Part Target Duration.
	PartHoldBack *time.Duration

	// CAN-SKIP-UNTIL
	// Indicates that the Server can produce Playlist Delta Updates in
	// response to the _HLS_skip Delivery Directive.  Its value is the
	// Skip Boundary, a decimal-floating-point number of seconds.  The
	// Skip Boundary MUST be at least six times the Target Duration.
	CanSkipUntil *time.Duration
}

func (t *MediaServerControl) unmarshal(v string) error {
	attrs, err := primitives.AttributesUnmarshal(v)
	if err != nil {
		return err
	}

	for key, val := range attrs {
		switch key {
		case "CAN-BLOCK-RELOAD":
			t.CanBlockReload = (val == "YES")

		case "PART-HOLD-BACK":
			tmp, err := primitives.DurationUnmarshal(val)
			if err != nil {
				return err
			}
			t.PartHoldBack = &tmp

		case "CAN-SKIP-UNTIL":
			tmp, err := primitives.DurationUnmarshal(val)
			if err != nil {
				return err
			}
			t.CanSkipUntil = &tmp
		}
	}

	return nil
}

func (t MediaServerControl) marshal() string {
	ret := "#EXT-X-SERVER-CONTROL:"

	if t.CanBlockReload {
		ret += "CAN-BLOCK-RELOAD=YES"
	}

	if t.PartHoldBack != nil {
		ret += ",PART-HOLD-BACK=" + strconv.FormatFloat(t.PartHoldBack.Seconds(), 'f', 5, 64)
	}

	if t.CanSkipUntil != nil {
		ret += ",CAN-SKIP-UNTIL=" + strconv.FormatFloat(t.CanSkipUntil.Seconds(), 'f', 5, 64)
	}

	ret += "\n"

	return ret
}
