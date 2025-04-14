package tvdb

import (
	"easydarwin/lnton/pkg/orm"
	"easydarwin/lnton/plugin/core/tvs"
	"gorm.io/gorm"
)

// DB 数据库实例
type DB struct {
	db *gorm.DB
	orm.Engine
}

// NewDB 返回新的数据库实例
func NewDB(db *gorm.DB) DB {
	return DB{db: db, Engine: orm.NewEngine(db)}
}

// AutoMerge 数据库自动迁移
func (d DB) AutoMerge(ok bool) DB {
	if !ok {
		return d
	}
	if err := d.db.AutoMigrate(
		new(tvs.GroupedTVs),
	); err != nil {
		panic(err)
	}
	return d
}

// ScanDBGetChannels 扫描数据库获取所有channels
func (d DB) ScanDBGetChannels(channels *[]string) error {
	if err := d.db.Table("channels").Order("id DESC").Pluck("id", channels).Error; err != nil {
		return err
	}
	return nil
}

// AddChannelGroups 创建channel组
func (d DB) AddChannelGroups(in *tvs.GroupedTVs) error {
	return d.db.Create(in).Error
}

// FindWalls 查询所有channel group
func (d DB) FindWalls(bs *[]*tvs.GroupedTVs) (int64, error) {
	db := d.db
	var count int64
	if err := db.Model(tvs.GroupedTVs{}).Count(&count).Error; err != nil || count == 0 {
		return count, err
	}
	err := db.Order("id asc").Find(bs).Error
	return count, err
}

// DeleteAllOldChannels 清除旧的groupedTV表数据
func (d DB) DeleteAllOldChannels() error {
	return d.db.Where("id>0").Delete(&tvs.GroupedTVs{}).Error
}
