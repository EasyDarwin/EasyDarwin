package test

import (
	"fmt"
	"log/slog"
	"os"
	"testing"
	"time"

	"easydarwin/lnton/pkg/orm"
	"easydarwin/lnton/plugin/core/alarm"
	"easydarwin/lnton/plugin/core/alarm/store/alarmdb"
	"github.com/glebarez/sqlite"
)

var core alarm.Core

// var db alarmdb.DB

// func TestMain(m *testing.M) {
// 	dsn := os.Getenv("TEST_DSN")
// 	gdb, err := orm.New(true, postgres.Open(dsn), orm.Config{
// 		MaxIdleConns:    10,
// 		MaxOpenConns:    10,
// 		ConnMaxLifetime: 1,
// 	}, orm.NewLogger(slog.Default()))
// 	if err != nil {
// 		panic(err)
// 	}
// 	db = alarmdb.NewDB(gdb)
// 	core = alarm.NewCore(db, slog.Default())
// 	os.Exit(m.Run())
// }

func TestMain(m *testing.M) {
	gdb, err := orm.New(true, sqlite.Open("E:\\lnton\\cvs\\configs/home.db"),
		orm.Config{
			MaxIdleConns:    1,
			MaxOpenConns:    1,
			SlowThreshold:   time.Second,
			ConnMaxLifetime: time.Minute,
		}, orm.NewLogger(slog.Default(), true, time.Second))
	gdb.Exec("PRAGMA timezone = 'Asia/Shanghai';")

	// gdb, err := orm.New(true, postgres.New(postgres.Config{
	// 	DriverName: "pgx",
	// 	DSN:        os.Getenv("TEST_DSN"),
	// }), orm.Config{
	// 	MaxIdleConns:    1,
	// 	MaxOpenConns:    1,
	// 	SlowThreshold:   time.Second,
	// 	ConnMaxLifetime: time.Minute,
	// }, orm.NewLogger(slog.Default()))
	if err != nil {
		panic(err)
	}
	core = alarm.NewCore(alarmdb.NewDB(gdb).AutoMerge(true), nil, nil, nil)
	os.Exit(m.Run())
}

func TestAlarmInfoByDeviceID(t *testing.T) {
	alarmInfo, err := core.FindAlarmInfoByDeviceID(alarm.FindAlarmInfoInput{})
	if err != nil {
		panic(err)
	}

	fmt.Println("查找结果:")
	for _, v := range alarmInfo {
		fmt.Println(v)
	}
}

func TestAlarmInfo(t *testing.T) {
	alarmInfo, err := core.FindAlarmInfo(alarm.FindAlarmInfoInput{})
	if err != nil {
		panic(err)
	}

	fmt.Println("查找结果:")
	fmt.Printf("%#v", alarmInfo)
}
