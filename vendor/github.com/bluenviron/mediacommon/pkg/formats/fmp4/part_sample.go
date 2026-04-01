package fmp4

import (
	"github.com/bluenviron/mediacommon/pkg/codecs/av1"
	"github.com/bluenviron/mediacommon/pkg/codecs/h264"
)

// PartSample is a sample of a PartTrack.
type PartSample struct {
	Duration        uint32
	PTSOffset       int32
	IsNonSyncSample bool
	Payload         []byte
}

// NewPartSampleAV1 creates a sample with AV1 data.
func NewPartSampleAV1(sequenceHeaderPresent bool, tu [][]byte) (*PartSample, error) {
	bs, err := av1.BitstreamMarshal(tu)
	if err != nil {
		return nil, err
	}

	return &PartSample{
		IsNonSyncSample: !sequenceHeaderPresent,
		Payload:         bs,
	}, nil
}

// NewPartSampleH26x creates a sample with H26x data.
func NewPartSampleH26x(ptsOffset int32, randomAccessPresent bool, au [][]byte) (*PartSample, error) {
	avcc, err := h264.AVCCMarshal(au)
	if err != nil {
		return nil, err
	}

	return &PartSample{
		PTSOffset:       ptsOffset,
		IsNonSyncSample: !randomAccessPresent,
		Payload:         avcc,
	}, nil
}

// GetAV1 gets AV1 data from the sample.
func (ps PartSample) GetAV1() ([][]byte, error) {
	tu, err := av1.BitstreamUnmarshal(ps.Payload, true)
	if err != nil {
		return nil, err
	}

	return tu, nil
}

// GetH26x gets H26x data from the sample.
func (ps PartSample) GetH26x() ([][]byte, error) {
	au, err := h264.AVCCUnmarshal(ps.Payload)
	if err != nil {
		return nil, err
	}

	return au, nil
}
