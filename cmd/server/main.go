package main

import (
	"context"
	"easydarwin/internal/conf"
	"easydarwin/internal/core/source"
	"easydarwin/internal/core/svr"
	"easydarwin/internal/gutils"
	"easydarwin/utils/pkg/conc"
	"easydarwin/utils/pkg/logger"
	"easydarwin/utils/pkg/server"
	"easydarwin/utils/pkg/system"
	"expvar"
	"flag"
	"fmt"
	"log/slog"
	"os"
	"os/exec"
	"os/signal"
	"path/filepath"
	"runtime"
	"strconv"
	"syscall"
	"time"
)

var (
	buildVersion = "0.0.1" // 构建版本号
	gitBranch    = "dev"   // git 分支
	gitHash      = "debug" // git 提交点哈希值
	release      string    // 发布模式 true/false
	buildTime    string    // 构建时间戳
)
var daemonAddr = flag.String("daemon", "", "")

// 自定义配置目录
func getBuildRelease() bool {
	v, _ := strconv.ParseBool(release)
	return v
}

func main() {
	flag.Parse()
	// 初始化配置
	var cfg conf.Bootstrap
	var err error
	filedir, _ := gutils.Abs(*gutils.ConfigDir)
	cfg, err = conf.SetupConfig(filedir)
	if err != nil {
		panic(err)
	}

	cfg.Debug = !getBuildRelease()
	cfg.BuildVersion = buildVersion
	cfg.DaemonAddr = *daemonAddr
	// 初始化日志
	{
		buildTime = GetBuildTime().Format(time.DateTime)
		expvar.NewString("version").Set(buildVersion)
		expvar.NewString("git_branch").Set(gitBranch)
		expvar.NewString("git_hash").Set(gitHash)
		expvar.NewString("build_time").Set(buildTime)
		expvar.Publish("timestamp", expvar.Func(func() any {
			return time.Now().Format(time.DateTime)
		}))
	}
	// 初始化日志
	logDir := filepath.Join(system.GetCWD(), cfg.BaseLog.Dir)
	log, _ := logger.SetupSlog(logger.Config{
		Dir:          logDir,                                                // 日志地址
		Debug:        cfg.Debug,                                             // 服务级别Debug/Release
		MaxAge:       time.Duration(cfg.BaseLog.MaxAge) * time.Second,       // 日志存储时间
		RotationTime: time.Duration(cfg.BaseLog.RotationTime) * time.Second, // 循环时间
		RotationSize: cfg.BaseLog.RotationSize * 1024 * 1024,                // 循环大小
		Level:        cfg.BaseLog.Level,                                     // 日志级别
	})
	cfg.BaseLog.Dir = logDir
	// 启动流媒体 sugar
	svr.Start(&cfg)
	g := conc.New(log)
	// 启动定时任务
	time.AfterFunc(time.Duration(15)*time.Second, func() {
		source.StartScheduler()
	})
	bin, _ := os.Executable()
	if err := os.Chdir(filepath.Dir(bin)); err != nil {
		slog.Error("change dir error")
	}
	handler, err := wireApp(&cfg)
	if err != nil {
		slog.Error("程序构建失败", "err", err)
		panic(err)
	}

	svcServer := server.New(handler,
		server.Port(cfg.DefaultHttpConfig.HttpListenAddr),
		server.ReadTimeout(time.Duration(cfg.Base.Timeout)*time.Second),
		server.WriteTimeout(time.Duration(cfg.Base.Timeout)*time.Second),
	)
	go svcServer.Start()
	svcServers := server.New(handler,
		server.Port(cfg.DefaultHttpConfig.HttpsListenAddr),
		server.ReadTimeout(time.Duration(cfg.Base.Timeout)*time.Second),
		server.WriteTimeout(time.Duration(cfg.Base.Timeout)*time.Second),
	)
	go svcServers.StartTLS(cfg.Cert(), cfg.Key())

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
