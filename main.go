package main

import (
	"context"
	"flag"
	"fmt"
	"log"
	"net/http"
	"strings"
	"time"

	"github.com/rikugun/EasyGoLib/db"

	"github.com/EasyDarwin/EasyDarwin/models"
	"github.com/EasyDarwin/EasyDarwin/routers"
	"github.com/EasyDarwin/EasyDarwin/rtsp"
	figure "github.com/common-nighthawk/go-figure"
	"github.com/penggy/service"
	"github.com/rikugun/EasyGoLib/utils"
)

var (
	gitCommitCode string
	buildDateTime string
)

type program struct {
	httpPort   int
	httpServer *http.Server
	rtspPort   int
	rtspServer *rtsp.Server
}

func (p *program) StopHTTP() (err error) {
	if p.httpServer == nil {
		err = fmt.Errorf("HTTP Server Not Found")
		return
	}
	ctx, cancel := context.WithTimeout(context.Background(), 5*time.Second)
	defer cancel()
	if err = p.httpServer.Shutdown(ctx); err != nil {
		return
	}
	return
}

func (p *program) StartHTTP() (err error) {
	p.httpServer = &http.Server{
		Addr:              fmt.Sprintf(":%d", p.httpPort),
		Handler:           routers.Router,
		ReadHeaderTimeout: 5 * time.Second,
	}
	link := fmt.Sprintf("http://%s:%d", utils.LocalIP(), p.httpPort)
	log.Println("http server start -->", link)
	go func() {
		if err := p.httpServer.ListenAndServe(); err != nil && err != http.ErrServerClosed {
			log.Println("start http server error", err)
		}
		log.Println("http server end")
	}()
	return
}

func (p *program) StartRTSP() (err error) {
	if p.rtspServer == nil {
		err = fmt.Errorf("RTSP Server Not Found")
		return
	}
	sport := ""
	if p.rtspPort != 554 {
		sport = fmt.Sprintf(":%d", p.rtspPort)
	}
	link := fmt.Sprintf("rtsp://%s%s", utils.LocalIP(), sport)
	log.Println("rtsp server start -->", link)
	go func() {
		if err := p.rtspServer.Start(); err != nil {
			log.Println("start rtsp server error", err)
		}
		log.Println("rtsp server end")
	}()
	return
}

func (p *program) StopRTSP() (err error) {
	if p.rtspServer == nil {
		err = fmt.Errorf("RTSP Server Not Found")
		return
	}
	p.rtspServer.Stop()
	return
}

func (p *program) Start(s service.Service) (err error) {
	log.Println("********** START **********")
	if utils.IsPortInUse(p.httpPort) {
		err = fmt.Errorf("HTTP port[%d] In Use", p.httpPort)
		return
	}
	if utils.IsPortInUse(p.rtspPort) {
		err = fmt.Errorf("RTSP port[%d] In Use", p.rtspPort)
		return
	}
	err = models.Init()
	if err != nil {
		return
	}
	err = routers.Init()
	if err != nil {
		return
	}
	p.StartRTSP()
	p.StartHTTP()

	if !utils.Debug {
		log.Println("log files -->", utils.LogDir())
		log.SetOutput(utils.GetLogWriter())
	}
	go func() {
		for range routers.API.RestartChan {
			p.StopHTTP()
			p.StopRTSP()
			utils.ReloadConf()
			p.StartRTSP()
			p.StartHTTP()
		}
	}()

	go func() {
		log.Printf("demon pull streams")
		for {
			var streams []models.Stream
			db.SQLite.Find(&streams)
			if err := db.SQLite.Find(&streams).Error; err != nil {
				log.Printf("find stream err:%v", err)
				return
			}
			for i := len(streams) - 1; i > -1; i-- {
				v := streams[i]
				agent := fmt.Sprintf("EasyDarwinGo/%s", routers.BuildVersion)
				if routers.BuildDateTime != "" {
					agent = fmt.Sprintf("%s(%s)", agent, routers.BuildDateTime)
				}
				client, err := rtsp.NewRTSPClient(rtsp.GetServer(), v.URL, int64(v.HeartbeatInterval)*1000, agent)
				if err != nil {
					continue
				}
				client.CustomPath = v.CustomPath

				pusher := rtsp.NewClientPusher(client)
				if rtsp.GetServer().GetPusher(pusher.Path()) != nil {
					continue
				}
				err = client.Start(time.Duration(v.IdleTimeout) * time.Second)
				if err != nil {
					log.Printf("Pull stream err :%v", err)
					continue
				}
				rtsp.GetServer().AddPusher(pusher)
				//streams = streams[0:i]
				//streams = append(streams[:i], streams[i+1:]...)
			}
			time.Sleep(10 * time.Second)
		}
	}()
	return
}

func (p *program) Stop(s service.Service) (err error) {
	defer log.Println("********** STOP **********")
	defer utils.CloseLogWriter()
	p.StopHTTP()
	p.StopRTSP()
	models.Close()
	return
}

func main() {
	flag.StringVar(&utils.FlagVarConfFile, "config", "", "configure file path")
	flag.Parse()
	tail := flag.Args()

	// log
	log.SetPrefix("[EasyDarwin] ")
	log.SetFlags(log.Lshortfile | log.LstdFlags)

	log.Printf("git commit code:%s", gitCommitCode)
	log.Printf("build date:%s", buildDateTime)
	routers.BuildVersion = fmt.Sprintf("%s.%s", routers.BuildVersion, gitCommitCode)
	routers.BuildDateTime = buildDateTime

	sec := utils.Conf().Section("service")
	svcConfig := &service.Config{
		Name:        sec.Key("name").MustString("EasyDarwin_Service"),
		DisplayName: sec.Key("display_name").MustString("EasyDarwin_Service"),
		Description: sec.Key("description").MustString("EasyDarwin_Service"),
	}

	httpPort := utils.Conf().Section("http").Key("port").MustInt(10008)
	rtspServer := rtsp.GetServer()
	p := &program{
		httpPort:   httpPort,
		rtspPort:   rtspServer.TCPPort,
		rtspServer: rtspServer,
	}
	s, err := service.New(p, svcConfig)
	if err != nil {
		log.Println(err)
		utils.PauseExit()
	}
	if len(tail) > 0 {
		cmd := strings.ToLower(tail[0])
		if cmd == "install" || cmd == "stop" || cmd == "start" || cmd == "uninstall" {
			figure.NewFigure("EasyDarwin", "", false).Print()
			log.Println(svcConfig.Name, cmd, "...")
			if err = service.Control(s, cmd); err != nil {
				log.Println(err)
				utils.PauseExit()
			}
			log.Println(svcConfig.Name, cmd, "ok")
			return
		}
	}
	figure.NewFigure("EasyDarwin", "", false).Print()
	if err = s.Run(); err != nil {
		log.Println(err)
		utils.PauseExit()
	}
}
