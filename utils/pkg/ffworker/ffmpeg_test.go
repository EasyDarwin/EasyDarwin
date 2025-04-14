// Author: xiexu
// Date: 2024-05-01

package ffworker

import (
	"context"
	"fmt"
	"os"
	"testing"
)

func TestKeyFrameToJpeg(t *testing.T) {
	b, _ := os.ReadFile("./demo.raw")
	out, err := KeyFrameToJpeg(context.Background(), b)
	if err != nil {
		t.Fatal(err)
	}
	os.WriteFile("./demo.jpg", out, 0o777)
}

func TestPSToMP4(t *testing.T) {
	if err := PSToMP4(context.Background(), "./demo.ps", "./demo.mp4"); err != nil {
		t.Fatal(err)
	}
}

func TestFMP4ToMP4(t *testing.T) {
	if err := FMP4ToMP4(context.Background(), "./a.mp4", "./out.mp4"); err != nil {
		fmt.Println(err.Error())
	}
}
