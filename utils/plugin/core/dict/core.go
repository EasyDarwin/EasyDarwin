package dict

import (
	"fmt"
	"log/slog"
	"strings"

	"easydarwin/lnton/pkg/conc"

	"easydarwin/lnton/pkg/fn"
	"easydarwin/lnton/pkg/orm"
	"easydarwin/lnton/pkg/web"
)

type TypeStorer interface {
	AddDictType(input *DictType) error                                           // 创建字典类型
	DeleteDictType(code string) error                                            // 删除字典类型和值
	UpdateTypeByID(id string, name string) error                                 // 更新类型名称
	GetDictTypeByID(b *DictType, id string) error                                // 查询字典类型
	GetDictTypeByCode(b *DictType, code string) error                            // 查询字典类型
	FindDictType(bs *[]*DictType, name string, limit, offset int) (int64, error) // 查询类型列表
}

type DataStorer interface {
	AddDictData(input *DictData) error                        // 创建字典
	DeleteDictData(id string) error                           // 删除字典
	GetDictDataByID(*DictData, string) error                  // 查询字典
	SaveDictData(*DictData) error                             // 修改字典
	FindDictData(*[]*DictData, string, string) (int64, error) // 查询字典列表
	// CountByDictData(code string, count *int64) error          // 允许删除的字典数量
	GetDictDataByType(*[]*DictData, string) error
}

type Storer interface {
	TypeStorer
	DataStorer
}

type Core struct {
	dType     *conc.Map[string, *DictDataInfo]
	typeStore TypeStorer
	dataStore DataStorer
	log       *slog.Logger
}

type DictDataInfo struct {
	Type string
	Data []*DictData
}

func NewCore(store Storer, log *slog.Logger) Core {
	var m conc.Map[string, *DictDataInfo]
	return Core{
		dType:     &m,
		typeStore: store,
		log:       log, dataStore: store,
	}
}

func (c Core) LoadDictData(dType string) error {
	out := make([]*DictData, 0, 8)
	err := c.dataStore.GetDictDataByType(&out, dType)
	if err != nil {
		return web.ErrDB.Withf(`err[%s] := c.dataStore.GetDictDataByType(&out, dType)`, err)
	}
	c.dType.Store(dType, &DictDataInfo{
		Type: dType,
		Data: out,
	})
	slog.Debug("load dict data", "type", dType, "data", out)
	return nil
}

// AddDictType 创建字典类型
func (c Core) AddDictType(input AddDictTypeInput) (*DictType, error) {
	input.Code = strings.TrimSpace(input.Code)
	if input.Code == "" {
		return nil, web.ErrBadRequest.Msg("编码不能为空")
	}
	if input.Name == "" {
		return nil, web.ErrBadRequest.Msg("名称不能为空")
	}
	now := orm.Now()
	dic := DictType{
		ID:        input.Code,
		Code:      input.Code,
		Name:      strings.TrimSpace(input.Name),
		CreatedAt: now,
		UpdatedAt: now,
	}
	if err := c.typeStore.AddDictType(&dic); orm.IsDuplicatedKey(err) {
		return nil, web.ErrBadRequest.Msg("编码已存在").Withf("code[%s]", input.Code)
	} else if err != nil {
		return nil, web.ErrDB.Withf(`err[%s] := c.store.AddDictType(&dic)`, err)
	}
	return &dic, nil
}

// DeleteDictType 删除字典类型
func (c Core) DeleteDictType(id string) error {
	var dic DictType
	if err := c.typeStore.GetDictTypeByID(&dic, id); orm.IsErrRecordNotFound(err) {
		return web.ErrBadRequest.Msg("字典不存在")
	} else if err != nil {
		return web.ErrDB.Withf(`err[%s] := c.store.GetByDictType(&dic, id[%d])`, err, id)
	}
	if dic.IsDefault {
		return web.ErrUsedLogic.Msg("系统默认参数无法删除")
	}
	code := dic.Code

	// var count int64
	// if err := c.dataStore.CountByDictData(code, &count); err != nil {
	// 	return web.ErrDB.Withf(`err[%s] := c.store.CountByDictData(code[%s],&count)`, err, code)
	// }
	if err := c.typeStore.DeleteDictType(code); err != nil {
		return web.ErrDB.Withf(`err[%s] := c.store.DeleteDictType(code[%s])`, err, code)
	}
	return nil
}

func (c Core) EditDictType(id, name string) error {
	if id == "" {
		return web.ErrBadRequest.Msg("ID不能为空")
	}
	if name == "" {
		return web.ErrBadRequest.Msg("名称不能为空")
	}
	var dic DictType
	if err := c.typeStore.GetDictTypeByID(&dic, id); orm.IsErrRecordNotFound(err) {
		return web.ErrBadRequest.Msg("字典不存在")
	} else if err != nil {
		return web.ErrDB.Withf(`err[%s] := c.store.GetByDictType(&dic, id[%s])`, err, id)
	}
	if dic.IsDefault {
		return web.ErrUsedLogic.Msg("系统默认参数无法修改")
	}
	if err := c.typeStore.UpdateTypeByID(id, name); err != nil {
		return web.ErrDB.Withf(`err[%s] := c.store.UpdateTypeByID(id[%s],name[%s])`, err, id, name)
	}
	return nil
}

func (c Core) FindDictType(in FindDictTypeInput) ([]*DictType, int64, error) {
	out := make([]*DictType, 0, in.Limit())
	total, err := c.typeStore.FindDictType(&out, in.Name, in.Limit(), in.Offset())
	if err != nil {
		return nil, 0, web.ErrDB.Withf(`err[%s] := c.store.FindDictType(&list, input)`, err)
	}
	return out, total, nil
}

func (c Core) AddDictData(in CreateDictDataInput) (*DictData, error) {
	in.Value = strings.TrimSpace(in.Value)
	now := orm.Now()
	dic := DictData{
		Label:     in.Label,
		Remark:    in.Remark,
		Value:     in.Value,
		Enabled:   in.Enabled,
		CreatedAt: now,
		UpdatedAt: now,
		Code:      in.Code,
		// Flag:      input.Flag,
		Sort: in.Sort,
		ID:   fmt.Sprintf("%s_%s", in.Code, in.Value),
	}

	if err := c.checkDictData(&dic); err != nil {
		return nil, err
	}
	if err := c.dataStore.AddDictData(&dic); orm.IsDuplicatedKey(err) {
		return nil, web.ErrAccountDisabled.Msg("字典重复").Withf("code[%s] label[%s] value[%s]", dic.Code, dic.Label, dic.Value)
	} else if err != nil {
		return nil, web.ErrDB.Withf(`err[%s] := c.dataStore.CreateDictData(&dic)`, err)
	}
	if err := c.LoadDictData(in.Code); err != nil {
		return nil, web.ErrDB.Withf(`err[%s] := c.LoadDictData(in.Code)`, err)
	}
	return &dic, nil
}

func (c Core) DeleteDictData(id string) error {
	var dic DictData
	if err := c.dataStore.GetDictDataByID(&dic, id); orm.IsErrRecordNotFound(err) {
		return web.ErrNotFound.Msg("字典不存在").Withf("id[%d]", id)
	} else if err != nil {
		return web.ErrDB.Withf(`err[%s] := c.dataStore.GetDictDataByID(&dic, id[%d])`, err, id)
	}
	if dic.IsDefault {
		return web.ErrUsedLogic.Msg("系统字典不可删除")
	}
	if err := c.dataStore.DeleteDictData(id); err != nil {
		return web.ErrDB.Withf(`err[%s] := c.dataStore.DeleteDictData(id[%d])`, err, id)
	}
	if err := c.LoadDictData(dic.Code); err != nil {
		return web.ErrDB.Withf(`err[%s] := c.LoadDictData(in.Code)`, err)
	}
	return nil
}

func (c Core) checkDictData(dic *DictData) error {
	if dic.Label == "" {
		return web.ErrBadRequest.Msg("标签不能为空")
	}
	if dic.Value == "" {
		return web.ErrBadRequest.Msg("值不能为空")
	}
	if dic.Code == "" {
		return web.ErrBadRequest.Msg("编码不能为空")
	}
	// 检查 code 是否存在
	var typ DictType
	code := dic.Code
	if err := c.typeStore.GetDictTypeByCode(&typ, code); orm.IsErrRecordNotFound(err) {
		return web.ErrBadRequest.Msg("编码不存在").Withf(`err[%s] code[%s]`, err, code)
	} else if err != nil {
		return web.ErrDB.Withf(`err[%s] := c.typeStore.GetDictTypeByCode(&typ, input.Code[%s])`, err, code)
	}
	return nil
}

func (c Core) EditDictData(input EditDictDataInput, id string) (*DictData, error) {
	input.Value = strings.TrimSpace(input.Value)
	if id == "" {
		return nil, web.ErrBadRequest.Msg("ID不能为空")
	}
	var dic DictData
	if err := c.dataStore.GetDictDataByID(&dic, id); orm.IsErrRecordNotFound(err) {
		return nil, web.ErrNotFound.Withf(`err[%s] id[%d]`, err, id)
	} else if err != nil {
		return nil, web.ErrDB.Withf(`err[%s] := c.dataStore.GetDictDataByID(&dic, id[%d])`, err, id)
	}
	if dic.IsDefault {
		return nil, web.ErrAccountDisabled.Msg("系统默认参数无法修改")
	}
	dic.Code = input.Code
	dic.UpdatedAt = orm.Now()
	dic.Enabled = input.Enabled
	dic.Flag = input.Flag
	dic.Label = input.Label
	dic.Remark = input.Remark
	dic.Sort = input.Sort
	dic.Value = input.Value
	if err := c.checkDictData(&dic); err != nil {
		return nil, err
	}
	if err := c.dataStore.SaveDictData(&dic); err != nil {
		return nil, web.ErrDB.Withf(`err[%s] := c.dataStore.SaveDictData(&dic)`, err)
	}
	if err := c.LoadDictData(dic.Code); err != nil {
		return nil, web.ErrDB.Withf(`err[%s] := c.LoadDictData(in.Code)`, err)
	}
	return &dic, nil
}

func (c Core) FindDictData(input FindDictDataInput) ([]*DictData, int64, error) {
	out := make([]*DictData, 0, 10)
	total, err := c.dataStore.FindDictData(&out, input.Code, input.Label)
	if err != nil {
		return nil, 0, web.ErrDB.Withf(`err[%s] := c.dataStore.FindDictData(&out,input.Code, input.Label)`, err)
	}
	out = fn.Filter[*DictData](out, func(dd *DictData) bool {
		return dd.Flag == input.Flag
	})
	return out, total, nil
}
