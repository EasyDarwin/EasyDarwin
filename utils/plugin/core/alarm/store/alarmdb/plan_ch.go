package alarmdb

import (
	"easydarwin/lnton/pkg/orm"
	"easydarwin/lnton/plugin/core/alarm"
)

// GetAlarmPlanCh
func (d DB) GetAlarmPlanCh(in *[]*alarm.PlanChannel, args ...orm.QueryOption) error {
	return orm.First(d.db, in, args...)
}

func (d DB) GetAlarmPlansByChID(chID string) ([]string, error) {
	var planIDs []string
	err := d.db.Model(&alarm.PlanChannel{}).Select("plan_id").Where("channel_id = ?", chID).Find(&planIDs).Error
	if err != nil {
		return nil, err
	}
	return planIDs, nil
}

func (d DB) DelPlanChannelByID(planID string) error {
	return d.db.Where("plan_id = ?", planID).Delete(&alarm.PlanChannel{}).Error
}

func (d DB) AddPlanChannel(pChannel []*alarm.PlanChannel) error {
	return d.db.Model(&alarm.PlanChannel{}).Create(&pChannel).Error
}

//func (d DB) FindPlanChannel(pChannel *[]*alarm.PlanChannel, args ...orm.QueryOption) error {
//	return orm.Find(d.db, pChannel, args)
//}

func (d DB) FindPlanChannelByPlanID(pChannel *[]*alarm.PlanChannel, planID string) error {
	return d.db.Model(&alarm.PlanChannel{}).Where("plan_id = ?", planID).Find(&pChannel).Error
}
