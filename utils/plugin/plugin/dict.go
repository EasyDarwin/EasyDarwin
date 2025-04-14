package plugin

import (
	"easydarwin/lnton/pkg/web"
	"easydarwin/lnton/plugin/core/dict"
	"github.com/gin-gonic/gin"
)

// RegisterDict 注册字典
func RegisterDict(g gin.IRouter, dict dict.Core, hf ...gin.HandlerFunc) {
	dic := Dict{core: dict}
	t := g.Group("/dicts/types", hf...)
	t.POST("", dic.createDictType)
	t.PUT("/:id", dic.editDictType)
	t.GET("", dic.findDictType)
	t.DELETE("/:id", dic.deleteDictType)

	d := g.Group("/dicts/datas", hf...)
	d.POST("", dic.CreateDictData)
	d.GET("", dic.FindDictData)
	d.PUT("/:id", dic.EditDictData)
	d.DELETE("/:id", dic.DeleteDictData)
}

// Dict 字典
type Dict struct {
	core dict.Core
}

func (d Dict) createDictType(c *gin.Context) {
	var input dict.AddDictTypeInput
	if err := c.ShouldBindJSON(&input); err != nil {
		web.Fail(c, web.ErrBadRequest.With(web.HanddleJSONErr(err).Error()))
		return
	}
	out, err := d.core.AddDictType(input)
	if err != nil {
		web.Fail(c, err)
		return
	}
	web.Success(c, out)
}

func (d Dict) editDictType(c *gin.Context) {
	id := c.Param("id")
	var input dict.EditDictTypeInput
	if err := c.ShouldBindJSON(&input); err != nil {
		web.Fail(c, web.ErrBadRequest.With(web.HanddleJSONErr(err).Error()))
		return
	}
	if err := d.core.EditDictType(id, input.Name); err != nil {
		web.Fail(c, err)
		return
	}
	web.Success(c, gin.H{"id": id})
}

// findDictType 查询字典类型列表
func (d Dict) findDictType(c *gin.Context) {
	var input dict.FindDictTypeInput
	if err := c.ShouldBindQuery(&input); err != nil {
		web.Fail(c, web.ErrBadRequest.With(web.HanddleJSONErr(err).Error()))
		return
	}
	output, _, err := d.core.FindDictType(input)
	if err != nil {
		web.Fail(c, err)
		return
	}
	web.Success(c, gin.H{
		"items": output,
		"total": len(output),
	})
}

func (d Dict) deleteDictType(c *gin.Context) {
	id := c.Param("id")
	if id == "" {
		web.Fail(c, web.ErrBadRequest.Msg("ID不能为空"))
		return
	}
	if err := d.core.DeleteDictType(id); err != nil {
		web.Fail(c, err)
		return
	}
	web.Success(c, gin.H{"id": id})
}

// CreateDictData 创建字典数据
func (d Dict) CreateDictData(c *gin.Context) {
	var input dict.CreateDictDataInput
	if err := c.ShouldBindJSON(&input); err != nil {
		web.Fail(c, web.ErrBadRequest.With(web.HanddleJSONErr(err).Error()))
		return
	}
	out, err := d.core.AddDictData(input)
	if err != nil {
		web.Fail(c, err)
		return
	}
	web.Success(c, out)
}

// FindDictData 查询字典数据
func (d Dict) FindDictData(c *gin.Context) {
	var input dict.FindDictDataInput
	if err := c.ShouldBindQuery(&input); err != nil {
		web.Fail(c, web.ErrBadRequest.With(web.HanddleJSONErr(err).Error()))
		return
	}
	output, total, err := d.core.FindDictData(input)
	if err != nil {
		web.Fail(c, err)
		return
	}
	web.Success(c, gin.H{
		"items": output,
		"total": total,
	})
}

// DeleteDictData 删除字典数据
func (d Dict) DeleteDictData(c *gin.Context) {
	id := c.Param("id")
	if err := d.core.DeleteDictData(id); err != nil {
		web.Fail(c, err)
		return
	}
	web.Success(c, gin.H{"id": id})
}

// EditDictData 修改字典数据
func (d Dict) EditDictData(c *gin.Context) {
	id := c.Param("id")
	var input dict.EditDictDataInput
	if err := c.ShouldBindJSON(&input); err != nil {
		web.Fail(c, web.ErrBadRequest.With(web.HanddleJSONErr(err).Error()))
		return
	}
	out, err := d.core.EditDictData(input, id)
	if err != nil {
		web.Fail(c, err)
		return
	}
	web.Success(c, out)
}
