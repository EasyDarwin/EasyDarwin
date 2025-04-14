package log

import (
	"database/sql/driver"
	"encoding/json"

	"easydarwin/lnton/pkg/orm"
	"gorm.io/gorm"
)

// Log ..
type Log struct {
	orm.Model
	Username   string  `gorm:"type:text;notNull;default:'';comment:用户名" json:"username"`
	Remark     string  `gorm:"type:text;notNull;default:'';comment:备注" json:"remark"`
	Type       string  `gorm:"type:text;notNull;index;default:'operate';comment:类型" json:"type"`
	TargetID   int     `gorm:"type:int;notNull;default:0;index;comment:目标 ID" json:"target_id"`
	TargetName string  `gorm:"type:text;notNull;default:'';index;comment:目标名称" json:"target_name"`
	Misc       MiscLog `gorm:"type:jsonb;notNull;default:'{}';comment:补充信息" json:"misc"`
}

// TableName ...
func (*Log) TableName() string {
	return "logs"
}

// Scan implements orm.Scaner.
func (d *Log) Scan(input interface{}) error {
	return orm.JsonUnmarshal(input, d)
}

func (d Log) Value() (driver.Value, error) {
	return json.Marshal(d)
}

// MiscLog ..
type MiscLog struct {
	Method string `json:"method,omitempty"`
	Path   string `json:"path,omitempty"`
}

// Scan implements scaner
func (m *MiscLog) Scan(input interface{}) error {
	return orm.JsonUnmarshal(input, m)
}

func (m MiscLog) Value() (driver.Value, error) {
	return json.Marshal(m)
}

func (l *Log) BeforeCreate(tx *gorm.DB) error {
	l.CreatedAt = orm.Now()
	l.UpdatedAt = orm.Now()
	return nil
}

func (l *Log) BeforeUpdate(tx *gorm.DB) error {
	l.UpdatedAt = orm.Now()
	return nil
}
