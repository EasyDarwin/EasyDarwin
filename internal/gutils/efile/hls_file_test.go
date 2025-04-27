package efile

import (
	"fmt"
	"testing"
	"time"
)

func TestGetTsTimes(t *testing.T) {
	fmt.Println(time.Now())
	maps, _ := GetTsTimes(`D:\Project\go\src\gitee.com\easydarwin\EasyDSSGo\data\record\dangshigou\20210331\dangshigou_20210331111919_20210331145329.m3u8`)
	fmt.Println(time.Now())
	fmt.Println(maps)
}