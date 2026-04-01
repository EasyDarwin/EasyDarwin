package m3u8

/*
 Part of M3U8 parser & generator library.
 This file defines data structures related to package.

 Copyright 2013-2017 The Project Developers.
 See the AUTHORS and LICENSE files at the top-level directory of this distribution
 and at https://github.com/grafov/m3u8/

 ॐ तारे तुत्तारे तुरे स्व
*/

import (
	"bytes"
	"io"
	"time"
)

const (
	/*
		Compatibility rules described in section 7:
		Clients and servers MUST implement protocol version 2 or higher to use:
		   o  The IV attribute of the EXT-X-KEY tag.
		   Clients and servers MUST implement protocol version 3 or higher to use:
		   o  Floating-point EXTINF duration values.
		   Clients and servers MUST implement protocol version 4 or higher to use:
		   o  The EXT-X-BYTERANGE tag.
		   o  The EXT-X-I-FRAME-STREAM-INF tag.
		   o  The EXT-X-I-FRAMES-ONLY tag.
		   o  The EXT-X-MEDIA tag.
		   o  The AUDIO and VIDEO attributes of the EXT-X-STREAM-INF tag.
	*/
	minver = uint8(3)

	// DATETIME represents format of the timestamps in encoded
	// playlists. Format for EXT-X-PROGRAM-DATE-TIME defined in
	// section 3.4.5
	DATETIME = time.RFC3339Nano
)

// ListType is type of the playlist.
type ListType uint

const (
	// use 0 for not defined type
	MASTER ListType = iota + 1
	MEDIA
)

// MediaType is the type for EXT-X-PLAYLIST-TYPE tag
type MediaType uint

const (
	// use 0 for not defined type
	EVENT MediaType = iota + 1
	VOD
)

// SCTE35Syntax defines the format of the SCTE-35 cue points which do not use
// the draft-pantos-http-live-streaming-19 EXT-X-DATERANGE tag and instead
// have their own custom tags
type SCTE35Syntax uint

const (
	// SCTE35_67_2014 will be the default due to backwards compatibility reasons.
	SCTE35_67_2014 SCTE35Syntax = iota // SCTE35_67_2014 defined in http://www.scte.org/documents/pdf/standards/SCTE%2067%202014.pdf
	SCTE35_OATCLS                      // SCTE35_OATCLS is a non-standard but common format
)

// SCTE35CueType defines the type of cue point, used by readers and writers to
// write a different syntax
type SCTE35CueType uint

const (
	SCTE35Cue_Start SCTE35CueType = iota // SCTE35Cue_Start indicates an out cue point
	SCTE35Cue_Mid                        // SCTE35Cue_Mid indicates a segment between start and end cue points
	SCTE35Cue_End                        // SCTE35Cue_End indicates an in cue point
)

// MediaPlaylist structure represents a single bitrate playlist aka
// media playlist. It related to both a simple media playlists and a
// sliding window media playlists. URI lines in the Playlist point to
// media segments.
//
// Simple Media Playlist file sample:
//
//    #EXTM3U
//    #EXT-X-VERSION:3
//    #EXT-X-TARGETDURATION:5220
//    #EXTINF:5219.2,
//    http://media.example.com/entire.ts
//    #EXT-X-ENDLIST
//
// Sample of Sliding Window Media Playlist, using HTTPS:
//
//    #EXTM3U
//    #EXT-X-VERSION:3
//    #EXT-X-TARGETDURATION:8
//    #EXT-X-MEDIA-SEQUENCE:2680
//
//    #EXTINF:7.975,
//    https://priv.example.com/fileSequence2680.ts
//    #EXTINF:7.941,
//    https://priv.example.com/fileSequence2681.ts
//    #EXTINF:7.975,
//    https://priv.example.com/fileSequence2682.ts
type MediaPlaylist struct {
	TargetDuration   float64
	SeqNo            uint64 // EXT-X-MEDIA-SEQUENCE
	Segments         []*MediaSegment
	Args             string // optional arguments placed after URIs (URI?Args)
	Iframe           bool   // EXT-X-I-FRAMES-ONLY
	Closed           bool   // is this VOD (closed) or Live (sliding) playlist?
	MediaType        MediaType
	DiscontinuitySeq uint64 // EXT-X-DISCONTINUITY-SEQUENCE
	StartTime        float64
	StartTimePrecise bool
	durationAsInt    bool // output durations as integers of floats?
	keyformat        int
	winsize          uint // max number of segments displayed in an encoded playlist; need set to zero for VOD playlists
	capacity         uint // total capacity of slice used for the playlist
	head             uint // head of FIFO, we add segments to head
	tail             uint // tail of FIFO, we remove segments from tail
	count            uint // number of segments added to the playlist
	buf              bytes.Buffer
	ver              uint8
	Key              *Key // EXT-X-KEY is optional encryption key displayed before any segments (default key for the playlist)
	Map              *Map // EXT-X-MAP is optional tag specifies how to obtain the Media Initialization Section (default map for the playlist)
	WV               *WV  // Widevine related tags outside of M3U8 specs
	Custom           map[string]CustomTag
	customDecoders   []CustomDecoder
}

// MasterPlaylist structure represents a master playlist which
// combines media playlists for multiple bitrates. URI lines in the
// playlist identify media playlists. Sample of Master Playlist file:
//
//    #EXTM3U
//    #EXT-X-STREAM-INF:PROGRAM-ID=1,BANDWIDTH=1280000
//    http://example.com/low.m3u8
//    #EXT-X-STREAM-INF:PROGRAM-ID=1,BANDWIDTH=2560000
//    http://example.com/mid.m3u8
//    #EXT-X-STREAM-INF:PROGRAM-ID=1,BANDWIDTH=7680000
//    http://example.com/hi.m3u8
//    #EXT-X-STREAM-INF:PROGRAM-ID=1,BANDWIDTH=65000,CODECS="mp4a.40.5"
//    http://example.com/audio-only.m3u8
type MasterPlaylist struct {
	Variants            []*Variant
	Args                string // optional arguments placed after URI (URI?Args)
	CypherVersion       string // non-standard tag for Widevine (see also WV struct)
	buf                 bytes.Buffer
	ver                 uint8
	independentSegments bool
	Custom              map[string]CustomTag
	customDecoders      []CustomDecoder
}

// Variant structure represents variants for master playlist.
// Variants included in a master playlist and point to media
// playlists.
type Variant struct {
	URI       string
	Chunklist *MediaPlaylist
	VariantParams
}

// VariantParams structure represents additional parameters for a
// variant used in EXT-X-STREAM-INF and EXT-X-I-FRAME-STREAM-INF
type VariantParams struct {
	ProgramId        uint32
	Bandwidth        uint32
	AverageBandwidth uint32 // EXT-X-STREAM-INF only
	Codecs           string
	Resolution       string
	Audio            string // EXT-X-STREAM-INF only
	Video            string
	Subtitles        string // EXT-X-STREAM-INF only
	Captions         string // EXT-X-STREAM-INF only
	Name             string // EXT-X-STREAM-INF only (non standard Wowza/JWPlayer extension to name the variant/quality in UA)
	Iframe           bool   // EXT-X-I-FRAME-STREAM-INF
	VideoRange       string
	HDCPLevel        string
	FrameRate        float64        // EXT-X-STREAM-INF
	Alternatives     []*Alternative // EXT-X-MEDIA
}

// Alternative structure represents EXT-X-MEDIA tag in variants.
type Alternative struct {
	GroupId         string
	URI             string
	Type            string
	Language        string
	Name            string
	Default         bool
	Autoselect      string
	Forced          string
	Characteristics string
	Subtitles       string
}

// MediaSegment structure represents a media segment included in a
// media playlist. Media segment may be encrypted. Widevine supports
// own tags for encryption metadata.
type MediaSegment struct {
	SeqId           uint64
	Title           string // optional second parameter for EXTINF tag
	URI             string
	Duration        float64   // first parameter for EXTINF tag; duration must be integers if protocol version is less than 3 but we are always keep them float
	Limit           int64     // EXT-X-BYTERANGE <n> is length in bytes for the file under URI
	Offset          int64     // EXT-X-BYTERANGE [@o] is offset from the start of the file under URI
	Key             *Key      // EXT-X-KEY displayed before the segment and means changing of encryption key (in theory each segment may have own key)
	Map             *Map      // EXT-X-MAP displayed before the segment
	Discontinuity   bool      // EXT-X-DISCONTINUITY indicates an encoding discontinuity between the media segment that follows it and the one that preceded it (i.e. file format, number and type of tracks, encoding parameters, encoding sequence, timestamp sequence)
	SCTE            *SCTE     // SCTE-35 used for Ad signaling in HLS
	ProgramDateTime time.Time // EXT-X-PROGRAM-DATE-TIME tag associates the first sample of a media segment with an absolute date and/or time
	Custom          map[string]CustomTag
}

// SCTE holds custom, non EXT-X-DATERANGE, SCTE-35 tags
type SCTE struct {
	Syntax  SCTE35Syntax  // Syntax defines the format of the SCTE-35 cue tag
	CueType SCTE35CueType // CueType defines whether the cue is a start, mid, end (if applicable)
	Cue     string
	ID      string
	Time    float64
	Elapsed float64
}

// Key structure represents information about stream encryption.
//
// Realizes EXT-X-KEY tag.
type Key struct {
	Method            string
	URI               string
	IV                string
	Keyformat         string
	Keyformatversions string
}

// Map structure represents specifies how to obtain the Media
// Initialization Section required to parse the applicable
// Media Segments.
//
// It applied to every Media Segment that appears after it in the
// Playlist until the next EXT-X-MAP tag or until the end of the
// playlist.
//
// Realizes EXT-MAP tag.
type Map struct {
	URI    string
	Limit  int64 // <n> is length in bytes for the file under URI
	Offset int64 // [@o] is offset from the start of the file under URI
}

// WV structure represents metadata  for Google Widevine playlists.
// This format not described in IETF draft but provied by Widevine Live Packager as
// additional tags with #WV-prefix.
type WV struct {
	AudioChannels          uint
	AudioFormat            uint
	AudioProfileIDC        uint
	AudioSampleSize        uint
	AudioSamplingFrequency uint
	CypherVersion          string
	ECM                    string
	VideoFormat            uint
	VideoFrameRate         uint
	VideoLevelIDC          uint
	VideoProfileIDC        uint
	VideoResolution        string
	VideoSAR               string
}

// Playlist interface applied to various playlist types.
type Playlist interface {
	Encode() *bytes.Buffer
	Decode(bytes.Buffer, bool) error
	DecodeFrom(reader io.Reader, strict bool) error
	WithCustomDecoders([]CustomDecoder) Playlist
	String() string
}

// CustomDecoder interface for decoding custom and unsupported tags
type CustomDecoder interface {
	// TagName should return the full indentifier including the leading '#' as well as the
	// trailing ':' if the tag also contains a value or attribute list
	TagName() string
	// Decode parses a line from the playlist and returns the CustomTag representation
	Decode(line string) (CustomTag, error)
	// SegmentTag should return true if this CustomDecoder should apply per segment.
	// Should returns false if it a MediaPlaylist header tag.
	// This value is ignored for MasterPlaylists.
	SegmentTag() bool
}

// CustomTag interface for encoding custom and unsupported tags
type CustomTag interface {
	// TagName should return the full indentifier including the leading '#' as well as the
	// trailing ':' if the tag also contains a value or attribute list
	TagName() string
	// Encode should return the complete tag string as a *bytes.Buffer. This will
	// be used by Playlist.Decode to write the tag to the m3u8.
	// Return nil to not write anything to the m3u8.
	Encode() *bytes.Buffer
	// String should return the encoded tag as a string.
	String() string
}

// Internal structure for decoding a line of input stream with a list type detection
type decodingState struct {
	listType           ListType
	m3u                bool
	tagWV              bool
	tagStreamInf       bool
	tagInf             bool
	tagSCTE35          bool
	tagRange           bool
	tagDiscontinuity   bool
	tagProgramDateTime bool
	tagKey             bool
	tagMap             bool
	tagCustom          bool
	programDateTime    time.Time
	limit              int64
	offset             int64
	duration           float64
	title              string
	variant            *Variant
	alternatives       []*Alternative
	xkey               *Key
	xmap               *Map
	scte               *SCTE
	custom             map[string]CustomTag
}
