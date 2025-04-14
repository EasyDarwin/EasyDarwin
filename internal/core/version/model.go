package version

// Version 数据库版本记录
// 每次迁移，则创建一条记录
// 每次程序启动，则按 id 倒序获取最后一条记录，当小于硬编码的版本号时，进行数据库迁移
type Version struct {
	ID      int    `gorm:"primaryKey;" json:"id"`
	Version string // 版本
	Remark  string // 迁移说明
}

// TableName ...
func (*Version) TableName() string {
	return "versions"
}
