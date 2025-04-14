package recorddb

import (
	"fmt"
	"testing"

	"easydarwin/lnton/rms/core/record"
	"gorm.io/driver/postgres"
	"gorm.io/gorm"
)

func TestDelCloudStrategy2(t *testing.T) {
	dsn := "postgres://postgres:123456789@localhost:1557/saida?sslmode=disable"
	db, err := gorm.Open(postgres.Open(dsn), &gorm.Config{})
	if err != nil {
		panic(err)
	}
	d := NewDB(db)
	out := make([]*record.RecordPlanWithBID, 0, 1)
	if err := d.FindRecordPlan(&out, "rms"); err != nil {
		t.Fatal(err)
	}
	for _, v := range out {
		fmt.Printf("%+v\n", v)
	}
}

func TestDelCloudStrategy(t *testing.T) {
	dsn := "postgres://postgres:7418AD28BBF54196@212.64.34.165:20001/saida?sslmode=disable"
	db, err := gorm.Open(postgres.Open(dsn), &gorm.Config{})
	if err != nil {
		panic(err)
	}
	d := NewDB(db)
	var result record.CloudStrategy
	err = d.DelCloudStrategy(&result, 0)
	if err == record.ErrUsingNotDelete {
		fmt.Println("被引用,err:", err)
	}
	if err != nil {
		fmt.Println("删除错误:", err)
	}
}

func TestDeleteCouldStorege(t *testing.T) {
	dsn := "postgres://postgres:7418AD28BBF54196@212.64.34.165:20001/saida?sslmode=disable"
	db, err := gorm.Open(postgres.Open(dsn), &gorm.Config{})
	if err != nil {
		panic(err)
	}
	d := NewDB(db)
	// var result record.CloudStrategy
	err = d.DeleteCouldStorege(5, func() error { return nil })
	if err == record.ErrUsingNotDelete {
		fmt.Println("被引用,err:", err)
	}
	if err != nil {
		fmt.Println("删除错误:", err)
	}
}

func TestDeleteRecordTemplates(t *testing.T) {
	dsn := "postgres://postgres:7418AD28BBF54196@212.64.34.165:20001/saida?sslmode=disable"
	db, err := gorm.Open(postgres.Open(dsn), &gorm.Config{})
	if err != nil {
		panic(err)
	}
	d := NewDB(db)
	var result record.RecordPlan
	err = d.DeleteRecordTemplates(&result, 34)
	if err == record.ErrUsingNotDelete {
		fmt.Println("被引用,err:", err)
	}
	if err != nil {
		fmt.Println("删除错误:", err)
	}
}

func TestEditCouldStorage(t *testing.T) {
	dsn := "postgres://postgres:7418AD28BBF54196@212.64.34.165:20001/saida?sslmode=disable"
	db, err := gorm.Open(postgres.Open(dsn), &gorm.Config{})
	if err != nil {
		panic(err)
	}
	d := NewDB(db)
	err = d.EditCouldStorage(&record.CloudStorage{
		ID:   43,
		Name: "123",
	})
	if err == record.ErrUsingNotDelete {
		fmt.Println("被引用,err:", err)
	}
	if err != nil {
		fmt.Println("删除错误:", err)
	}
}
