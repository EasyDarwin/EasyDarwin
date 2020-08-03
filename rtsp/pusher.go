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
	players           map[string]*Player //SessionID <-> Player
	playersLock       sync.RWMutex
	gopCacheEnable    bool
	gopCache          []*RTPPack
	gopCacheLock      sync.RWMutex
	UDPServer         *UDPServer
	spsppsInSTAPaPack bool
	cond              *sync.Cond
	queue             []*RTPPack
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
	if pusher.RTSPClient.CustomPath != "" {
		return pusher.RTSPClient.CustomPath
	}
	return pusher.RTSPClient.Path
}

func (pusher *Pusher) ID() string {
	if pusher.Session != nil {
		return pusher.Session.ID
	}
	return pusher.RTSPClient.ID
}

func (pusher *Pusher) Logger() *log.Logger {
	if pusher.Session != nil {
		return pusher.Session.logger
	}
	return pusher.RTSPClient.logger
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

func (pusher *Pusher) Source() string {
	if pusher.Session != nil {
		return pusher.Session.URL
	}
	return pusher.RTSPClient.URL
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
	pusher.bindSession(session)
	return
}

func (pusher *Pusher) bindSession(session *Session) {
	pusher.Session = session
	session.RTPHandles = append(session.RTPHandles, func(pack *RTPPack) {
		if session != pusher.Session {
			session.logger.Printf("Session recv rtp to pusher.but pusher got a new session[%v].", pusher.Session.ID)
			return
		}
		pusher.QueueRTP(pack)
	})
	session.StopHandles = append(session.StopHandles, func() {
		if session != pusher.Session {
			session.logger.Printf("Session stop to release pusher.but pusher got a new session[%v].", pusher.Session.ID)
			return
		}
		pusher.ClearPlayer()
		pusher.Server().RemovePusher(pusher)
		pusher.cond.Broadcast()
		if pusher.UDPServer != nil {
			pusher.UDPServer.Stop()
			pusher.UDPServer = nil
		}
	})
}

func (pusher *Pusher) RebindSession(session *Session) bool {
	if pusher.RTSPClient != nil {
		pusher.Logger().Printf("call RebindSession[%s] to a Client-Pusher. got false", session.ID)
		return false
	}
	sess := pusher.Session
	pusher.bindSession(session)
	session.Pusher = pusher

	pusher.gopCacheLock.Lock()
	pusher.gopCache = make([]*RTPPack, 0)
	pusher.gopCacheLock.Unlock()
	if sess != nil {
		sess.Stop()
	}
	return true
}

func (pusher *Pusher) RebindClient(client *RTSPClient) bool {
	if pusher.Session != nil {
		pusher.Logger().Printf("call RebindClient[%s] to a Session-Pusher. got false", client.ID)
		return false
	}
	sess := pusher.RTSPClient
	pusher.RTSPClient = client
	if sess != nil {
		sess.Stop()
	}
	return true
}

func (pusher *Pusher) QueueRTP(pack *RTPPack) *Pusher {
	pusher.cond.L.Lock()
	pusher.queue = append(pusher.queue, pack)
	pusher.cond.Signal()
	pusher.cond.L.Unlock()
	return pusher
}

func (pusher *Pusher) Start() {
	logger := pusher.Logger()
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
				logger.Printf("pusher not stoped, but queue take out nil pack")
			}
			continue
		}

		if pusher.gopCacheEnable && pack.Type == RTP_TYPE_VIDEO {
			pusher.gopCacheLock.Lock()
			if rtp := ParseRTP(pack.Buffer.Bytes()); rtp != nil && pusher.shouldSequenceStart(rtp) {
				pusher.gopCache = make([]*RTPPack, 0)
			}
			pusher.gopCache = append(pusher.gopCache, pack)
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

func (pusher *Pusher) HasPlayer(player *Player) bool {
	pusher.playersLock.Lock()
	_, ok := pusher.players[player.ID]
	pusher.playersLock.Unlock()
	return ok
}

func (pusher *Pusher) AddPlayer(player *Player) *Pusher {
	logger := pusher.Logger()
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
		logger.Printf("%v start, now player size[%d]", player, len(pusher.players))
	}
	pusher.playersLock.Unlock()
	return pusher
}

func (pusher *Pusher) RemovePlayer(player *Player) *Pusher {
	logger := pusher.Logger()
	pusher.playersLock.Lock()
	if len(pusher.players) == 0 {
		pusher.playersLock.Unlock()
		return pusher
	}
	delete(pusher.players, player.ID)
	logger.Printf("%v end, now player size[%d]\n", player, len(pusher.players))
	pusher.playersLock.Unlock()
	return pusher
}

func (pusher *Pusher) ClearPlayer() {
	// copy a new map to avoid deadlock
	pusher.playersLock.Lock()
	players := pusher.players
	pusher.players = make(map[string]*Player)
	pusher.playersLock.Unlock()
	go func() { // do not block
		for _, v := range players {
			v.Stop()
		}
	}()
}

func (pusher *Pusher) shouldSequenceStart(rtp *RTPInfo) bool {
	if strings.EqualFold(pusher.VCodec(), "h264") {
		var realNALU uint8
		payloadHeader := rtp.Payload[0] //https://tools.ietf.org/html/rfc6184#section-5.2
		NaluType := uint8(payloadHeader & 0x1F)
		// log.Printf("RTP Type:%d", NaluType)
		switch {
		case NaluType <= 23:
			realNALU = rtp.Payload[0]
			// log.Printf("Single NAL:%d", NaluType)
		case NaluType == 28 || NaluType == 29:
			realNALU = rtp.Payload[1]
			if realNALU&0x40 != 0 {
				// log.Printf("FU NAL End :%02X", realNALU)
			}
			if realNALU&0x80 != 0 {
				// log.Printf("FU NAL Begin :%02X", realNALU)
			} else {
				return false
			}
		case NaluType == 24:
			// log.Printf("STAP-A")
			off := 1
			singleSPSPPS := 0
			for {
				nalSize := ((uint16(rtp.Payload[off])) << 8) | uint16(rtp.Payload[off+1])
				if nalSize < 1 {
					return false
				}
				off += 2
				nalUnit := rtp.Payload[off : off+int(nalSize)]
				off += int(nalSize)
				realNALU = nalUnit[0]
				singleSPSPPS += int(realNALU & 0x1F)
				if off >= len(rtp.Payload) {
					break
				}
			}
			if singleSPSPPS == 0x0F {
				pusher.spsppsInSTAPaPack = true
				return true
			}
		}
		if realNALU&0x1F == 0x05 {
			if pusher.spsppsInSTAPaPack {
				return false
			}
			return true
		}
		if realNALU&0x1F == 0x07 { // maybe sps pps header + key frame?
			if len(rtp.Payload) < 200 { // consider sps pps header only.
				return true
			}
			return true
		}
		return false
	} else if strings.EqualFold(pusher.VCodec(), "h265") {
		if len(rtp.Payload) >= 3 {
			firstByte := rtp.Payload[0]
			headerType := (firstByte >> 1) & 0x3f
			var frameType uint8
			if headerType == 49 { //Fragmentation Units

				FUHeader := rtp.Payload[2]
				/*
				   +---------------+
				   |0|1|2|3|4|5|6|7|
				   +-+-+-+-+-+-+-+-+
				   |S|E|  FuType   |
				   +---------------+
				*/
				rtpStart := (FUHeader & 0x80) != 0
				if !rtpStart {
					if (FUHeader & 0x40) != 0 {
						//log.Printf("FU frame end")
					}
					return false
				} else {
					//log.Printf("FU frame start")
				}
				frameType = FUHeader & 0x3f
			} else if headerType == 48 { //Aggregation Packets

			} else if headerType == 50 { //PACI Packets

			} else { // Single NALU
				/*
					+---------------+---------------+
					|0|1|2|3|4|5|6|7|0|1|2|3|4|5|6|7|
					+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
					|F|   Type    |  LayerId  | TID |
					+-------------+-----------------+
				*/
				frameType = firstByte & 0x7e
			}
			if frameType >= 16 && frameType <= 21 {
				return true
			}
			if frameType == 32 {
				// vps sps pps...
				if len(rtp.Payload) < 200 { // consider sps pps header only.
					return false
				}
				return true
			}
		}
		return false
	}
	return false
}
