package main

import (
	"easydarwin/internal/conf"
	"easydarwin/internal/gutils"
	"easydarwin/utils/pkg/logger"
	"easydarwin/utils/pkg/system"
	"expvar"
	"flag"
	"github.com/kardianos/service"
	"log/slog"
	"net/http"
	"path/filepath"
	"strconv"
	"time"
)

var (
	buildVersion = "0.0.1" // 构建版本号
	gitBranch    = "dev"   // git 分支
	gitHash      = "debug" // git 提交点哈希值
	release      string    // 发布模式 true/false
	buildTime    string    // 构建时间戳

	// 初始化配置
	gCfg         conf.Bootstrap
	gLog         *slog.Logger
	gHttpHandler http.Handler
	daemonAddr   = flag.String("daemon", "", "")
)

// 自定义配置目录
func getBuildRelease() bool {
	v, _ := strconv.ParseBool(release)
	return v
}

func main() {

	var err error
	filedir, _ := gutils.Abs(*gutils.ConfigDir)
	gCfg, err = conf.SetupConfig(filedir)
	if err != nil {
		panic(err)
	}

	gCfg.Debug = !getBuildRelease()
	gCfg.BuildVersion = buildVersion
	gCfg.DaemonAddr = *daemonAddr
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
	logDir := filepath.Join(system.GetCWD(), gCfg.BaseLog.Dir)
	gLog, _ = logger.SetupSlog(logger.Config{
		Dir:          logDir,                                                 // 日志地址
		Debug:        gCfg.Debug,                                             // 服务级别Debug/Release
		MaxAge:       time.Duration(gCfg.BaseLog.MaxAge) * time.Second,       // 日志存储时间
		RotationTime: time.Duration(gCfg.BaseLog.RotationTime) * time.Second, // 循环时间
		RotationSize: gCfg.BaseLog.RotationSize * 1024 * 1024,                // 循环大小
		Level:        gCfg.BaseLog.Level,                                     // 日志级别
	})
	gCfg.BaseLog.Dir = logDir

	var prg = &program{}
	svcConfig := &service.Config{
		Name:        "EasyDarwin_Service",
		DisplayName: "EasyDarwin服务",
		Description: "EasyDarwin服务，开源免费 www.easydarwin.org",
	}

	s, err := service.New(prg, svcConfig)
	if err != nil {
		slog.Error("new service error", slog.Any("err", err))
		panic(err)
	}

	// easydarwin -service [commond],commond 有:"start", "stop", "restart", "install", "uninstall"
	svcFlag := flag.String("service", "", "Control the system service.")
	flag.Parse()
	if len(*svcFlag) != 0 {
		err := service.Control(s, *svcFlag)
		if err != nil {
			slog.Error("service run error", slog.Any("err", err))
			panic(err)
		}
		return
	}

	if err = s.Run(); err != nil {
		slog.Error("service run error", slog.Any("err", err))
		panic(err)
	}
}
