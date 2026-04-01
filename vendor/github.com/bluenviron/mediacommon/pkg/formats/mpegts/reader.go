package mpegts

import (
	"context"
	"errors"
	"fmt"
	"io"
	"strings"

	"github.com/asticode/go-astits"

	"github.com/bluenviron/mediacommon/pkg/codecs/ac3"
	"github.com/bluenviron/mediacommon/pkg/codecs/h264"
	"github.com/bluenviron/mediacommon/pkg/codecs/h265"
	"github.com/bluenviron/mediacommon/pkg/codecs/mpeg1audio"
	"github.com/bluenviron/mediacommon/pkg/codecs/mpeg4audio"
)

// ReaderOnDecodeErrorFunc is the prototype of the callback passed to OnDecodeError.
type ReaderOnDecodeErrorFunc func(err error)

// ReaderOnDataH26xFunc is the prototype of the callback passed to OnDataH26x.
type ReaderOnDataH26xFunc func(pts int64, dts int64, au [][]byte) error

// ReaderOnDataMPEGxVideoFunc is the prototype of the callback passed to OnDataMPEGxVideo.
type ReaderOnDataMPEGxVideoFunc func(pts int64, frame []byte) error

// ReaderOnDataOpusFunc is the prototype of the callback passed to OnDataOpus.
type ReaderOnDataOpusFunc func(pts int64, packets [][]byte) error

// ReaderOnDataMPEG4AudioFunc is the prototype of the callback passed to OnDataMPEG4Audio.
type ReaderOnDataMPEG4AudioFunc func(pts int64, aus [][]byte) error

// ReaderOnDataMPEG1AudioFunc is the prototype of the callback passed to OnDataMPEG1Audio.
type ReaderOnDataMPEG1AudioFunc func(pts int64, frames [][]byte) error

// ReaderOnDataAC3Func is the prototype of the callback passed to OnDataAC3.
type ReaderOnDataAC3Func func(pts int64, frame []byte) error

func findPMT(dem *astits.Demuxer) (*astits.PMTData, error) {
	for {
		data, err := dem.NextData()
		if err != nil {
			return nil, err
		}

		if data.PMT != nil {
			return data.PMT, nil
		}
	}
}

// Reader is a MPEG-TS reader.
type Reader struct {
	tracks        []*Track
	dem           *astits.Demuxer
	onDecodeError ReaderOnDecodeErrorFunc
	onData        map[uint16]func(int64, int64, []byte) error
}

// NewReader allocates a Reader.
func NewReader(br io.Reader) (*Reader, error) {
	rr := &recordReader{r: br}

	dem := astits.NewDemuxer(
		context.Background(),
		rr,
		astits.DemuxerOptPacketSize(188))

	pmt, err := findPMT(dem)
	if err != nil {
		return nil, err
	}

	var tracks []*Track //nolint:prealloc

	for _, es := range pmt.ElementaryStreams {
		var track Track
		err := track.unmarshal(dem, es)
		if err != nil {
			return nil, err
		}

		tracks = append(tracks, &track)
	}

	// rewind demuxer
	dem = astits.NewDemuxer(
		context.Background(),
		&playbackReader{r: br, buf: rr.buf},
		astits.DemuxerOptPacketSize(188))

	return &Reader{
		tracks:        tracks,
		dem:           dem,
		onDecodeError: func(error) {},
		onData:        make(map[uint16]func(int64, int64, []byte) error),
	}, nil
}

// Tracks returns detected tracks.
func (r *Reader) Tracks() []*Track {
	return r.tracks
}

// OnDecodeError sets a callback that is called when a non-fatal decode error occurs.
func (r *Reader) OnDecodeError(cb ReaderOnDecodeErrorFunc) {
	r.onDecodeError = cb
}

// OnDataH26x sets a callback that is called when data from an H265 or H264 track is received.
//
// Deprecated: replaced by OnDataH264, OnDataH265
func (r *Reader) OnDataH26x(track *Track, cb ReaderOnDataH26xFunc) {
	if _, ok := track.Codec.(*CodecH265); ok {
		r.OnDataH265(track, cb)
	} else {
		r.OnDataH264(track, cb)
	}
}

// OnDataH265 sets a callback that is called when data from an H265 track is received.
func (r *Reader) OnDataH265(track *Track, cb ReaderOnDataH26xFunc) {
	r.onData[track.PID] = func(pts int64, dts int64, data []byte) error {
		au, err := h264.AnnexBUnmarshal(data)
		if err != nil {
			r.onDecodeError(err)
			return nil
		}

		if au[0][0] == byte(h265.NALUType_AUD_NUT<<1) {
			au = au[1:]
		}

		return cb(pts, dts, au)
	}
}

// OnDataH264 sets a callback that is called when data from an H264 track is received.
func (r *Reader) OnDataH264(track *Track, cb ReaderOnDataH26xFunc) {
	r.onData[track.PID] = func(pts int64, dts int64, data []byte) error {
		au, err := h264.AnnexBUnmarshal(data)
		if err != nil {
			r.onDecodeError(err)
			return nil
		}

		if au[0][0] == byte(h264.NALUTypeAccessUnitDelimiter) {
			au = au[1:]
		}

		return cb(pts, dts, au)
	}
}

// OnDataMPEGxVideo sets a callback that is called when data from an MPEG-1/2/4 Video track is received.
func (r *Reader) OnDataMPEGxVideo(track *Track, cb ReaderOnDataMPEGxVideoFunc) {
	r.onData[track.PID] = func(pts int64, _ int64, data []byte) error {
		return cb(pts, data)
	}
}

// OnDataOpus sets a callback that is called when data from an Opus track is received.
func (r *Reader) OnDataOpus(track *Track, cb ReaderOnDataOpusFunc) {
	r.onData[track.PID] = func(pts int64, dts int64, data []byte) error {
		if pts != dts {
			r.onDecodeError(fmt.Errorf("PTS is not equal to DTS"))
			return nil
		}

		pos := 0
		var packets [][]byte

		for {
			var au opusAccessUnit
			n, err := au.unmarshal(data[pos:])
			if err != nil {
				r.onDecodeError(err)
				return nil
			}
			pos += n

			packets = append(packets, au.Packet)

			if len(data[pos:]) == 0 {
				break
			}
		}

		return cb(pts, packets)
	}
}

// OnDataMPEG4Audio sets a callback that is called when data from an MPEG-4 Audio track is received.
func (r *Reader) OnDataMPEG4Audio(track *Track, cb ReaderOnDataMPEG4AudioFunc) {
	r.onData[track.PID] = func(pts int64, dts int64, data []byte) error {
		if pts != dts {
			r.onDecodeError(fmt.Errorf("PTS is not equal to DTS"))
			return nil
		}

		var pkts mpeg4audio.ADTSPackets
		err := pkts.Unmarshal(data)
		if err != nil {
			r.onDecodeError(fmt.Errorf("invalid ADTS: %w", err))
			return nil
		}

		aus := make([][]byte, len(pkts))
		for i, pkt := range pkts {
			aus[i] = pkt.AU
		}

		return cb(pts, aus)
	}
}

// OnDataMPEG1Audio sets a callback that is called when data from an MPEG-1 Audio track is received.
func (r *Reader) OnDataMPEG1Audio(track *Track, cb ReaderOnDataMPEG1AudioFunc) {
	r.onData[track.PID] = func(pts int64, dts int64, data []byte) error {
		if pts != dts {
			r.onDecodeError(fmt.Errorf("PTS is not equal to DTS"))
			return nil
		}

		var frames [][]byte

		for len(data) > 0 {
			var h mpeg1audio.FrameHeader
			err := h.Unmarshal(data)
			if err != nil {
				r.onDecodeError(err)
				return nil
			}

			fl := h.FrameLen()
			if len(data) < fl {
				r.onDecodeError(fmt.Errorf("buffer is too short"))
				return nil
			}

			var frame []byte
			frame, data = data[:fl], data[fl:]

			frames = append(frames, frame)
		}

		return cb(pts, frames)
	}
}

// OnDataAC3 sets a callback that is called when data from an AC-3 track is received.
func (r *Reader) OnDataAC3(track *Track, cb ReaderOnDataAC3Func) {
	r.onData[track.PID] = func(pts int64, dts int64, data []byte) error {
		if pts != dts {
			r.onDecodeError(fmt.Errorf("PTS is not equal to DTS"))
			return nil
		}

		var syncInfo ac3.SyncInfo
		err := syncInfo.Unmarshal(data)
		if err != nil {
			r.onDecodeError(err)
			return nil
		}
		size := syncInfo.FrameSize()

		if size != len(data) {
			r.onDecodeError(fmt.Errorf("unexpected frame size: got %d, expected %d", len(data), size))
			return nil
		}

		return cb(pts, data)
	}
}

// Read reads data.
func (r *Reader) Read() error {
	for {
		data, err := r.dem.NextData()
		if err != nil {
			// https://github.com/asticode/go-astits/blob/b0b19247aa31633650c32638fb55f597fa6e2468/packet_buffer.go#L133C1-L133C5
			if errors.Is(err, astits.ErrNoMorePackets) || strings.Contains(err.Error(), "astits: reading ") {
				return err
			}
			r.onDecodeError(err)
			continue
		}

		if data.PES == nil {
			return nil
		}

		if data.PES.Header.OptionalHeader == nil ||
			data.PES.Header.OptionalHeader.PTSDTSIndicator == astits.PTSDTSIndicatorNoPTSOrDTS ||
			data.PES.Header.OptionalHeader.PTSDTSIndicator == astits.PTSDTSIndicatorIsForbidden {
			r.onDecodeError(fmt.Errorf("PTS is missing"))
			return nil
		}

		pts := data.PES.Header.OptionalHeader.PTS.Base

		var dts int64
		if data.PES.Header.OptionalHeader.PTSDTSIndicator == astits.PTSDTSIndicatorBothPresent {
			dts = data.PES.Header.OptionalHeader.DTS.Base
		} else {
			dts = pts
		}

		onData, ok := r.onData[data.PID]
		if !ok {
			return nil
		}

		return onData(pts, dts, data.PES.Data)
	}
}
