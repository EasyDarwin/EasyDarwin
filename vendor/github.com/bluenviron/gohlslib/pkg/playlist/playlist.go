// Package playlist contains a M3U8 playlist decoder and encoder.
package playlist

import (
	"bufio"
	"bytes"
	"strings"
)

const (
	maxSupportedVersion = 10
)

// Playlist is either Media or Multivariant.
type Playlist interface {
	isPlaylist()
	Unmarshal([]byte) error
}

func findType(buf []byte) (Playlist, error) {
	r := bufio.NewReader(bytes.NewReader(buf))

	for {
		line, err := r.ReadString('\n')
		if err != nil {
			return nil, err
		}

		switch {
		case strings.HasPrefix(line, "#EXT-X-STREAM-INF:"):
			return &Multivariant{}, nil

		case strings.HasPrefix(line, "#EXTINF:"):
			return &Media{}, nil
		}
	}
}

// Unmarshal decodes a playlist.
func Unmarshal(byts []byte) (Playlist, error) {
	pl, err := findType(byts)
	if err != nil {
		return nil, err
	}

	err = pl.Unmarshal(byts)
	if err != nil {
		return nil, err
	}

	return pl, nil
}
