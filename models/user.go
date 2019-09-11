package models

import (
	"github.com/jinzhu/gorm"
	"github.com/rikugun/EasyGoLib/utils"
)

type User struct {
	ID       string `structs:"id" gorm:"primary_key;type:TEXT;not null" form:"id" json:"id"`
	Username string `gorm:"type:TEXT"`
	Password string `gorm:"type:TEXT"`
	Role     string `gorm:"type:TEXT"`
	Reserve1 string `gorm:"type:TEXT"`
	Reserve2 string `gorm:"type:TEXT"`
}

func (user *User) BeforeCreate(scope *gorm.Scope) error {
	scope.SetColumn("ID", utils.ShortID())
	return nil
}
