package fmp4

import (
	"github.com/abema/go-mp4"
)

// PartTrack is a track of Part.
type PartTrack struct {
	ID       int
	BaseTime uint64
	Samples  []*PartSample
}

func (pt *PartTrack) marshal(w *mp4Writer) (*mp4.Trun, int, error) {
	/*
		|traf|
		|    |tfhd|
		|    |tfdt|
		|    |trun|
	*/

	_, err := w.writeBoxStart(&mp4.Traf{}) // <traf>
	if err != nil {
		return nil, 0, err
	}

	flags := 0

	_, err = w.writeBox(&mp4.Tfhd{ // <tfhd/>
		FullBox: mp4.FullBox{
			Flags: [3]byte{2, byte(flags >> 8), byte(flags)},
		},
		TrackID: uint32(pt.ID),
	})
	if err != nil {
		return nil, 0, err
	}

	_, err = w.writeBox(&mp4.Tfdt{ // <tfdt/>
		FullBox: mp4.FullBox{
			Version: 1,
		},
		// sum of decode durations of all earlier samples
		BaseMediaDecodeTimeV1: pt.BaseTime,
	})
	if err != nil {
		return nil, 0, err
	}

	flags = trunFlagDataOffsetPreset |
		trunFlagSampleDurationPresent |
		trunFlagSampleSizePresent

	for _, sample := range pt.Samples {
		if sample.IsNonSyncSample {
			flags |= trunFlagSampleFlagsPresent
		}
		if sample.PTSOffset != 0 {
			flags |= trunFlagSampleCompositionTimeOffsetPresentOrV1
		}
	}

	trun := &mp4.Trun{ // <trun/>
		FullBox: mp4.FullBox{
			Version: 1,
			Flags:   [3]byte{0, byte(flags >> 8), byte(flags)},
		},
		SampleCount: uint32(len(pt.Samples)),
	}

	for _, sample := range pt.Samples {
		var flags uint32
		if sample.IsNonSyncSample {
			flags |= sampleFlagIsNonSyncSample
		}

		trun.Entries = append(trun.Entries, mp4.TrunEntry{
			SampleDuration:                sample.Duration,
			SampleSize:                    uint32(len(sample.Payload)),
			SampleFlags:                   flags,
			SampleCompositionTimeOffsetV1: sample.PTSOffset,
		})
	}

	trunOffset, err := w.writeBox(trun)
	if err != nil {
		return nil, 0, err
	}

	err = w.writeBoxEnd() // </traf>
	if err != nil {
		return nil, 0, err
	}

	return trun, trunOffset, nil
}
