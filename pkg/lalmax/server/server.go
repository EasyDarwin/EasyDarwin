package server

import (
	"context"
	"easydarwin/pkg/lalmax/onvif"
	"easydarwin/pkg/lalmax/rtc"
	"easydarwin/pkg/lalmax/srt"
	"encoding/json"

	"easydarwin/pkg/lalmax/hook"

	"easydarwin/pkg/lalmax/gb28181"

	httpfmp4 "easydarwin/pkg/lalmax/fmp4/http-fmp4"

	"easydarwin/pkg/lalmax/fmp4/hls"

	config "easydarwin/pkg/lalmax/conf"

	"github.com/gin-gonic/gin"
	"github.com/q191201771/lal/pkg/logic"
	"github.com/q191201771/naza/pkg/nazalog"
)

type LalMaxServer struct {
	lalsvr      logic.ILalServer
	conf        *config.Config
	srtsvr      *srt.SrtServer
	rtcsvr      *rtc.RtcServer
	router      *gin.Engine
	routerTls   *gin.Engine
	httpfmp4svr *httpfmp4.HttpFmp4Server
	hlssvr      *hls.HlsServer
	gbsbr       *gb28181.GB28181Server
	onvifsvr    *onvif.OnvifServer
	//roomsvr     *room.RoomServer
}

type PublicConfig struct {
	*logic.Config
	LogConfig struct {
		Level nazalog.Level `json:"level"` // 日志级别，大于等于该级别的日志才会被输出

		// 文件输出和控制台输出可同时打开
		// 控制台输出主要用做开发时调试，打开后level字段使用彩色输出
		Filename   string `json:"filename"`     // 输出日志文件名，如果为空，则不写日志文件。可包含路径，路径不存在时，将自动创建
		IsToStdout bool   `json:"is_to_stdout"` // 是否以stdout输出到控制台 TODO(chef): 再增加一个stderr的配置

		IsRotateDaily  bool `json:"is_rotate_daily"`  // 日志按天翻转
		IsRotateHourly bool `json:"is_rotate_hourly"` // 日志按小时翻滚，整点翻滚

		ShortFileFlag       bool `json:"short_file_flag"`        // 是否在每行日志尾部添加源码文件及行号的信息
		TimestampFlag       bool `json:"timestamp_flag"`         // 是否在每行日志首部添加时间戳的信息
		TimestampWithMsFlag bool `json:"timestamp_with_ms_flag"` // 时间戳是否精确到毫秒
		LevelFlag           bool `json:"level_flag"`             // 日志是否包含日志级别字段

		AssertBehavior   nazalog.AssertBehavior `json:"assert_behavior"` // 断言失败时的行为
		HookBackendOutFn *struct{}              `json:"-"`
	} `json:"log"`
}

func (cfg *PublicConfig) SetLogConfig(config2 *logic.Config) {
	cfg.LogConfig.Level = config2.LogConfig.Level
	cfg.LogConfig.Filename = config2.LogConfig.Filename
	cfg.LogConfig.IsToStdout = config2.LogConfig.IsToStdout
	cfg.LogConfig.IsRotateDaily = config2.LogConfig.IsRotateDaily
	cfg.LogConfig.IsRotateHourly = config2.LogConfig.IsRotateHourly
	cfg.LogConfig.ShortFileFlag = config2.LogConfig.ShortFileFlag
	cfg.LogConfig.TimestampFlag = config2.LogConfig.TimestampFlag
	cfg.LogConfig.TimestampWithMsFlag = config2.LogConfig.TimestampWithMsFlag
	cfg.LogConfig.LevelFlag = config2.LogConfig.LevelFlag
	cfg.LogConfig.AssertBehavior = config2.LogConfig.AssertBehavior
}

func NewLalMaxServer(conf *config.Config, config2 *logic.Config) (*LalMaxServer, error) {
	lalsvr := logic.NewLalServer(func(option *logic.Option) {
		cfg := PublicConfig{
			Config: config2,
		}
		cfg.SetLogConfig(config2)
		data, err := json.Marshal(cfg)
		if err != nil {
			nazalog.Error("logic.Config failed, err:", err)
		}
		option.ConfRawContent = data

		option.ConfFilename = conf.LalSvrConfigPath
		//option.NotifyHandler = NewHttpNotify(conf.HttpNotifyConfig, conf.ServerId)
	})
	maxsvr := &LalMaxServer{
		lalsvr: lalsvr,
		conf:   conf,
	}

	if conf.SrtConfig.Enable {
		maxsvr.srtsvr = srt.NewSrtServer(conf.SrtConfig.Addr, lalsvr, func(option *srt.SrtOption) {
			option.Latency = 300
			option.PeerLatency = 300
		})
	}

	if conf.RtcConfig.Enable {
		var err error
		maxsvr.rtcsvr, err = rtc.NewRtcServer(conf.RtcConfig, lalsvr)
		if err != nil {
			nazalog.Error("create rtc svr failed, err:", err)
			return nil, err
		}
	}

	if conf.HttpFmp4Config.Enable {
		maxsvr.httpfmp4svr = httpfmp4.NewHttpFmp4Server()
	}

	if conf.HlsConfig.Enable {
		maxsvr.hlssvr = hls.NewHlsServer(conf.HlsConfig)
	}

	if conf.GB28181Config.Enable {
		maxsvr.gbsbr = gb28181.NewGB28181Server(conf.GB28181Config, lalsvr)
	}

	if conf.OnvifConfig.Enable {
		maxsvr.onvifsvr = onvif.NewOnvifServer()
	}

	//if conf.RoomConfig.Enable {
	//	maxsvr.roomsvr = room.NewRoomServer(conf.RoomConfig.APIKey, conf.RoomConfig.APISecret)
	//}

	//maxsvr.router = gin.Default()
	maxsvr.router = gin.New()
	maxsvr.InitRouter(maxsvr.router)
	if conf.HttpConfig.EnableHttps {
		//maxsvr.routerTls = gin.Default()
		maxsvr.routerTls = gin.New()
		maxsvr.InitRouter(maxsvr.routerTls)
	}

	return maxsvr, nil
}

func (s *LalMaxServer) GetILalServer() logic.ILalServer {
	return s.lalsvr
}

func (s *LalMaxServer) GetRouter() *gin.Engine {
	return s.router
}

func (s *LalMaxServer) GetRouterTls() *gin.Engine {
	return s.routerTls
}

func (s *LalMaxServer) Run() (err error) {
	s.lalsvr.WithOnHookSession(func(uniqueKey string, streamName string) logic.ICustomizeHookSessionContext {
		// 有新的流了，创建业务层的对象，用于hook这个流
		return hook.NewHookSession(uniqueKey, streamName, s.hlssvr, s.conf.HookConfig.GopCacheNum, s.conf.HookConfig.SingleGopMaxFrameNum)
	})

	ctx, cancel := context.WithCancel(context.Background())
	defer cancel()

	if s.srtsvr != nil {
		go s.srtsvr.Run(ctx)
	}

	//go func() {
	//	nazalog.Infof("lalmax http listen. addr=%s", s.conf.HttpConfig.ListenAddr)
	//	if err = s.router.Run(s.conf.HttpConfig.ListenAddr); err != nil {
	//		nazalog.Infof("lalmax http stop. addr=%s", s.conf.HttpConfig.ListenAddr)
	//	}
	//}()

	if s.conf.HttpConfig.EnableHttps {
		//server := &http.Server{Addr: s.conf.HttpConfig.HttpsListenAddr, Handler: s.routerTls, TLSNextProto: map[string]func(*http.Server, *tls.Conn, http.Handler){}}
		//go func() {
		//	nazalog.Infof("lalmax https listen. addr=%s", s.conf.HttpConfig.HttpsListenAddr)
		//	if err = server.ListenAndServeTLS(s.conf.HttpConfig.HttpsCertFile, s.conf.HttpConfig.HttpsKeyFile); err != nil {
		//		nazalog.Infof("lalmax https stop. addr=%s", s.conf.HttpConfig.ListenAddr)
		//	}
		//}()
	}

	if s.gbsbr != nil {
		go s.gbsbr.Start()
	}

	//if s.roomsvr != nil {
	//	go s.roomsvr.Start()
	//}

	return s.lalsvr.RunLoop()
}
