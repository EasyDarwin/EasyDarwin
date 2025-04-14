package plugin

import (
	"strings"
	"time"

	"easydarwin/lnton/pkg/web"
	"easydarwin/lnton/plugin/core/log"
	"github.com/gin-gonic/gin"
)

// RegisterLog 注册日志
func RegisterLog(router gin.IRouter, core log.Core, hf ...gin.HandlerFunc) {
	l := Log{core: core}
	logs := router.Group("/logs", hf...)
	logs.GET("", l.find)
}

// Log 日志模块
type Log struct {
	core log.Core
}

// NewLog 记录日志
func NewLog(core log.Core) Log {
	return Log{core: core}
}

// RecordLog 记录日志
func (l Log) RecordLogForHome(remark string) gin.HandlerFunc {
	return func(ctx *gin.Context) {
		ctx.Next()
		uid := web.GetUID(ctx)
		username := web.GetUsername(ctx)
		// 暂不记录 rms 的
		if username == "rms" {
			return
		}
		v := log.Log{
			Username:   username,
			TargetName: "",
			Remark:     remark,
			TargetID:   uid,
			Misc: log.MiscLog{
				Method: ctx.Request.Method,
				Path:   ctx.Request.URL.RequestURI(),
			},
		}
		l.core.Write(&v)
	}
}

// RecordLog 记录日志
func (l Log) RecordLog(remark string) gin.HandlerFunc {
	return func(ctx *gin.Context) {
		ctx.Next()
		uid := web.GetUID(ctx)
		username := web.GetUsername(ctx)
		v := log.Log{
			Username:   username,
			TargetName: "",
			Remark:     remark,
			TargetID:   uid,
			Misc: log.MiscLog{
				Method: ctx.Request.Method,
				Path:   ctx.Request.URL.RequestURI(),
			},
		}
		l.core.Write(&v)
	}
}

func (l Log) find(ctx *gin.Context) {
	var input log.FindLogsInput
	if err := ctx.ShouldBindQuery(&input); err != nil {
		web.Fail(ctx, web.ErrBadRequest.With(web.HanddleJSONErr(err).Error()))
		return
	}
	// 参数修正
	input.Username = strings.TrimSpace(input.Username)
	input.Remark = strings.TrimSpace(input.Remark)
	// 时间戳默认设置
	if input.StartAt == 0 {
		// 默认查询15天内的日志
		input.StartAt = time.Now().AddDate(0, 0, -15).Unix()
	}
	if input.EndAt == 0 {
		// 默认结束时间为当前
		input.EndAt = time.Now().Unix()
	}
	// sort := input.MustSortColumn() + " " + input.SortDirection()
	out, total, err := l.core.Find(input)
	if err != nil {
		web.Fail(ctx, err)
		return
	}
	web.Success(ctx, gin.H{
		"items": out,
		"total": total,
	})
}
