package routers

import "github.com/rikugun/EasyDarwin/utils"

var BuildVersion = "v8.1"
var BuildDateTime = ""

type PercentData struct {
	Time utils.DateTime `json:"time"`
	Used float64        `json:"使用"`
}

type DiskData struct {
	Disk  string `json:"disk"`
	Total int    `json:"total"`
	Used  int    `json:"used"`
}

type CountData struct {
	Time  utils.DateTime `json:"time"`
	Total uint           `json:"总数"`
}
