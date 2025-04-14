package hik

import (
	"fmt"
	"testing"
	"time"
)

func TestHK(t *testing.T) {
	hk := NewHK()
	hk.Discover()
	time.Sleep(time.Second)
	fmt.Println("end")
}
