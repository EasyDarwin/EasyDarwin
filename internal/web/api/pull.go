// Package api Copyright 2025 EasyDarwin.
// http://www.easydarwin.org
// 对拉流的路由操作
// History (ID, Time, Desc)
// (xukongzangpusa, 20250424, 添加注释，增加url功能)
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
	"time"
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

// 获取播放url地址
func (l LiveStreamAPI) getPlayUrl(c *gin.Context) {
	id, err := strconv.Atoi(c.Param("id"))
	if err != nil {
		web.Fail(c, err)
		return
	}
	urlInfo := l.GetLiveUrl(c, id)
	web.Success(c, gin.H{"info": urlInfo})
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

		err = source.LiveCore.UpdateLiveStreamString(id, "last_at", time.Now().String())
		if err != nil {
			slog.Error(fmt.Sprintf("pub start update session id live id:[%d]  err:[%v]", id, err))
			return
		}
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

func (l LiveStreamAPI) GetLiveStreamUrl(c *gin.Context, live livestream.LiveStream) livestream.LivePlayer {
	return l.getLiveUrl(c, live.ID, live.Name)
}

// GetLiveUrl 根据id获取流的url
func (l LiveStreamAPI) GetLiveUrl(c *gin.Context, id int) livestream.LivePlayer {
	return l.getLiveUrl(c, id, "")
}

func (l LiveStreamAPI) getLiveUrl(c *gin.Context, id int, name string) livestream.LivePlayer {
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
		ID:      id,
		Name:    name,
		HttpFlv: fmt.Sprintf("%s://%s%s/flv/live/stream_%d.flv", httpStr, host, httpPort, id),
		HttpHls: fmt.Sprintf("%s://%s%s/ts/hls/stream_%d/playlist.m3u8", httpStr, host, httpPort, id),
		WsFlv:   fmt.Sprintf("%s://%s%s/ws_flv/live/stream_%d.flv", wsStr, host, httpPort, id),
		WEBRTC:  fmt.Sprintf("webrtc://%s%s/webrtc/play/live/stream_%d", host, rtcPort, id),
		//RTMP:    fmt.Sprintf("rtmp://%s%s/live/stream_%d", host, rtmpPort, live.ID),
		RTSP: fmt.Sprintf("rtsp://%s%s/live/stream_%d", host, rtspPort, id),
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
