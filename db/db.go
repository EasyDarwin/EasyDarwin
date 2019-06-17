package db

import (
	"log"

	"github.com/jinzhu/gorm"
	//_ "github.com/jinzhu/gorm/dialects/sqlite"
	_ "github.com/jinzhu/gorm/dialects/mysql"
	"github.com/rikugun/EasyDarwin/utils"
)

type Model struct {
	ID        string         `structs:"id" gorm:"primary_key" form:"id" json:"id"`
	CreatedAt utils.DateTime `structs:"-" json:"createdAt" gorm:"type:datetime"`
	UpdatedAt utils.DateTime `structs:"-" json:"updatedAt" gorm:"type:datetime"`
	// DeletedAt *time.Time `sql:"index" structs:"-"`
}

var DB *gorm.DB

func Init() (err error) {
	gorm.DefaultTableNameHandler = func(db *gorm.DB, defaultTablename string) string {
		return "t_" + defaultTablename
	}
	//dbFile := utils.DBFile()
	//log.Println("db file -->", utils.DBFile())
	log.Println("db conStr -->", utils.MysqlConnStr())
	//DB, err = gorm.Open("sqlite3", fmt.Sprintf("%s?loc=Asia/Shanghai", dbFile))
	DB, err = gorm.Open("mysql", utils.MysqlConnStr())
	if err != nil {
		return
	}
	// Sqlite cannot handle concurrent writes, so we limit sqlite to one connection.
	// see https://github.com/mattn/go-sqlite3/issues/274
	//DB.DB().SetMaxOpenConns(1)
	DB.SetLogger(DefaultGormLogger)
	DB.LogMode(false)
	return
}

func Close() {
	if DB != nil {
		DB.Close()
		DB = nil
	}
}
