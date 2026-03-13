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
			liveAll, liveErr := l.GroupsAll()
			if liveErr != nil {

			}

			// 如果查询出错，返回错误信息
			if err != nil {
				continue
			}
			for _, live := range lives {

				if live.LiveType == livestream.LIVE_PUSH {
					StreamName := fmt.Sprintf("stream_%d", live.ID)
					if live.CustomId != "" {
						StreamName = fmt.Sprintf("%s", live.CustomId)
					}
					if live.OnDemand {
						continue
					}
					isLive := true
					if liveAll.ErrorCode == 0 && liveErr == nil && len(liveAll.Data.Groups) > 0 {
						for _, group := range liveAll.Data.Groups {
							if group.StreamName == StreamName && group.Subs != nil {
								isLive = false
							}
						}
					}
					if time.Since(live.LastAt) > time.Second*15 && isLive {
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
				} else {
					StreamName := fmt.Sprintf("stream_%d", live.ID)
					if live.CustomId != "" {
						StreamName = fmt.Sprintf("%s", live.CustomId)
					}
					if live.OnDemand {
						continue
					}
					isLive := true
					if liveAll.ErrorCode == 0 && liveErr == nil && len(liveAll.Data.Groups) > 0 {
						for _, group := range liveAll.Data.Groups {
							if group.StreamName == StreamName && len(group.Subs) > 0 {
								isLive = false
							}
						}
					}
					if isLive {
						err = source.StopStream(live)
						if err != nil {
							slog.Error(fmt.Sprintf("pub  out live:[%s] err:[%v]", StreamName, err))
							continue
						}
						source.LiveCore.UpdateLiveStreamInt(live.ID, "online", 1)
					}
				}

			}

		}
	}()
}
