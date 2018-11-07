package rtsp

import (
	"fmt"
	"log"
	"net"
	"sync"

	"github.com/penggy/EasyGoLib/utils"
)

type Server struct {
	TCPListener *net.TCPListener
	TCPPort     int
	Stoped      bool
	pushers     map[string]*Pusher // Path <-> Pusher
	pushersLock sync.RWMutex
}

var Instance *Server = &Server{
	Stoped:  true,
	TCPPort: utils.Conf().Section("rtsp").Key("port").MustInt(554),
	pushers: make(map[string]*Pusher),
}

func GetServer() *Server {
	return Instance
}

func (server *Server) Start() (err error) {
	addr, err := net.ResolveTCPAddr("tcp", fmt.Sprintf(":%d", server.TCPPort))
	if err != nil {
		return
	}
	listener, err := net.ListenTCP("tcp", addr)
	if err != nil {
		return
	}

	server.Stoped = false
	server.TCPListener = listener
	log.Println("rtsp server start on", server.TCPPort)
	networkBuffer := utils.Conf().Section("rtsp").Key("network_buffer").MustInt(1048576)
	for !server.Stoped {
		conn, err := server.TCPListener.AcceptTCP()
		if err != nil {
			log.Println(err)
			continue
		}
		if err := conn.SetReadBuffer(networkBuffer); err != nil {
			log.Printf("rtsp server conn set read buffer error, %v", err)
		}
		if err := conn.SetWriteBuffer(networkBuffer); err != nil {
			log.Printf("rtsp server conn set write buffer error, %v", err)
		}
		session := NewSession(server, conn)
		go session.Start()
	}
	return
}

func (server *Server) Stop() {
	log.Println("rtsp server stop on", server.TCPPort)
	server.Stoped = true
	if server.TCPListener != nil {
		server.TCPListener.Close()
		server.TCPListener = nil
	}
	server.pushersLock.Lock()
	server.pushers = make(map[string]*Pusher)
	server.pushersLock.Unlock()
}

func (server *Server) AddPusher(pusher *Pusher) {
	server.pushersLock.Lock()
	if _, ok := server.pushers[pusher.Path]; !ok {
		server.pushers[pusher.Path] = pusher
		go pusher.Start()
		log.Printf("%v start, now pusher size[%d]", pusher, len(server.pushers))
	}
	server.pushersLock.Unlock()
}

func (server *Server) RemovePusher(pusher *Pusher) {
	server.pushersLock.Lock()
	if _pusher, ok := server.pushers[pusher.Path]; ok && pusher.ID == _pusher.ID {
		delete(server.pushers, pusher.Path)
		log.Printf("%v end, now pusher size[%d]\n", pusher, len(server.pushers))
	}
	server.pushersLock.Unlock()
}

func (server *Server) GetPusher(path string) (pusher *Pusher) {
	server.pushersLock.RLock()
	pusher = server.pushers[path]
	server.pushersLock.RUnlock()
	return
}

func (server *Server) GetPushers() (pushers map[string]*Pusher) {
	pushers = make(map[string]*Pusher)
	server.pushersLock.RLock()
	for k, v := range server.pushers {
		pushers[k] = v
	}
	server.pushersLock.RUnlock()
	return
}

func (server *Server) GetPusherSize() (size int) {
	server.pushersLock.RLock()
	size = len(server.pushers)
	server.pushersLock.RUnlock()
	return
}
