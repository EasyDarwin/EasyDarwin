package livestream

import (
	"easydarwin/internal/gutils"
	"easydarwin/utils/pkg/web"
	"fmt"
)

// Storer 依赖反转的数据持久化接口
type Storer interface {
	Create(*LiveStream) error                             // 创建
	Delete(id int) error                                  // 删除
	Find(v *[]*LiveStream, in PagerFilter) (int64, error) // 查询列表
	FindAll(v *[]LiveStream) (int64, error)               // 查询全部列表
	FindPushAll(v *[]LiveStream) (int64, error)           // 查询全部列表
	GetByID(v *LiveStream, id int) error                  // 查询单条
	Update(v *LiveStream, id int) error                   // 更新
	UpdateInt(id int, k string, v int) error              // 更新单个字段
	UpdateString(id int, k string, v string) error        // 更新单个字段
	UpdateSnap(id int, raw, jpg string) error             // 更新快照
	UpdateOnlineAll(v int) error
}

// Core 业务对象
type Core struct {
	Storer Storer
}

// NewCore 创建业务对象
func NewCore(store Storer) *Core {
	return &Core{
		Storer: store,
	}
}

func (c Core) CreateLiveStream(input LiveInput) (LiveStream, error) {
	var live LiveStream
	live.LiveType = LIVE_PULL
	live.Name = input.Name
	live.Url = input.Url
	live.Audio = input.Audio
	live.TransType = input.TransType
	live.OnDemand = input.OnDemand
	live.Enable = input.Enable
	live.IsLive = input.IsLive
	live.SpeedEnum = input.SpeedEnum
	err := c.Storer.Create(&live)
	if err != nil {
		return live, err
	}
	return live, nil
}
func (c Core) DeleteLiveStream(id int) error {
	return c.Storer.Delete(id)
}
func (c Core) UpdateLiveStream(input LiveInput, id int) error {
	var live LiveStream
	if err := c.Storer.GetByID(&live, id); err != nil {
		return web.ErrUsedLogic.Msg("直播不存在").With(err.Error(), fmt.Sprintf(`UpdateLiveStream c.Storer.GetByID(&l, %d)`, id))
	}
	live.Name = input.Name
	live.ID = id
	live.Url = input.Url
	live.TransType = input.TransType
	live.Audio = input.Audio
	live.OnDemand = input.OnDemand
	live.Enable = input.Enable
	live.IsLive = input.IsLive
	live.SpeedEnum = input.SpeedEnum
	return c.Storer.Update(&live, id)
}
func (c Core) GetLiveStreamByID(id int) error {
	var l LiveStream
	if err := c.Storer.GetByID(&l, id); err != nil {
		return web.ErrUsedLogic.Msg("直播不存在").With(err.Error(), fmt.Sprintf(`c.Storer.GetByID(&l, %d)`, id))
	}
	return nil
}
func (c Core) FindInfoLiveStream(id int) (LiveStream, error) {
	var l LiveStream
	err := c.Storer.GetByID(&l, id)
	// 如果查询出错，返回错误信息
	if err != nil {
		return l, web.ErrDB.Withf("err[%s] := c.Storer.GetByID(&l, id)", err)
	}
	return l, err
}
func (c Core) FindLiveStream(in PagerFilter) ([]*LiveStream, int64, error) {
	lives := make([]*LiveStream, 0, in.Limit())
	total, err := c.Storer.Find(&lives, in)
	// 如果查询出错，返回错误信息
	if err != nil {
		return nil, 0, web.ErrDB.Withf("total, err[%s] := c.Storer.Find(&lives, limit, offset)", err)
	}
	return lives, total, err
}
func (c Core) FindLiveStreamALl() ([]LiveStream, int64, error) {
	lives := make([]LiveStream, 0)
	total, err := c.Storer.FindAll(&lives)
	// 如果查询出错，返回错误信息
	if err != nil {
		return nil, 0, web.ErrDB.Withf("total, err[%s] := c.Storer.FindAll(&lives)", err)
	}
	return lives, total, err
}
func (c Core) FindLiveStreamPushALl() ([]LiveStream, int64, error) {
	lives := make([]LiveStream, 0)
	total, err := c.Storer.FindPushAll(&lives)
	// 如果查询出错，返回错误信息
	if err != nil {
		return nil, 0, web.ErrDB.Withf("total, err[%s] := c.Storer.FindPushAll(&lives)", err)
	}
	return lives, total, err
}
func (c Core) UpdateLiveStreamInt(id int, key string, value int) error {
	if err := c.Storer.UpdateInt(id, key, value); err != nil {
		return web.ErrUsedLogic.Msg("更新状态失败").With(err.Error(), fmt.Sprintf(`c.Storer.UpdateInt(%d, "online", %d)`, id, value))
	}
	return nil
}
func (c Core) UpdateLiveStreamString(id int, key string, value string) error {
	if err := c.Storer.UpdateString(id, key, value); err != nil {
		return web.ErrUsedLogic.Msg("更新状态失败").With(err.Error(), fmt.Sprintf(`c.Storer.UpdateString(%d, "online", %s)`, id, value))
	}
	return nil
}
func (c Core) UpdateLiveStreamSnap(id int, raw, jpg string) error {
	if err := c.Storer.UpdateSnap(id, raw, jpg); err != nil {
		return web.ErrUsedLogic.Msg("更新快照失败").With(err.Error(), fmt.Sprintf(`c.Storer.UpdateLiveStreamSnap(%d, "%s", %s)`, id, raw, jpg))
	}
	return nil
}
func (c Core) UpdateOnlineAll(id int) error {
	if err := c.Storer.UpdateOnlineAll(id); err != nil {
		return web.ErrUsedLogic.Msg("更新快照失败").With(err.Error(), fmt.Sprintf(`c.Storer.UpdateOnlineAll(%d)`, id))
	}
	return nil
}

func (c Core) CreatePushStream(input PushInput) (LiveStream, error) {
	var live LiveStream
	live.LiveType = LIVE_PUSH
	live.Name = input.Name
	live.Enable = input.Enable
	live.Authed = input.Authed
	live.OnDemand = input.OnDemand
	live.Sign = gutils.GenerateRandomString(10)
	err := c.Storer.Create(&live)
	if err != nil {
		return live, err
	}
	return live, nil
}
func (c Core) UpdatePushStream(input PushInput, id int) error {
	var live LiveStream
	if err := c.Storer.GetByID(&live, id); err != nil {
		return web.ErrUsedLogic.Msg("直播不存在").With(err.Error(), fmt.Sprintf(`UpdateLiveStream c.Storer.GetByID(&l, %d)`, id))
	}
	live.Name = input.Name
	live.ID = id
	live.Enable = input.Enable
	live.Authed = input.Authed
	live.OnDemand = input.OnDemand
	return c.Storer.Update(&live, id)
}
func (c Core) UpdatePush(live LiveStream, id int) error {
	return c.Storer.Update(&live, id)
}
