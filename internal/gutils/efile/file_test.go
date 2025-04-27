// Copyright 2020 TSINGSEE.
// http://www.tsingsee.com
// 测试文件
// Creat By Sam
// History (Name, Time, Desc)
// (Sam, 20203025, 增加注释)
package efile

import (
	"encoding/json"
	"fmt"
	"io/ioutil"
	"log"
	"os"
	"strings"
	"testing"
	"time"
)

func TestGetDirNames(t *testing.T) {
	list := GetDirNames("D:/share/data/record/123/")
	log.Println(list)
}

func TestReadFile(t *testing.T) {
	ReadFile(`D:\07Go\GOPATH\src\gitee.com\easydarwin\EasyDSSGo\easydss\www\hls\teet\20180901\20180911101130\teet_record.m3u8`)
}

// 测试是否可以删除含文件夹的目录
func TestDeleteFiles(t *testing.T) {
	fmt.Println(time.Now())
	os.RemoveAll(`D:\Project\go\src\gitee.com\easydarwin\EasyDSSGo\web_src\node_modules\`)
	fmt.Println(time.Now())
}

func TestGetImportFiles(t *testing.T) {
	_, importD := GetDirNamesWithoutImportant("123", "D:/share/data/record/123/20200312/")
	fmt.Println(importD)
}

func TestDir_AddChild(t *testing.T) {
	str := "adasds/dasd"
	substr := "/"
	index := strings.Index(str, substr)
	restDir := ""

	if index == -1 {
		fmt.Println(str)
		fmt.Println(restDir)
	} else {
		fmt.Println(str[0:index])
		fmt.Println(str[index+1:])
	}
}

func TestDir_AddChild2(t *testing.T) {
	root := Dir{
		Label:    "root",
		Path:     "root",
		Children: make([]*Dir, 0, 10),
	}

	root.AddChild("1/22/3/4/5")
	root.AddChild("1/2/3/4/5")
	root.AddChild("2/3/4/5")
	root.AddChild("3/4/5")
	root.AddChild("5/6/7")
	root.AddChild("5/6")
	root.AddChild("18")
	root.Sort()

	if b, err := json.MarshalIndent(root, "", "    "); err == nil {
		fmt.Println(string(b))
	}
}

func TestDir_AddChild3(t *testing.T) {
	root := NDir{
		Label:    "root",
		Path:     "root",
		Children: make([]NDir, 0, 10),
	}

	root = AddChild(root, "1/22/3/4/5")
	root = AddChild(root, "1/2/3/4/5")
	root = AddChild(root, "2/3/4/5")
	root = AddChild(root, "3/4/5")
	root = AddChild(root, "5/6/7")
	root = AddChild(root, "1/22/3s/4/5")

	if b, err := json.MarshalIndent(root, "", "    "); err == nil {
		fmt.Println(string(b))
	}
}

func TestCreateNdir(t *testing.T) {
	ndir := CreateNdir("root", "as/fgh/hg/d/c")
	if b, err := json.MarshalIndent(ndir, "", "    "); err == nil {
		fmt.Println(string(b))
	}
}

// 获取 TS 文件的信息
func TestGetTSInfo(t *testing.T) {
	file, err := os.Open("C:/Users/Administrator/Desktop/正在进行/交控/record/04661E1A-6D76-4898-B908-CDA591B162B6/20200229/20200229000447.ts") // For read access.
	//file, err := os.Open("C:/Users/Administrator/Desktop/正在进行/交控/record/04661E1A-6D76-4898-B908-CDA591B162B6/20200229")
	if err != nil {
		log.Fatal(err)
	}
	defer file.Close()

	info, _ := file.Stat()
	fmt.Println("name =", info.Name())
	fmt.Println("size =", info.Size())
	fmt.Println("mode =", info.Mode())
	fmt.Println("modtime =", info.ModTime())
	fmt.Println("isDir =", info.IsDir())
	fmt.Println("sys =", info.Sys())

	/*Atim := reflect.ValueOf(info.Sys()).Elem().FieldByName("Atim").Field(0).Int()
	println("文件的访问时间：\n", Atim, )*/
}

// 测试写入文件
func TestWriteFile(t *testing.T) {
	ioutil.WriteFile("d:/test.txt", []byte("zhangqi"), 0644)
}

// 测试
func TestDirNames(t *testing.T) {
	//dir := "c:/asd/dfdg\\asfdas/"
	dir := "c:/asd/dfdg\\asfdas"

	if !strings.HasSuffix(dir, "/") && !strings.HasSuffix(dir, "\\") {
		dir = dir + string(os.PathSeparator)
	}
	fmt.Println(dir)
}

// 测试生成 m3u8 文件
func TestGenerateM3U8(t *testing.T) {
	GenerateM3U8("D:/Project/go/src/gitee.com/easydarwin/EasyDSSGo/data/vod/20200204")
}

// 获取 TS 文件的时间
func Test_getTSfileTime(t *testing.T) {
	getTSfileTime("D:/Develop/ffmpeg/bin/20200204234654.ts")
}