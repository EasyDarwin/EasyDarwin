package test

import (
	"fmt"
	"log/slog"
	"os"
	"testing"
	"time"

	"easydarwin/lnton/pkg/orm"
	"easydarwin/lnton/plugin/core/tvs"
	"easydarwin/lnton/plugin/core/tvs/store/tvdb"
	"gorm.io/driver/postgres"
)

var core tvs.Core

var db tvdb.DB

func TestMain(m *testing.M) {
	dsn := os.Getenv("TEST_DSN")
	fmt.Println(dsn)
	gdb, err := orm.New(true, postgres.Open(dsn), orm.Config{
		MaxIdleConns:    10,
		MaxOpenConns:    10,
		ConnMaxLifetime: 1,
	}, orm.NewLogger(slog.Default(), true, time.Second))
	if err != nil {
		panic(err)
	}
	db = tvdb.NewDB(gdb)
	core = tvs.NewCore(db, slog.Default())
	os.Exit(m.Run())
}

func TestNE(t *testing.T) {
	if out, _, err := core.FindWalls(); err != nil {
		panic(err)
		_ = out
	}
}
