package api

import (
	"easydarwin/internal/core/livestream"
	"easydarwin/internal/core/source"
	"fmt"
	"golang.org/x/exp/slog"
	"time"
)

func (l LiveStreamAPI) TickCheckPush() {
	t := time.NewTicker(5 * time.Second)
	go func() {
		for range t.C {
			lives := make([]livestream.LiveStream, 0)
			lives, _, err := source.LiveCore.FindLiveStreamPushALl()
			// 如果查询出错，返回错误信息
			if err != nil {
				continue
			}
			for _, live := range lives {
				if live.LiveType == livestream.LIVE_PUSH {
					if live.OnDemand {
						continue
					}
					if time.Since(live.LastAt) > time.Second*15 {

						StreamName := fmt.Sprintf("stream_%d", live.ID)
						_, err = l.KickOutLive(StreamName, live.SessionId)
						if err != nil {
							slog.Error(fmt.Sprintf("pub start kick out live:[%s] err:[%v]", StreamName, err))
							continue
						}
						err = source.LiveCore.UpdateLiveStreamInt(live.ID, "online", 1)
						if err != nil {
							slog.Error(fmt.Sprintf("pub start update session id live id:[%d] name:[%s] err:[%v]", live.ID, StreamName, err))
							continue
						}
					}
				}

			}

		}
	}()
}
