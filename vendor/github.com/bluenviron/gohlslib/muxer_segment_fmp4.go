package gohlslib

import (
	"fmt"
	"io"
	"time"

	"github.com/bluenviron/gohlslib/pkg/storage"
)

type muxerSegmentFMP4 struct {
	lowLatency     bool
	id             uint64
	startNTP       time.Time
	startDTS       time.Duration
	segmentMaxSize uint64
	videoTrack     *Track
	audioTrack     *Track
	audioTimeScale uint32
	prefix         string
	forceSwitched  bool
	factory        storage.Factory
	takePartID     func() uint64
	givePartID     func()
	publishPart    func(*muxerPart) error

	name        string
	storage     storage.File
	size        uint64
	parts       []*muxerPart
	currentPart *muxerPart
	endDTS      time.Duration
}

func (s *muxerSegmentFMP4) initialize() error {
	s.name = segmentName(s.prefix, s.id, true)

	var err error
	s.storage, err = s.factory.NewFile(s.name)
	if err != nil {
		return err
	}

	s.currentPart = &muxerPart{
		startDTS:       s.startDTS,
		videoTrack:     s.videoTrack,
		audioTrack:     s.audioTrack,
		audioTimeScale: s.audioTimeScale,
		prefix:         s.prefix,
		id:             s.takePartID(),
		storage:        s.storage.NewPart(),
	}
	s.currentPart.initialize()

	return nil
}

func (s *muxerSegmentFMP4) close() {
	s.storage.Remove()
}

func (s *muxerSegmentFMP4) getName() string {
	return s.name
}

func (s *muxerSegmentFMP4) getDuration() time.Duration {
	return s.endDTS - s.startDTS
}

func (s *muxerSegmentFMP4) getSize() uint64 {
	return s.storage.Size()
}

func (s *muxerSegmentFMP4) isForceSwitched() bool {
	return s.forceSwitched
}

func (s *muxerSegmentFMP4) reader() (io.ReadCloser, error) {
	return s.storage.Reader()
}

func (s *muxerSegmentFMP4) finalize(nextDTS time.Duration) error {
	if s.currentPart.videoSamples != nil || s.currentPart.audioSamples != nil {
		err := s.currentPart.finalize(nextDTS)
		if err != nil {
			return err
		}
		part := s.currentPart
		s.currentPart = nil

		s.parts = append(s.parts, part)
		err = s.publishPart(part)
		if err != nil {
			return err
		}
	} else {
		s.givePartID()
	}

	s.currentPart = nil

	s.storage.Finalize()

	s.endDTS = nextDTS

	return nil
}

func (s *muxerSegmentFMP4) writeVideo(
	sample *augmentedSample,
	nextDTS time.Duration,
	adjustedPartDuration time.Duration,
) error {
	size := uint64(len(sample.Payload))
	if (s.size + size) > s.segmentMaxSize {
		return fmt.Errorf("reached maximum segment size")
	}
	s.size += size

	s.currentPart.writeVideo(sample)

	// switch part
	if s.lowLatency &&
		s.currentPart.computeDuration(nextDTS) >= adjustedPartDuration {
		err := s.currentPart.finalize(nextDTS)
		if err != nil {
			return err
		}
		part := s.currentPart
		s.currentPart = nil

		s.parts = append(s.parts, part)
		err = s.publishPart(part)
		if err != nil {
			return err
		}

		s.currentPart = &muxerPart{
			startDTS:       nextDTS,
			videoTrack:     s.videoTrack,
			audioTrack:     s.audioTrack,
			audioTimeScale: s.audioTimeScale,
			prefix:         s.prefix,
			id:             s.takePartID(),
			storage:        s.storage.NewPart(),
		}
		s.currentPart.initialize()
	}

	return nil
}

func (s *muxerSegmentFMP4) writeAudio(
	sample *augmentedSample,
	nextAudioSampleDTS time.Duration,
	adjustedPartDuration time.Duration,
) error {
	size := uint64(len(sample.Payload))
	if (s.size + size) > s.segmentMaxSize {
		return fmt.Errorf("reached maximum segment size")
	}
	s.size += size

	s.currentPart.writeAudio(sample)

	// switch part
	if s.lowLatency && s.videoTrack == nil &&
		s.currentPart.computeDuration(nextAudioSampleDTS) >= adjustedPartDuration {
		err := s.currentPart.finalize(nextAudioSampleDTS)
		if err != nil {
			return err
		}
		part := s.currentPart
		s.currentPart = nil

		s.parts = append(s.parts, part)
		err = s.publishPart(part)
		if err != nil {
			return err
		}

		s.currentPart = &muxerPart{
			startDTS:       nextAudioSampleDTS,
			videoTrack:     s.videoTrack,
			audioTrack:     s.audioTrack,
			audioTimeScale: s.audioTimeScale,
			prefix:         s.prefix,
			id:             s.takePartID(),
			storage:        s.storage.NewPart(),
		}
		s.currentPart.initialize()
	}

	return nil
}
