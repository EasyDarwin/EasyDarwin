package mpegts

import (
	"bytes"
	"context"
	"fmt"
	"io"

	"github.com/asticode/go-astits"

	"github.com/bluenviron/mediacommon/pkg/codecs/h264"
	"github.com/bluenviron/mediacommon/pkg/codecs/h265"
	"github.com/bluenviron/mediacommon/pkg/codecs/mpeg1audio"
	"github.com/bluenviron/mediacommon/pkg/codecs/mpeg4audio"
	"github.com/bluenviron/mediacommon/pkg/codecs/mpeg4video"
)

const (
	streamIDVideo = 224
	streamIDAudio = 192

	// PCR is needed to read H265 tracks with VLC+VDPAU hardware encoder
	// (and is probably needed by other combinations too)
	dtsPCRDiff = (90000 / 10)
)

func opusMarshalSize(packets [][]byte) int {
	n := 0
	for _, packet := range packets {
		au := opusAccessUnit{
			ControlHeader: opusControlHeader{
				PayloadSize: len(packet),
			},
			Packet: packet,
		}
		n += au.marshalSize()
	}
	return n
}

func mpeg1AudioMarshalSize(frames [][]byte) int {
	n := 0
	for _, frame := range frames {
		n += len(frame)
	}
	return n
}

// Writer is a MPEG-TS writer.
type Writer struct {
	nextPID            uint16
	mux                *astits.Muxer
	pcrCounter         int
	leadingTrackChosen bool
}

// NewWriter allocates a Writer.
func NewWriter(
	bw io.Writer,
	tracks []*Track,
) *Writer {
	w := &Writer{
		nextPID: 256,
	}

	w.mux = astits.NewMuxer(
		context.Background(),
		bw)

	for _, track := range tracks {
		if track.PID == 0 {
			track.PID = w.nextPID
			w.nextPID++
		}
		es, _ := track.marshal()

		err := w.mux.AddElementaryStream(*es)
		if err != nil {
			panic(err) // TODO: return error instead of panicking
		}
	}

	// WriteTables() is not necessary
	// since it's called automatically when WriteData() is called with
	// * PID == PCRPID
	// * AdaptationField != nil
	// * RandomAccessIndicator = true

	return w
}

// WriteH26x writes a H26x access unit.
//
// Deprecated: replaced by WriteH264 and WriteH265.
func (w *Writer) WriteH26x(
	track *Track,
	pts int64,
	dts int64,
	randomAccess bool,
	au [][]byte,
) error {
	if _, ok := track.Codec.(*CodecH265); ok {
		return w.WriteH265(track, pts, dts, randomAccess, au)
	}
	return w.WriteH264(track, pts, dts, randomAccess, au)
}

// WriteH265 writes a H265 access unit.
func (w *Writer) WriteH265(
	track *Track,
	pts int64,
	dts int64,
	randomAccess bool,
	au [][]byte,
) error {
	// prepend an AUD. This is required by video.js, iOS, QuickTime
	if au[0][0] != byte(h265.NALUType_AUD_NUT<<1) {
		au = append([][]byte{
			{byte(h265.NALUType_AUD_NUT) << 1, 1, 0x50},
		}, au...)
	}

	enc, err := h264.AnnexBMarshal(au)
	if err != nil {
		return err
	}

	return w.writeVideo(track, pts, dts, randomAccess, enc)
}

// WriteH264 writes a H264 access unit.
func (w *Writer) WriteH264(
	track *Track,
	pts int64,
	dts int64,
	randomAccess bool,
	au [][]byte,
) error {
	// prepend an AUD. This is required by video.js, iOS, QuickTime
	if au[0][0] != byte(h264.NALUTypeAccessUnitDelimiter) {
		au = append([][]byte{
			{byte(h264.NALUTypeAccessUnitDelimiter), 240},
		}, au...)
	}

	enc, err := h264.AnnexBMarshal(au)
	if err != nil {
		return err
	}

	return w.writeVideo(track, pts, dts, randomAccess, enc)
}

// WriteMPEG4Video writes a MPEG-4 Video frame.
func (w *Writer) WriteMPEG4Video(
	track *Track,
	pts int64,
	frame []byte,
) error {
	randomAccess := bytes.Contains(frame, []byte{0, 0, 1, byte(mpeg4video.GroupOfVOPStartCode)})

	return w.writeVideo(track, pts, pts, randomAccess, frame)
}

// WriteMPEG1Video writes a MPEG-1/2 Video frame.
func (w *Writer) WriteMPEG1Video(
	track *Track,
	pts int64,
	frame []byte,
) error {
	randomAccess := bytes.Contains(frame, []byte{0, 0, 1, 0xB8})

	return w.writeVideo(track, pts, pts, randomAccess, frame)
}

// WriteOpus writes Opus packets.
func (w *Writer) WriteOpus(
	track *Track,
	pts int64,
	packets [][]byte,
) error {
	enc := make([]byte, opusMarshalSize(packets))
	n := 0
	for _, packet := range packets {
		au := opusAccessUnit{
			ControlHeader: opusControlHeader{
				PayloadSize: len(packet),
			},
			Packet: packet,
		}
		mn, err := au.marshalTo(enc[n:])
		if err != nil {
			return err
		}
		n += mn
	}

	return w.writeAudio(track, pts, enc)
}

// WriteMPEG4Audio writes MPEG-4 Audio access units.
func (w *Writer) WriteMPEG4Audio(
	track *Track,
	pts int64,
	aus [][]byte,
) error {
	aacCodec := track.Codec.(*CodecMPEG4Audio)
	pkts := make(mpeg4audio.ADTSPackets, len(aus))

	for i, au := range aus {
		pkts[i] = &mpeg4audio.ADTSPacket{
			Type:         aacCodec.Config.Type,
			SampleRate:   aacCodec.SampleRate,
			ChannelCount: aacCodec.Config.ChannelCount,
			AU:           au,
		}
	}

	enc, err := pkts.Marshal()
	if err != nil {
		return err
	}

	return w.writeAudio(track, pts, enc)
}

// WriteMPEG1Audio writes MPEG-1 Audio packets.
func (w *Writer) WriteMPEG1Audio(
	track *Track,
	pts int64,
	frames [][]byte,
) error {
	if !track.mp3Checked {
		var h mpeg1audio.FrameHeader
		err := h.Unmarshal(frames[0])
		if err != nil {
			return err
		}

		if h.MPEG2 {
			return fmt.Errorf("only MPEG-1 audio is supported")
		}

		track.mp3Checked = true
	}

	enc := make([]byte, mpeg1AudioMarshalSize(frames))
	n := 0
	for _, frame := range frames {
		n += copy(enc[n:], frame)
	}

	return w.writeAudio(track, pts, enc)
}

// WriteAC3 writes a AC-3 frame.
func (w *Writer) WriteAC3(
	track *Track,
	pts int64,
	frame []byte,
) error {
	return w.writeAudio(track, pts, frame)
}

func (w *Writer) writeVideo(
	track *Track,
	pts int64,
	dts int64,
	randomAccess bool,
	data []byte,
) error {
	if !w.leadingTrackChosen {
		w.leadingTrackChosen = true
		track.isLeading = true
		w.mux.SetPCRPID(track.PID)
	}

	var af *astits.PacketAdaptationField

	if randomAccess {
		af = &astits.PacketAdaptationField{}
		af.RandomAccessIndicator = true
	}

	if track.isLeading {
		if randomAccess || w.pcrCounter == 0 {
			if af == nil {
				af = &astits.PacketAdaptationField{}
			}
			af.HasPCR = true
			af.PCR = &astits.ClockReference{Base: dts - dtsPCRDiff}
			w.pcrCounter = 3
		}
		w.pcrCounter--
	}

	oh := &astits.PESOptionalHeader{
		MarkerBits: 2,
	}

	if dts == pts {
		oh.PTSDTSIndicator = astits.PTSDTSIndicatorOnlyPTS
		oh.PTS = &astits.ClockReference{Base: pts}
	} else {
		oh.PTSDTSIndicator = astits.PTSDTSIndicatorBothPresent
		oh.DTS = &astits.ClockReference{Base: dts}
		oh.PTS = &astits.ClockReference{Base: pts}
	}

	_, err := w.mux.WriteData(&astits.MuxerData{
		PID:             track.PID,
		AdaptationField: af,
		PES: &astits.PESData{
			Header: &astits.PESHeader{
				OptionalHeader: oh,
				StreamID:       streamIDVideo,
			},
			Data: data,
		},
	})
	return err
}

func (w *Writer) writeAudio(track *Track, pts int64, data []byte) error {
	if !w.leadingTrackChosen {
		w.leadingTrackChosen = true
		track.isLeading = true
		w.mux.SetPCRPID(track.PID)
	}

	af := &astits.PacketAdaptationField{
		RandomAccessIndicator: true,
	}

	if track.isLeading {
		if w.pcrCounter == 0 {
			af.HasPCR = true
			af.PCR = &astits.ClockReference{Base: pts - dtsPCRDiff}
			w.pcrCounter = 3
		}
		w.pcrCounter--
	}

	_, err := w.mux.WriteData(&astits.MuxerData{
		PID:             track.PID,
		AdaptationField: af,
		PES: &astits.PESData{
			Header: &astits.PESHeader{
				OptionalHeader: &astits.PESOptionalHeader{
					MarkerBits:      2,
					PTSDTSIndicator: astits.PTSDTSIndicatorOnlyPTS,
					PTS:             &astits.ClockReference{Base: pts},
				},
				StreamID: streamIDAudio,
			},
			Data: data,
		},
	})
	return err
}
