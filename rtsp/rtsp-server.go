package rtsp

import (
	"fmt"
	"log"
	"net"
	"os"
	"os/exec"
	"path"
	"sync"
	"syscall"
	"time"

	"github.com/penggy/EasyGoLib/utils"
)

type Server struct {
	TCPListener    *net.TCPListener
	TCPPort        int
	Stoped         bool
	pushers        map[string]*Pusher // Path <-> Pusher
	pushersLock    sync.RWMutex
	addPusherCh    chan *Pusher
	removePusherCh chan *Pusher
}

var Instance *Server = nil

func GetServer() *Server {
	if Instance == nil {
		Instance = &Server{
			Stoped:         true,
			TCPPort:        utils.Conf().Section("rtsp").Key("port").MustInt(554),
			pushers:        make(map[string]*Pusher),
			addPusherCh:    make(chan *Pusher),
			removePusherCh: make(chan *Pusher),
		}
	}
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

	localRecord := utils.Conf().Section("rtsp").Key("save_stream_to_mp4").MustInt(0)
	ffmpeg := utils.Conf().Section("rtsp").Key("ffmpeg_path").MustString("")
	mp4Path := utils.Conf().Section("rtsp").Key("mp4_dir_path").MustString("")
	SaveStreamToLocal := false
	if (len(ffmpeg) > 0) && localRecord > 0 && len(mp4Path) > 0 {
		err := utils.EnsureDir(mp4Path)
		if err != nil {
			log.Printf("Create mp4_dir_path[%s] err:%v.", mp4Path, err)
		} else {
			SaveStreamToLocal = true
		}
	}
	go func() { // save to local.
		pusher2ffmpegMap := make(map[*Pusher]*exec.Cmd)
		if SaveStreamToLocal {
			log.Printf("Prepare to save stream to local....")
			defer log.Printf("End save stream to local....")
		}
		var pusher *Pusher
		addChnOk := true
		removeChnOk := true
		for addChnOk || removeChnOk {
			select {
			case pusher, addChnOk = <-server.addPusherCh:
				if SaveStreamToLocal {
					if addChnOk {
						dir := path.Join(mp4Path, pusher.Path())
						err := utils.EnsureDir(dir)
						if err != nil {
							log.Printf("EnsureDir:[%s] err:%v.", dir, err)
							continue
						}
						path := path.Join(dir, fmt.Sprintf("%s.mp4", time.Now().Format("20060102150405")))
						cmd := exec.Command(ffmpeg, "-i", pusher.URL(), "-c:v", "copy", "-c:a", "copy", path)
						cmd.Stdout = os.Stdout
						cmd.Stderr = os.Stderr
						err = cmd.Start()
						if err != nil {
							log.Printf("Start ffmpeg err:%v", err)
						}
						pusher2ffmpegMap[pusher] = cmd
						log.Printf("add ffmpeg to pull stream from pusher[%v]", pusher)
					} else {
						log.Printf("addPusherChan closed")
					}
				}
			case pusher, removeChnOk = <-server.removePusherCh:
				if SaveStreamToLocal {
					if removeChnOk {
						cmd := pusher2ffmpegMap[pusher]
						proc := cmd.Process
						if proc != nil {
							log.Printf("prepare to SIGTERM to process:%v", proc)
							proc.Signal(syscall.SIGTERM)
							// proc.Kill()
						}
						delete(pusher2ffmpegMap, pusher)
						log.Printf("delete ffmpeg from pull stream from pusher[%v]", pusher)
					} else {
						for _, cmd := range pusher2ffmpegMap {
							proc := cmd.Process
							if proc != nil {
								log.Printf("prepare to SIGTERM to process:%v", proc)
								proc.Signal(syscall.SIGTERM)
							}
						}
						pusher2ffmpegMap = make(map[*Pusher]*exec.Cmd)
						log.Printf("removePusherChan closed")
					}
				}
			}
		}
	}()

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

	close(server.addPusherCh)
	close(server.removePusherCh)
}

func (server *Server) AddPusher(pusher *Pusher) {
	added := false
	server.pushersLock.Lock()
	if _, ok := server.pushers[pusher.Path()]; !ok {
		server.pushers[pusher.Path()] = pusher
		go pusher.Start()
		log.Printf("%v start, now pusher size[%d]", pusher, len(server.pushers))
		added = true
	}
	server.pushersLock.Unlock()
	if added {
		server.addPusherCh <- pusher
	}
}

func (server *Server) RemovePusher(pusher *Pusher) {
	removed := false
	server.pushersLock.Lock()
	if _pusher, ok := server.pushers[pusher.Path()]; ok && pusher.ID() == _pusher.ID() {
		delete(server.pushers, pusher.Path())
		log.Printf("%v end, now pusher size[%d]\n", pusher, len(server.pushers))
		removed = true
	}
	server.pushersLock.Unlock()
	if removed {
		server.removePusherCh <- pusher
	}
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
