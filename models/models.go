package models

import (
	"github.com/rikugun/EasyDarwin/db"
	"github.com/rikugun/EasyDarwin/utils"
)

func Init() (err error) {
	err = db.Init()
	if err != nil {
		return
	}
	db.DB.AutoMigrate(User{}, Stream{})
	count := 0
	sec := utils.Conf().Section("http")
	defUser := sec.Key("default_username").MustString("admin")
	defPass := sec.Key("default_password").MustString("admin")
	db.DB.Model(User{}).Where("username = ?", defUser).Count(&count)
	if count == 0 {
		db.DB.Create(&User{
			Username: defUser,
			Password: utils.MD5(defPass),
		})
	}
	return
}

func Close() {
	db.Close()
}
