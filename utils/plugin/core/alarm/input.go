package alarm

import "easydarwin/lnton/pkg/web"

type AddAlarmPlanInput struct {
	Name           string `json:"name"`
	SnapInterval   int    `json:"snap_interval"`
	RecordDuration int    `json:"record_duration"`
	Priority       string `json:"priority"`
	Method         string `json:"method"`
	Type           string `json:"type"`
	EventType      string `json:"event_type"`
	Enable         bool   `json:"enable"`
}

type UpdateAlarmPlanInput struct {
	ID             string `json:"id"`
	Name           string `json:"name"`
	SnapInterval   int    `json:"snap_interval"`
	RecordDuration int    `json:"record_duration"`
	Priority       string `json:"priority"`
	Method         string `json:"method"`
	Type           string `json:"type"`
	EventType      string `json:"event_type"`
	Enabled        bool   `json:"enabled"`
}

type FindAlarmPlanInput struct {
	web.PagerFilter
}

// FindInput 事件列表查询
type FindInput struct {
	web.PagerFilter
	// 非必填项，如果存在则为查询条件
	Priority int    `form:"priority"`
	Method   int    `form:"method"`
	StartAt  int64  `form:"start_at"`
	EndAt    int64  `form:"end_at"`
	DeviceID string `form:"device_id"`
	Type     int    `form:"type"`
}

type FindAlarmInfoInput struct {
	DeviceID []string `from:"-"`
	Top      int      `from:"top"` // 获取top排行
	Priority int      `from:"-"`
	Method   int      `from:"-"`
	StartAt  int64    `from:"start_at"` // 开始开始时间
	EndAt    int64    `from:"end_at"`   // 报警结束时间
	Type     int      `from:"type"`     // 报警类型
}

// ModelInput 模型
type ModelInput struct {
	ID        string `json:"id"`
	SN        string `json:"sn"`         // 设备唯一编码
	ChannelID string `json:"channel_id"` // 通道 id
	Priority  int    `json:"priority"`   // 优先级, 1/2/3/4，报警级别越小，优先级越高
	Method    int    `json:"method"`     // 报警方式;1:电话报警;2:设备报警;3:短信报警;4:GPS报警;5:视频报警;6:设备故障报警;7:其它报警
	Timestamp int64  `json:"timestamp"`  // 开始报警时间(时间戳毫秒)
	Type      int    `json:"type"`       // 报警类型，根据报警方式区分
	SnapPaths string `json:"snap_paths"` // 图片链接地址
	VideoPath string `json:"video_path"` // 视频链接地址
	LogPath   string `json:"log_path"`   // 日志文件地址
	Template  string `json:"template"`   // 临时，仅供开发人员使用
	Ext       string `json:"ext"`        // 扩展
}

type SetPlanChannelInput struct {
	AlarmPlanID string   `json:"-"`
	ChannelIDs  []string `json:"channel_ids"`
}
