package rtsp

import (
	"log"
	"strings"
	"sync"
	"time"

	"github.com/penggy/EasyGoLib/utils"
)

type Pusher struct {
	*Session
	*RTSPClient
	players        map[string]*Player //SessionID <-> Player
	playersLock    sync.RWMutex
	gopCacheEnable bool
	gopCache       []*RTPPack
	gopCacheLock   sync.RWMutex
	UDPServer      *UDPServer

	cond  *sync.Cond
	queue []*RTPPack
}

func (pusher *Pusher) String() string {
	if pusher.Session != nil {
		return pusher.Session.String()
	}
	return pusher.RTSPClient.String()
}

func (pusher *Pusher) Server() *Server {
	if pusher.Session != nil {
		return pusher.Session.Server
	}
	return pusher.RTSPClient.Server
}

func (pusher *Pusher) SDPRaw() string {
	if pusher.Session != nil {
		return pusher.Session.SDPRaw
	}
	return pusher.RTSPClient.SDPRaw
}

func (pusher *Pusher) Stoped() bool {
	if pusher.Session != nil {
		return pusher.Session.Stoped
	}
	return pusher.RTSPClient.Stoped
}

func (pusher *Pusher) Path() string {
	if pusher.Session != nil {
		return pusher.Session.Path
	}
	return pusher.RTSPClient.Path
}

func (pusher *Pusher) ID() string {
	if pusher.Session != nil {
		return pusher.Session.ID
	}
	return pusher.RTSPClient.ID
}

func (pusher *Pusher) VCodec() string {
	if pusher.Session != nil {
		return pusher.Session.VCodec
	}
	return pusher.RTSPClient.VCodec
}

func (pusher *Pusher) ACodec() string {
	if pusher.Session != nil {
		return pusher.Session.ACodec
	}
	return pusher.RTSPClient.ACodec
}

func (pusher *Pusher) AControl() string {
	if pusher.Session != nil {
		return pusher.Session.AControl
	}
	return pusher.RTSPClient.AControl
}

func (pusher *Pusher) VControl() string {
	if pusher.Session != nil {
		return pusher.Session.VControl
	}
	return pusher.RTSPClient.VControl
}

func (pusher *Pusher) URL() string {
	if pusher.Session != nil {
		return pusher.Session.URL
	}
	return pusher.RTSPClient.URL
}

func (pusher *Pusher) AddOutputBytes(size int) {
	if pusher.Session != nil {
		pusher.Session.OutBytes += size
		return
	}
	pusher.RTSPClient.OutBytes += size
}

func (pusher *Pusher) InBytes() int {
	if pusher.Session != nil {
		return pusher.Session.InBytes
	}
	return pusher.RTSPClient.InBytes
}

func (pusher *Pusher) OutBytes() int {
	if pusher.Session != nil {
		return pusher.Session.OutBytes
	}
	return pusher.RTSPClient.OutBytes
}

func (pusher *Pusher) TransType() string {
	if pusher.Session != nil {
		return pusher.Session.TransType.String()
	}
	return pusher.RTSPClient.TransType.String()
}

func (pusher *Pusher) StartAt() time.Time {
	if pusher.Session != nil {
		return pusher.Session.StartAt
	}
	return pusher.RTSPClient.StartAt
}

func NewClientPusher(client *RTSPClient) (pusher *Pusher) {
	pusher = &Pusher{
		RTSPClient:     client,
		Session:        nil,
		players:        make(map[string]*Player),
		gopCacheEnable: utils.Conf().Section("rtsp").Key("gop_cache_enable").MustBool(true),
		gopCache:       make([]*RTPPack, 0),

		cond:  sync.NewCond(&sync.Mutex{}),
		queue: make([]*RTPPack, 0),
	}
	client.RTPHandles = append(client.RTPHandles, func(pack *RTPPack) {
		pusher.QueueRTP(pack)
	})
	client.StopHandles = append(client.StopHandles, func() {
		pusher.ClearPlayer()
		pusher.Server().RemovePusher(pusher)
		pusher.cond.Broadcast()
	})
	return
}

func NewPusher(session *Session) (pusher *Pusher) {
	pusher = &Pusher{
		Session:        session,
		RTSPClient:     nil,
		players:        make(map[string]*Player),
		gopCacheEnable: utils.Conf().Section("rtsp").Key("gop_cache_enable").MustBool(true),
		gopCache:       make([]*RTPPack, 0),

		cond:  sync.NewCond(&sync.Mutex{}),
		queue: make([]*RTPPack, 0),
	}
	session.RTPHandles = append(session.RTPHandles, func(pack *RTPPack) {
		pusher.QueueRTP(pack)
	})
	session.StopHandles = append(session.StopHandles, func() {
		pusher.ClearPlayer()
		pusher.Server().RemovePusher(pusher)
		pusher.cond.Broadcast()
		if pusher.UDPServer != nil {
			pusher.UDPServer.Stop()
			pusher.UDPServer = nil
		}
	})
	return
}

func (pusher *Pusher) QueueRTP(pack *RTPPack) *Pusher {
	pusher.cond.L.Lock()
	pusher.queue = append(pusher.queue, pack)
	pusher.cond.Signal()
	pusher.cond.L.Unlock()
	return pusher
}

func (pusher *Pusher) Start() {
	for !pusher.Stoped() {
		var pack *RTPPack
		pusher.cond.L.Lock()
		if len(pusher.queue) == 0 {
			pusher.cond.Wait()
		}
		if len(pusher.queue) > 0 {
			pack = pusher.queue[0]
			pusher.queue = pusher.queue[1:]
		}
		pusher.cond.L.Unlock()
		if pack == nil {
			if !pusher.Stoped() {
				log.Printf("pusher not stoped, but queue take out nil pack")
			}
			continue
		}

		if pusher.gopCacheEnable {
			pusher.gopCacheLock.Lock()
			if strings.EqualFold(pusher.VCodec(), "h264") {
				if rtp := ParseRTP(pack.Buffer.Bytes()); rtp != nil && rtp.IsKeyframeStart() {
					pusher.gopCache = make([]*RTPPack, 0)
				}
				pusher.gopCache = append(pusher.gopCache, pack)
			} else if strings.EqualFold(pusher.VCodec(), "h265") {
				if rtp := ParseRTP(pack.Buffer.Bytes()); rtp != nil && rtp.IsKeyframeStartH265() {
					pusher.gopCache = make([]*RTPPack, 0)
				}
				pusher.gopCache = append(pusher.gopCache, pack)
			}
			pusher.gopCacheLock.Unlock()
		}

		pusher.BroadcastRTP(pack)
	}
}

func (pusher *Pusher) Stop() {
	if pusher.Session != nil {
		pusher.Session.Stop()
		return
	}
	pusher.RTSPClient.Stop()
}

func (pusher *Pusher) BroadcastRTP(pack *RTPPack) *Pusher {
	for _, player := range pusher.GetPlayers() {
		player.QueueRTP(pack)
		pusher.AddOutputBytes(pack.Buffer.Len())
	}
	return pusher
}

func (pusher *Pusher) GetPlayers() (players map[string]*Player) {
	players = make(map[string]*Player)
	pusher.playersLock.RLock()
	for k, v := range pusher.players {
		players[k] = v
	}
	pusher.playersLock.RUnlock()
	return
}

func (pusher *Pusher) AddPlayer(player *Player) *Pusher {
	if pusher.gopCacheEnable {
		pusher.gopCacheLock.RLock()
		for _, pack := range pusher.gopCache {
			player.QueueRTP(pack)
			pusher.AddOutputBytes(pack.Buffer.Len())
		}
		pusher.gopCacheLock.RUnlock()
	}

	pusher.playersLock.Lock()
	if _, ok := pusher.players[player.ID]; !ok {
		pusher.players[player.ID] = player
		go player.Start()
		log.Printf("%v start, now player size[%d]", player, len(pusher.players))
	}
	pusher.playersLock.Unlock()
	return pusher
}

func (pusher *Pusher) RemovePlayer(player *Player) *Pusher {
	pusher.playersLock.Lock()
	if len(pusher.players) == 0 {
		pusher.playersLock.Unlock()
		return pusher
	}
	delete(pusher.players, player.ID)
	log.Printf("%v end, now player size[%d]\n", player, len(pusher.players))
	pusher.playersLock.Unlock()
	return pusher
}

func (pusher *Pusher) ClearPlayer() {
	// copy a new map to avoid deadlock
	players := make(map[string]*Player)
	pusher.playersLock.Lock()
	for k, v := range pusher.players {
		//v.Stop()
		players[k] = v
	}
	pusher.players = make(map[string]*Player)
	pusher.playersLock.Unlock()

	for _, v := range players {
		v.Stop()
	}
}
