package playlist

import (
	"fmt"
	"strconv"

	"github.com/bluenviron/gohlslib/pkg/playlist/primitives"
)

// MediaSkip is a EXT-X-SKIP tag.
type MediaSkip struct {
	// SKIPPED-SEGMENTS
	// required
	SkippedSegments int
}

func (t *MediaSkip) unmarshal(v string) error {
	attrs, err := primitives.AttributesUnmarshal(v)
	if err != nil {
		return err
	}

	skipSegFound := false

	for key, val := range attrs {
		if key == "SKIPPED-SEGMENTS" {
			tmp, err := strconv.ParseUint(val, 10, 31)
			if err != nil {
				return err
			}
			t.SkippedSegments = int(tmp)
			skipSegFound = true
		}
	}

	if !skipSegFound {
		return fmt.Errorf("SKIPPED-SEGMENTS missing")
	}

	return nil
}

func (t MediaSkip) marshal() string {
	return "#EXT-X-SKIP:SKIPPED-SEGMENTS=" + strconv.FormatInt(int64(t.SkippedSegments), 10) + "\n"
}
