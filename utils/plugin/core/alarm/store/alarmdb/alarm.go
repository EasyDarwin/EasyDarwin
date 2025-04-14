package alarmdb

import (
	"time"

	"easydarwin/lnton/plugin/core/alarm"
	"gorm.io/gorm"
	"gorm.io/gorm/clause"
)

// AddAlarm 实现了alarm.Storer
func (d DB) AddAlarm(in *alarm.Alarm) error {
	return d.db.Create(in).Error
}

// FindAlarms 实现了 alarm.Storer
func (d DB) FindAlarms(bs *[]*alarm.Alarm, in alarm.FindInput) (int64, error) {
	db := d.db.Model(&alarm.Alarm{})

	//  添加筛选条件
	if in.Priority > 0 {
		db = db.Where("priority = ?", in.Priority)
	}
	if in.Method > 0 {
		db = db.Where("method = ?", in.Method)
	}
	if in.StartAt > 0 {
		db = db.Where("created_at >= ?", time.Unix(in.StartAt, 0))
	}
	if in.EndAt > 0 {
		db = db.Where("created_at <= ?", time.Unix(in.EndAt, 0))
	}
	if in.DeviceID != "" {
		db = db.Where("device_id = ?", in.DeviceID)
	}
	if in.Type > 0 {
		db = db.Where("type=?", in.Type)
	}
	var count int64
	if err := db.Count(&count).Error; err != nil || count == 0 {
		return count, err
	}
	err := db.Limit(in.Limit()).Offset(in.Offset()).Order("created_at desc").Find(bs).Error
	return count, err
}

func (d DB) FindAlarmInfoByDeviceID(bs *[]*alarm.AlarmInfoByDeviceID, in alarm.FindAlarmInfoInput) error {
	db := d.db.Model(&alarm.Alarm{}).Select("device_id, count(*) as count")
	if len(in.DeviceID) != 0 {
		db.Where("device_id in (?)", in.DeviceID)
	}
	if in.StartAt > 0 {
		db = db.Where("created_at >= ?", time.Unix(in.StartAt, 0))
	}
	if in.EndAt > 0 {
		db = db.Where("created_at <= ?", time.Unix(in.EndAt, 0))
	}

	return db.Group("device_id").Order("count desc").Limit(in.Top).Find(&bs).Error
}

// CleanAlarm implements alarm.Storer.
func (d DB) CleanAlarm(expire time.Duration) error {
	return d.db.Where("created_at < ?", time.Now().Add(-expire)).Delete(&alarm.Alarm{}).Error
}

func (d DB) UpdateAlarmWithLock(ap *alarm.Alarm, id string, fn func(ap *alarm.Alarm)) error {
	return d.db.Transaction(func(tx *gorm.DB) error {
		if err := tx.Clauses(clause.Locking{Strength: "UPDATE"}).Where("id = ?", id).First(ap).Error; err != nil {
			return err
		}
		fn(ap)
		return tx.Save(ap).Error
	})
}
