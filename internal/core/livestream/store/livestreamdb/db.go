package livestreamdb

import (
	"easydarwin/internal/core/livestream"
	"gorm.io/gorm"
)

type DB struct {
	db *gorm.DB
}

func NewDB(db *gorm.DB) DB {
	return DB{db: db}
}

// AutoMigrate 表迁移
func (d DB) AutoMigrate(ok bool) DB {
	if !ok {
		return d
	}
	if err := d.db.AutoMigrate(
		new(livestream.LiveStream),
	); err != nil {
		panic(err)
	}

	return d
}
func (d DB) Create(v *livestream.LiveStream) error {
	return d.db.Create(v).Error
}
func (d DB) Delete(id int) error {
	return d.db.Where("id=?", id).Delete(new(livestream.LiveStream)).Error
}

func (d DB) Update(v *livestream.LiveStream, id int) error {
	if err := d.db.Model(&livestream.LiveStream{}).Where("id=?", id).Save(&v).Error; err != nil {
		return err
	}
	return nil
}
func (d DB) Find(v *[]*livestream.LiveStream, in livestream.PagerFilter) (int64, error) {
	// 使用传入的数据库实例
	db := d.db.Model(new(livestream.LiveStream))

	if in.Q != "" {
		db = db.Where("name like ?", "%"+in.Q+"%")
	}

	if in.Type != "" { // 用来查询指定等级的用户
		db = db.Where("live_type = ?", in.Type)
	}
	var total int64
	// 查询符合条件的总数
	if err := db.Count(&total).Error; err != nil {
		return 0, err
	}
	// 查询符合条件的数据，并按照id降序排列，限制返回的条数，并设置偏移量
	err := db.Limit(in.Limit()).Offset(in.Offset()).Order("id DESC").Find(v).Error
	// 返回符合条件的总数和查询结果
	return total, err
}
func (d DB) FindAll(v *[]livestream.LiveStream) (int64, error) {
	// 使用传入的数据库实例
	db := d.db.Model(new(livestream.LiveStream))
	var total int64
	// 查询符合条件的总数
	if err := db.Count(&total).Where(`live_type = ? and url !=''`, livestream.LIVE_PULL).Error; err != nil {
		return 0, err
	}
	// 查询符合条件的数据，并按照id降序排列，限制返回的条数，并设置偏移量
	err := db.Order("id DESC").Where(`live_type = ? and url !=''`, livestream.LIVE_PULL).Find(v).Error
	// 返回符合条件的总数和查询结果
	return total, err
}
func (d DB) FindPushAll(v *[]livestream.LiveStream) (int64, error) {
	// 使用传入的数据库实例
	db := d.db.Model(new(livestream.LiveStream))
	var total int64
	// 查询符合条件的总数
	if err := db.Count(&total).Where(`live_type = ?`, livestream.LIVE_PUSH).Error; err != nil {
		return 0, err
	}
	// 查询符合条件的数据，并按照id降序排列，限制返回的条数，并设置偏移量
	err := db.Order("id DESC").Where(`live_type = ?`, livestream.LIVE_PUSH).Find(v).Error
	// 返回符合条件的总数和查询结果
	return total, err
}
func (d DB) GetByID(v *livestream.LiveStream, id int) error {
	return d.db.Model(&livestream.LiveStream{}).Where(`id=?`, id).First(v).Error
}
func (d DB) UpdateInt(id int, key string, v int) error {
	if err := d.db.Model(&livestream.LiveStream{}).Where("id=?", id).Update(key, v).Error; err != nil {
		return err
	}
	return nil
}
func (d DB) UpdateString(id int, key string, v string) error {
	if err := d.db.Model(&livestream.LiveStream{}).Where("id=?", id).Update(key, v).Error; err != nil {
		return err
	}
	return nil
}
func (d DB) UpdateSnap(id int, raw, jpg string) error {
	if err := d.db.Model(&livestream.LiveStream{}).Where("id=?", id).Updates(map[string]any{
		"key_frame": raw,
		"snap_url":  jpg,
	}).Error; err != nil {
		return err
	}

	return nil
}
func (d DB) UpdateOnlineAll(v int) error {
	if err := d.db.Model(&livestream.LiveStream{}).Where(`id != ?`, 0).Updates(map[string]any{
		"online": v,
	}).Error; err != nil {
		return err
	}
	return nil
}
