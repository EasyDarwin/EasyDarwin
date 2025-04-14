package source

import (
	"fmt"
	"github.com/q191201771/lal/pkg/rtsp"
	"log/slog"
)

func (client *StreamClient) Open() (err error) {
	if client.Status != STREAM_STOPED {
		return
	}
	client.Status = STREAM_OPENING
	if client.pullSession != nil {
		client.Status = STREAM_OPENED
		return
	}

	defer func() {
		if err != nil {
			client.Status = STREAM_STOPED
			client.pullSession.Dispose()
			client.pullSession = nil
		}
	}()

	client.pullSession = rtsp.NewPullSession(client, func(option *rtsp.PullSessionOption) {
		option.PullTimeoutMs = 10000
		option.OverTcp = client.TransType == TransTypeTCP
	})
	err = client.pullSession.Pull(client.URL)
	if err != nil {
		client.SetOnline(OffLineState)
		return
	}
	if client.Status == STREAM_STOPING {
		err = fmt.Errorf("%v opened but too late", client.ChannelID)
		return
	}
	client.IsSnap = true
	client.Status = STREAM_OPENED
	go client.Start()
	slog.Info(fmt.Sprintf("%v opened", client.ChannelID))
	return
}

func (client *StreamClient) Stop() (err error) {
	if client.Status == STREAM_STOPED || client.Status == STREAM_STOPING {
		return
	}
	if client.Status == STREAM_OPENING {
		client.Status = STREAM_STOPING
		return
	}
	client.Status = STREAM_STOPING

	if client.pullSession == nil {
		return
	}
	client.pullSession.Dispose()
	client.pullSession = nil
	if client.Online == LivingState {
		client.SetOnline(OnLineState)
	}
	client.Status = STREAM_STOPED
	client.IsSnap = false
	slog.Info(fmt.Sprintf("%v stoped", client.ChannelID))
	return
}
