package alarm

import (
	"errors"

	"easydarwin/lnton/pkg/orm"
	"easydarwin/lnton/pkg/web"
	"gorm.io/gorm"
)

type AlarmPlanChanneler interface {
	// GetAlarmPlanCh(*PlanChannel, ...orm.QueryOption) error
	GetAlarmPlansByChID(chID string) ([]string, error)
	AddPlanChannel(pChannel []*PlanChannel) error
	DelPlanChannelByID(planID string) error

	// FindPlanChannel(*[]*PlanChannel, ...orm.QueryOption) error
	FindPlanChannelByPlanID(*[]*PlanChannel, string) error
}

func (c Core) SetPlanChannel(in *SetPlanChannelInput) error {
	// 检查报警预案是否存在
	var plan AlarmPlan
	err := c.storer.GetPlan(&plan, orm.Where("id =?", in.AlarmPlanID))
	if errors.Is(err, gorm.ErrRecordNotFound) {
		return web.ErrBadRequest.Msg("报警预案不存在")
	}

	// 构造记录
	pChannel := make([]*PlanChannel, 0, len(in.ChannelIDs))
	for _, channelID := range in.ChannelIDs {
		// ToDo:校验通道ID是否合法
		pChannel = append(pChannel, &PlanChannel{
			PlanID:    in.AlarmPlanID,
			ChannelID: channelID,
		})
	}
	if len(pChannel) == 0 {
		return web.ErrBadRequest.Withf("channelIDs is empty")
	}
	// 删除原由通道
	if err := c.storer.DelPlanChannelByID(in.AlarmPlanID); err != nil {
		return web.ErrDB.Withf("err[%s] := c.storer.DelPlanChannelByID[%s]", err, in.AlarmPlanID)
	}
	// 设置新的通道
	if err := c.storer.AddPlanChannel(pChannel); err != nil {
		return web.ErrDB.Withf("err[%s] := c.storer.AddPlanChannel[%s]", err, in.AlarmPlanID)
	}
	return nil
}

func (c Core) GetPlanChannel(planID string) ([]*PlanChannel, error) {
	pChannel := make([]*PlanChannel, 0, 8)
	if err := c.storer.FindPlanChannelByPlanID(&pChannel, planID); err != nil {
		return nil, web.ErrDB.Withf("err[%s] := c.storer.FindPlanChannelByPlanID[%s]", err, planID)
	}
	return pChannel, nil
}
