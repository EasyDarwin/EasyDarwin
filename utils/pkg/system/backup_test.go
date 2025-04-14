package system

import (
	"fmt"
	"testing"
)

func TestBackup(t *testing.T) {
	f := NewFileBackup("./h.txt")
	for i := range 10 {
		f.Write([]byte(fmt.Sprintf("%d", i)))
	}
}
