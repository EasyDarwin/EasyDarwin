package logdb

import (
	"time"

	"easydarwin/lnton/pkg/web"
	"easydarwin/lnton/plugin/core/log"
	"gorm.io/gorm"
)

// DB ...
type DB struct {
	db *gorm.DB
}

// Clear implements log.Storer.
func (d DB) Clear(expire time.Duration) error {
	return d.db.Where("created_at < ?", time.Now().Add(-expire)).Delete(&log.Log{}).Error
}

// Create implements log.Storer.
func (d DB) Create(bs []*log.Log) error {
	return d.db.Create(bs).Error
}

// NewDB ...
func NewDB(db *gorm.DB) DB {
	return DB{db: db}
}

// AutoMigrate 表迁移
func (d DB) AutoMigrate(ok bool) DB {
	if !ok {
		return d
	}
	if err := d.db.AutoMigrate(new(log.Log)); err != nil {
		panic(err)
	}
	return d
}

var _ log.Storer = (*DB)(nil)

// Find implements log.Storer.
func (d DB) Find(out *[]*log.Log, input log.FindLogsInput) (total int64, err error) {
	db := d.db.Model(&log.Log{})
	if input.Username != "" {
		db = db.Where("username = ?", input.Username)
	}
	if input.Remark != "" {
		db = db.Where("remark like ?", "%"+input.Remark+"%")
	}
	// db = db.Where("created_at BETWEEN TO_TIMESTAMP(?) AND TO_TIMESTAMP(?) ", input.StartAt, input.EndAt)
	if input.StartAt > 0 {
		db = db.Where("created_at >= ?", time.Unix(input.StartAt, 0))
	}
	if input.EndAt > 0 {
		db = db.Where("created_at <= ?", time.Unix(input.EndAt, 0))
	}

	if err = db.Count(&total).Error; err != nil {
		err = web.ErrDB.With(err.Error())
		return
	}
	if err = db.Limit(input.Limit()).Offset(input.Offset()).Order("id DESC").Find(out).Error; err != nil {
		err = web.ErrDB.With(err.Error())
		return
	}
	return
}
