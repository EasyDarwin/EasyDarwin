package gohlslib

import (
	"fmt"
	"io"
	"time"

	"github.com/bluenviron/mediacommon/pkg/codecs/h264"
	"github.com/bluenviron/mediacommon/pkg/formats/mpegts"

	"github.com/bluenviron/gohlslib/pkg/codecs"
	"github.com/bluenviron/gohlslib/pkg/storage"
)

const (
	mpegtsSegmentMinAUCount = 100
)

type switchableWriter struct {
	w io.Writer
}

func (w *switchableWriter) Write(p []byte) (int, error) {
	return w.w.Write(p)
}

type muxerSegmenterMPEGTS struct {
	segmentMinDuration time.Duration
	segmentMaxSize     uint64
	videoTrack         *Track
	audioTrack         *Track
	prefix             string
	factory            storage.Factory
	publishSegment     func(muxerSegment) error

	writerVideoTrack  *mpegts.Track
	writerAudioTrack  *mpegts.Track
	switchableWriter  *switchableWriter
	writer            *mpegts.Writer
	nextSegmentID     uint64
	currentSegment    *muxerSegmentMPEGTS
	videoDTSExtractor *h264.DTSExtractor
}

func (m *muxerSegmenterMPEGTS) initialize() {
	var tracks []*mpegts.Track

	if m.videoTrack != nil {
		m.writerVideoTrack = &mpegts.Track{
			Codec: codecs.ToMPEGTS(m.videoTrack.Codec),
		}
		tracks = append(tracks, m.writerVideoTrack)
	}

	if m.audioTrack != nil {
		m.writerAudioTrack = &mpegts.Track{
			Codec: codecs.ToMPEGTS(m.audioTrack.Codec),
		}
		tracks = append(tracks, m.writerAudioTrack)
	}

	m.switchableWriter = &switchableWriter{}

	m.writer = mpegts.NewWriter(m.switchableWriter, tracks)
}

func (m *muxerSegmenterMPEGTS) close() {
	if m.currentSegment != nil {
		m.currentSegment.finalize(0) //nolint:errcheck
		m.currentSegment.close()
	}
}

func (m *muxerSegmenterMPEGTS) takeSegmentID() uint64 {
	id := m.nextSegmentID
	m.nextSegmentID++
	return id
}

func (m *muxerSegmenterMPEGTS) writeAV1(
	_ time.Time,
	_ time.Duration,
	_ [][]byte,
	_ bool,
	_ bool,
) error {
	return fmt.Errorf("unimplemented")
}

func (m *muxerSegmenterMPEGTS) writeVP9(
	_ time.Time,
	_ time.Duration,
	_ []byte,
	_ bool,
	_ bool,
) error {
	return fmt.Errorf("unimplemented")
}

func (m *muxerSegmenterMPEGTS) writeH26x(
	ntp time.Time,
	pts time.Duration,
	au [][]byte,
	randomAccessPresent bool,
	forceSwitch bool,
) error {
	var dts time.Duration

	if m.currentSegment == nil {
		// skip groups silently until we find one with a IDR
		if !randomAccessPresent {
			return nil
		}

		m.videoDTSExtractor = h264.NewDTSExtractor()

		var err error
		dts, err = m.videoDTSExtractor.Extract(au, pts)
		if err != nil {
			return fmt.Errorf("unable to extract DTS: %w", err)
		}

		// create first segment
		seg := &muxerSegmentMPEGTS{
			id:               m.takeSegmentID(),
			startNTP:         ntp,
			segmentMaxSize:   m.segmentMaxSize,
			writerVideoTrack: m.writerVideoTrack,
			writerAudioTrack: m.writerAudioTrack,
			switchableWriter: m.switchableWriter,
			writer:           m.writer,
			prefix:           m.prefix,
			factory:          m.factory,
		}
		err = seg.initialize()
		if err != nil {
			return err
		}
		m.currentSegment = seg
	} else {
		var err error
		dts, err = m.videoDTSExtractor.Extract(au, pts)
		if err != nil {
			return fmt.Errorf("unable to extract DTS: %w", err)
		}

		// switch segment
		if randomAccessPresent &&
			((dts-*m.currentSegment.startDTS) >= m.segmentMinDuration ||
				forceSwitch) {
			err := m.currentSegment.finalize(dts)
			if err != nil {
				return err
			}
			seg := m.currentSegment
			m.currentSegment = nil

			err = m.publishSegment(seg)
			if err != nil {
				seg.close()
				return err
			}

			seg = &muxerSegmentMPEGTS{
				id:               m.takeSegmentID(),
				startNTP:         ntp,
				segmentMaxSize:   m.segmentMaxSize,
				writerVideoTrack: m.writerVideoTrack,
				writerAudioTrack: m.writerAudioTrack,
				switchableWriter: m.switchableWriter,
				writer:           m.writer,
				prefix:           m.prefix,
				factory:          m.factory,
			}
			err = seg.initialize()
			if err != nil {
				return err
			}
			m.currentSegment = seg
		}
	}

	err := m.currentSegment.writeH264(
		pts,
		dts,
		randomAccessPresent,
		au)
	if err != nil {
		return err
	}

	return nil
}

func (m *muxerSegmenterMPEGTS) writeOpus(_ time.Time, _ time.Duration, _ [][]byte) error {
	return fmt.Errorf("unimplemented")
}

func (m *muxerSegmenterMPEGTS) writeMPEG4Audio(ntp time.Time, pts time.Duration, aus [][]byte) error {
	if m.videoTrack == nil {
		if m.currentSegment == nil {
			// create first segment
			seg := &muxerSegmentMPEGTS{
				id:               m.takeSegmentID(),
				startNTP:         ntp,
				segmentMaxSize:   m.segmentMaxSize,
				writerVideoTrack: m.writerVideoTrack,
				writerAudioTrack: m.writerAudioTrack,
				switchableWriter: m.switchableWriter,
				writer:           m.writer,
				prefix:           m.prefix,
				factory:          m.factory,
			}
			err := seg.initialize()
			if err != nil {
				return err
			}
			m.currentSegment = seg
		} else if m.currentSegment.audioAUCount >= mpegtsSegmentMinAUCount && // switch segment
			(pts-*m.currentSegment.startDTS) >= m.segmentMinDuration {
			err := m.currentSegment.finalize(pts)
			if err != nil {
				return err
			}
			seg := m.currentSegment
			m.currentSegment = nil

			err = m.publishSegment(seg)
			if err != nil {
				seg.close()
				return err
			}

			seg = &muxerSegmentMPEGTS{
				id:               m.takeSegmentID(),
				startNTP:         ntp,
				segmentMaxSize:   m.segmentMaxSize,
				writerVideoTrack: m.writerVideoTrack,
				writerAudioTrack: m.writerAudioTrack,
				switchableWriter: m.switchableWriter,
				writer:           m.writer,
				prefix:           m.prefix,
				factory:          m.factory,
			}
			err = seg.initialize()
			if err != nil {
				return err
			}
			m.currentSegment = seg
		}
	} else {
		// wait for the video track
		if m.currentSegment == nil {
			return nil
		}
	}

	err := m.currentSegment.writeMPEG4Audio(pts, aus)
	if err != nil {
		return err
	}

	return nil
}
