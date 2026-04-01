package gohlslib

import (
	"context"
	"fmt"
	"io"
	"net/http"
	"net/url"
	"strings"

	"github.com/bluenviron/gohlslib/pkg/playlist"
)

func checkSupport(codecs []string) bool {
	for _, codec := range codecs {
		if !strings.HasPrefix(codec, "avc1.") &&
			!strings.HasPrefix(codec, "hvc1.") &&
			!strings.HasPrefix(codec, "hev1.") &&
			!strings.HasPrefix(codec, "mp4a.") &&
			codec != "opus" {
			return false
		}
	}
	return true
}

func cloneURL(ur *url.URL) *url.URL {
	return &url.URL{
		Scheme:      ur.Scheme,
		Opaque:      ur.Opaque,
		User:        ur.User,
		Host:        ur.Host,
		Path:        ur.Path,
		RawPath:     ur.RawPath,
		OmitHost:    ur.OmitHost,
		ForceQuery:  ur.ForceQuery,
		RawQuery:    ur.RawQuery,
		Fragment:    ur.Fragment,
		RawFragment: ur.RawFragment,
	}
}

func clientDownloadPlaylist(
	ctx context.Context,
	httpClient *http.Client,
	onRequest ClientOnRequestFunc,
	ur *url.URL,
) (playlist.Playlist, error) {
	req, err := http.NewRequestWithContext(ctx, http.MethodGet, ur.String(), nil)
	if err != nil {
		return nil, err
	}

	onRequest(req)

	res, err := httpClient.Do(req)
	if err != nil {
		return nil, err
	}
	defer res.Body.Close()

	if res.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("bad status code: %d", res.StatusCode)
	}

	byts, err := io.ReadAll(res.Body)
	if err != nil {
		return nil, err
	}

	return playlist.Unmarshal(byts)
}

func pickLeadingPlaylist(variants []*playlist.MultivariantVariant) *playlist.MultivariantVariant {
	var candidates []*playlist.MultivariantVariant //nolint:prealloc
	for _, v := range variants {
		if !checkSupport(v.Codecs) {
			continue
		}
		candidates = append(candidates, v)
	}
	if candidates == nil {
		return nil
	}

	// pick the variant with the greatest bandwidth
	var leadingPlaylist *playlist.MultivariantVariant
	for _, v := range candidates {
		if leadingPlaylist == nil ||
			v.Bandwidth > leadingPlaylist.Bandwidth {
			leadingPlaylist = v
		}
	}
	return leadingPlaylist
}

func pickAudioPlaylist(alternatives []*playlist.MultivariantRendition, groupID string) *playlist.MultivariantRendition {
	candidates := func() []*playlist.MultivariantRendition {
		var ret []*playlist.MultivariantRendition
		for _, alt := range alternatives {
			if alt.GroupID == groupID {
				ret = append(ret, alt)
			}
		}
		return ret
	}()
	if candidates == nil {
		return nil
	}

	// pick the default audio playlist
	for _, alt := range candidates {
		if alt.Default {
			return alt
		}
	}

	// alternatively, pick the first one
	return candidates[0]
}

type streamTracksEntry struct {
	isLeading bool
	tracks    []*Track
}

type clientPrimaryDownloader struct {
	primaryPlaylistURL        *url.URL
	httpClient                *http.Client
	onRequest                 ClientOnRequestFunc
	onDownloadPrimaryPlaylist ClientOnDownloadPrimaryPlaylistFunc
	onDownloadStreamPlaylist  ClientOnDownloadStreamPlaylistFunc
	onDownloadSegment         ClientOnDownloadSegmentFunc
	onDownloadPart            ClientOnDownloadPartFunc
	onDecodeError             ClientOnDecodeErrorFunc
	rp                        *clientRoutinePool
	setTracks                 func([]*Track) (map[*Track]*clientTrack, error)
	setLeadingTimeConv        func(ts clientTimeConv)
	getLeadingTimeConv        func(ctx context.Context) (clientTimeConv, bool)

	clientTracks map[*Track]*clientTrack

	// in
	chStreamTracks chan streamTracksEntry
	chStreamEnded  chan struct{}

	// out
	startStreaming chan struct{}
}

func (d *clientPrimaryDownloader) initialize() {
	d.chStreamTracks = make(chan streamTracksEntry)
	d.chStreamEnded = make(chan struct{})
	d.startStreaming = make(chan struct{})
}

func (d *clientPrimaryDownloader) run(ctx context.Context) error {
	d.onDownloadPrimaryPlaylist(d.primaryPlaylistURL.String())

	pl, err := clientDownloadPlaylist(ctx, d.httpClient, d.onRequest, d.primaryPlaylistURL)
	if err != nil {
		return err
	}

	streamCount := 0

	switch plt := pl.(type) {
	case *playlist.Media:
		ds := &clientStreamDownloader{
			isLeading:                true,
			httpClient:               d.httpClient,
			onRequest:                d.onRequest,
			onDownloadStreamPlaylist: d.onDownloadStreamPlaylist,
			onDownloadSegment:        d.onDownloadSegment,
			onDownloadPart:           d.onDownloadPart,
			onDecodeError:            d.onDecodeError,
			playlistURL:              d.primaryPlaylistURL,
			firstPlaylist:            plt,
			rp:                       d.rp,
			setStreamTracks:          d.setStreamTracks,
			setStreamEnded:           d.setStreamEnded,
			setLeadingTimeConv:       d.setLeadingTimeConv,
			getLeadingTimeConv:       d.getLeadingTimeConv,
		}
		d.rp.add(ds)
		streamCount++

	case *playlist.Multivariant:
		leadingPlaylist := pickLeadingPlaylist(plt.Variants)
		if leadingPlaylist == nil {
			return fmt.Errorf("no variants with supported codecs found")
		}

		var u *url.URL
		u, err = clientAbsoluteURL(d.primaryPlaylistURL, leadingPlaylist.URI)
		if err != nil {
			return err
		}

		ds := &clientStreamDownloader{
			isLeading:                true,
			httpClient:               d.httpClient,
			onRequest:                d.onRequest,
			onDownloadStreamPlaylist: d.onDownloadStreamPlaylist,
			onDownloadSegment:        d.onDownloadSegment,
			onDownloadPart:           d.onDownloadPart,
			onDecodeError:            d.onDecodeError,
			playlistURL:              u,
			firstPlaylist:            nil,
			rp:                       d.rp,
			setStreamTracks:          d.setStreamTracks,
			setStreamEnded:           d.setStreamEnded,
			setLeadingTimeConv:       d.setLeadingTimeConv,
			getLeadingTimeConv:       d.getLeadingTimeConv,
		}
		d.rp.add(ds)
		streamCount++

		if leadingPlaylist.Audio != "" {
			audioPlaylist := pickAudioPlaylist(plt.Renditions, leadingPlaylist.Audio)
			if audioPlaylist == nil {
				return fmt.Errorf("audio playlist with id \"%s\" not found", leadingPlaylist.Audio)
			}

			if audioPlaylist.URI != "" {
				u, err = clientAbsoluteURL(d.primaryPlaylistURL, audioPlaylist.URI)
				if err != nil {
					return err
				}

				ds := &clientStreamDownloader{
					isLeading:                false,
					onRequest:                d.onRequest,
					httpClient:               d.httpClient,
					onDownloadStreamPlaylist: d.onDownloadStreamPlaylist,
					onDownloadSegment:        d.onDownloadSegment,
					onDownloadPart:           d.onDownloadPart,
					onDecodeError:            d.onDecodeError,
					playlistURL:              u,
					firstPlaylist:            nil,
					rp:                       d.rp,
					setStreamTracks:          d.setStreamTracks,
					setLeadingTimeConv:       d.setLeadingTimeConv,
					getLeadingTimeConv:       d.getLeadingTimeConv,
					setStreamEnded:           d.setStreamEnded,
				}
				d.rp.add(ds)
				streamCount++
			}
		}

	default:
		return fmt.Errorf("invalid playlist")
	}

	var tracks []*Track

	for i := 0; i < streamCount; i++ {
		select {
		case entry := <-d.chStreamTracks:
			if entry.isLeading {
				tracks = append(append([]*Track(nil), entry.tracks...), tracks...)
			} else {
				tracks = append(tracks, entry.tracks...)
			}

		case <-ctx.Done():
			return fmt.Errorf("terminated")
		}
	}

	if len(tracks) == 0 {
		return fmt.Errorf("no supported tracks found")
	}

	d.clientTracks, err = d.setTracks(tracks)
	if err != nil {
		return err
	}

	close(d.startStreaming)

	for i := 0; i < streamCount; i++ {
		select {
		case <-d.chStreamEnded:
		case <-ctx.Done():
			return fmt.Errorf("terminated")
		}
	}

	return ErrClientEOS
}

func (d *clientPrimaryDownloader) setStreamTracks(
	ctx context.Context,
	isLeading bool,
	tracks []*Track,
) ([]*clientTrack, bool) {
	select {
	case d.chStreamTracks <- streamTracksEntry{
		isLeading: isLeading,
		tracks:    tracks,
	}:
	case <-ctx.Done():
		return nil, false
	}

	select {
	case <-d.startStreaming:
	case <-ctx.Done():
		return nil, false
	}

	streamClientTracks := make([]*clientTrack, len(tracks))
	for i, track := range tracks {
		streamClientTracks[i] = d.clientTracks[track]
	}

	return streamClientTracks, true
}

func (d *clientPrimaryDownloader) setStreamEnded(ctx context.Context) {
	select {
	case d.chStreamEnded <- struct{}{}:
	case <-ctx.Done():
	}
}
