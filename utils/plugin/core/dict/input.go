package dict

import "easydarwin/lnton/pkg/web"

// AddDictTypeInput 创建字典类型
type AddDictTypeInput struct {
	Code string `json:"code"`
	Name string `json:"name"`
}

// EditDictTypeInput 修改字典类型
type EditDictTypeInput struct {
	Name string `json:"name"`
}

// FindDictTypeInput 查询字典类型
type FindDictTypeInput struct {
	Name string `form:"name"`
	web.PagerFilter
}

// CreateDictDataInput 创建字典数据
type CreateDictDataInput struct {
	Code    string `json:"code"`
	Label   string `json:"label"`
	Value   string `json:"value"`
	Sort    int    `json:"sort"`
	Enabled bool   `json:"enabled"`
	Remark  string `json:"remark"`
	Flag    string `json:"flag"`
}

// EditDictDataInput 修改字典数据
type EditDictDataInput CreateDictDataInput

// FindDictDataInput 查询字典
type FindDictDataInput struct {
	Code  string `form:"code"`  // 字典编码
	Flag  string `form:"flag"`  // 字典标识
	Label string `form:"label"` // 标签模糊搜索
}
