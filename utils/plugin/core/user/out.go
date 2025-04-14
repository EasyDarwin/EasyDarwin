package user

import "easydarwin/utils/pkg/orm"

// FindAppsOutput ...
// 定义FindAppsOutput结构体，用于存储查找应用的结果
type FindAppsOutput struct {
	ID        int      `json:"id"`         // 应用ID
	AppID     string   `json:"app_id"`     // 应用ID
	IPs       []string `json:"ips"`        // 应用IP地址列表
	CreatedAt orm.Time `json:"created_at"` // 创建时间
}
