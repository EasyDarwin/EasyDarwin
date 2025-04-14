package log

import "easydarwin/lnton/pkg/web"

// FindLogsInput 日志查询结构
type FindLogsInput struct {
	web.PagerFilter
	Username string `form:"username" json:"username"`
	Remark   string `form:"remark" json:"remark"`
	StartAt  int64  `form:"start_at" json:"start_at"`
	EndAt    int64  `form:"end_at" json:"end_at"`
}
