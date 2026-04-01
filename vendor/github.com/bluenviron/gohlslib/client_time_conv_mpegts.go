package gohlslib

import (
	"sync"
	"time"

	"github.com/bluenviron/mediacommon/pkg/formats/mpegts"
)

type clientTimeConvMPEGTS struct {
	startDTS int64

	td    *mpegts.TimeDecoder
	mutex sync.Mutex
}

func (ts *clientTimeConvMPEGTS) initialize() {
	ts.td = mpegts.NewTimeDecoder(ts.startDTS)
}

func (ts *clientTimeConvMPEGTS) convert(v int64) time.Duration {
	ts.mutex.Lock()
	defer ts.mutex.Unlock()
	return ts.td.Decode(v)
}
