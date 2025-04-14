package dict

import (
	"easydarwin/lnton/pkg/orm"
	"gorm.io/gorm"
)

// DictType 字典类型
type DictType struct {
	ID        string   `gorm:"primaryKey" json:"id"`
	Code      string   `gorm:"column:code" json:"code"`             // 类型编码
	Name      string   `gorm:"notNull;default:''" json:"name"`      // 类型名称
	CreatedAt orm.Time `gorm:"column:created_at" json:"created_at"` // 创建时间
	UpdatedAt orm.Time `gorm:"column:updated_at" json:"updated_at"` // 更新时间
	IsDefault bool     `gorm:"column:is_default" json:"is_default"` // 是否系统默认
}

func (*DictType) TableName() string {
	return "dict_types"
}

func (d *DictType) BeforeCreate(tx *gorm.DB) error {
	d.CreatedAt = orm.Now()
	d.UpdatedAt = orm.Now()
	return nil
}

func (d *DictType) BeforeUpdate(tx *gorm.DB) error {
	d.UpdatedAt = orm.Now()
	return nil
}

// DictData 字典数据
type DictData struct {
	ID        string   `gorm:"primaryKey;column:id" json:"id"`          // 主键ID
	Label     string   `gorm:"notNull;default:''" json:"label"`         // 标签
	Value     string   `gorm:"notNull;default:''" json:"value"`         // 值
	Sort      int      `gorm:"notNull;default:0" json:"sort"`           // 排序
	Remark    string   `gorm:"notNull;default:''" json:"remark"`        // 描述
	Enabled   bool     `gorm:"notNull;default:TRUE" json:"enabled"`     // 是否启用
	CreatedAt orm.Time `gorm:"column:created_at" json:"created_at"`     // 创建时间
	UpdatedAt orm.Time `gorm:"column:updated_at" json:"updated_at"`     // 更新时间
	IsDefault bool     `gorm:"notNull;default:FALSE" json:"is_default"` // 是否系统默认
	Code      string   `gorm:"notNull;default:''" json:"code"`          // 字典类型
	Flag      string   `gorm:"notNull;default:''" json:"flag"`          // 预留标志(语言)
}

func (*DictData) TableName() string {
	return "dict_datas"
}

func (d *DictData) BeforeCreate(tx *gorm.DB) error {
	d.CreatedAt = orm.Now()
	d.UpdatedAt = orm.Now()
	return nil
}

func (d *DictData) BeforeUpdate(tx *gorm.DB) error {
	d.UpdatedAt = orm.Now()
	return nil
}
