package alarmdb

import (
	"easydarwin/lnton/pkg/orm"
	"easydarwin/lnton/plugin/core/alarm"
	"gorm.io/gorm"
	"gorm.io/gorm/clause"
)

// AddAlarmPlan 添加告警计划
func (d DB) AddAlarmPlan(in *alarm.AlarmPlan) error {
	return d.db.Clauses(clause.Returning{}).Create(in).Error
}

func (d DB) DelAlarmPlan(ap *alarm.AlarmPlan, id string) error {
	return d.db.Clauses(clause.Returning{}).Where("id = ?", id).Delete(ap).Error
}

func (d DB) UpdateAlarmPlanWithLock(ap *alarm.AlarmPlan, id string, fn func(ap *alarm.AlarmPlan)) error {
	return d.db.Transaction(func(tx *gorm.DB) error {
		if err := tx.Clauses(clause.Locking{Strength: "UPDATE"}).Where("id = ?", id).First(ap).Error; err != nil {
			return err
		}
		fn(ap)
		return tx.Save(ap).Error
	})
}

func (d DB) FindAlarmPlans(ap *[]*alarm.AlarmPlan, in *alarm.FindAlarmPlanInput) (int64, error) {
	db := d.db.Model(&alarm.AlarmPlan{})
	var total int64
	if err := db.Count(&total).Error; err != nil {
		return 0, err
	}
	if total <= 0 {
		return 0, nil
	}
	err := db.Limit(in.Limit()).Offset(in.Offset()).Order("created_at desc").Find(ap).Error
	return total, err
}

func (d DB) FindAlarmPlanByIDs(ap *[]*alarm.AlarmPlan, ids []string) error {
	return d.db.Model(&alarm.AlarmPlan{}).Where("id in (?)", ids).Find(ap).Error
}

func (d DB) GetPlan(ap *alarm.AlarmPlan, args ...orm.QueryOption) error {
	return orm.First(d.db, ap, args...)
}
