package svr

import (
	"easydarwin/internal/conf"
	"easydarwin/pkg/lalmax/server"
	"github.com/q191201771/naza/pkg/nazalog"
	"time"
)

var (
	count int
	Lals  *server.LalMaxServer
)

func getSleepDuration() time.Duration {
	count++
	if count > 55 {
		count = 0
	}
	return time.Duration(5+count) * time.Second
}

func Start(config *conf.Bootstrap) {
	svr, err := server.NewLalMaxServer(config.Config, &config.LogicCfg)
	if err != nil {
		nazalog.Fatalf("create lalmax server failed. err=%+v", err)
		return
	}
	Lals = svr
	go func() {
		if err = svr.Run(); err != nil {
			nazalog.Infof("server manager done. err=%+v", err)
		}
	}()
}
