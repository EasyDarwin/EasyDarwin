// Author: xiexu
// Date: 2024-05-01

package ffworker

import (
	"bytes"
	"context"
	"fmt"
	"io"
	"os/exec"
	"path/filepath"
	"runtime"
	"strings"

	"easydarwin/lnton/pkg/system"
)

// KeyFrameToJpeg 关键帧转图片
func KeyFrameToJpeg(ctx context.Context, keyFrame []byte) ([]byte, error) {
	//  cat demo.raw | ffmpeg -i pipe:0 -frames:v 1 -f mjpeg pipe:1 >> a.jpeg
	in := bytes.NewReader(keyFrame)
	out := bytes.NewBuffer(nil)
	err := ffmpegPipeExec(ctx, in, out, "-frames:v", "1", "-f", "MJPEG")
	// fmt.Println(">>>", out.String())
	return out.Bytes(), err
}

// PSToMP4 ps 转 mp4
func PSToMP4(ctx context.Context, ifile, ofile string) error {
	// ffmpeg -i demo.ps -vcodec copy -an -f mp4 demo.mp4
	return ffmpegExec(ctx, ifile, ofile, "-c:v", "copy", "-c:a", "aac", "-b:a", "128k", "-f", "mp4")
}

func ffmpegPipeExec(ctx context.Context, in io.Reader, out io.Writer, args ...string) error {
	// argsData := append(append([]string{"-y", "-i", "pipe:0"}, args...), "pipe:1")
	argsData := append(append([]string{"-y", "-hide_banner", "-loglevel", "error", "-i", "pipe:0"}, args...), "pipe:1")

	ff := getFFmpegPath()
	cmd := exec.CommandContext(ctx, ff, argsData...) // nolint
	cmd.Stdin = in
	cmd.Stdout = out
	errOut := bytes.NewBuffer(nil)
	cmd.Stderr = errOut
	_ = cmd.Run()
	// TODO: err 的判断是有内容，还是判断是否包含关键字?
	if err := errOut.String(); len(err) > 0 && strings.Contains(err, "Error") {
		return fmt.Errorf(err)
	}
	return nil
}

func ffmpegExec(ctx context.Context, ifile, ofile string, args ...string) error {
	argsData := append(append([]string{"-y", "-hide_banner", "-loglevel", "error", "-i", ifile}, args...), ofile)

	ff := getFFmpegPath()
	cmd := exec.CommandContext(ctx, ff, argsData...) // nolint
	// cmd.Stdin = in
	// cmd.Stdout = out
	errOut := bytes.NewBuffer(nil)
	cmd.Stderr = errOut
	_ = cmd.Run()
	// TODO: err 的判断是有内容，还是判断是否包含关键字?
	if err := errOut.String(); len(err) > 0 && strings.Contains(err, "Error") {
		return fmt.Errorf(err)
	}
	return nil
}

func getFFmpegPath() string {
	if runtime.GOOS == "windows" {
		return filepath.Join(system.Getwd(), "ffmpeg.exe")
	}
	// 测试环境
	if runtime.GOOS == "darwin" {
		return "ffmpeg"
	}
	return filepath.Join(system.Getwd(), "ffmpeg")
}

// FMP4ToMP4 fmp4 转 mp4
func FMP4ToMP4(ctx context.Context, ifile, ofile string) error {
	// ffmpeg.exe -i 222.mp4 -vcodec copy -af asetpts=N/SR/TB/PTS-STARTPTS -acodec aac -f mp4 ok-222.mp4
	return ffmpegExec(ctx, ifile, ofile, "-vcodec", "copy", "-af", "asetpts=N/SR/TB/PTS-STARTPTS", "-acodec", "aac", "-f", "mp4")
}
