package fmp4

import (
	"bytes"
	"fmt"
	"io"

	"github.com/abema/go-mp4"
)

// Parts is a sequence of fMP4 parts.
type Parts []*Part

// Unmarshal decodes one or more fMP4 parts.
func (ps *Parts) Unmarshal(byts []byte) error {
	type readState int

	const (
		waitingMoof readState = iota
		waitingMfhd
		waitingTraf
		waitingTfdtTfhdTrun
	)

	state := waitingMoof
	var curPart *Part
	var moofOffset uint64
	var curTrack *PartTrack
	var tfdt *mp4.Tfdt
	var tfhd *mp4.Tfhd

	_, err := mp4.ReadBoxStructure(bytes.NewReader(byts), func(h *mp4.ReadHandle) (interface{}, error) {
		if h.BoxInfo.IsSupportedType() {
			switch h.BoxInfo.Type.String() {
			case "moof":
				if state != waitingMoof {
					return nil, fmt.Errorf("unexpected moof")
				}

				curPart = &Part{}
				*ps = append(*ps, curPart)
				moofOffset = h.BoxInfo.Offset
				state = waitingMfhd
				return h.Expand()

			case "mfhd":
				if state != waitingMfhd {
					return nil, fmt.Errorf("unexpected mfhd")
				}

				box, _, err := h.ReadPayload()
				if err != nil {
					return nil, err
				}
				mfhd := box.(*mp4.Mfhd)

				curPart.SequenceNumber = mfhd.SequenceNumber
				state = waitingTraf

			case "traf":
				if state != waitingTraf && state != waitingTfdtTfhdTrun {
					return nil, fmt.Errorf("unexpected traf")
				}

				if curTrack != nil {
					if tfdt == nil || tfhd == nil || curTrack.Samples == nil {
						return nil, fmt.Errorf("parse error")
					}
				}

				curTrack = &PartTrack{}
				curPart.Tracks = append(curPart.Tracks, curTrack)
				tfdt = nil
				tfhd = nil
				state = waitingTfdtTfhdTrun
				return h.Expand()

			case "tfhd":
				if state != waitingTfdtTfhdTrun || tfhd != nil {
					return nil, fmt.Errorf("unexpected tfhd")
				}

				box, _, err := h.ReadPayload()
				if err != nil {
					return nil, err
				}
				tfhd = box.(*mp4.Tfhd)

				curTrack.ID = int(tfhd.TrackID)

			case "tfdt":
				if state != waitingTfdtTfhdTrun || tfdt != nil {
					return nil, fmt.Errorf("unexpected tfdt")
				}

				box, _, err := h.ReadPayload()
				if err != nil {
					return nil, err
				}
				tfdt = box.(*mp4.Tfdt)

				if tfdt.FullBox.Version != 1 {
					return nil, fmt.Errorf("unsupported tfdt version")
				}

				curTrack.BaseTime = tfdt.BaseMediaDecodeTimeV1

			case "trun":
				if state != waitingTfdtTfhdTrun || tfhd == nil {
					return nil, fmt.Errorf("unexpected trun")
				}

				// prevent RAM exhaustion due to unlimited Trun unmarshaling
				rawBox := byts[h.BoxInfo.Offset:]
				if len(rawBox) >= 16 {
					sampleCount := uint32(rawBox[12])<<24 | uint32(rawBox[13])<<16 | uint32(rawBox[14])<<8 | uint32(rawBox[15])
					if sampleCount > maxSamplesPerTrun {
						return nil, fmt.Errorf("sample count (%d) exceeds maximum (%d)", sampleCount, maxSamplesPerTrun)
					}
				}

				box, _, err := h.ReadPayload()
				if err != nil {
					return nil, err
				}
				trun := box.(*mp4.Trun)

				trunFlags := uint16(trun.Flags[1])<<8 | uint16(trun.Flags[2])
				if (trunFlags & trunFlagDataOffsetPreset) == 0 {
					return nil, fmt.Errorf("unsupported flags")
				}

				existing := len(curTrack.Samples)
				tmp := make([]*PartSample, existing+len(trun.Entries))
				copy(tmp, curTrack.Samples)
				curTrack.Samples = tmp

				pos := uint64(trun.DataOffset) + moofOffset
				if uint64(len(byts)) < pos {
					return nil, fmt.Errorf("invalid data_offset / moof_offset")
				}

				ptr := byts[pos:]

				for i, e := range trun.Entries {
					s := &PartSample{}

					if (trunFlags & trunFlagSampleDurationPresent) != 0 {
						s.Duration = e.SampleDuration
					} else {
						s.Duration = tfhd.DefaultSampleDuration
					}

					s.PTSOffset = e.SampleCompositionTimeOffsetV1

					var sampleFlags uint32
					if (trunFlags & trunFlagSampleFlagsPresent) != 0 {
						sampleFlags = e.SampleFlags
					} else {
						sampleFlags = tfhd.DefaultSampleFlags
					}
					s.IsNonSyncSample = ((sampleFlags & sampleFlagIsNonSyncSample) != 0)

					var size uint32
					if (trunFlags & trunFlagSampleSizePresent) != 0 {
						size = e.SampleSize
					} else {
						size = tfhd.DefaultSampleSize
					}

					if len(ptr) < int(size) {
						return nil, fmt.Errorf("invalid sample size")
					}

					s.Payload = ptr[:size]
					ptr = ptr[size:]

					curTrack.Samples[existing+i] = s
				}

			case "mdat":
				if state != waitingTraf && state != waitingTfdtTfhdTrun {
					return nil, fmt.Errorf("unexpected mdat")
				}

				if curTrack != nil {
					if tfdt == nil || tfhd == nil || curTrack.Samples == nil {
						return nil, fmt.Errorf("parse error")
					}
				}

				state = waitingMoof
			}
		}

		return nil, nil
	})
	if err != nil {
		return err
	}

	if state != waitingMoof {
		return fmt.Errorf("decode error")
	}

	return nil
}

// Marshal encodes a one or more fMP4 part.
func (ps *Parts) Marshal(w io.WriteSeeker) error {
	for _, p := range *ps {
		err := p.Marshal(w)
		if err != nil {
			return err
		}
	}
	return nil
}
