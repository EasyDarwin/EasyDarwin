package msgpushdb

import (
	"easydarwin/lnton/plugin/core/msgpush"
	"gorm.io/gorm"
)

// MsgPush Get business instance
func (d DB) MsgPush() msgpush.MsgPushStorer {
	return MsgPush{db: d.db}
}

// NewDB 创建一个新的DB实例
func NewDB(db *gorm.DB) *DB {
	return &DB{
		db: db,
	}
}

// DB 结构体，包含一个 *gorm.DB 类型的 db 字段
type DB struct {
	db *gorm.DB
}

// AutoMerge 数据库自动迁移
func (d DB) AutoMerge(ok bool) DB {
	// 如果ok为false，则直接返回d
	if !ok {
		return d
	}
	// 如果数据库自动迁移失败，则抛出异常
	if err := d.db.AutoMigrate(
		new(msgpush.MsgPush),
	); err != nil {
		panic(err)
	}
	// 返回d
	return d
}
