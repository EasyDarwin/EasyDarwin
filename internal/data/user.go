package data

import (
	"easydarwin/utils/pkg/orm"
)

type User struct {
	ID        int      `gorm:"column:id" json:"id"`                                           // id
	Name      string   `gorm:"column:name;notNull;default:''" json:"name"`                    // 昵称
	Username  string   `gorm:"column:username;notNull;default:'';unique" json:"username"`     // 登录用户名
	Password  string   `gorm:"column:password" json:"-"`                                      // 登录密码
	Role      string   `gorm:"column:role;notNull;default:''" json:"role"`                    // 角色名称
	CreatedAt orm.Time `gorm:"column:created_at;notNull;CURRENT_TIMESTAMP" json:"created_at"` // 创建时间
	Remark    string   `gorm:"column:remark;notNull;default:''" json:"remark"`                // 描述
}

func (*User) TableName() string {
	return "users"
}
