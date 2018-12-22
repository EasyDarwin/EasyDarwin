package rtsp

import (
	"sync"
)

type Player struct {
	*Session
	Pusher *Pusher
	cond   *sync.Cond
	queue  []*RTPPack
}

func NewPlayer(session *Session, pusher *Pusher) (player *Player) {
	player = &Player{
		Session: session,
		Pusher:  pusher,
		cond:    sync.NewCond(&sync.Mutex{}),
		queue:   make([]*RTPPack, 0),
	}
	session.StopHandles = append(session.StopHandles, func() {
		pusher.RemovePlayer(player)
		player.cond.Broadcast()
	})
	return
}

func (player *Player) QueueRTP(pack *RTPPack) *Player {
	logger := player.logger
	if pack == nil {
		logger.Printf("player queue enter nil pack, drop it")
		return player
	}
	player.cond.L.Lock()
	player.queue = append(player.queue, pack)
	player.cond.Signal()
	player.cond.L.Unlock()
	return player
}

func (player *Player) Start() {
	logger := player.logger
	for !player.Stoped {
		var pack *RTPPack
		player.cond.L.Lock()
		if len(player.queue) == 0 {
			player.cond.Wait()
		}
		if len(player.queue) > 0 {
			pack = player.queue[0]
			player.queue = player.queue[1:]
		}
		player.cond.L.Unlock()
		if pack == nil {
			if !player.Stoped {
				logger.Printf("player not stoped, but queue take out nil pack")
			}
			continue
		}
		if err := player.SendRTP(pack); err != nil {
			logger.Println(err)
		}
	}
}
