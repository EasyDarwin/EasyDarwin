package mp4

import (
	"fmt"
	"io"
)

// TopBoxInfo - information about a top-level box
type TopBoxInfo struct {
	// Type - box type
	Type string
	// Size - box size
	Size uint64
	// StartPos - where in file does box start
	StartPos uint64
}

// GetTopBoxInfoList - get top boxes until stopBoxType or end of file
func GetTopBoxInfoList(rs io.ReadSeeker, stopBoxType string) ([]TopBoxInfo, error) {
	var pos uint64 = 0
	var topBoxList []TopBoxInfo

	for {
		h, err := DecodeHeader(rs)
		if err == io.EOF {
			break
		}
		if err != nil {
			return nil, err
		}
		if h.Name == stopBoxType {
			break
		}
		topBoxList = append(topBoxList, TopBoxInfo{h.Name, h.Size, pos})
		nextBoxStart := pos + h.Size
		ipos, err := rs.Seek(int64(nextBoxStart), io.SeekStart)
		if err != nil {
			return nil, err
		}
		pos = uint64(ipos)
		if pos != nextBoxStart {
			return nil, fmt.Errorf("seeked pos %d != next box start %d", pos, nextBoxStart)
		}
	}

	return topBoxList, nil
}
