package tvs

// GroupdTVs 分组后的数据
type GroupedTVs struct {
	ID       int    `gorm:"primaryKey;" json:"id"`
	Name     string `json:"name"`
	Channels string `json:"channels"`
}

// TableName 返回表名
func (*GroupedTVs) TableName() string {
	return "grouped_tvs"
}
