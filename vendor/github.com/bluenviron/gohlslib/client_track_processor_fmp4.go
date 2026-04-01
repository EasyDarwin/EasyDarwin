package gohlslib

import (
	"context"
	"fmt"
	"time"

	"github.com/bluenviron/gohlslib/pkg/codecs"
	"github.com/bluenviron/mediacommon/pkg/formats/fmp4"
)

type procEntryFMP4 struct {
	ntpAvailable bool
	ntpAbsolute  time.Time
	ntpRelative  time.Duration
	partTrack    *fmp4.PartTrack
}

type clientTrackProcessorFMP4 struct {
	track                *clientTrack
	timeScale            uint32
	timeConv             *clientTimeConvFMP4
	onPartTrackProcessed func(ctx context.Context)

	decodePayload func(sample *fmp4.PartSample) ([][]byte, error)

	// in
	queue chan *procEntryFMP4
}

func (t *clientTrackProcessorFMP4) initialize() error {
	switch t.track.track.Codec.(type) {
	case *codecs.AV1:
		t.decodePayload = func(sample *fmp4.PartSample) ([][]byte, error) {
			return sample.GetAV1()
		}

	case *codecs.VP9:
		t.decodePayload = func(sample *fmp4.PartSample) ([][]byte, error) {
			return [][]byte{sample.Payload}, nil
		}

	case *codecs.H265, *codecs.H264:
		t.decodePayload = func(sample *fmp4.PartSample) ([][]byte, error) {
			return sample.GetH26x()
		}

	case *codecs.Opus:
		t.decodePayload = func(sample *fmp4.PartSample) ([][]byte, error) {
			return [][]byte{sample.Payload}, nil
		}

	case *codecs.MPEG4Audio:
		t.decodePayload = func(sample *fmp4.PartSample) ([][]byte, error) {
			return [][]byte{sample.Payload}, nil
		}
	}

	t.queue = make(chan *procEntryFMP4)

	return nil
}

func (t *clientTrackProcessorFMP4) run(ctx context.Context) error {
	for {
		select {
		case entry := <-t.queue:
			err := t.process(ctx, entry)
			if err != nil {
				return err
			}

		case <-ctx.Done():
			return nil
		}
	}
}

func (t *clientTrackProcessorFMP4) process(ctx context.Context, entry *procEntryFMP4) error {
	rawDTS := entry.partTrack.BaseTime

	for _, sample := range entry.partTrack.Samples {
		data, err := t.decodePayload(sample)
		if err != nil {
			return err
		}

		pts := t.timeConv.convert(rawDTS+uint64(sample.PTSOffset), t.timeScale)
		dts := t.timeConv.convert(rawDTS, t.timeScale)
		rawDTS += uint64(sample.Duration)

		ntp := time.Time{}
		if entry.ntpAvailable {
			ntp = entry.ntpAbsolute.Add(dts - entry.ntpRelative)
		}

		err = t.track.handleData(ctx, pts, dts, ntp, data)
		if err != nil {
			return err
		}
	}

	t.onPartTrackProcessed(ctx)
	return nil
}

func (t *clientTrackProcessorFMP4) push(ctx context.Context, entry *procEntryFMP4) error {
	select {
	case t.queue <- entry:
		return nil

	case <-ctx.Done():
		return fmt.Errorf("terminated")
	}
}
