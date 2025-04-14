package srt

import (
	"errors"
	"strings"
)

type StreamID struct {
	User      string
	Host      string
	Resource  string
	SessionID string
	Type      string
	Mode      string
}

func parseStreamID(streamID string) (*StreamID, error) {
	if !strings.Contains(streamID, "#!::") {
		return nil, errors.New("invalid streamid")
	}
	split := strings.Split(strings.TrimPrefix(streamID, "#!::"), ",")
	id := &StreamID{}

	for _, s := range split {
		if strings.Contains(s, "=") {
			kv := strings.Split(s, "=")
			if len(kv) != 2 {
				return nil, errors.New("invalid streamid")
			}

			if kv[0] == "u" {
				id.User = kv[1]
			}
			if kv[0] == "h" {
				id.Host = kv[1]
			}
			if kv[0] == "r" {
				id.Resource = kv[1]
			}
			if kv[0] == "s" {
				id.SessionID = kv[1]
			}
			if kv[0] == "t" {
				id.Type = kv[1]
			}
			if kv[0] == "m" {
				id.Mode = kv[1]
			}
		}
	}
	return id, nil
}
