package record

import (
	"easydarwin/lnton/pkg/web"
)

type EditDeviceInput struct {
	DeviceID  string `json:"device_id"`
	Name      string `json:"name"`
	Protocol  string `json:"protocol"`
	Action    string `json:"action"`
	UpdatedAt int64  `json:"updated_at,string"`
	Ip        string `json:"ip"`
	Port      int    `json:"port"`
	UserName  string `json:"username"`
	Password  string `json:"password"`
}

type EditRecordPlanInput struct {
	StorageID    int      `json:"storage_id,string"`
	StrategyID   int      `json:"strategy_id,string"`
	Stream       string   `json:"stream"`
	TemplateID   int      `json:"template_id,string"`
	Enabled      bool     `json:"enabled"`
	CloudEnabled bool     `json:"cloud_enabled"`
	ChannelIDs   []string `json:"channel_ids" binding:"required"`
	RMSID        string   `json:"rms_id"`
	StoreType    int      `json:"store_type"`
}

// type Cloud struct {
//	Enabled    string `json:"enabled"`
//	TemplateID int    `json:"template_id"  `
//	Stream     string `json:"stream"`
//	StorageID  int    `json:"storage_id"`
//	StrategyID int    `json:"strategy_id"`
// }

type EditStorageInput struct {
	Name     string `json:"name" binding:"required"`
	Type     string `json:"type" binding:"required"`
	EndPoint string `json:"end_point" binding:"required"`
	Bucket   string `json:"bucket" binding:"required"`
	KeyID    string `json:"key_id" binding:"required"`
	Secret   string `json:"secret" binding:"required"`
	Region   string `json:"region" binding:"required"`
}

type CreateRecordPlanInput struct {
	ID      int    `json:"id"`
	Name    string `json:"name" binding:"required"`
	Enabled bool   `json:"enabled"`
	Plans   string `json:"plans" binding:"required"`
	Days    int    `json:"days"`
}

type FindRecordTemplatesInput struct {
	web.PagerFilter
	Fields string `form:"fields"`
	Name   string `form:"name"`
}

type FindRecordStoragesInput struct {
	web.PagerFilter
	Fields string `form:"fields"`
}

type EditCloudStrategyInput struct {
	Name  string `json:"name" binding:"required"`
	Type  string `json:"type" binding:"required"`
	Value int    `json:"value" binding:"required"`
}

// FindRecordChannelInput 获取录像通道的数据模型
type FindRecordChannelInput struct {
	web.PagerFilter
}
