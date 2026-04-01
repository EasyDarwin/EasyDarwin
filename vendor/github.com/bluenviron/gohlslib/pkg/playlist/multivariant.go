package playlist

import (
	"fmt"
	"strconv"
	"strings"

	"github.com/bluenviron/gohlslib/pkg/playlist/primitives"
)

// Multivariant is a multivariant playlist.
type Multivariant struct {
	// #EXT-X-VERSION
	// required
	Version int

	// #EXT-X-INDEPENDENT-SEGMENTS
	IndependentSegments bool

	// #EXT-X-START
	Start *MultivariantStart

	// #EXT-X-STREAM-INF
	// at least one is required
	Variants []*MultivariantVariant

	// #EXT-X-MEDIA
	Renditions []*MultivariantRendition
}

func (m Multivariant) isPlaylist() {}

// Unmarshal decodes the playlist.
func (m *Multivariant) Unmarshal(buf []byte) error {
	s := string(buf)

	s, err := primitives.HeaderUnmarshal(s)
	if err != nil {
		return err
	}

	for {
		var line string
		line, s = primitives.ReadLine(s)
		if line == "" && s == "" {
			break
		}

		switch {
		case strings.HasPrefix(line, "#EXT-X-VERSION:"):
			line = line[len("#EXT-X-VERSION:"):]

			var tmp uint64
			tmp, err = strconv.ParseUint(line, 10, 31)
			if err != nil {
				return err
			}
			m.Version = int(tmp)

			if m.Version > maxSupportedVersion {
				return fmt.Errorf("unsupported HLS version (%d)", m.Version)
			}

		case strings.HasPrefix(line, "#EXT-X-INDEPENDENT-SEGMENTS"):
			m.IndependentSegments = true

		case strings.HasPrefix(line, "#EXT-X-START:"):
			line = line[len("#EXT-X-START:"):]

			m.Start = &MultivariantStart{}
			err = m.Start.unmarshal(line)
			if err != nil {
				return err
			}

		case strings.HasPrefix(line, "#EXT-X-STREAM-INF:"):
			line = line[len("#EXT-X-STREAM-INF:"):]

			var line2 string
			line2, s = primitives.ReadLine(s)
			line += "\n" + line2

			var v MultivariantVariant
			err = v.unmarshal(line)
			if err != nil {
				return fmt.Errorf("invalid variant: %w", err)
			}

			m.Variants = append(m.Variants, &v)

		case strings.HasPrefix(line, "#EXT-X-MEDIA:"):
			line = line[len("#EXT-X-MEDIA:"):]

			var r MultivariantRendition
			err = r.unmarshal(line)
			if err != nil {
				return fmt.Errorf("invalid rendition: %w", err)
			}

			m.Renditions = append(m.Renditions, &r)
		}
	}

	if len(m.Variants) == 0 {
		return fmt.Errorf("no variants found")
	}

	return nil
}

// Marshal encodes the playlist.
func (m Multivariant) Marshal() ([]byte, error) {
	ret := "#EXTM3U\n" +
		"#EXT-X-VERSION:" + strconv.FormatInt(int64(m.Version), 10) + "\n"

	if m.IndependentSegments {
		ret += "#EXT-X-INDEPENDENT-SEGMENTS\n"
	}

	if m.Start != nil {
		ret += m.Start.marshal()
	}

	ret += "\n"

	for _, v := range m.Variants {
		ret += v.marshal()
	}

	if len(m.Renditions) != 0 {
		ret += "\n"

		for _, r := range m.Renditions {
			ret += r.marshal()
		}
	}

	return []byte(ret), nil
}
