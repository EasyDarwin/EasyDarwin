package gohlslib

import (
	"bytes"
	"crypto/rand"
	"encoding/hex"
	"fmt"
	"net/http"
	"time"

	"github.com/bluenviron/mediacommon/pkg/codecs/av1"
	"github.com/bluenviron/mediacommon/pkg/codecs/h264"
	"github.com/bluenviron/mediacommon/pkg/codecs/h265"
	"github.com/bluenviron/mediacommon/pkg/codecs/vp9"

	"github.com/bluenviron/gohlslib/pkg/codecs"
	"github.com/bluenviron/gohlslib/pkg/storage"
)

// a prefix is needed to prevent usage of cached segments
// from previous muxing sessions.
func generatePrefix() (string, error) {
	var buf [6]byte
	_, err := rand.Read(buf[:])
	if err != nil {
		return "", err
	}

	return hex.EncodeToString(buf[:]), nil
}

// MuxerVariant is a muxer variant.
type MuxerVariant int

// supported variants.
const (
	MuxerVariantMPEGTS MuxerVariant = iota + 1
	MuxerVariantFMP4
	MuxerVariantLowLatency
)

// Muxer is a HLS muxer.
type Muxer struct {
	//
	// parameters (all optional except VideoTrack or AudioTrack).
	//
	// video track.
	VideoTrack *Track
	// audio track.
	AudioTrack *Track
	// Variant to use.
	// It defaults to MuxerVariantLowLatency
	Variant MuxerVariant
	// Number of HLS segments to keep on the server.
	// Segments allow to seek through the stream.
	// Their number doesn't influence latency.
	// It defaults to 7.
	SegmentCount int
	// Minimum duration of each segment.
	// This is adjusted in order to include at least one IDR frame in each segment.
	// A player usually puts 3 segments in a buffer before reproducing the stream.
	// It defaults to 1sec.
	SegmentMinDuration time.Duration
	// Minimum duration of each part.
	// Parts are used in Low-Latency HLS in place of segments.
	// This is adjusted in order to produce segments with a similar duration.
	// A player usually puts 3 parts in a buffer before reproducing the stream.
	// It defaults to 200ms.
	PartMinDuration time.Duration
	// Maximum size of each segment.
	// This prevents RAM exhaustion.
	// It defaults to 50MB.
	SegmentMaxSize uint64
	// Directory in which to save segments.
	// This decreases performance, since saving segments on disk is less performant
	// than saving them on RAM, but allows to preserve RAM.
	Directory string

	// Deprecated: replaced with SegmentMinDuration
	SegmentDuration time.Duration
	// Deprecated: replaced with PartMinDuration
	PartDuration time.Duration

	//
	// private
	//

	prefix         string
	storageFactory storage.Factory
	server         *muxerServer
	segmenter      muxerSegmenter
	forceSwitch    bool
}

// Start initializes the muxer.
func (m *Muxer) Start() error {
	if m.Variant == 0 {
		m.Variant = MuxerVariantLowLatency
	}
	if m.SegmentCount == 0 {
		m.SegmentCount = 7
	}
	if m.SegmentDuration != 0 {
		m.SegmentMinDuration = m.SegmentDuration
	}
	if m.SegmentMinDuration == 0 {
		m.SegmentMinDuration = 1 * time.Second
	}
	if m.PartDuration != 0 {
		m.PartMinDuration = m.PartDuration
	}
	if m.PartMinDuration == 0 {
		m.PartMinDuration = 200 * time.Millisecond
	}
	if m.SegmentMaxSize == 0 {
		m.SegmentMaxSize = 50 * 1024 * 1024
	}

	switch m.Variant {
	case MuxerVariantLowLatency:
		if m.SegmentCount < 7 {
			return fmt.Errorf("Low-Latency HLS requires at least 7 segments")
		}

	default:
		if m.SegmentCount < 3 {
			return fmt.Errorf("The minimum number of HLS segments is 3")
		}
	}

	if m.Variant == MuxerVariantMPEGTS {
		if m.VideoTrack != nil {
			if _, ok := m.VideoTrack.Codec.(*codecs.H264); !ok {
				return fmt.Errorf(
					"the MPEG-TS variant of HLS only supports H264 video. Use the fMP4 or Low-Latency variants instead")
			}
		}

		if m.AudioTrack != nil {
			if _, ok := m.AudioTrack.Codec.(*codecs.MPEG4Audio); !ok {
				return fmt.Errorf(
					"the MPEG-TS variant of HLS only supports MPEG-4 Audio. Use the fMP4 or Low-Latency variants instead")
			}
		}
	}

	var err error
	m.prefix, err = generatePrefix()
	if err != nil {
		return err
	}

	if m.Directory != "" {
		m.storageFactory = storage.NewFactoryDisk(m.Directory)
	} else {
		m.storageFactory = storage.NewFactoryRAM()
	}

	m.server = &muxerServer{
		variant:      m.Variant,
		segmentCount: m.SegmentCount,
		videoTrack:   m.VideoTrack,
		audioTrack:   m.AudioTrack,
		prefix:       m.prefix,
	}
	m.server.initialize()

	if m.Variant == MuxerVariantMPEGTS {
		m.segmenter = &muxerSegmenterMPEGTS{
			segmentMinDuration: m.SegmentMinDuration,
			segmentMaxSize:     m.SegmentMaxSize,
			videoTrack:         m.VideoTrack,
			audioTrack:         m.AudioTrack,
			prefix:             m.prefix,
			factory:            m.storageFactory,
			publishSegment:     m.server.publishSegment,
		}
		m.segmenter.initialize()
	} else {
		m.segmenter = &muxerSegmenterFMP4{
			lowLatency:         m.Variant == MuxerVariantLowLatency,
			segmentMinDuration: m.SegmentMinDuration,
			partMinDuration:    m.PartMinDuration,
			segmentMaxSize:     m.SegmentMaxSize,
			videoTrack:         m.VideoTrack,
			audioTrack:         m.AudioTrack,
			prefix:             m.prefix,
			factory:            m.storageFactory,
			publishSegment:     m.server.publishSegment,
			publishPart:        m.server.publishPart,
		}
		m.segmenter.initialize()
	}

	return nil
}

// Close closes a Muxer.
func (m *Muxer) Close() {
	m.server.close()
	m.segmenter.close()
}

// WriteAV1 writes an AV1 temporal unit.
func (m *Muxer) WriteAV1(ntp time.Time, pts time.Duration, tu [][]byte) error {
	codec := m.VideoTrack.Codec.(*codecs.AV1)
	randomAccess := false

	for _, obu := range tu {
		var h av1.OBUHeader
		err := h.Unmarshal(obu)
		if err != nil {
			return err
		}

		if h.Type == av1.OBUTypeSequenceHeader {
			randomAccess = true

			if !bytes.Equal(codec.SequenceHeader, obu) {
				m.forceSwitch = true
				codec.SequenceHeader = obu
			}
		}
	}

	forceSwitch := false
	if randomAccess && m.forceSwitch {
		m.forceSwitch = false
		forceSwitch = true
	}

	return m.segmenter.writeAV1(ntp, pts, tu, randomAccess, forceSwitch)
}

// WriteVP9 writes a VP9 frame.
func (m *Muxer) WriteVP9(ntp time.Time, pts time.Duration, frame []byte) error {
	var h vp9.Header
	err := h.Unmarshal(frame)
	if err != nil {
		return err
	}

	codec := m.VideoTrack.Codec.(*codecs.VP9)
	randomAccess := false

	if !h.NonKeyFrame {
		randomAccess = true

		if v := h.Width(); v != codec.Width {
			m.forceSwitch = true
			codec.Width = v
		}
		if v := h.Height(); v != codec.Height {
			m.forceSwitch = true
			codec.Height = v
		}
		if h.Profile != codec.Profile {
			m.forceSwitch = true
			codec.Profile = h.Profile
		}
		if h.ColorConfig.BitDepth != codec.BitDepth {
			m.forceSwitch = true
			codec.BitDepth = h.ColorConfig.BitDepth
		}
		if v := h.ChromaSubsampling(); v != codec.ChromaSubsampling {
			m.forceSwitch = true
			codec.ChromaSubsampling = v
		}
		if h.ColorConfig.ColorRange != codec.ColorRange {
			m.forceSwitch = true
			codec.ColorRange = h.ColorConfig.ColorRange
		}
	}

	forceSwitch := false
	if randomAccess && m.forceSwitch {
		m.forceSwitch = false
		forceSwitch = true
	}

	return m.segmenter.writeVP9(ntp, pts, frame, randomAccess, forceSwitch)
}

// WriteH26x writes an H264 or an H265 access unit.
//
// Deprecated: replaced by WriteH264 and WriteH265.
func (m *Muxer) WriteH26x(ntp time.Time, pts time.Duration, au [][]byte) error {
	if _, ok := m.VideoTrack.Codec.(*codecs.H265); ok {
		return m.WriteH265(ntp, pts, au)
	}

	return m.WriteH264(ntp, pts, au)
}

// WriteH265 writes an H265 access unit.
func (m *Muxer) WriteH265(ntp time.Time, pts time.Duration, au [][]byte) error {
	randomAccess := false
	codec := m.VideoTrack.Codec.(*codecs.H265)

	for _, nalu := range au {
		typ := h265.NALUType((nalu[0] >> 1) & 0b111111)

		switch typ {
		case h265.NALUType_IDR_W_RADL, h265.NALUType_IDR_N_LP, h265.NALUType_CRA_NUT:
			randomAccess = true

		case h265.NALUType_VPS_NUT:
			if !bytes.Equal(codec.VPS, nalu) {
				m.forceSwitch = true
				codec.VPS = nalu
			}

		case h265.NALUType_SPS_NUT:
			if !bytes.Equal(codec.SPS, nalu) {
				m.forceSwitch = true
				codec.SPS = nalu
			}

		case h265.NALUType_PPS_NUT:
			if !bytes.Equal(codec.PPS, nalu) {
				m.forceSwitch = true
				codec.PPS = nalu
			}
		}
	}

	forceSwitch := false
	if randomAccess && m.forceSwitch {
		m.forceSwitch = false
		forceSwitch = true
	}

	return m.segmenter.writeH26x(ntp, pts, au, randomAccess, forceSwitch)
}

// WriteH264 writes an H264 access unit.
func (m *Muxer) WriteH264(ntp time.Time, pts time.Duration, au [][]byte) error {
	randomAccess := false
	codec := m.VideoTrack.Codec.(*codecs.H264)
	nonIDRPresent := false

	for _, nalu := range au {
		typ := h264.NALUType(nalu[0] & 0x1F)

		switch typ {
		case h264.NALUTypeIDR:
			randomAccess = true

		case h264.NALUTypeNonIDR:
			nonIDRPresent = true

		case h264.NALUTypeSPS:
			if !bytes.Equal(codec.SPS, nalu) {
				m.forceSwitch = true
				codec.SPS = nalu
			}

		case h264.NALUTypePPS:
			if !bytes.Equal(codec.PPS, nalu) {
				m.forceSwitch = true
				codec.PPS = nalu
			}
		}
	}

	if !randomAccess && !nonIDRPresent {
		return nil
	}

	forceSwitch := false
	if randomAccess && m.forceSwitch {
		m.forceSwitch = false
		forceSwitch = true
	}

	return m.segmenter.writeH26x(ntp, pts, au, randomAccess, forceSwitch)
}

// WriteOpus writes Opus packets.
func (m *Muxer) WriteOpus(ntp time.Time, pts time.Duration, packets [][]byte) error {
	return m.segmenter.writeOpus(ntp, pts, packets)
}

// WriteMPEG4Audio writes MPEG-4 Audio access units.
func (m *Muxer) WriteMPEG4Audio(ntp time.Time, pts time.Duration, aus [][]byte) error {
	return m.segmenter.writeMPEG4Audio(ntp, pts, aus)
}

// Handle handles a HTTP request.
func (m *Muxer) Handle(w http.ResponseWriter, r *http.Request) {
	m.server.handle(w, r)
}
