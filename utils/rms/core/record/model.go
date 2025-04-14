package record

import (
	"time"

	"easydarwin/lnton/pkg/orm"
	"gorm.io/gorm"
)

// RecordPlan 录像计划模板
type RecordPlan struct { // nolint
	orm.Model
	Name    string `json:"name" gorm:"column:name;notNull;default:'';comment:名称"`
	Plans   string `json:"plans" gorm:"column:plans;notNull;default:'';comment:计划"`
	Enabled bool   `json:"enabled" gorm:"column:enabled;notNull;default:TRUE;comment:是否启用"`
	Days    int    `json:"storage_days" gorm:"column:days;notNull;default:0;comment:存储天数"`
}

// TableName ...
func (*RecordPlan) TableName() string {
	return "record_plans"
}

// // Plans 录像计划，key 为英文前 3 个字母，例如 fri
// type Plans map[string][]Span

// func (i *Plans) Scan(input interface{}) error {
// 	in := input.([]uint8)
// 	return json.Unmarshal(in, i)
// }

// Span 每天的碎片时间段
type Span struct {
	Start string `json:"start"` // 开始时间(时:分)
	End   string `json:"end"`   // 结束时间(时:分)
}

func (rt *RecordPlan) BeforeCreate(tx *gorm.DB) error {
	rt.CreatedAt = orm.Now()
	return nil
}

// 存储目的地
const (
	RecordPlanTypeDeviceStorage = "DEVICE"
	RecordPlanTypeCloudStorage  = "CLOUD"
	RecordPlanTypeCenterStorage = "CENTER"
)

// 存储策略类型
const (
	StrategyTypeByDays     = "DAYS"
	StrategyTypeByCapacity = "CAPS"
)

// RecordWithChannels 录像计划绑定通道
type RecordWithChannels struct {
	ChannelID string   `gorm:"primaryKey;notNull;default:0" json:"channel_id"`
	PlanID    int      `gorm:"notNull;default:0" json:"plan_id"`
	CreatedAt orm.Time `gorm:"notNull;default:CURRENT_TIMESTAMP;index;comment:创建时间" json:"created_at"`
}

func (*RecordWithChannels) TableName() string {
	return "record_with_channels"
}

// RecordPlan2 录像计划
type RecordPlan2 struct {
	ChannelID    string    `gorm:"primaryKey;column:channel_id" json:"channel_id"`
	StorageID    int       `gorm:"column:storage_id;notNull;default:0;comment:云服务配置 id" json:"storage_id,string"`
	StrategyID   int       `gorm:"column:strategy_id;notNull;default:0;comment:云存储策略 id" json:"strategy_id,string"`
	Stream       string    `gorm:"column:stream;notNull;default:'';comment:主子码流" json:"stream"`
	TemplateID   int       `gorm:"column:template_id;notNull;default:0;comment:录像计划模板 id" json:"template_id,string"`
	Enabled      bool      `gorm:"notNull;default:TRUE;comment:是否启用" json:"enabled"`
	CloudEnabled bool      `gorm:"column:cloud_enabled;notNull;default:FALSE;comment:云直存是否启用" json:"cloud_enabled"`
	UpdatedAt    orm.Time  `gorm:"column:updated_at;notNull;default:CURRENT_TIMESTAMP;comment:更新时间" json:"updated_at"` // 更新时间
	NotifiedAt   *orm.Time `gorm:"column:notified_at;comment:通知到设备的时间" json:"notified_at"`                             // 通知到设备的时间
	RMSID        string    `gorm:"column:rms_id;comment:录像服务 ID" json:"rms_id"`
	StoreType    int       `gorm:"column:store_type;notNull;default:0;comment:事件存储/全天存储" json:"store_type"`
}

// TableName ....
func (*RecordPlan2) TableName() string {
	return "record_plans2"
}

// BeforeUpdate 更新时间
func (l *RecordPlan2) BeforeUpdate(tx *gorm.DB) error {
	l.UpdatedAt = orm.Now()
	l.NotifiedAt = &orm.Time{Time: time.Now()}
	return nil
}

//// RecordChannel 录像通道
//type RecordChannel struct {
//	DeviceID  string   `gorm:"column:device_id" json:"device_id"`
//	ChannelID string   `gorm:"column:channel_id" json:"channel_id"`
//	CreatedAt orm.Time `gorm:"type:timestamptz;notNull;default:CURRENT_TIMESTAMP;index;comment:创建时间" json:"created_at"`
//	UpdatedAt orm.Time `gorm:"type:timestamptz;notNull;default:CURRENT_TIMESTAMP;comment:更新时间" json:"updated_at"`
//}
//
//// TableName ....
//func (*RecordChannel) TableName() string {
//	return "record_channels"
//}
//
//// BeforeCreate ...
//func (p *RecordChannel) BeforeCreate(tx *gorm.DB) error {
//	p.UpdatedAt = orm.Now()
//	return nil
//}
//
//// BeforeUpdate ...
//func (p *RecordChannel) BeforeUpdate(tx *gorm.DB) error {
//	p.UpdatedAt = orm.Now()
//
//	return nil
//}

// CloudStorage 云存储服务
type CloudStorage struct {
	ID        int      `json:"id,string"        gorm:"primaryKey"`
	Name      string   `json:"name"      gorm:"notNull;default:'';comment:云存储服务名称"`
	Type      string   `json:"type"      gorm:"notNull;default:'';comment:云存储服务类型"`
	EndPoint  string   `json:"end_point" gorm:"notNull;default:'';comment:云存储服务地址"`
	Bucket    string   `json:"bucket"    gorm:"notNUll;default:'';comment:云存储服务桶名"`
	KeyID     string   `json:"key_id"    gorm:"column:key_id;notNUll;default:'';comment:云存储服务 Key ID"`
	Secret    string   `json:"secret"    gorm:"notNUll;default:'';comment:云存储服务 Secret"`
	Region    string   `json:"region"    gorm:"notNUll;default:'';comment:云存储服务区域" `
	UpdatedAt orm.Time `gorm:"column:updated_at;notNull;default:CURRENT_TIMESTAMP;comment:更新时间" json:"updated_at"` // 更新时间
}

func (*CloudStorage) TableName() string {
	return "cloud_storages"
}

func (c *CloudStorage) BeforeCreate(tx *gorm.DB) error {
	c.UpdatedAt = orm.Now()
	return nil
}

func (c *CloudStorage) BeforeUpdate(tx *gorm.DB) error {
	c.UpdatedAt = orm.Now()
	return nil
}

// CloudStrategy 存储策略
type CloudStrategy struct {
	ID        int      `gorm:"pk" json:"id,string"`
	Name      string   `gorm:"notNull;default:'';comment:策略名称" json:"name"`
	Type      string   `gorm:"notNull;default:'';comment:策略类型" json:"type"`
	Value     int      `gorm:"notNUll;default:0;comment:策略值" json:"value"`
	UpdatedAt orm.Time `gorm:"column:updated_at;notNull;default:CURRENT_TIMESTAMP;comment:更新时间" json:"updated_at"` // 更新时间
}

func (*CloudStrategy) TableName() string {
	return "cloud_strategies"
}

func (cy *CloudStrategy) BeforeCreate(tx *gorm.DB) error {
	cy.UpdatedAt = orm.Now()
	return nil
}

func (cy *CloudStrategy) BeforeUpdate(tx *gorm.DB) error {
	cy.UpdatedAt = orm.Now()
	return nil
}
