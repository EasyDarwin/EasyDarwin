package log

import (
	"fmt"
	"strconv"
	"testing"
)

func TestLog(t *testing.T) {
	out := make([]*Log, 0, 8)
	fmt.Printf("len %d cap %d\n", len(out), cap(out))

	for i := 0; i < 8; i++ {
		out = append(out, &Log{Remark: strconv.Itoa(i)})
	}

	fmt.Printf("len %d cap %d\n", len(out), cap(out))
	clear(out)
	fmt.Printf("len %d cap %d\n", len(out), cap(out))
	out = out[:0]
	fmt.Printf("len %d cap %d\n", len(out), cap(out))
}
