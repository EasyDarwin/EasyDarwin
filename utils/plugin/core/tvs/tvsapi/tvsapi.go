package tvsapi

import (
	"strings"

	"easydarwin/lnton/pkg/web"
	"easydarwin/lnton/plugin/core/tvs"
	"github.com/gin-gonic/gin"
)

// RegisterWalls ...
func RegisterWalls(g gin.IRouter, core tvs.Core, hf ...gin.HandlerFunc) {
	al := wallAPI{core: core}
	event := g.Group("/tv/walls", hf...)
	event.GET("", al.find)
}

type wallAPI struct {
	core tvs.Core
}

type TVSInput struct {
	Num int `form:"num"`
}
type resOutput struct {
	Name     string   `json:"name"`
	Channels []string `json:"channels"`
}

func (wa *wallAPI) find(c *gin.Context) {
	var in TVSInput
	if err := c.ShouldBindQuery(&in); err != nil {
		web.Fail(c, web.ErrBadRequest.With(web.HanddleJSONErr(err).Error()))
		return
	}

	output, total, err := wa.core.FindWalls()
	if err != nil {
		web.Fail(c, err)
		return
	}
	var outs []resOutput

	for _, v := range output {
		var out resOutput
		str := strings.Split(v.Channels, ",")
		out.Channels = append(out.Channels, str...)
		out.Name = v.Name
		outs = append(outs, out)
	}

	web.Success(c, gin.H{
		"items": outs,
		"total": total,
	})
}
