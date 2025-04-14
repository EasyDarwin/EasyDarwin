package alarm

import (
	"encoding"
	"encoding/json"

	"easydarwin/lnton/pkg/orm"
	"github.com/lib/pq"
	"gorm.io/gorm"
)

// Alarm 告警
type Alarm struct {
	orm.ModelWithStrID
	DeviceID         string  `gorm:"notNull;default:'';index;comment:设备 ID" json:"device_id"`
	ChannelID        string  `gorm:"notNull;default:'';comment:通道 ID" json:"channel_id"`
	Priority         int     `gorm:"notNull;default:0;comment:报警级别" json:"priority"` // 1/2/3/4
	Method           int     `gorm:"notNull;default:0;comment:告警方法" json:"method"`
	Type             string  `gorm:"notNull;default:0;comment:告警类型" json:"type"`
	EventType        int     `gorm:"notNull;default:0;comment:事件类型" json:"event_type"`
	AlarmTime        string  `gorm:"notNull;default:'';comment:报警时间" json:"alarm_time"` // 报警时间
	AlarmDescription string  `xml:"AlarmDescription"`                                   // 报警描述,可选
	Longitude        float64 `gorm:"notNull;default:0;comment:经度" json:"longitude"`
	Latitude         float64 `gorm:"notNull;default:0;comment:纬度" json:"latitude"`

	// Snapshot  string         `gorm:"notNull;default:'';comment:快照" json:"snapshot"`
	Snapshots pq.StringArray `gorm:"notNull;type:text[];default:'{}';comment:快照" json:"snapshots"`
	SnapPath  string         `gorm:"notNull;default:'';comment:快照路径" json:"snap_path"`
	VideoPath string         `gorm:"notNull;default:'';comment:视频" json:"video_path"`
	LogPath   string         `gorm:"notNull;default:'';comment:日志" json:"log_path"`

	Template string `gorm:"notNull;default:'';comment:模板" json:"template"`
}

// TableName ...
func (*Alarm) TableName() string {
	return "alarms"
}

func (a *Alarm) BeforeCreate(tx *gorm.DB) error {
	a.CreatedAt = orm.Now()
	a.UpdatedAt = orm.Now()
	return nil
}

func (a *Alarm) BeforeUpdate(tx *gorm.DB) error {
	a.UpdatedAt = orm.Now()
	return nil
}

// UnmarshalBinary implements encoding.BinaryUnmarshaler.
func (m *ModelInput) UnmarshalBinary(data []byte) error {
	return json.Unmarshal(data, m)
}

// MarshalBinary implements encoding.BinaryMarshaler.
func (m ModelInput) MarshalBinary() (data []byte, err error) {
	return json.Marshal(m)
}

var (
	_ encoding.BinaryMarshaler   = ModelInput{}
	_ encoding.BinaryUnmarshaler = (*ModelInput)(nil)
)

// 报警方式，可自由组合如1/2为电话报警或设备报警
// 报警方式为2-设备报警时不携带AlarmType为默认的报警设备报警
var alarmMethod = map[int]string{
	0: "全部",
	1: "电话报警",
	2: "设备报警",
	3: "短信报警",
	4: "GPS报警",
	5: "视频报警",
	6: "设备故障报警",
	7: "其他报警",
}

var alarmMethodType = map[int]map[int]string{
	2: {
		1: "视频丢失报警",
		2: "设备防拆报警",
		3: "存储设备磁盘满报警",
		4: "设备高温报警",
		5: "设备低温报警",
	},
	5: {
		1:  "人工视频报警",
		2:  "运动目标检测报警",
		3:  "遗留物检测报警",
		4:  "物体移除检测报警",
		5:  "绊线检测报警",
		6:  "入侵检测报警",
		7:  "逆行检测报警",
		8:  "徘徊检测报警",
		9:  "流量统计报警",
		10: "密度检测报警",
		11: "视频异常检测报警",
		12: "快速移动报警",
	},
	6: {
		1: "存储设备磁盘故障报警",
		2: "存储设备风扇故障报警",
	},
}

type AlarmInfoByDeviceID struct {
	DeviceID string // 设备数据
	Count    int    // 当前设备报警数量
}

type AlarmInfo struct {
	Total     int64 // 全部设备报警数量
	DeviceTop []*AlarmInfoByDeviceID
}

type AlarmPlan struct {
	orm.ModelWithStrID
	Name           string `gorm:"notNull;default:'';comment：“名称" json:"name"`
	SnapInterval   int    `gorm:"notNull;default:0;comment:快照周期" json:"snap_interval"`
	RecordDuration int    `gorm:"notNull;default:0;comment:录像时长" json:"record_duration"`
	Priority       string `gorm:"notNull;default:'';comment:报警级别" json:"priority"`
	Method         string `gorm:"notNull;default:'';comment:报警方式" json:"method"`
	Type           string `gorm:"notNull;default:'';comment:报警类型" json:"type"`
	EventType      string `gorm:"notNull;default:'';comment:事件类型" json:"event_type"`
	Enabled        bool   `gorm:"notNull;default:true;comment:是否启用" json:"enabled"`
}

func (*AlarmPlan) TableName() string {
	return "alarm_plans"
}

type PlanChannel struct {
	PlanID    string `json:"plan_id"`     // 报警计划ID
	ChannelID string `json:"channel_ids"` // 通道ID
	DeviceID  string `json:"device_id"`   // 设备ID
}

func (*PlanChannel) TableName() string {
	return "alarm_plan_channels"
}
