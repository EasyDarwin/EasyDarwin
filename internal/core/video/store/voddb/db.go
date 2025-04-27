package voddb

import (
	"easydarwin/internal/core/video"
	"gorm.io/gorm"
)

type DB struct {
	db *gorm.DB
}

func NewDB(db *gorm.DB) DB {
	return DB{db: db}
}

// 若表不存在则创建
func (d DB) AutoMigrate(ok bool) DB {
	if !ok {
		return d
	}
	if err := d.db.AutoMigrate(
		new(video.TVod), new(video.TVodStore),
	); err != nil {
		panic(err)
	}

	return d
}
