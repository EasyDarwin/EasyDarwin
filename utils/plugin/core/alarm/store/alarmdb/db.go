package alarmdb

import (
	"easydarwin/lnton/pkg/orm"
	"easydarwin/lnton/plugin/core/alarm"
	"gorm.io/gorm"
)

// DB 告警数据库实例
type DB struct {
	db *gorm.DB
	orm.Engine
}

// NewDB 返回新的告警数据库实例
func NewDB(db *gorm.DB) DB {
	return DB{db: db, Engine: orm.NewEngine(db)}
}

// AutoMerge 告警数据库自动迁移
func (d DB) AutoMerge(ok bool) DB {
	if !ok {
		return d
	}
	if err := d.db.AutoMigrate(
		new(alarm.Alarm),
		new(alarm.AlarmPlan),
		new(alarm.PlanChannel),
	); err != nil {
		panic(err)
	}
	return d
}
