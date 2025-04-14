package dictdb

import (
	"easydarwin/lnton/pkg/orm"
	"easydarwin/lnton/plugin/core/dict"
	"gorm.io/gorm"
)

type DB struct {
	db *gorm.DB
	orm.Engine
}

func NewDB(db *gorm.DB) DB {
	return DB{db: db, Engine: orm.NewEngine(db)}
}

func (d DB) AutoMerge(ok bool) DB {
	if !ok {
		return d
	}
	if err := d.db.AutoMigrate(
		new(dict.DictType),
		new(dict.DictData),
	); err != nil {
		panic(err)
	}
	return d
}

// AddDictType implements dict.Storer.
func (d DB) AddDictType(input *dict.DictType) error {
	return d.db.Create(input).Error
}

// DeleteDictType implements dict.Storer.
func (d DB) DeleteDictType(code string) error {
	return d.db.Transaction(func(tx *gorm.DB) error {
		if err := tx.Where("code=?", code).Delete(&dict.DictType{}).Error; err != nil {
			return err
		}
		return tx.Where("code=?", code).Delete(&dict.DictData{}).Error
	})
}

// CountByDictData implements dict.Storer.
func (d DB) CountByDictData(code string, count *int64) error {
	db := d.db.Model(&dict.DictData{})
	return db.Where("code=? AND NOT is_default", code).Count(count).Error
}

// UpdateTypeByID implements dict.Storer.
func (d DB) UpdateTypeByID(id, name string) error {
	return d.db.Model(&dict.DictType{}).Where("id=?", id).UpdateColumn("name", name).Error
}

// GetDictTypeByID implements dict.Storer.
func (d DB) GetDictTypeByID(b *dict.DictType, id string) error {
	return d.db.Where("id=?", id).First(b).Error
}

// GetDictTypeByCode implements dict.Storer.
func (d DB) GetDictTypeByCode(b *dict.DictType, code string) error {
	return d.db.Where("code=?", code).First(b).Error
}

// FindDictType implements dict.Storer.
func (d DB) FindDictType(bs *[]*dict.DictType, name string, limit, offset int) (int64, error) {
	db := d.db.Model(&dict.DictType{})
	if name != "" {
		db = db.Where("name LIKE ?", "%"+name+"%")
	}
	var count int64
	if err := db.Count(&count).Error; err != nil {
		return count, err
	}
	return count, db.Order("id ASC").Limit(limit).Offset(offset).Find(bs).Error
}

// FindDictdata implements dict.Storer.
func (d DB) FindDictdata(bs *[]*dict.DictData) (int64, error) {
	db := d.db.Model(&dict.DictData{})
	var count int64
	if err := db.Count(&count).Error; err != nil {
		return count, err
	}
	return count, db.Order("id ASC").Limit(100000).Offset(0).Find(bs).Error
}

// AddDictData implements dict.Storer.
func (d DB) AddDictData(input *dict.DictData) error {
	return d.db.Create(input).Error
}

// DeleteDictData implements dict.Storer.
func (d DB) DeleteDictData(id string) error {
	return d.db.Model(&dict.DictData{}).Where("id=?", id).Delete(dict.DictData{}).Error
}

// GetDictDataByID implements dict.Storer.
func (d DB) GetDictDataByID(b *dict.DictData, id string) error {
	return d.db.Where("id=?", id).First(b).Error
}

// SaveDictData implements dict.Storer.
func (d DB) SaveDictData(b *dict.DictData) error {
	return d.db.Save(b).Error
}

// FindDictData implements dict.Storer.
func (d DB) FindDictData(bs *[]*dict.DictData, code string, label string) (int64, error) {
	db := d.db.Model(&dict.DictData{})
	if code != "" {
		db = db.Where("code=?", code)
	}
	if label != "" {
		db = db.Where("label like ?", "%"+label+"%")
	}
	var total int64
	if err := db.Count(&total).Error; err != nil {
		return 0, err
	}
	return total, db.Order("sort ASC").Find(bs).Error
}

// CreateOrInitDictType implements dict.Storer.
func (d DB) CreateOrInitDictType(dt []*dict.DictType) error {
	for _, v := range dt {
		if v.ID == "" {
			continue
		}
		if err := d.db.FirstOrCreate(v).Error; err != nil {
			return err
		}
	}
	return nil
}

// CreateOrInitDictData implements dict.Storer.
func (d DB) CreateOrInitDictData(dd []*dict.DictData) error {
	for _, v := range dd {
		if v.ID == "" {
			continue
		}
		if err := d.db.FirstOrCreate(v).Error; err != nil {
			return err
		}
	}
	return nil
}

func (d DB) GetDictDataByType(dt *[]*dict.DictData, dType string) error {
	return d.db.Model(&dict.DictData{}).Where("code = ?", dType).Find(&dt).Error
}
