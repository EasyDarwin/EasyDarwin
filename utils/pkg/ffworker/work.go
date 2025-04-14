// Author: xiexu
// Date: 2024-05-01

package ffworker

import (
	"context"
)

// Engine ...
type Engine struct {
	task chan struct{}
}

// NewEngine ...
func NewEngine(taskMaxNum int) *Engine {
	if taskMaxNum <= 0 {
		taskMaxNum = 1
	}
	e := Engine{
		task: make(chan struct{}, taskMaxNum),
	}
	return &e
}

// KeyFrameToJpeg 关键帧转图片
func (e *Engine) KeyFrameToJpeg(ctx context.Context, input []byte) ([]byte, error) {
	select {
	case e.task <- struct{}{}:
		out, err := KeyFrameToJpeg(ctx, input)
		<-e.task
		return out, err
	case <-ctx.Done():
		return nil, ctx.Err()
	}
}

// PSToMP4 ps 转 mp4
func (e *Engine) PSToMP4(ctx context.Context, ifile, ofile string) error {
	select {
	case e.task <- struct{}{}:
		err := PSToMP4(ctx, ifile, ofile)
		<-e.task
		return err
	case <-ctx.Done():
		return ctx.Err()
	}
}
