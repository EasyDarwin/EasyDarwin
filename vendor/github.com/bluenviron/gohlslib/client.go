/*
Package gohlslib is a HLS client and muxer library for the Go programming language.

Examples are available at https://github.com/bluenviron/gohlslib/tree/main/examples
*/
package gohlslib

import (
	"context"
	"errors"
	"fmt"
	"log"
	"net/http"
	"net/url"
	"time"
)

const (
	clientMaxTracksPerStream     = 10
	clientMPEGTSSampleQueueSize  = 100
	clientLiveInitialDistance    = 3
	clientLiveMaxDistanceFromEnd = 5
	clientMaxDTSRTCDiff          = 10 * time.Second
)

// ErrClientEOS is returned by Wait() when the stream has ended.
var ErrClientEOS = errors.New("end of stream")

// ClientOnDownloadPrimaryPlaylistFunc is the prototype of Client.OnDownloadPrimaryPlaylist.
type ClientOnDownloadPrimaryPlaylistFunc func(url string)

// ClientOnDownloadStreamPlaylistFunc is the prototype of Client.OnDownloadStreamPlaylist.
type ClientOnDownloadStreamPlaylistFunc func(url string)

// ClientOnDownloadSegmentFunc is the prototype of Client.OnDownloadSegment.
type ClientOnDownloadSegmentFunc func(url string)

// ClientOnDownloadPartFunc is the prototype of Client.OnDownloadPart.
type ClientOnDownloadPartFunc func(url string)

// ClientOnDecodeErrorFunc is the prototype of Client.OnDecodeError.
type ClientOnDecodeErrorFunc func(err error)

// ClientOnRequestFunc is the prototype of the function passed to OnRequest().
type ClientOnRequestFunc func(*http.Request)

// ClientOnTracksFunc is the prototype of the function passed to OnTracks().
type ClientOnTracksFunc func([]*Track) error

// ClientOnDataAV1Func is the prototype of the function passed to OnDataAV1().
type ClientOnDataAV1Func func(pts time.Duration, tu [][]byte)

// ClientOnDataVP9Func is the prototype of the function passed to OnDataVP9().
type ClientOnDataVP9Func func(pts time.Duration, frame []byte)

// ClientOnDataH26xFunc is the prototype of the function passed to OnDataH26x().
type ClientOnDataH26xFunc func(pts time.Duration, dts time.Duration, au [][]byte)

// ClientOnDataMPEG4AudioFunc is the prototype of the function passed to OnDataMPEG4Audio().
type ClientOnDataMPEG4AudioFunc func(pts time.Duration, aus [][]byte)

// ClientOnDataOpusFunc is the prototype of the function passed to OnDataOpus().
type ClientOnDataOpusFunc func(pts time.Duration, packets [][]byte)

type clientOnStreamTracksFunc func(ctx context.Context, isLeading bool, tracks []*Track) ([]*clientTrack, bool)

type clientOnDataFunc func(pts time.Duration, dts time.Duration, data [][]byte)

func clientAbsoluteURL(base *url.URL, relative string) (*url.URL, error) {
	u, err := url.Parse(relative)
	if err != nil {
		return nil, err
	}
	return base.ResolveReference(u), nil
}

// Client is a HLS client.
type Client struct {
	//
	// parameters (all optional except URI)
	//
	// URI of the playlist.
	URI string
	// HTTP client.
	// It defaults to http.DefaultClient.
	HTTPClient *http.Client

	//
	// callbacks (all optional)
	//
	// called when sending a request to the server.
	OnRequest ClientOnRequestFunc
	// called when tracks are available.
	OnTracks ClientOnTracksFunc
	// called before downloading a primary playlist.
	OnDownloadPrimaryPlaylist ClientOnDownloadPrimaryPlaylistFunc
	// called before downloading a stream playlist.
	OnDownloadStreamPlaylist ClientOnDownloadStreamPlaylistFunc
	// called before downloading a segment.
	OnDownloadSegment ClientOnDownloadSegmentFunc
	// called before downloading a part.
	OnDownloadPart ClientOnDownloadPartFunc
	// called when a non-fatal decode error occurs.
	OnDecodeError ClientOnDecodeErrorFunc

	//
	// private
	//

	ctx               context.Context
	ctxCancel         func()
	playlistURL       *url.URL
	primaryDownloader *clientPrimaryDownloader
	leadingTimeConv   clientTimeConv
	tracks            map[*Track]*clientTrack

	// out
	outErr               chan error
	leadingTimeConvReady chan struct{}
}

// Start starts the client.
func (c *Client) Start() error {
	if c.HTTPClient == nil {
		c.HTTPClient = http.DefaultClient
	}
	if c.OnRequest == nil {
		c.OnRequest = func(_ *http.Request) {}
	}
	if c.OnTracks == nil {
		c.OnTracks = func(_ []*Track) error {
			return nil
		}
	}
	if c.OnDownloadPrimaryPlaylist == nil {
		c.OnDownloadPrimaryPlaylist = func(u string) {
			log.Printf("downloading primary playlist %v", u)
		}
	}
	if c.OnDownloadStreamPlaylist == nil {
		c.OnDownloadStreamPlaylist = func(u string) {
			log.Printf("downloading stream playlist %v", u)
		}
	}
	if c.OnDownloadSegment == nil {
		c.OnDownloadSegment = func(u string) {
			log.Printf("downloading segment %v", u)
		}
	}
	if c.OnDownloadPart == nil {
		c.OnDownloadPart = func(u string) {
			log.Printf("downloading part %v", u)
		}
	}
	if c.OnDecodeError == nil {
		c.OnDecodeError = func(err error) {
			log.Println(err.Error())
		}
	}

	var err error
	c.playlistURL, err = url.Parse(c.URI)
	if err != nil {
		return err
	}

	c.ctx, c.ctxCancel = context.WithCancel(context.Background())

	c.outErr = make(chan error, 1)
	c.leadingTimeConvReady = make(chan struct{})

	go c.run()

	return nil
}

// Close closes all the Client resources.
func (c *Client) Close() {
	c.ctxCancel()
}

// Wait waits for any error of the Client.
func (c *Client) Wait() chan error {
	return c.outErr
}

// OnDataAV1 sets a callback that is called when data from an AV1 track is received.
func (c *Client) OnDataAV1(track *Track, cb ClientOnDataAV1Func) {
	c.tracks[track].onData = func(pts time.Duration, _ time.Duration, data [][]byte) {
		cb(pts, data)
	}
}

// OnDataVP9 sets a callback that is called when data from a VP9 track is received.
func (c *Client) OnDataVP9(track *Track, cb ClientOnDataVP9Func) {
	c.tracks[track].onData = func(pts time.Duration, _ time.Duration, data [][]byte) {
		cb(pts, data[0])
	}
}

// OnDataH26x sets a callback that is called when data from an H26x track is received.
func (c *Client) OnDataH26x(track *Track, cb ClientOnDataH26xFunc) {
	c.tracks[track].onData = func(pts time.Duration, dts time.Duration, data [][]byte) {
		cb(pts, dts, data)
	}
}

// OnDataMPEG4Audio sets a callback that is called when data from a MPEG-4 Audio track is received.
func (c *Client) OnDataMPEG4Audio(track *Track, cb ClientOnDataMPEG4AudioFunc) {
	c.tracks[track].onData = func(pts time.Duration, _ time.Duration, data [][]byte) {
		cb(pts, data)
	}
}

// OnDataOpus sets a callback that is called when data from an Opus track is received.
func (c *Client) OnDataOpus(track *Track, cb ClientOnDataOpusFunc) {
	c.tracks[track].onData = func(pts time.Duration, _ time.Duration, data [][]byte) {
		cb(pts, data)
	}
}

var zero time.Time

// AbsoluteTime returns the absolute timestamp of the last sample.
func (c *Client) AbsoluteTime(track *Track) (time.Time, bool) {
	return c.tracks[track].absoluteTime()
}

func (c *Client) run() {
	c.outErr <- c.runInner()
}

func (c *Client) runInner() error {
	rp := &clientRoutinePool{}
	rp.initialize()

	c.primaryDownloader = &clientPrimaryDownloader{
		primaryPlaylistURL:        c.playlistURL,
		httpClient:                c.HTTPClient,
		onRequest:                 c.OnRequest,
		onDownloadPrimaryPlaylist: c.OnDownloadPrimaryPlaylist,
		onDownloadStreamPlaylist:  c.OnDownloadStreamPlaylist,
		onDownloadSegment:         c.OnDownloadSegment,
		onDownloadPart:            c.OnDownloadPart,
		onDecodeError:             c.OnDecodeError,
		rp:                        rp,
		setTracks:                 c.setTracks,
		setLeadingTimeConv:        c.setLeadingTimeConv,
		getLeadingTimeConv:        c.getLeadingTimeConv,
	}
	c.primaryDownloader.initialize()
	rp.add(c.primaryDownloader)

	select {
	case err := <-rp.errorChan():
		rp.close()
		return err

	case <-c.ctx.Done():
		rp.close()
		return fmt.Errorf("terminated")
	}
}

func (c *Client) setTracks(tracks []*Track) (map[*Track]*clientTrack, error) {
	c.tracks = make(map[*Track]*clientTrack)
	for _, track := range tracks {
		c.tracks[track] = &clientTrack{
			track:  track,
			onData: func(_, _ time.Duration, _ [][]byte) {},
		}
	}

	err := c.OnTracks(tracks)
	if err != nil {
		return nil, err
	}

	return c.tracks, nil
}

func (c *Client) setLeadingTimeConv(ts clientTimeConv) {
	c.leadingTimeConv = ts

	startRTC := time.Now()

	for _, track := range c.tracks {
		track.startRTC = startRTC
	}

	close(c.leadingTimeConvReady)
}

func (c *Client) getLeadingTimeConv(ctx context.Context) (clientTimeConv, bool) {
	select {
	case <-c.leadingTimeConvReady:
	case <-ctx.Done():
		return nil, false
	}
	return c.leadingTimeConv, true
}
