package rtptime

import (
	"sync"
	"time"

	"github.com/pion/rtp"
)

var timeNow = time.Now

// avoid an int64 overflow and preserve resolution by splitting division into two parts:
// first add the integer part, then the decimal part.
func multiplyAndDivide(v, m, d time.Duration) time.Duration {
	secs := v / d
	dec := v % d
	return (secs*m + dec*m/d)
}

type globalDecoderTrackData struct {
	startPTS  time.Duration
	clockRate time.Duration
	overall   time.Duration
	prev      uint32
}

func newGlobalDecoderTrackData(
	startPTS time.Duration,
	clockRate int,
	startTimestamp uint32,
) *globalDecoderTrackData {
	return &globalDecoderTrackData{
		startPTS:  startPTS,
		clockRate: time.Duration(clockRate),
		prev:      startTimestamp,
	}
}

func (d *globalDecoderTrackData) decode(ts uint32) time.Duration {
	diff := int32(ts - d.prev)
	d.prev = ts
	d.overall += time.Duration(diff)

	return d.startPTS + multiplyAndDivide(d.overall, time.Second, d.clockRate)
}

// GlobalDecoderTrack is a track (RTSP format or WebRTC track) of a GlobalDecoder.
type GlobalDecoderTrack interface {
	ClockRate() int
	PTSEqualsDTS(*rtp.Packet) bool
}

// GlobalDecoder is a RTP timestamp decoder.
type GlobalDecoder struct {
	mutex        sync.Mutex
	leadingTrack GlobalDecoderTrack
	startNTP     time.Time
	startPTS     time.Duration
	tracks       map[GlobalDecoderTrack]*globalDecoderTrackData
}

// NewGlobalDecoder allocates a GlobalDecoder.
func NewGlobalDecoder() *GlobalDecoder {
	return &GlobalDecoder{
		tracks: make(map[GlobalDecoderTrack]*globalDecoderTrackData),
	}
}

// Decode decodes a timestamp.
func (d *GlobalDecoder) Decode(
	track GlobalDecoderTrack,
	pkt *rtp.Packet,
) (time.Duration, bool) {
	if track.ClockRate() == 0 {
		return 0, false
	}

	d.mutex.Lock()
	defer d.mutex.Unlock()

	df, ok := d.tracks[track]

	// track never seen before
	if !ok {
		if !track.PTSEqualsDTS(pkt) {
			return 0, false
		}

		now := timeNow()

		if d.leadingTrack == nil {
			d.leadingTrack = track
			d.startNTP = now
			d.startPTS = 0
		}

		df = newGlobalDecoderTrackData(
			d.startPTS+now.Sub(d.startNTP),
			track.ClockRate(),
			pkt.Timestamp)

		d.tracks[track] = df

		return df.startPTS, true
	}

	// update startNTP / startPTS
	if d.leadingTrack == track && track.PTSEqualsDTS(pkt) {
		pts := df.decode(pkt.Timestamp)

		now := timeNow()
		d.startNTP = now
		d.startPTS = pts

		return pts, true
	}

	return df.decode(pkt.Timestamp), true
}
