package gohlslib

import (
	"io"
	"math"
	"net/http"
	"net/url"
	"path/filepath"
	"strconv"
	"strings"
	"sync"
	"time"

	"github.com/bluenviron/mediacommon/pkg/codecs/av1"
	"github.com/bluenviron/mediacommon/pkg/codecs/h264"
	"github.com/bluenviron/mediacommon/pkg/codecs/h265"
	"github.com/bluenviron/mediacommon/pkg/formats/fmp4"
	"github.com/bluenviron/mediacommon/pkg/formats/fmp4/seekablebuffer"

	"github.com/bluenviron/gohlslib/pkg/codecparams"
	"github.com/bluenviron/gohlslib/pkg/codecs"
	"github.com/bluenviron/gohlslib/pkg/playlist"
)

const (
	multivariantPlaylistMaxAge = "30"
	initMaxAge                 = "30"
	segmentMaxAge              = "3600"
)

func boolPtr(v bool) *bool {
	return &v
}

func targetDuration(segments []muxerSegment) int {
	ret := int(0)

	// EXTINF, when rounded to the nearest integer, must be <= EXT-X-TARGETDURATION
	for _, sog := range segments {
		v := int(math.Round(sog.getDuration().Seconds()))
		if v > ret {
			ret = v
		}
	}

	return ret
}

func partTargetDuration(
	segments []muxerSegment,
	nextSegmentParts []*muxerPart,
) time.Duration {
	var ret time.Duration

	for _, sog := range segments {
		seg, ok := sog.(*muxerSegmentFMP4)
		if !ok {
			continue
		}

		for _, part := range seg.parts {
			if part.finalDuration > ret {
				ret = part.finalDuration
			}
		}
	}

	for _, part := range nextSegmentParts {
		if part.finalDuration > ret {
			ret = part.finalDuration
		}
	}

	// round to milliseconds to avoid changes, that are illegal on iOS
	return time.Millisecond * time.Duration(math.Ceil(float64(ret)/float64(time.Millisecond)))
}

func parseMSNPart(msn string, part string) (uint64, uint64, error) {
	var msnint uint64
	if msn != "" {
		var err error
		msnint, err = strconv.ParseUint(msn, 10, 64)
		if err != nil {
			return 0, 0, err
		}
	}

	var partint uint64
	if part != "" {
		var err error
		partint, err = strconv.ParseUint(part, 10, 64)
		if err != nil {
			return 0, 0, err
		}
	}

	return msnint, partint, nil
}

func bandwidth(segments []muxerSegment) (int, int) {
	if len(segments) == 0 {
		return 0, 0
	}

	var maxBandwidth uint64
	var sizes uint64
	var durations time.Duration

	for _, seg := range segments {
		if _, ok := seg.(*muxerGap); !ok {
			bandwidth := 8 * seg.getSize() * uint64(time.Second) / uint64(seg.getDuration())
			if bandwidth > maxBandwidth {
				maxBandwidth = bandwidth
			}
			sizes += seg.getSize()
			durations += seg.getDuration()
		}
	}

	averageBandwidth := 8 * sizes * uint64(time.Second) / uint64(durations)

	return int(maxBandwidth), int(averageBandwidth)
}

func generateMultivariantPlaylist(
	variant MuxerVariant,
	videoTrack *Track,
	audioTrack *Track,
	segments []muxerSegment,
	rawQuery string,
) ([]byte, error) {
	maxBandwidth, averageBandwidth := bandwidth(segments)
	var resolution string
	var frameRate *float64

	if videoTrack != nil {
		switch codec := videoTrack.Codec.(type) {
		case *codecs.AV1:
			var sh av1.SequenceHeader
			err := sh.Unmarshal(codec.SequenceHeader)
			if err != nil {
				return nil, err
			}

			resolution = strconv.FormatInt(int64(sh.Width()), 10) + "x" + strconv.FormatInt(int64(sh.Height()), 10)

			// TODO: FPS

		case *codecs.VP9:
			resolution = strconv.FormatInt(int64(codec.Width), 10) + "x" + strconv.FormatInt(int64(codec.Height), 10)

			// TODO: FPS

		case *codecs.H265:
			var sps h265.SPS
			err := sps.Unmarshal(codec.SPS)
			if err != nil {
				return nil, err
			}

			resolution = strconv.FormatInt(int64(sps.Width()), 10) + "x" + strconv.FormatInt(int64(sps.Height()), 10)

			f := sps.FPS()
			if f != 0 {
				frameRate = &f
			}

		case *codecs.H264:
			var sps h264.SPS
			err := sps.Unmarshal(codec.SPS)
			if err != nil {
				return nil, err
			}

			resolution = strconv.FormatInt(int64(sps.Width()), 10) + "x" + strconv.FormatInt(int64(sps.Height()), 10)

			f := sps.FPS()
			if f != 0 {
				frameRate = &f
			}
		}
	}

	mediaPlaylistURI := "stream.m3u8"
	if rawQuery != "" {
		mediaPlaylistURI += "?" + rawQuery
	}

	pl := &playlist.Multivariant{
		Version: func() int {
			if variant == MuxerVariantMPEGTS {
				return 3
			}
			return 9
		}(),
		IndependentSegments: true,
		Variants: []*playlist.MultivariantVariant{{
			Bandwidth:        maxBandwidth,
			AverageBandwidth: &averageBandwidth,
			Codecs: func() []string {
				var codecs []string
				if videoTrack != nil {
					codecs = append(codecs, codecparams.Marshal(videoTrack.Codec))
				}
				if audioTrack != nil {
					codecs = append(codecs, codecparams.Marshal(audioTrack.Codec))
				}
				return codecs
			}(),
			Resolution: resolution,
			FrameRate:  frameRate,
			URI:        mediaPlaylistURI,
		}},
	}

	return pl.Marshal()
}

func generateInitFile(
	videoTrack *Track,
	audioTrack *Track,
) ([]byte, error) {
	var init fmp4.Init
	trackID := 1

	if videoTrack != nil {
		init.Tracks = append(init.Tracks, &fmp4.InitTrack{
			ID:        trackID,
			TimeScale: 90000,
			Codec:     codecs.ToFMP4(videoTrack.Codec),
		})
		trackID++
	}

	if audioTrack != nil {
		init.Tracks = append(init.Tracks, &fmp4.InitTrack{
			ID:        trackID,
			TimeScale: fmp4TimeScale(audioTrack.Codec),
			Codec:     codecs.ToFMP4(audioTrack.Codec),
		})
	}

	var w seekablebuffer.Buffer
	err := init.Marshal(&w)
	if err != nil {
		return nil, err
	}

	return w.Bytes(), nil
}

func generateMediaPlaylistMPEGTS(
	segments []muxerSegment,
	segmentDeleteCount int,
	rawQuery string,
) ([]byte, error) {
	pl := &playlist.Media{
		Version:        3,
		AllowCache:     boolPtr(false),
		TargetDuration: targetDuration(segments),
		MediaSequence:  segmentDeleteCount,
	}

	for _, s := range segments {
		if seg, ok := s.(*muxerSegmentMPEGTS); ok {
			u := seg.name
			if rawQuery != "" {
				u += "?" + rawQuery
			}

			pl.Segments = append(pl.Segments, &playlist.MediaSegment{
				DateTime: &seg.startNTP,
				Duration: seg.getDuration(),
				URI:      u,
			})
		}
	}

	return pl.Marshal()
}

func filterOutHLSParams(rawQuery string) string {
	if rawQuery != "" {
		if q, err := url.ParseQuery(rawQuery); err == nil {
			for k := range q {
				if strings.HasPrefix(k, "_HLS_") {
					delete(q, k)
				}
			}
			rawQuery = q.Encode()
		}
	}
	return rawQuery
}

func generateMediaPlaylistFMP4(
	isDeltaUpdate bool,
	variant MuxerVariant,
	segments []muxerSegment,
	nextSegmentParts []*muxerPart,
	nextPartID uint64,
	segmentDeleteCount int,
	prefix string,
	rawQuery string,
) ([]byte, error) {
	targetDuration := targetDuration(segments)
	skipBoundary := time.Duration(targetDuration) * 6 * time.Second
	rawQuery = filterOutHLSParams(rawQuery)

	pl := &playlist.Media{
		Version:        10,
		TargetDuration: targetDuration,
		MediaSequence:  segmentDeleteCount,
	}

	if variant == MuxerVariantLowLatency {
		partTarget := partTargetDuration(segments, nextSegmentParts)
		partHoldBack := (partTarget * 25) / 10

		pl.ServerControl = &playlist.MediaServerControl{
			CanBlockReload: true,
			PartHoldBack:   &partHoldBack,
			CanSkipUntil:   &skipBoundary,
		}

		pl.PartInf = &playlist.MediaPartInf{
			PartTarget: partTarget,
		}
	}

	skipped := 0

	if !isDeltaUpdate {
		u := prefix + "_init.mp4"
		if rawQuery != "" {
			u += "?" + rawQuery
		}

		pl.Map = &playlist.MediaMap{
			URI: u,
		}
	} else {
		var curDuration time.Duration
		shown := 0
		for _, segment := range segments {
			curDuration += segment.getDuration()
			if curDuration >= skipBoundary {
				break
			}
			shown++
		}
		skipped = len(segments) - shown

		pl.Skip = &playlist.MediaSkip{
			SkippedSegments: skipped,
		}
	}

	for i, sog := range segments {
		if i < skipped {
			continue
		}

		switch seg := sog.(type) {
		case *muxerSegmentFMP4:
			u := seg.name
			if rawQuery != "" {
				u += "?" + rawQuery
			}

			plse := &playlist.MediaSegment{
				Duration: seg.getDuration(),
				URI:      u,
			}

			if (len(segments) - i) <= 2 {
				plse.DateTime = &seg.startNTP
			}

			if variant == MuxerVariantLowLatency && (len(segments)-i) <= 2 {
				for _, part := range seg.parts {
					u = part.getName()
					if rawQuery != "" {
						u += "?" + rawQuery
					}

					plse.Parts = append(plse.Parts, &playlist.MediaPart{
						Duration:    part.finalDuration,
						URI:         u,
						Independent: part.isIndependent,
					})
				}
			}

			pl.Segments = append(pl.Segments, plse)

		case *muxerGap:
			pl.Segments = append(pl.Segments, &playlist.MediaSegment{
				Gap:      true,
				Duration: seg.duration,
				URI:      "gap.mp4",
			})
		}
	}

	if variant == MuxerVariantLowLatency {
		for _, part := range nextSegmentParts {
			u := part.getName()
			if rawQuery != "" {
				u += "?" + rawQuery
			}

			pl.Parts = append(pl.Parts, &playlist.MediaPart{
				Duration:    part.finalDuration,
				URI:         u,
				Independent: part.isIndependent,
			})
		}

		// preload hint must always be present
		// otherwise hls.js goes into a loop
		u := partName(prefix, nextPartID)
		if rawQuery != "" {
			u += "?" + rawQuery
		}
		pl.PreloadHint = &playlist.MediaPreloadHint{
			URI: u,
		}
	}

	return pl.Marshal()
}

func generateMediaPlaylist(
	isDeltaUpdate bool,
	variant MuxerVariant,
	segments []muxerSegment,
	nextSegmentParts []*muxerPart,
	nextPartID uint64,
	segmentDeleteCount int,
	prefix string,
	rawQuery string,
) ([]byte, error) {
	if variant == MuxerVariantMPEGTS {
		return generateMediaPlaylistMPEGTS(
			segments,
			segmentDeleteCount,
			rawQuery,
		)
	}

	return generateMediaPlaylistFMP4(
		isDeltaUpdate,
		variant,
		segments,
		nextSegmentParts,
		nextPartID,
		segmentDeleteCount,
		prefix,
		rawQuery,
	)
}

type muxerServer struct {
	variant      MuxerVariant
	segmentCount int
	videoTrack   *Track
	audioTrack   *Track
	prefix       string

	mutex              sync.Mutex
	cond               *sync.Cond
	closed             bool
	segments           []muxerSegment
	segmentsByName     map[string]muxerSegment
	segmentDeleteCount int
	partsByName        map[string]*muxerPart
	nextSegmentID      uint64
	nextSegmentParts   []*muxerPart
	nextPartID         uint64
	init               []byte
}

func (s *muxerServer) initialize() {
	s.segmentsByName = make(map[string]muxerSegment)
	s.partsByName = make(map[string]*muxerPart)
	s.cond = sync.NewCond(&s.mutex)
}

func (s *muxerServer) close() {
	func() {
		s.mutex.Lock()
		defer s.mutex.Unlock()
		s.closed = true
	}()

	s.cond.Broadcast()

	for _, segment := range s.segments {
		segment.close()
	}
}

func (s *muxerServer) hasContent() bool {
	if s.variant == MuxerVariantFMP4 {
		return len(s.segments) >= 2
	}
	return len(s.segments) >= 1
}

func (s *muxerServer) hasPart(segmentID uint64, partID uint64) bool {
	if segmentID == s.nextSegmentID {
		if partID < uint64(len(s.nextSegmentParts)) {
			return true
		}
	} else {
		for _, sop := range s.segments {
			if seg, ok := sop.(*muxerSegmentFMP4); ok && segmentID == seg.id {
				// If the Client requests a Part Index greater than that of the final
				// Partial Segment of the Parent Segment, the Server MUST treat the
				// request as one for Part Index 0 of the following Parent Segment.
				if partID >= uint64(len(seg.parts)) {
					segmentID++
					partID = 0
					continue
				}

				return true
			}
		}
	}

	return false
}

func queryVal(q url.Values, key string) string {
	vals, ok := q[key]
	if ok && len(vals) >= 1 {
		return vals[0]
	}
	return ""
}

func (s *muxerServer) handle(w http.ResponseWriter, r *http.Request) {
	name := filepath.Base(r.URL.Path)

	switch {
	case name == "index.m3u8":
		s.handleMultivariantPlaylist(w, r)

	case name == "stream.m3u8":
		q := r.URL.Query()
		msn := queryVal(q, "_HLS_msn")
		part := queryVal(q, "_HLS_part")
		skip := queryVal(q, "_HLS_skip")
		s.handleMediaPlaylist(msn, part, skip, w, r)

	case s.variant != MuxerVariantMPEGTS && name == s.prefix+"_init.mp4":
		s.handleInitFile(w)

	case (s.variant != MuxerVariantMPEGTS && strings.HasSuffix(name, ".mp4")) ||
		(s.variant == MuxerVariantMPEGTS && strings.HasSuffix(name, ".ts")):
		s.handleSegmentOrPart(name, w)
	}
}

func (s *muxerServer) handleMultivariantPlaylist(w http.ResponseWriter, r *http.Request) {
	buf := func() []byte {
		s.mutex.Lock()
		defer s.mutex.Unlock()

		for !s.closed && !s.hasContent() {
			s.cond.Wait()
		}

		buf, err := generateMultivariantPlaylist(s.variant, s.videoTrack, s.audioTrack, s.segments, r.URL.RawQuery)
		if err != nil {
			return nil
		}

		return buf
	}()

	if buf == nil {
		w.WriteHeader(http.StatusInternalServerError)
		return
	}

	// allow caching but use a small period in order to
	// allow a stream to change tracks or bitrate
	w.Header().Set("Cache-Control", "max-age="+multivariantPlaylistMaxAge)
	w.Header().Set("Content-Type", `application/vnd.apple.mpegurl`)
	w.WriteHeader(http.StatusOK)
	w.Write(buf)
}

func (s *muxerServer) handleMediaPlaylist(
	msn string,
	part string,
	skip string,
	w http.ResponseWriter,
	r *http.Request,
) {
	isDeltaUpdate := false

	if s.variant == MuxerVariantLowLatency {
		isDeltaUpdate = skip == "YES" || skip == "v2"

		msnint, partint, err := parseMSNPart(msn, part)
		if err != nil {
			w.WriteHeader(http.StatusBadRequest)
			return
		}

		switch {
		case msn != "":
			byts := func() []byte {
				s.mutex.Lock()
				defer s.mutex.Unlock()

				for {
					if s.closed {
						w.WriteHeader(http.StatusInternalServerError)
						return nil
					}

					// If the _HLS_msn is greater than the Media Sequence Number of the last
					// Media Segment in the current Playlist plus two, or if the _HLS_part
					// exceeds the last Partial Segment in the current Playlist by the
					// Advance Part Limit, then the server SHOULD immediately return Bad
					// Request, such as HTTP 400.
					if msnint > (s.nextSegmentID+1) || msnint < (s.nextSegmentID-uint64(len(s.segments)-1)) {
						w.WriteHeader(http.StatusBadRequest)
						return nil
					}

					if s.hasContent() && s.hasPart(msnint, partint) {
						break
					}

					s.cond.Wait()
				}

				byts, err := generateMediaPlaylist(
					isDeltaUpdate,
					s.variant,
					s.segments,
					s.nextSegmentParts,
					s.nextPartID,
					s.segmentDeleteCount,
					s.prefix,
					r.URL.RawQuery,
				)
				if err != nil {
					w.WriteHeader(http.StatusInternalServerError)
					return nil
				}

				return byts
			}()

			if byts != nil {
				w.Header().Set("Cache-Control", "no-cache")
				w.Header().Set("Content-Type", `application/vnd.apple.mpegurl`)
				w.WriteHeader(http.StatusOK)
				w.Write(byts)
			}
			return

		case part != "": // part without msn is not supported.
			w.WriteHeader(http.StatusBadRequest)
			return
		}
	}

	byts := func() []byte {
		s.mutex.Lock()
		defer s.mutex.Unlock()

		for {
			if s.closed {
				w.WriteHeader(http.StatusInternalServerError)
				return nil
			}

			if s.hasContent() {
				break
			}

			s.cond.Wait()
		}

		byts, err := generateMediaPlaylist(
			isDeltaUpdate,
			s.variant,
			s.segments,
			s.nextSegmentParts,
			s.nextPartID,
			s.segmentDeleteCount,
			s.prefix,
			r.URL.RawQuery,
		)
		if err != nil {
			w.WriteHeader(http.StatusInternalServerError)
			return nil
		}

		return byts
	}()

	if byts != nil {
		w.Header().Set("Cache-Control", "no-cache")
		w.Header().Set("Content-Type", `application/vnd.apple.mpegurl`)
		w.WriteHeader(http.StatusOK)
		w.Write(byts)
	}
}

func (s *muxerServer) handleInitFile(w http.ResponseWriter) {
	init := func() []byte {
		s.mutex.Lock()
		defer s.mutex.Unlock()

		for !s.closed && !s.hasContent() {
			s.cond.Wait()
		}

		return s.init
	}()

	if init == nil {
		w.WriteHeader(http.StatusInternalServerError)
		return
	}

	// allow caching but use a small period in order to
	// allow a stream to change track parameters
	w.Header().Set("Cache-Control", "max-age="+initMaxAge)
	w.Header().Set("Content-Type", "video/mp4")
	w.WriteHeader(http.StatusOK)
	w.Write(init)
}

func (s *muxerServer) handleSegmentOrPart(fname string, w http.ResponseWriter) {
	switch {
	case strings.HasPrefix(fname, s.prefix+"_"+"seg"):
		s.mutex.Lock()
		segment, ok := s.segmentsByName[fname]
		s.mutex.Unlock()

		if !ok {
			return
		}

		r, err := segment.reader()
		if err != nil {
			w.WriteHeader(http.StatusInternalServerError)
			return
		}
		defer r.Close()

		w.Header().Set("Cache-Control", "max-age="+segmentMaxAge)
		w.Header().Set(
			"Content-Type",
			func() string {
				if s.variant == MuxerVariantMPEGTS {
					return "video/MP2T"
				}
				return "video/mp4"
			}(),
		)
		w.WriteHeader(http.StatusOK)
		io.Copy(w, r)

	case s.variant == MuxerVariantLowLatency && strings.HasPrefix(fname, s.prefix+"_"+"part"):
		s.mutex.Lock()

		part := s.partsByName[fname]

		// support for EXT-X-PRELOAD-HINT
		if part == nil && fname == partName(s.prefix, s.nextPartID) {
			partID := s.nextPartID

			for !s.closed && s.nextPartID <= partID {
				s.cond.Wait()
			}

			part = s.partsByName[fname]
		}

		s.mutex.Unlock()

		if part == nil {
			return
		}

		r, err := part.reader()
		if err != nil {
			w.WriteHeader(http.StatusInternalServerError)
			return
		}
		defer r.Close()

		w.Header().Set("Cache-Control", "max-age="+segmentMaxAge)
		w.Header().Set("Content-Type", "video/mp4")
		w.WriteHeader(http.StatusOK)
		io.Copy(w, r)
	}
}

func (s *muxerServer) publishPartInner(part *muxerPart) error {
	s.partsByName[part.getName()] = part
	s.nextSegmentParts = append(s.nextSegmentParts, part)
	s.nextPartID = part.id + 1

	return nil
}

func (s *muxerServer) publishPart(part *muxerPart) error {
	s.mutex.Lock()
	err := s.publishPartInner(part)
	s.mutex.Unlock()

	if err != nil {
		return err
	}

	s.cond.Broadcast()
	return nil
}

func (s *muxerServer) publishSegmentInner(segment muxerSegment) error {
	// add initial gaps, required by iOS LL-HLS
	if s.variant == MuxerVariantLowLatency && len(s.segments) == 0 {
		for i := 0; i < 7; i++ {
			s.segments = append(s.segments, &muxerGap{
				duration: segment.getDuration(),
			})
		}
	}

	s.segmentsByName[segment.getName()] = segment
	s.segments = append(s.segments, segment)

	if seg, ok := segment.(*muxerSegmentFMP4); ok {
		s.nextSegmentID = seg.id + 1
	}

	s.nextSegmentParts = s.nextSegmentParts[:0]

	if len(s.segments) > s.segmentCount {
		toDelete := s.segments[0]

		if toDeleteSeg, ok := toDelete.(*muxerSegmentFMP4); ok {
			for _, part := range toDeleteSeg.parts {
				delete(s.partsByName, part.getName())
			}
		}

		toDelete.close()
		delete(s.segmentsByName, toDelete.getName())

		s.segments = s.segments[1:]
		s.segmentDeleteCount++
	}

	// regenerate init.mp4 only if missing or codec parameters have changed
	if s.variant != MuxerVariantMPEGTS && (s.init == nil || segment.isForceSwitched()) {
		byts, err := generateInitFile(s.videoTrack, s.audioTrack)
		if err != nil {
			return err
		}
		s.init = byts
	}

	// do not pregenerate media playlist since it's too dynamic.

	return nil
}

func (s *muxerServer) publishSegment(segment muxerSegment) error {
	s.mutex.Lock()
	err := s.publishSegmentInner(segment)
	s.mutex.Unlock()

	if err != nil {
		return err
	}

	s.cond.Broadcast()
	return nil
}
