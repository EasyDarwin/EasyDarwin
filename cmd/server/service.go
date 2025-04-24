// Copyright 2025 EasyDarwin.
// http://www.easydarwin.org
// 将软件制作成服务程序
// History (ID, Time, Desc)
// (xukongzangpusa, 20250424, add)
package main

import (
	"context"
	"easydarwin/internal/core/source"
	"easydarwin/internal/core/svr"
	"easydarwin/utils/pkg/conc"
	"easydarwin/utils/pkg/server"
	"fmt"
	"github.com/kardianos/service"
	"log/slog"
	"os"
	"os/exec"
	"os/signal"
	"path/filepath"
	"runtime"
	"syscall"
	"time"
)

type program struct {
}

func (p *program) Start(s service.Service) error {
	// Start should not block. Do the actual work async.
	go p.run()
	return nil
}

func (p *program) Stop(s service.Service) error {
	// Stop should not block. Return with a few seconds.
	return nil
}

func (p *program) run() {
	// Do work here
	// 启动流媒体 sugar
	svr.Start(&gCfg)
	g := conc.New(gLog)
	// 启动定时任务
	time.AfterFunc(time.Duration(15)*time.Second, func() {
		source.StartScheduler()
	})

	bin, _ := os.Executable()
	if err := os.Chdir(filepath.Dir(bin)); err != nil {
		slog.Error("change dir error")
	}
	var err error
	gHttpHandler, err = wireApp(&gCfg)
	if err != nil {
		slog.Error("程序构建失败", "err", err)
		panic(err)
		os.Exit(0)
	}

	svcServer := server.New(gHttpHandler,
		server.Port(gCfg.DefaultHttpConfig.HttpListenAddr),
		server.ReadTimeout(time.Duration(gCfg.Base.Timeout)*time.Second),
		server.WriteTimeout(time.Duration(gCfg.Base.Timeout)*time.Second),
	)
	go svcServer.Start()
	svcServers := server.New(gHttpHandler,
		server.Port(gCfg.DefaultHttpConfig.HttpsListenAddr),
		server.ReadTimeout(time.Duration(gCfg.Base.Timeout)*time.Second),
		server.WriteTimeout(time.Duration(gCfg.Base.Timeout)*time.Second),
	)
	go svcServers.StartTLS(gCfg.Cert(), gCfg.Key())

	go func() {
		interrupt := make(chan os.Signal, 1)
		signal.Notify(interrupt, syscall.SIGINT, syscall.SIGTERM)
		select {
		case s := <-interrupt:
			slog.Info(`<-interrupt`, "signal", s.String())
		case err := <-svcServer.Notify():
			slog.Error(`<-server.Notify()`, "err", err)
		case err := <-svcServers.Notify():
			slog.Error(`<-servers.Notify()`, "err", err)
		}

		if err := svcServer.Shutdown(); err != nil {
			slog.Error(`server.Shutdown()`, "err", err)
		}
		if err := svcServers.Shutdown(); err != nil {
			slog.Error(`servers.Shutdown()`, "err", err)
		}

		{
			ctx, cancel := context.WithTimeout(context.Background(), 3*time.Second)
			defer cancel()
			if err := g.UnsafeWaitWithContext(ctx); err != nil {
				slog.Error("UnsafeWaitWithContext", slog.Any("err", err))
			}
		}

		fmt.Printf("server closed\n")
	}()
}

func openUrl(url string) {
	var cmd *exec.Cmd
	// 在Windows上使用"start"，在macOS和Linux上使用"open"
	switch runtime.GOOS {
	case "linux":
		cmd = exec.Command("xdg-open", url)
	case "windows":
		cmd = exec.Command("cmd", "/c", "start", url)
	case "darwin":
		cmd = exec.Command("open", url)
	}
	err := cmd.Start()
	if err != nil {
		slog.Error(fmt.Sprintf("open err: %v", err))
	}
	return
}
