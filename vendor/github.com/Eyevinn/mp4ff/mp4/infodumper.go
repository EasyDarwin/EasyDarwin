package mp4

import (
	"fmt"
	"io"
	"strconv"
	"strings"
)

const (
	infoVersionNone         = -1
	infoVersionGroupingType = -2
	infoVersionDescriptor   = -3
)

type boxLike interface {
	Type() string
	Size() uint64
	Info(w io.Writer, specificBoxLevels, indent, indentStep string) error
}

// infoDumper - dump box name and size. Allow for more with write.
type infoDumper struct {
	w      io.Writer
	indent string
	box    boxLike
	err    error
}

// fixStartingCopyrightChar - replace starting one byte © with two-bytes UTF-8
func fixStartingCopyrightChar(boxType string) string {
	// © is 0xa9 in latin1 (and in Apple boxes/atoms)
	// In UTF-8 it is two bytes: 0xc2 0xa9
	bType := []byte(boxType)
	if bType[0] == 0xa9 {
		bType = append([]byte{0xc2}, bType...)
	}
	return string(bType)
}

// newInfoDumper - make an infoDumper with indent
// write version if >= 0
// set Version to -1 if not present for box
// set Version to -2 for sample group entries
// set Version to -3 for descriptors
func newInfoDumper(w io.Writer, indent string, b boxLike, version int, flags uint32) *infoDumper {
	bd := infoDumper{w, indent, b, nil}
	utf8BoxType := fixStartingCopyrightChar(b.Type())
	switch {
	case version >= 0:
		bd.write("[%s] size=%d version=%d flags=%06x", utf8BoxType, b.Size(), version, flags)
	case version == infoVersionNone:
		bd.write("[%s] size=%d", utf8BoxType, b.Size())
	case version == infoVersionGroupingType:
		bd.write("GroupingType %q size=%d", utf8BoxType, b.Size())
	case version == infoVersionDescriptor:
		bd.write("Descriptor %q size=2+%d", b.Type(), b.Size())
	default:
		bd.write("Unknown version %d", version)
	}
	return &bd
}

// write - write formated objecds if level <= bd.level
func (b *infoDumper) write(format string, p ...interface{}) {
	if b.err != nil {
		return
	}
	_, err := fmt.Fprintf(b.w, "%s", b.indent)
	if err != nil {
		b.err = err
		return
	}
	_, b.err = fmt.Fprintf(b.w, format+"\n", p...)
}

// getInfoLevel - get info level for specific boxLike, or from all
func getInfoLevel(b boxLike, specificBoxLevels string) (level int) {
	if len(specificBoxLevels) == 0 {
		return level
	}
	boxesLevels := strings.Split(specificBoxLevels, ",")
	boxType := b.Type()
	if _, ok := b.(Descriptor); ok {
		boxType = "esds"
	}
	var err error
	for _, bl := range boxesLevels {
		splitPos := strings.Index(bl, ":")
		if splitPos < 1 {
			continue
		}
		bt := bl[:splitPos]
		nr := bl[splitPos+1:]
		if bt == boxType {
			level, err = strconv.Atoi(nr)
			if err != nil {
				level = 0
			}
			return level
		} else if bt == "all" {
			level, err = strconv.Atoi(nr)
			if err != nil {
				level = 0
			}
		}
	}
	return level
}
