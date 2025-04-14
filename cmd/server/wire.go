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
	liveStreamcore := api.NewLiveStream(db)
	api.NewUserCore(db)
	source.InitDb(liveStreamcore)
	handler := api.NewHTTPHandler(cfg)
	if handler == nil {
		return nil, fmt.Errorf("handle is nil")
	}
	data.SetConfig(cfg)
	return handler, nil
}
