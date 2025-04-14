package versiondb

import (
	"easydarwin/lnton/plugin/core/version"
	"gorm.io/gorm"
)

// DB ...
type DB struct {
	db *gorm.DB
}

// NewDB ...
func NewDB(db *gorm.DB) DB {
	return DB{db: db}
}

// AutoMigrate ...
func (d DB) AutoMigrate(ok bool) DB {
	if !ok {
		return d
	}
	if err := d.db.AutoMigrate(
		new(version.Version),
	); err != nil {
		panic(err)
	}
	return d
}

// First ...
func (d DB) First(v *version.Version) error {
	return d.db.Order("id DESC").First(v).Error
}

// Add ...
func (d DB) Add(v *version.Version) error {
	return d.db.Create(v).Error
}
