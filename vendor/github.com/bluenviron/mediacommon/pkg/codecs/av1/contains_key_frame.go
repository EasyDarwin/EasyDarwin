package av1

import (
	"fmt"
)

// ContainsKeyFrame checks whether a temporal unit contain a key frame.
func ContainsKeyFrame(tu [][]byte) (bool, error) {
	if len(tu) == 0 {
		return false, fmt.Errorf("temporal unit is empty")
	}

	var h OBUHeader
	err := h.Unmarshal(tu[0])
	if err != nil {
		return false, err
	}

	return (h.Type == OBUTypeSequenceHeader), nil
}
