package rtsp

import (
	"sync"
	"time"
	"github.com/penggy/EasyGoLib/utils"
)

type Player struct {
	*Session
	Pusher *Pusher
	cond   *sync.Cond
	queue  []*RTPPack
	queueLimit int
	dropPacketWhenPaused bool
	paused bool
}

func NewPlayer(session *Session, pusher *Pusher) (player *Player) {
	queueLimit := utils.Conf().Section("rtsp").Key("player_queue_limit").MustInt(0)
	dropPacketWhenPaused := utils.Conf().Section("rtsp").Key("drop_packet_when_paused").MustInt(0)
	player = &Player{
		Session: session,
		Pusher:  pusher,
		cond:    sync.NewCond(&sync.Mutex{}),
		queue:   make([]*RTPPack, 0),
		queueLimit: queueLimit,
		dropPacketWhenPaused: dropPacketWhenPaused != 0,
		paused:  false,
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
	if player.paused && player.dropPacketWhenPaused {
		return player
	}
	player.cond.L.Lock()
	player.queue = append(player.queue, pack)
	if oldLen := len(player.queue); player.queueLimit > 0 && oldLen > player.queueLimit {
		player.queue = player.queue[1:]
		if player.debugLogEnable {
			len := len(player.queue)
			logger.Printf("Player %s, QueueRTP, exceeds limit(%d), drop %d old packets, current queue.len=%d\n", player.String(), player.queueLimit, oldLen - len, len)
		}
	}
	player.cond.Signal()
	player.cond.L.Unlock()
	return player
}

func (player *Player) Start() {
	logger := player.logger
	timer := time.Unix(0, 0)
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
		queueLen := len(player.queue)
		player.cond.L.Unlock()
		if player.paused {
			continue
		}
		if pack == nil {
			if !player.Stoped {
				logger.Printf("player not stoped, but queue take out nil pack")
			}
			continue
		}
		if err := player.SendRTP(pack); err != nil {
			logger.Println(err)
		}
		elapsed := time.Now().Sub(timer)
		if player.debugLogEnable && elapsed >= 30*time.Second {
			logger.Printf("Player %s, Send a package.type:%d, queue.len=%d\n", player.String(), pack.Type, queueLen)
			timer = time.Now()
		}
	}
}

func (player *Player) Pause(paused bool) {
	if paused {
		player.logger.Printf("Player %s, Pause\n", player.String())
	} else {
		player.logger.Printf("Player %s, Play\n", player.String())
	}
	player.cond.L.Lock()
	if paused && player.dropPacketWhenPaused && len(player.queue) > 0 {
		player.queue = make([]*RTPPack, 0)
	}
	player.paused = paused
	player.cond.L.Unlock()
}