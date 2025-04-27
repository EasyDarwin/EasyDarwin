package main

import (
	"easydarwin/internal/conf"
	"easydarwin/internal/core/source"
	"easydarwin/internal/data"
	"easydarwin/internal/web/api"
	"fmt"
	"net/http"
)

func wireApp(cfg *conf.Bootstrap) (http.Handler, error) {
	db, err := data.SetupDB(cfg)
	if err != nil {
		return nil, err
	}

	// 消除 sqlite 空闲页，防止数据库过大
	db.Exec("VACUUM;")

	liveStreamcore := api.NewLiveStream(db)
	api.NewUserCore(db)

	api.NewVodCore(db)

	source.InitDb(liveStreamcore)
	handler := api.NewHTTPHandler(cfg)
	if handler == nil {
		return nil, fmt.Errorf("handle is nil")
	}
	data.SetConfig(cfg)
	return handler, nil
}
