package source

import (
	"easydarwin/internal/core/livestream"
	"fmt"
	"log/slog"
	"sync"
	"time"
)

var (
	channels     = make(map[int]*StreamClient)
	channelsLock sync.RWMutex
	LiveCore     *livestream.Core
)

func InitDb(Core *livestream.Core) {
	LiveCore = Core
}

func NewStreamClient(name, url string, id int) *StreamClient {
	var client = &StreamClient{
		Name:                name,
		URL:                 url,
		ChannelID:           id,
		TransType:           TransTypeTCP,
		ErrorString:         "Initializing…",
		Online:              0,
		OnDemandCloseSource: false,
		quit:                make(chan bool, 3),
		loopDuration:        time.Duration(10) * time.Second,
		TouchDuration:       time.Duration(60) * time.Second,
	}

	return client
}
func StartStream(live livestream.LiveStream) error {
	client, err := GetPgUacHandler(live.ID)
	if err != nil && client != nil {
		slog.Error(fmt.Sprintf("start stream %d %v", live.ID, err))
		return err
	}
	if client != nil {
		err = client.AddSession()
		if err != nil {
			slog.Error(fmt.Sprintf("start add session stream %d %v", live.ID, err))
			return err
		}
		client.UpdateTouchTime()
		if client.IsOpen() {
			return nil
		}
		client.UpdateOnDemandCloseSource(false)
		err = client.Open()
		if err != nil {
			slog.Error(fmt.Sprintf("start stream open %d %v", live.ID, err))
			return err
		}
	} else {
		client, err = AddStreamClient(live)
		if err != nil {
			slog.Error(fmt.Sprintf("start stream add client %d %v", live.ID, err))
			return err
		}
		if client != nil {
			err = client.AddSession()
			if err != nil {
				slog.Error(fmt.Sprintf("start add session stream %d %v", live.ID, err))
				return err
			}
			client.UpdateTouchTime()
			if client.IsOpen() {
				return nil
			}
			client.UpdateOnDemandCloseSource(false)
			err = client.Open()
			if err != nil {
				slog.Error(fmt.Sprintf("start stream add open %d %v", live.ID, err))
				return err
			}
		} else {
			slog.Error(fmt.Sprintf("start stream add open err %d %v", live.ID, err))
			return err
		}
	}
	return nil
}

func StopStream(live livestream.LiveStream) error {
	client, err := GetPgUacHandler(live.ID)
	if err != nil && client != nil {
		slog.Error(fmt.Sprintf("stop stream %d %v", live.ID, err))
		return err
	}
	if client != nil {
		if client.Session != nil {
			client.DelSession()
		}
		err = client.Stop()
		if err != nil {
			slog.Error(fmt.Sprintf("stop stream open %d %v", live.ID, err))
			return err
		}
	}

	return nil
}
func UpdateOnlineStream(live livestream.LiveStream) error {
	client, err := GetPgUacHandler(live.ID)
	if err != nil && client != nil {
		slog.Error(fmt.Sprintf("update start stream %d %v", live.ID, err))
		return err
	}
	if client != nil {
		if !client.OnDemand {
			err = client.AddSession()
			if err != nil {
				slog.Error(fmt.Sprintf("update start add session stream %d %v", live.ID, err))
				return err
			}
		}
		if client.IsOpen() && !client.IsTouchTime() {
			return nil
		}
		client.UpdateOnDemandCloseSource(true)
		err = client.Open()
		if err != nil {
			slog.Error(fmt.Sprintf("update start stream open %d %v", live.ID, err))
			return err
		}
	} else {
		client, err = AddStreamClient(live)
		if err != nil {
			slog.Error(fmt.Sprintf("update start stream add client %d %v", live.ID, err))
			return err
		}
		if client != nil {
			if !client.OnDemand {
				err = client.AddSession()
				if err != nil {
					slog.Error(fmt.Sprintf("update start add session stream %d %v", live.ID, err))
					return err
				}
			}
			if client.IsOpen() && !client.IsTouchTime() {
				return nil
			}
			client.UpdateOnDemandCloseSource(true)
			err = client.Open()
			if err != nil {
				slog.Error(fmt.Sprintf("update start stream add open %d %v", live.ID, err))
				return err
			}
		} else {
			slog.Error(fmt.Sprintf("update start stream add open err %d %v", live.ID, err))
			return err
		}
	}
	return nil
}
func UpdateStreamOnDemand(live livestream.LiveStream) error {
	client, err := GetPgUacHandler(live.ID)
	if err != nil && client != nil {
		slog.Error(fmt.Sprintf("update stop stream demand %d %v", live.ID, err))
		return err
	}
	if client != nil {
		client.UpdateOnDemandCloseSource(false)
	}
	return nil
}

func AddStreamClient(live livestream.LiveStream) (*StreamClient, error) {
	id := live.ID
	customizePubStreamName := fmt.Sprintf("%s%d", StreamName, id)
	client := NewStreamClient(customizePubStreamName, live.Url, id)
	client.OnDemand = live.OnDemand
	client.AudioEnable = live.Audio
	switch live.TransType {
	case "TCP":
		client.TransType = TransTypeTCP
	case "UDP":
		client.TransType = TransTypeUDP
	case "Multicast":
		client.TransType = TransTypeMulticast
	}
	client.TouchTime = time.Now()

	var exist bool
	channelsLock.Lock()
	if _, exist = channels[id]; !exist {
		channels[id] = client
		slog.Info(fmt.Sprintf("add stream ok %d", id))
	} else {
		slog.Error(fmt.Sprintf("add stream is already exist %d", id))
	}
	channelsLock.Unlock()
	if exist {
		return client, fmt.Errorf("add stream is already exist %d", id)
	}
	return client, nil
}

func DelStreamClient(id int) {
	channelsLock.Lock()
	exist := false
	ChannelID := 0
	for key, _ := range channels {
		if key == id {
			exist = true
			break
		}
	}
	if exist {
		ChannelID = channels[id].ChannelID
		delete(channels, ChannelID)
	}
	channelsLock.Unlock()
	slog.Info(fmt.Sprintf("del stream ok %d", ChannelID))

}

func GetPgUacHandler(id int) (obj *StreamClient, err error) {
	channelsLock.Lock()
	defer channelsLock.Unlock()

	for key, value := range channels {
		if key == id {
			return value, err
		}
	}
	err = fmt.Errorf("can not find channel[%d]", id)
	slog.Error(fmt.Sprintf("get handler %v", err))
	return nil, err
}
