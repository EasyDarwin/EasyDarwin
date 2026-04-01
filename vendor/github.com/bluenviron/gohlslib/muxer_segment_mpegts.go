package gohlslib

import (
	"bufio"
	"fmt"
	"io"
	"time"

	"github.com/bluenviron/gohlslib/pkg/storage"

	"github.com/bluenviron/mediacommon/pkg/formats/mpegts"
)

func durationGoToMPEGTS(v time.Duration) int64 {
	return int64(v.Seconds() * 90000)
}

type muxerSegmentMPEGTS struct {
	id               uint64
	startNTP         time.Time
	segmentMaxSize   uint64
	writerVideoTrack *mpegts.Track
	writerAudioTrack *mpegts.Track
	switchableWriter *switchableWriter
	writer           *mpegts.Writer
	prefix           string
	factory          storage.Factory

	storage      storage.File
	storagePart  storage.Part
	bw           *bufio.Writer
	size         uint64
	name         string
	startDTS     *time.Duration
	endDTS       time.Duration
	audioAUCount int
}

func (t *muxerSegmentMPEGTS) initialize() error {
	t.name = segmentName(t.prefix, t.id, false)

	var err error
	t.storage, err = t.factory.NewFile(t.name)
	if err != nil {
		return err
	}

	t.storagePart = t.storage.NewPart()
	t.bw = bufio.NewWriter(t.storagePart.Writer())

	t.switchableWriter.w = t.bw

	return nil
}

func (t *muxerSegmentMPEGTS) close() {
	t.storage.Remove()
}

func (t *muxerSegmentMPEGTS) getName() string {
	return t.name
}

func (t *muxerSegmentMPEGTS) getDuration() time.Duration {
	return t.endDTS - *t.startDTS
}

func (t *muxerSegmentMPEGTS) getSize() uint64 {
	return t.storage.Size()
}

func (*muxerSegmentMPEGTS) isForceSwitched() bool {
	return false
}

func (t *muxerSegmentMPEGTS) reader() (io.ReadCloser, error) {
	return t.storage.Reader()
}

func (t *muxerSegmentMPEGTS) finalize(nextDTS time.Duration) error {
	err := t.bw.Flush()
	if err != nil {
		return err
	}

	t.bw = nil
	t.storage.Finalize()
	t.endDTS = nextDTS
	return nil
}

func (t *muxerSegmentMPEGTS) writeH264(
	pts time.Duration,
	dts time.Duration,
	idrPresent bool,
	au [][]byte,
) error {
	size := uint64(0)
	for _, nalu := range au {
		size += uint64(len(nalu))
	}
	if (t.size + size) > t.segmentMaxSize {
		return fmt.Errorf("reached maximum segment size")
	}
	t.size += size

	err := t.writer.WriteH264(t.writerVideoTrack, durationGoToMPEGTS(pts), durationGoToMPEGTS(dts), idrPresent, au)
	if err != nil {
		return err
	}

	if t.startDTS == nil {
		t.startDTS = &dts
	}
	t.endDTS = dts

	return nil
}

func (t *muxerSegmentMPEGTS) writeMPEG4Audio(
	pts time.Duration,
	aus [][]byte,
) error {
	size := uint64(0)
	for _, au := range aus {
		size += uint64(len(au))
	}

	if (t.size + size) > t.segmentMaxSize {
		return fmt.Errorf("reached maximum segment size")
	}
	t.size += size

	err := t.writer.WriteMPEG4Audio(t.writerAudioTrack, durationGoToMPEGTS(pts), aus)
	if err != nil {
		return err
	}

	if t.writerVideoTrack == nil {
		t.audioAUCount++

		if t.startDTS == nil {
			t.startDTS = &pts
		}
		t.endDTS = pts
	}

	return nil
}
