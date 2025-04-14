package alarm

import (
	"easydarwin/lnton/pkg/orm"
	"easydarwin/lnton/pkg/web"
)

type Planer interface {
	AddAlarmPlan(input *AlarmPlan) error
	UpdateAlarmPlanWithLock(ap *AlarmPlan, id string, fn func(ap *AlarmPlan)) error
	DelAlarmPlan(ap *AlarmPlan, id string) error
	FindAlarmPlans(ap *[]*AlarmPlan, in *FindAlarmPlanInput) (int64, error)

	FindAlarmPlanByIDs(ap *[]*AlarmPlan, ids []string) error
	GetPlan(*AlarmPlan, ...orm.QueryOption) error
}

// todo:ID没有自增，删除数据没有返回信息，未测试完成 2024/08/05 king
func (c *Core) AddAlarmPlan(in *AddAlarmPlanInput) (*AlarmPlan, error) {
	ap := AlarmPlan{
		Name:           in.Name,
		SnapInterval:   in.SnapInterval,
		RecordDuration: in.RecordDuration,
		Priority:       in.Priority,
		Method:         in.Method,
		Type:           in.Type,
		EventType:      in.EventType,
		Enabled:        in.Enable,
	}
	ap.ID = orm.GenerateRandomString(10)
	err := c.storer.AddAlarmPlan(&ap)
	if err != nil {
		return nil, err
	}
	return &ap, nil
}

func (c *Core) EditAlarmPlan(in *UpdateAlarmPlanInput) (*AlarmPlan, error) {
	var ap AlarmPlan
	if err := c.storer.UpdateAlarmPlanWithLock(&ap, in.ID, func(a *AlarmPlan) {
		a.ID = in.ID
		a.Name = in.Name
		a.RecordDuration = in.RecordDuration
		a.Priority = in.Priority
		a.Method = in.Method
		a.Type = in.Type
		a.EventType = in.EventType
		a.Enabled = in.Enabled
		a.SnapInterval = in.SnapInterval
	}); err != nil {
		return nil, web.ErrDB.Withf(`err[%s] := c.storer.UpdateAlarmPlanWithLock(&ap, in.ID[%s], func(a *AlarmPlan)`, err, in.ID)
	}
	return &ap, nil
}

// DeleteAlarmPlan 删除报警预案
func (c *Core) DeleteAlarmPlan(id string) (*AlarmPlan, error) {
	var ap AlarmPlan
	// 删除关联的通道
	err := c.storer.DelPlanChannelByID(id)
	if err != nil {
		return nil, web.ErrDB.Withf(`err[%s] := c.storer.DelPlanChannelByID(id)`, err)
	}
	// 删除预案
	err = c.storer.DelAlarmPlan(&ap, id)
	if err != nil {
		return nil, web.ErrDB.Withf(`err[%s] := c.storer.DelAlarmPlan(&ap, id)`, err)
	}
	return &ap, nil
}

func (c *Core) FindAlarmPlan(in *FindAlarmPlanInput) ([]*AlarmPlan, int64, error) {
	out := make([]*AlarmPlan, 0, in.Limit())
	total, err := c.storer.FindAlarmPlans(&out, in)
	if err != nil {
		return nil, 0, web.ErrDB.Withf(`err[%s] := c.storer.FindAlarmPlans(&out, in)`, err)
	}
	return out, total, nil
}
