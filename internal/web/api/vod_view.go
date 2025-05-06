package api

import (
	"easydarwin/internal/core/video"
	"easydarwin/internal/data"
	"easydarwin/internal/gutils/consts"
	"easydarwin/internal/gutils/estring"
	"fmt"
	"github.com/gin-gonic/gin"
	"strings"
)

func attachPlayTokenIfPlayAuthed(id, url string) string {
	/*if dao.SYSConfig().PlayAuthed {
		return AttachPlayToken(id, url)
	}*/
	return url
}

// 根据 do 创建对应前端显示的 viewobject
func newVODRow(c *gin.Context, vod video.TVod) video.VodView {
	row := &video.VodView{}
	vod.Folder = estring.FormatPath(vod.Folder)
	if vod.Resolution == consts.EmptyString {
		vod.IsResolution = false
	} else {
		vod.IsResolution = gCfg.VodConfig.OpenDefinition
	}
	vod.ResolutionDefault = gCfg.VodConfig.DefaultDefinition
	row.TVod = vod

	hostStr := strings.Split(c.Request.Host, ":")
	host := hostStr[0]
	//httpPort := l.Conf.Server.HTTP.Port
	Conf := data.GetConfig()
	httpPort := Conf.DefaultHttpConfig.HttpListenAddr
	httpStr := "http"

	if c.Request.TLS != nil {
		httpPort = Conf.DefaultHttpConfig.HttpsListenAddr
		httpStr = "https"
	}

	row.SnapURL = attachPlayTokenIfPlayAuthed(vod.ID, fmt.Sprintf("%s://%s%s%s/%s/%s", httpStr, host, httpPort, consts.RouteStaticVOD, vod.Folder, consts.VodCover))
	row.VideoURL = attachPlayTokenIfPlayAuthed(vod.ID, fmt.Sprintf("%s://%s%s%s/%s/%s", httpStr, host, httpPort, consts.RouteStaticVOD, vod.Folder, "video.m3u8"))

	row.SharedLink = fmt.Sprintf("%s://%s%s/share.html?id=%s&type=vod", httpStr, host, httpPort, vod.ID)
	/*row.FlowNum = flow.GetVODFlowNum(vod.ID)
	row.PlayNum = flow.GetVODPlayNum(vod.ID)*/
	if row.Status == consts.VodStatusTransing {
		row.Progress, _ = video.TransProgress.Get(vod.ID)
	}
	return *row
}
