package record

import "easydarwin/lnton/pkg/orm"

type FindStoragesOutput struct {
	ID       int    `json:"id,string"`
	Name     string `json:"name"`
	Type     string `json:"type"`
	EndPoint string `json:"end_point"`
	Bucket   string `json:"bucket"`
	KeyID    string `json:"key_id"`
	Secret   string `json:"secret"`
	Region   string `json:"region"`
}

type GetChannelRecordPlanOutput struct {
	ID             int    `json:"id,string"`
	ChannelID      int    `json:"channel_id"` // 管道
	TemplateID     int    `json:"template_id"`
	Type           string `json:"type"` // 区分是云存还是设备存储
	StreamType     int    `json:"stream_type"`
	Duration       int    `json:"duration"`
	CouldStorageID int    `json:"cloud_storage_id"`
	CreatedAt      string `json:"created_at"`
}

type BaseSotrageOutput struct {
	ID   int    `json:"id,string"`
	Name string `json:"name"`
	Type string `json:"type"`
}

type FindCloudStrategyOutput struct {
	ID    int    `json:"id,string"`
	Name  string `json:"name"`
	Type  string `json:"type"`
	Value int    `json:"value"`
}

type RecordTemplateBaseOutput struct {
	ID   int    `json:"id,string"`
	Name string `json:"name"`
}

type GetRecordPlanDetailOutput struct {
	Plan     RecordPlan2
	Storage  CloudStorage
	Strategy CloudStrategy
	Template RecordPlan
}

type RecordPlanWithBID struct {
	RecordPlan2
	BID      string `gorm:"column:bid" json:"bid"`
	DeviceID string
}

type RecordPlanDetailOutput struct {
	Plan     RecordPlanWithBID
	Storage  CloudStorage
	Strategy CloudStrategy
	Template RecordPlan
}

// RecordPlanWithChannelOutput 获取录像计划关联通道输出模型
type RecordPlanWithChannelOutput struct {
	RecordWithChannels
	Name       string `gorm:"column:name;notNull;default:'';comment:设备名称" json:"name"`         // 设备名称
	Protocol   string `gorm:"column:protocol;notNull;default:'';comment:设备协议" json:"protocol"` // 设备协议 rtsp/rtmp
	IP         string `gorm:"column:ip;notNull;default:'';comment:设备IP" json:"ip"`             // ip 地址
	Port       int    `gorm:"column:port;notNull;default:0;comment:端口" json:"port"`            // 端口号
	Remark     string `gorm:"column:remark;notNull;default:'';comment:备注" json:"remark"`       // 备注描述
	Username   string `gorm:"column:username;notNull;default:'';comment:账号" json:"username"`
	ChildCount int    `gorm:"column:child_count;notNull;default:0" json:"child_count"` // 子通道数量(不包含子孙通道)
	Status     bool   `gorm:"column:status;notNull;default:false;comment:通道状态" json:"status"`
	Playing    bool   `gorm:"-" json:"playing"`
	IsDir      bool   `gorm:"-" json:"is_dir"`
}

type FindRecordChannelOutput struct {
	ID         string   `gorm:"primaryKey;column:id" json:"id"` // ID
	CreatedAt  orm.Time `gorm:"type:timestamptz;notNull;default:CURRENT_TIMESTAMP;index;comment:创建时间" json:"created_at"`
	UpdatedAt  orm.Time `gorm:"type:timestamptz;notNull;default:CURRENT_TIMESTAMP;comment:更新时间" json:"updated_at"`
	Enabled    bool     `gorm:"column:enabled;notNull;default:true;comment:是否启用" json:"enabled"`         // 是否启用
	Name       string   `gorm:"column:name;notNull;default:'';comment:通道名称" json:"name"`                 // 通道名称
	DeviceID   string   `gorm:"column:device_id;notNull;default:'';index;comment:设备ID" json:"device_id"` // 设备 id
	Protocol   string   `gorm:"column:protocol;notNull;default:'';comment:通道协议" json:"protocol"`         // 通道协议
	PTZType    int      `gorm:"column:ptz_type;notNull;default:0;comment:云台类型" json:"ptz_type"`          // 云台类型
	Remark     string   `gorm:"column:remark;notNull;default:'';comment:备注" json:"remark"`               // 备注描述
	Transport  string   `gorm:"column:transport;notNull;default:'TCP';comment:传输协议" json:"transport"`    // TCP/UDP
	IP         string   `gorm:"column:ip;notNull;default:'';comment:IP" json:"ip"`                       // ip 地址
	Port       int      `gorm:"column:port;notNull;default:0;comment:端口号" json:"port"`                   // 端口号
	Username   string   `gorm:"column:username;notNull;default:'';comment:用户名" json:"-"`                 // 用户名
	Password   string   `gorm:"column:password;notNull;default:'';comment:密码" json:"-"`                  // 密码
	BID        string   `gorm:"column:bid;notNull;default:'';comment:协议专属 id" json:"bid"`
	PTZ        bool     `gorm:"column:ptz;notNull;default:FALSE;comment:是否支持 ptz" json:"ptz"` // 是否支持 ptz
	Talk       bool     `gorm:"column:talk;notNull;default:FALSE;comment:是否支持对讲" json:"talk"` // 是否支持语音对讲
	PID        string   `gorm:"column:pid;notNull;index;default:'';comment:父通道 ID" json:"pid"`
	ChildCount int      `gorm:"column:child_count;notNull;default:0" json:"child_count"` // 子通道数量(不包含子孙通道)
	Status     bool     `gorm:"column:status;notNull;default:false;comment:通道状态" json:"status"`
	Playing    bool     `gorm:"-" json:"playing"`
	IsDir      bool     `gorm:"-" json:"is_dir"`
}
