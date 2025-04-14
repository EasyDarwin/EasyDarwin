package orm

import (
	"database/sql/driver"
	"encoding/json"
	"fmt"
	"strconv"
	"strings"
	"time"
	"unsafe"

	"gorm.io/gorm"
)

// EnabledAutoMigrate 是否开启自动迁移
// 每次表迁移耗时，提供此全局变量，程序可根据需要是否迁移
var EnabledAutoMigrate bool

// Scaner 所有模型内组合的结构体，必须满足该接口
type Scaner interface {
	Scan(input interface{}) error
}

// Model int id 模型
// sqlite 不支持 default:now()，支持 CURRENT_TIMESTAMP
type Model struct {
	ID        int  `gorm:"primaryKey;" json:"id"`
	CreatedAt Time `gorm:"notNull;default:CURRENT_TIMESTAMP;index;comment:创建时间" json:"created_at"`
	UpdatedAt Time `gorm:"notNull;default:CURRENT_TIMESTAMP;comment:更新时间" json:"updated_at"`
}

// ModelWithStrID string id 模型
type ModelWithStrID struct {
	ID        string `gorm:"primaryKey;" json:"id"`
	CreatedAt Time   `gorm:"notNull;default:CURRENT_TIMESTAMP;index;comment:创建时间" json:"created_at"`
	UpdatedAt Time   `gorm:"notNull;default:CURRENT_TIMESTAMP;comment:更新时间" json:"updated_at"`
}

func (d *ModelWithStrID) BeforeCreate(*gorm.DB) error {
	d.CreatedAt = Now()
	d.UpdatedAt = Now()
	return nil
}

func (d *ModelWithStrID) BeforeUpdate(*gorm.DB) error {
	d.UpdatedAt = Now()
	return nil
}

func (d *Model) BeforeCreate(*gorm.DB) error {
	d.CreatedAt = Now()
	d.UpdatedAt = Now()
	return nil
}

func (d *Model) BeforeUpdate(*gorm.DB) error {
	d.UpdatedAt = Now()
	return nil
}

// NewModelWithStrID 新建模型
func NewModelWithStrID(id string) ModelWithStrID {
	return ModelWithStrID{ID: id, CreatedAt: Now(), UpdatedAt: Now()}
}

// DeletedModel 删除模型
type DeletedModel struct {
	Model
	DeletedAt gorm.DeletedAt `gorm:"index;comment:删除时间" json:"-"`
}

func (d *DeletedModel) BeforeCreate(*gorm.DB) error {
	d.CreatedAt = Now()
	d.UpdatedAt = Now()
	return nil
}

func (d *DeletedModel) BeforeUpdate(*gorm.DB) error {
	d.UpdatedAt = Now()
	return nil
}

type DeletedAt = gorm.DeletedAt

type Time struct {
	time.Time
}

var _ json.Unmarshaler = &Time{}

// UnmarshalJSON implements json.Unmarshaler.
func (t *Time) UnmarshalJSON(b []byte) error {
	l := len(b)

	s := unsafe.String(unsafe.SliceData(b), l)
	if v, err := strconv.Atoi(s); err == nil {
		if l == 10 {
			*t = Time{time.Unix(int64(v), 0)}
		} else if l == 13 {
			*t = Time{time.UnixMilli(int64(v))}
		} else if l == 1 { // 没有判断 v==0，而判断长度为 1，是想获得更多的兼容性
			*t = Time{}
		} else {
			return json.Unmarshal(b, &t.Time)
		}
		return nil
	}

	str := strings.Trim(s, `"`)
	if str == "" {
		*t = Time{}
		return nil
	}

	date, err := time.ParseInLocation(time.DateTime, str, time.Local)
	if err == nil {
		t.Time = date
		return nil
	}
	return json.Unmarshal(b, &t.Time)
}

func Now() Time {
	return Time{time.Now()}
}

// ParseTimeToLayout 解析字符串对应的 layout
// 仅支持 年-月-日 或 年/月/日 等这种格式
func ParseTimeToLayout(value string) string {
	var layout string
	// 拼凑日期
	if len(value) >= 7 {
		layout += fmt.Sprintf("2006%c01%c02", value[4], value[7])
	}
	// 拼凑时间
	if len(value) >= 19 {
		layout += " 15:04:05"
	}
	if len(value) > 19 {
		suffix := value[19:]
		var rear string
		for _, c := range suffix {
			if c == '.' {
				rear += "."
			} else if c == '+' || c == '-' {
				rear += "-07:00"
				break
			} else {
				rear += "9"
			}
		}
		return layout + rear
	}
	return layout
}

// Scan implements scaner
func (t *Time) Scan(input interface{}) error {
	var date time.Time
	switch value := input.(type) {
	case time.Time:
		date = value
	// 兼容 sqlite，其存储是字符串
	case string:
		layout := ParseTimeToLayout(value)
		d, err := time.Parse(layout, value)
		if err != nil {
			return fmt.Errorf("pkg: can not convert %v to timestamptz layout[%s]", input, layout)
		}
		date = d
	default:
		return fmt.Errorf("pkg: can not convert %v to timestamptz", input)
	}
	*t = Time{Time: date}
	return nil
}

func (t Time) MarshalJSON() ([]byte, error) {
	return []byte(`"` + t.Format(time.DateTime) + `"`), nil
}

func (t Time) Value() (driver.Value, error) {
	if t.Time.IsZero() {
		return nil, nil
	}
	return t.Time, nil
}

// Tabler 模型需要用指针接收器实现接口
type Tabler interface {
	TableName() string
}

// JsonUnmarshal 将 input 反序列化到 obj 上
func JsonUnmarshal(input, obj any) error {
	if v, ok := input.([]byte); ok {
		return json.Unmarshal(v, obj)
	}
	if v, ok := input.(string); ok {
		return json.Unmarshal([]byte(v), obj)
	}
	return nil
}
