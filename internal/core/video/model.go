// Package video
// Copyright 2025 EasyDarwin.
// http://www.easydarwin.org
// 点播文件程序
// History (ID, Time, Desc)
// (xukongzangpusa, 20250424, add)
package video

import "easydarwin/internal/gutils/etime"

// TVod 点播表
type TVod struct {
	ID                 string          `gorm:"primaryKey;" json:"id"`
	Name               string          `json:"name"`
	Size               int             `json:"size" gorm:"type:decimal"`
	Type               string          `json:"type"`
	Path               string          `json:"-"`
	RealPath           string          `json:"path" gorm:"-"`
	Folder             string          `json:"folder"`
	Status             string          `json:"status"`
	Duration           int             `json:"duration"  gorm:"type:decimal"`
	VideoCodec         string          `json:"videoCodec"`
	AudioCodec         string          `json:"audioCodec"`
	VidioCodecOriginal string          `json:"videoCodecOriginal"`
	AudioCodecOriginal string          `json:"audioCodecOriginal"`
	Aspect             string          `json:"aspect"`
	Error              string          `json:"error"`
	Shared             bool            `json:"shared" gorm:"default:0"`                               // 分享开关
	ShareBeginTime     *etime.DateTime `json:"shareBeginTime" gorm:"type:datetime;currentDate=null;"` // 分享有效期开始时间
	ShareEndTime       *etime.DateTime `json:"shareEndTime" gorm:"type:datetime;currentDate=null;"`   // 分享有效期结束时间
	Rotate             int             `json:"rotate"`
	Resolution         string          `json:"resolution"`
	IsResolution       bool            `json:"isresolution" gorm:"-"`
	ResolutionDefault  string          `json:"resolutiondefault" gorm:"-"`
	TransVideo         bool            `json:"transvideo" gorm:"default:0"`
}

// 表名称
func (TVod) TableName() string {
	return "video"
}

// 判断 video 是否处于分享状态
func (vod *TVod) IsSharing() bool {
	if vod.Shared {
		return etime.TimeIsBetween(vod.ShareBeginTime, vod.ShareEndTime)
	} else {
		return false
	}
}

// TVodStore 视频点播存储目录
type TVodStore struct {
	ID       string `gorm:"primaryKey;" json:"id"`
	Folder   string `json:"folder"`
	Name     string `json:"name"`
	Desc     string `json:"desc" gorm:"type:text"`
	RealPath string `json:"realPath"`
	Sort     int    `json:"sort" gorm:"default:0"`
}

func (TVodStore) TableName() string {
	return "vod_store"
}

type VodView struct {
	TVod
	SnapURL    string `json:"snapUrl"`
	VideoURL   string `json:"videoUrl"`
	SharedLink string `json:"sharedLink"`
	FlowNum    int64  `json:"flowNum"`
	Progress   int    `json:"progress"`
	PlayNum    int64  `json:"playNum"`
}
