package msgpush

import "easydarwin/lnton/pkg/orm"

type MsgPush struct {
	ID        string   `gorm:"primaryKey" json:"id"`
	CreatedAt orm.Time `gorm:"notNull;default:CURRENT_TIMESTAMP;index;comment:创建时间" json:"created_at"`
	UpdatedAt orm.Time `gorm:"notNull;default:CURRENT_TIMESTAMP;comment:更新时间" json:"updated_at"`
	DeviceID  string   `gorm:"notNull;default:'',comment:设备ID" json:"device_id"`
	ChannelID string   `gorm:"notNull;default:'',comment:通道ID" json:"channel_id"`
	Msg       string   `gorm:"notNull;default:'',comment:消息" json:"msg"`
}

func (*MsgPush) TableName() string {
	return "msg_pushes"
}

// Message 消息结构体
type Message struct {
	Msg string `json:"msg"`
}
