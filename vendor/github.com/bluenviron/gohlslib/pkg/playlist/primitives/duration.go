package primitives

import (
	"strconv"
	"time"
)

// DurationUnmarshal decodes a duration.
func DurationUnmarshal(val string) (time.Duration, error) {
	tmp, err := strconv.ParseFloat(val, 64)
	if err != nil {
		return 0, err
	}

	return time.Duration(tmp * float64(time.Second)), nil
}
