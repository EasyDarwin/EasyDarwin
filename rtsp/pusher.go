package rtsp

import (
	"log"
	"strings"
	"sync"

	"github.com/penggy/EasyGoLib/utils"
)

type Pusher struct {
	*Session

	players        map[string]*Player //SessionID <-> Player
	playersLock    sync.RWMutex
	gopCacheEnable bool
	gopCache       []*RTPPack
	gopCacheLock   sync.RWMutex
	UDPServer      *UDPServer

	cond  *sync.Cond
	queue []*RTPPack
}

func NewPusher(session *Session) (pusher *Pusher) {
	pusher = &Pusher{
		Session:        session,
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
		pusher.Server.RemovePusher(pusher)
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
	for !pusher.Stoped {
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
			if !pusher.Stoped {
				log.Printf("pusher not stoped, but queue take out nil pack")
			}
			continue
		}

		if pusher.gopCacheEnable {
			pusher.gopCacheLock.Lock()
			if strings.EqualFold(pusher.VCodec, "h264") {
				if rtp := ParseRTP(pack.Buffer.Bytes()); rtp != nil && rtp.IsKeyframeStart() {
					pusher.gopCache = make([]*RTPPack, 0)
				}
				pusher.gopCache = append(pusher.gopCache, pack)
			} else if strings.EqualFold(pusher.VCodec, "h265") {
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

func (pusher *Pusher) BroadcastRTP(pack *RTPPack) *Pusher {
	for _, player := range pusher.GetPlayers() {
		player.QueueRTP(pack)
		pusher.OutBytes += pack.Buffer.Len()
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
			pusher.OutBytes += pack.Buffer.Len()
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
	delete(pusher.players, player.ID)
	log.Printf("%v end, now player size[%d]\n", player, len(pusher.players))
	pusher.playersLock.Unlock()
	return pusher
}
