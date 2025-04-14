package plugin

import (
	"easydarwin/lnton/pkg/web"
	"easydarwin/lnton/plugin/core/alarm"
	"github.com/gin-gonic/gin"
	"os"
	"path"
	"time"
)

// RegisterAlarmReceiv 注册告警事件
func RegisterAlarmReceiv(g gin.IRouter, core alarm.Core, hf ...gin.HandlerFunc) {
	al := alaramAPI{core: core}
	g.POST("/alarms/notify", append(hf, al.push)...)

}

// RegisterAlarm 告警时间查改删
func RegisterAlarm(g gin.IRouter, core alarm.Core, hf ...gin.HandlerFunc) {
	al := alaramAPI{core: core}
	a := g.Group("/alarms", hf...)

	a.GET("", al.find)
	a.GET("/info", al.findAlarmInfo)

	g.GET("alarm/*path", web.WarpH(al.alarmImage)) // 国标本地录像下载

	{
		plan := a.Group("/plans")

		plan.POST("", web.WarpH(al.addAlarmPlan))
		plan.GET("", web.WarpH(al.findAlarmPlan))
		plan.PUT("/:id", web.WarpH(al.editAlarmPlan))
		plan.DELETE("/:id", web.WarpH(al.deleteAlarmPlan))

		plan.GET("/:id/channels", web.WarpH(al.getPlanChannel))
		plan.PUT("/:id/channels", web.WarpH(al.setAlarmPlanChannel))
	}
}

type alaramAPI struct {
	core alarm.Core
}

type snapshotInput struct {
	W     int `form:"w" json:"w"`
	H     int `form:"h" json:"h"`
	TimeS int `form:"time_s" json:"time_s"` // 秒
	// EnabledServerDecode bool `form:"enabled_server_decode" json:"enabled_server_decode"` // true: 服务端解析关键帧
	// 生成接口中未使用到
	Img bool `form:"img" json:"img"` // true 则直接写入图片，而非返回 json
}

func (al *alaramAPI) alarmImage(c *gin.Context, in *snapshotInput) (any, error) {
	pathURL := c.Param("path")

	out, err := os.ReadFile(path.Join(al.core.Filer.Prefix(), pathURL)) // nolint
	if err != nil {
		return nil, web.ErrBadRequest.With(err.Error())
	}

	return responseSnapshot(c, in, out, false, time.Now().Unix())

}

// responseSnapshot 函数用于响应快照请求
func responseSnapshot(c *gin.Context, in *snapshotInput, out []byte, isKeyframe bool, createdAt int64) (any, error) {
	// 如果不是关键帧，则设置内容类型为image/jpeg
	xContentType := "keyframe"
	if !isKeyframe {
		xContentType = "image/jpeg"
	}
	// 设置响应头X-Content-Type
	c.Header("X-Content-Type", xContentType)
	// 返回成功响应，包含图片数据、内容类型和创建时间
	return gin.H{"img": out, "type": xContentType, "created_at": createdAt}, nil
}

func (al *alaramAPI) push(c *gin.Context) {
	var in alarm.ModelInput
	if err := c.ShouldBindJSON(&in); err != nil {
		web.Fail(c, web.ErrBadRequest.With(web.HanddleJSONErr(err).Error()))
		return
	}
	// 存储到数据库
	out, err := al.core.AddAlarm(in)
	if err != nil {
		web.Fail(c, err)
		return
	}
	web.Success(c, out)
}

func (al *alaramAPI) findAlarmInfo(c *gin.Context) {
	var in alarm.FindAlarmInfoInput
	if err := c.ShouldBindQuery(&in); err != nil {
		web.Fail(c, web.ErrBadRequest.With(web.HanddleJSONErr(err).Error()))
		return
	}
	// 存储到数据库
	out, err := al.core.FindAlarmInfo(in)
	if err != nil {
		web.Fail(c, err)
		return
	}
	web.Success(c, out)
}

func (al *alaramAPI) find(c *gin.Context) {
	var in alarm.FindInput
	if err := c.ShouldBindQuery(&in); err != nil {
		web.Fail(c, web.ErrBadRequest.With(web.HanddleJSONErr(err).Error()))
		return
	}
	output, total, err := al.core.FindAlarms(in)
	if err != nil {
		web.Fail(c, err)
		return
	}
	web.Success(c, gin.H{
		"items": output,
		"total": total,
	})
}

func (al *alaramAPI) addAlarmPlan(c *gin.Context, in *alarm.AddAlarmPlanInput) (any, error) {
	// 存储到数据库
	out, err := al.core.AddAlarmPlan(in)
	if err != nil {
		return nil, err
	}
	return out, nil
}

func (al *alaramAPI) editAlarmPlan(c *gin.Context, in *alarm.UpdateAlarmPlanInput) (any, error) {
	id := c.Param("id")
	in.ID = id
	// 存储到数据库
	out, err := al.core.EditAlarmPlan(in)
	if err != nil {
		return nil, err
	}
	return &out, nil
}

func (al *alaramAPI) deleteAlarmPlan(c *gin.Context, _ *struct{}) (any, error) {
	id := c.Param("id")
	// 存储到数据库
	out, err := al.core.DeleteAlarmPlan(id)
	if err != nil {
		return nil, err
	}
	return out, nil
}

func (al *alaramAPI) findAlarmPlan(c *gin.Context, in *alarm.FindAlarmPlanInput) (*web.PageOutput, error) {
	// 存储到数据库
	out, total, err := al.core.FindAlarmPlan(in)
	if err != nil {
		return nil, err
	}
	return &web.PageOutput{Total: total, Items: out}, nil
}

func (al *alaramAPI) setAlarmPlanChannel(c *gin.Context, in *alarm.SetPlanChannelInput) (any, error) {
	in.AlarmPlanID = c.Param("id")
	if in.AlarmPlanID == "" {
		return nil, web.ErrBadRequest.With("AlarmPlanID不能为空")
	}
	err := al.core.SetPlanChannel(in)
	if err != nil {
		return nil, err // 底层封装的错误为用户不存在
	}
	return gin.H{"result": "ok"}, nil
}

func (al *alaramAPI) getPlanChannel(c *gin.Context, _ *struct{}) (any, error) {
	planID := c.Param("id")

	pChannel, err := al.core.GetPlanChannel(planID)
	channels := make([]string, 0, len(pChannel))
	for _, v := range pChannel {
		channels = append(channels, v.ChannelID)
	}
	if err != nil {
		return nil, err
	}
	return channels, nil
}
