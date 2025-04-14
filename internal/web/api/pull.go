package api

import (
	"easydarwin/internal/core/livestream"
	"easydarwin/internal/core/source"
	"easydarwin/internal/data"
	"easydarwin/utils/pkg/web"
	"fmt"
	"github.com/gin-gonic/gin"
	"gorm.io/gorm"
	"log/slog"
	"strconv"
	"strings"
)

type LiveStreamAPI struct {
	database *gorm.DB
}

func (l LiveStreamAPI) find(c *gin.Context) {
	// 定义一个 PagerFilter 结构体变量
	var input livestream.PagerFilter
	// 绑定查询参数到 input 变量
	if err := c.ShouldBindQuery(&input); err != nil {
		// 如果绑定失败，返回错误信息
		web.Fail(c, web.ErrBadRequest.With(
			web.HanddleJSONErr(err).Error(),
			fmt.Sprintf("请检查请求类型 %s", c.GetHeader("content-type"))),
		)
		return
	}
	lives := make([]livestream.LiveStream, 0, input.Limit())
	db := l.database.Model(new(livestream.LiveStream))
	if input.Q != "" {
		db = db.Where("name like ?", "%"+input.Q+"%")
	}
	if input.Type != "" { // 用来查询指定等级的用户
		db = db.Where("live_type = ?", input.Type)
	}
	var total int64
	// 查询符合条件的总数
	if err := db.Count(&total).Error; err != nil {
		// 如果查询失败，返回错误信息
		web.Fail(c, err)
		return
	}
	// 查询符合条件的数据，并按照id降序排列，限制返回的条数，并设置偏移量
	err := db.Limit(input.Limit()).Offset(input.Offset()).Order("id DESC").Find(&lives).Error
	if err != nil {
		web.Fail(c, err)
		return
	}

	//rtmpPort := data.GetConfig().RtmpConfig.Addr
	rtmpPort := data.GetConfig().LogicCfg.RtmpConfig.Addr
	hostStr := strings.Split(c.Request.Host, ":")
	host := hostStr[0]
	for i, stream := range lives {
		if stream.LiveType == livestream.LIVE_PUSH {
			lives[i].Url = fmt.Sprintf("rtmp://%s%s/live/stream_%d?sign=%s", host, rtmpPort, stream.ID, stream.Sign)
		}
	}
	// 返回查询结果
	web.Success(c, gin.H{"items": lives, "total": total})
}

func (l LiveStreamAPI) findInfo(c *gin.Context) {
	id, err := strconv.Atoi(c.Param("id"))
	if err != nil {
		web.Fail(c, err)
		return
	}
	out, err := source.LiveCore.FindInfoLiveStream(id)
	if err != nil {
		// 如果查询失败，返回错误信息
		web.Fail(c, err)
		return
	}
	web.Success(c, gin.H{"info": out})
}

func (l LiveStreamAPI) playStart(c *gin.Context) {
	id, err := strconv.Atoi(c.Param("id"))
	if err != nil {
		web.Fail(c, err)
		return
	}
	out, err := source.LiveCore.FindInfoLiveStream(id)
	if err != nil {
		// 如果查询失败，返回错误信息
		web.Fail(c, err)
		return
	}
	if !out.Enable {
		web.Fail(c, web.ErrBadRequest.Msg("直播通道未开启"))
		return
	}
	if out.Online == 0 {
		web.Fail(c, web.ErrBadRequest.Msg("直播通道已离线"))
		return
	}
	if out.LiveType == livestream.LIVE_PUSH {
		urlInfo := l.GetLiveStreamUrl(c, out)
		web.Success(c, gin.H{"info": urlInfo})
		return
	}
	err = source.StartStream(out)
	if err != nil {
		web.Fail(c, web.ErrBadRequest.Msg("拉流失败"))
		return
	}
	// 返回查询结果

	urlInfo := l.GetLiveStreamUrl(c, out)
	web.Success(c, gin.H{"info": urlInfo})
}

func (l LiveStreamAPI) createPull(c *gin.Context) {
	var input livestream.LiveInput
	// 将请求的JSON数据绑定到input变量上
	if err := c.ShouldBindJSON(&input); err != nil {
		// 如果绑定失败，返回错误信息
		web.Fail(c, web.ErrBadRequest.With(
			web.HanddleJSONErr(err).Error(),
			fmt.Sprintf("请检查请求类型 %s", c.GetHeader("content-type"))),
		)
		return
	}

	live, err := source.LiveCore.CreateLiveStream(input)
	if err != nil {
		web.Fail(c, err)
		return
	}
	_, err = source.AddStreamClient(live)
	if err != nil {
		web.Fail(c, web.ErrBadRequest.Msg("添加流失败"))
		return
	}
	err = source.UpdateOnlineStream(live)
	if err != nil {
		slog.Error(fmt.Sprintf("添加 拉流失败[%d]%s\n", live.ID, err))
	}

	rawStr := fmt.Sprintf("/snap/stream_%d/stream_%d.raw", live.ID, live.ID)
	jpgStr := fmt.Sprintf("/snap/stream_%d/stream_%d.jpg", live.ID, live.ID)
	err = source.LiveCore.UpdateLiveStreamSnap(live.ID, rawStr, jpgStr)
	if err != nil {
		web.Fail(c, err)
		return
	}

	web.Success(c, gin.H{
		"name": input.Name,
	})
}

func (l LiveStreamAPI) updatePull(c *gin.Context) {
	id, err := strconv.Atoi(c.Param("id"))
	if err != nil {
		web.Fail(c, err)
		return
	}
	var input livestream.LiveInput
	// 将请求的JSON数据绑定到input变量上
	if err := c.ShouldBindJSON(&input); err != nil {
		// 如果绑定失败，返回错误信息
		web.Fail(c, web.ErrBadRequest.With(
			web.HanddleJSONErr(err).Error(),
			fmt.Sprintf("请检查请求类型 %s", c.GetHeader("content-type"))),
		)
		return
	}

	err = source.LiveCore.UpdateLiveStream(input, id)
	if err != nil {
		web.Fail(c, err)
		return
	}
	live, err := source.LiveCore.FindInfoLiveStream(id)
	if err != nil {
		web.Fail(c, err)
		return
	}
	err = source.StopStream(live)
	if err != nil {
		web.Fail(c, web.ErrBadRequest.Msg("更新停流失败"))
		return
	}
	source.DelStreamClient(live.ID)
	_, err = source.AddStreamClient(live)
	if err != nil {
		web.Fail(c, web.ErrBadRequest.Msg("更新流失败"))
		return
	}
	err = source.UpdateOnlineStream(live)
	if err != nil {
		slog.Error(fmt.Sprintf("更新 拉流失败[%d]%s\n", live.ID, err))
	}
	web.Success(c, gin.H{
		"name": input.Name,
	})
}

func registerLiveStream(r gin.IRouter) {
	l := LiveStreamAPI{
		database: data.GetDatabase(),
	}
	{
		r.Any("/push/on_pub_start", l.pubStart)
		r.Any("/push/on_pub_stop", l.pubStop)
		r.Any("/push/on_rtmp_connect", l.pubRtmpConnect)
	}
	{
		group := r.Group("/live")
		group.GET("", l.find)
		group.GET("/info/:id", l.findInfo)

		group.GET("/play/start/:id", l.playStart)
		group.GET("/play/stop/:id", l.playStop)
		group.GET("/stream/info/:id", l.StreamInfo)
		group.DELETE("/:id", l.delete)

		pull := group.Group("/pull")
		pull.POST("", l.createPull) // 创建
		pull.PUT(":id", l.updatePull)
		pull.PUT(":id/:type/:value", l.updateOnePull)

		push := group.Group("/push")
		push.POST("", l.createPush) // 创建
		push.PUT(":id", l.updatePush)
		push.PUT(":id/:type/:value", l.updateOnePush)
	}
}

func (l LiveStreamAPI) GetLiveStreamUrl(c *gin.Context, live livestream.LiveStream) livestream.LivePlayer {
	hostStr := strings.Split(c.Request.Host, ":")
	host := hostStr[0]
	//httpPort := l.Conf.Server.HTTP.Port
	Conf := data.GetConfig()
	httpPort := Conf.DefaultHttpConfig.HttpListenAddr
	rtcPort := Conf.DefaultHttpConfig.HttpListenAddr
	//rtmpPort := Conf.RtmpConfig.Addr
	rtspPort := Conf.RtspConfig.Addr
	httpStr := "http"
	wsStr := "ws"
	//rtspUsername := Conf.RtspConfig.UserName
	//rtspPassword := Conf.RtspConfig.PassWord
	if c.Request.TLS != nil {
		httpPort = Conf.DefaultHttpConfig.HttpsListenAddr
		rtcPort = Conf.DefaultHttpConfig.HttpsListenAddr
		//rtmpPort = Conf.RtmpConfig.RtmpsAddr
		rtspPort = Conf.RtspConfig.RtspsAddr
		httpStr = "https"
		wsStr = "wss"
	}
	urlInfo := livestream.LivePlayer{
		ID:      live.ID,
		Name:    live.Name,
		HttpFlv: fmt.Sprintf("%s://%s%s/flv/live/stream_%d.flv", httpStr, host, httpPort, live.ID),
		HttpHls: fmt.Sprintf("%s://%s%s/ts/hls/stream_%d/playlist.m3u8", httpStr, host, httpPort, live.ID),
		WsFlv:   fmt.Sprintf("%s://%s%s/ws_flv/live/stream_%d.flv", wsStr, host, httpPort, live.ID),
		WEBRTC:  fmt.Sprintf("webrtc://%s%s/webrtc/play/live/stream_%d", host, rtcPort, live.ID),
		//RTMP:    fmt.Sprintf("rtmp://%s%s/live/stream_%d", host, rtmpPort, live.ID),
		RTSP: fmt.Sprintf("rtsp://%s%s/live/stream_%d", host, rtspPort, live.ID),
	}

	return urlInfo
}

func (l LiveStreamAPI) updateOnePull(c *gin.Context) {
	id, err := strconv.Atoi(c.Param("id"))
	if err != nil {
		web.Fail(c, err)
		return
	}
	value, err := strconv.Atoi(c.Param("value"))
	if err != nil {
		web.Fail(c, err)
		return
	}
	key := ""
	switch c.Param("type") {
	case "enable":
		key = "enable"
		var live livestream.LiveStream
		live, err = source.LiveCore.FindInfoLiveStream(id)
		if err != nil {
			web.Fail(c, err)
			return
		}
		if value == 1 {
			err = source.UpdateOnlineStream(live)
			if err != nil {
				web.Fail(c, web.ErrBadRequest.Msg("拉流失败"))
				return
			}
		} else if value == 0 {
			err = source.StopStream(live)
			if err != nil {
				web.Fail(c, web.ErrBadRequest.Msg("停流失败"))
				return
			}

		}

	case "onDemand":
		key = "on_demand"
		var live livestream.LiveStream
		live, err = source.LiveCore.FindInfoLiveStream(id)
		if err != nil {
			web.Fail(c, err)
			return
		}
		if value == 1 {
			live.OnDemand = true
		} else {
			live.OnDemand = false
		}

		if live.OnDemand {
			err = source.UpdateStreamOnDemand(live)
			if err != nil {
				web.Fail(c, web.ErrBadRequest.Msg("开启按需失败"))
				return
			}
		} else {
			err = source.StartStream(live)
			if err != nil {
				web.Fail(c, web.ErrBadRequest.Msg("拉流失败"))
				return
			}
		}
	case "audio":
		key = "audio"
		var live livestream.LiveStream
		live, err = source.LiveCore.FindInfoLiveStream(id)
		if err != nil {
			web.Fail(c, err)
			return
		}
		if value == 1 {
			live.Audio = true
		} else {
			live.Audio = false
		}
		err = source.StopStream(live)
		if err != nil {
			web.Fail(c, web.ErrBadRequest.Msg("更新停流失败"))
			return
		}
		source.DelStreamClient(live.ID)
		_, err = source.AddStreamClient(live)
		if err != nil {
			web.Fail(c, web.ErrBadRequest.Msg("更新流失败"))
			return
		}
	default:
		web.Fail(c, web.ErrBadRequest.Msg("更新失败"))
		return
	}
	err = source.LiveCore.UpdateLiveStreamInt(id, key, value)
	if err != nil {
		web.Fail(c, web.ErrBadRequest.Msg("更新失败"))
		return
	}
	web.Success(c, gin.H{
		"id": id,
	})
}
