package recorddb

import (
	"time"

	"easydarwin/lnton/pkg/orm"
	"easydarwin/lnton/rms/core/record"
	"gorm.io/gorm"
	"gorm.io/gorm/clause"
)

var _ record.Storer = DB{}

type DB struct {
	db *gorm.DB
}

// FindPlanWithChannelsByPlanID implements record.Storer.
func (d DB) FindPlanWithChannelsByPlanID(channelIDs *[]string, planID int) error {
	return d.db.Model(&record.RecordWithChannels{}).Where("plan_id = ?", planID).Pluck("channel_id", channelIDs).Error
}

// CountPlanWithChannel implements record.Storer.
func (d DB) CountPlanWithChannel(channelID string) (int64, error) {
	var total int64
	return total, d.db.Model(&record.RecordWithChannels{}).Where("channel_id=?", channelID).Count(&total).Error
}

// CountPlanWithChannels implements record.Storer.
func (d DB) CountPlanWithChannels(planID int, channelID string) (int64, error) {
	var total int64
	return total, d.db.Model(&record.RecordWithChannels{}).Where("plan_id=? and channel_id=?", planID, channelID).Count(&total).Error
}

// DelRecordWithChannels implements record.Storer.
func (d DB) DelRecordWithChannels(channelIDs []string) error {
	return d.db.Debug().Where("channel_id in ?", channelIDs).Delete(&record.RecordWithChannels{}).Error
}

// EditRecordWithChannels implements record.Storer.
func (d DB) EditRecordWithChannels(planID int, channelIDs []string) error {
	return d.db.Transaction(func(tx *gorm.DB) error {
		for _, cid := range channelIDs {
			if err := tx.Where("channel_id=?", cid).Delete(&record.RecordWithChannels{}).Error; err != nil {
				return err
			}

			if err := tx.Save(&record.RecordWithChannels{
				PlanID:    planID,
				ChannelID: cid,
				CreatedAt: orm.Now(),
			}).Error; err != nil {
				return err
			}
		}
		return nil
	})
}

func (d DB) FindRecordWithChannelsByPlanID(rp *[]*record.RecordPlanWithChannelOutput, id int) (int64, error) {
	var total int64
	err := d.db.Table("channels").Select("channels.*,record_with_channels.*").
		Joins("left join record_with_channels on channels.device_id = record_with_channels.channel_id").
		Where("record_with_channels.plan_id = ?", id).Find(rp).Count(&total).Error
	if err != nil {
		return 0, err
	}
	return total, nil
}

// FindRecordPlan implements record.Storer.
func (d DB) FindRecordPlan(bs *[]*record.RecordPlanWithBID, rmsID string) error {
	return d.db.Model(&record.RecordPlan2{}).Raw(`SELECT a.*,b.bid,b.device_id FROM record_plans a JOIN channels b ON a.channel_id = b.id WHERE a.rms_id=?`, rmsID).Find(bs).Error
}

func NewDB(db *gorm.DB) DB {
	return DB{db: db}
}

func (d DB) AutoMigrate(ok bool) DB {
	if !ok {
		return d
	}
	if err := d.db.AutoMigrate(
		&record.RecordPlan{},
		&record.CloudStorage{},
		&record.CloudStrategy{},
		&record.RecordWithChannels{},
		//&record.RecordChannel{},
	); err != nil {
		panic(err)
	}
	return d
}

func (d DB) EditRecordTemplates(plan *record.RecordPlan) error {
	return d.db.Save(plan).Error
}

func (d DB) FindRecordTemplates(temples *[]*record.RecordPlan, in *record.FindRecordTemplatesInput) (int64, error) {
	var total int64
	db := d.db.Model(&record.RecordPlan{})
	if in.Name != "" {
		db = db.Where("name like ?", "%"+in.Name+"%")
	}
	if err := db.Count(&total).Error; err != nil || total <= 0 {
		return 0, err
	}
	return total, db.Limit(in.Limit()).Offset(in.Offset()).Order("id DESC").Find(temples).Error
}

// GetRecordTemplatesByID 通过ID查询单个模板
func (d DB) GetRecordTemplatesByID(temples *record.RecordPlan, id int) error {
	return d.db.Model(temples).Where("id = ?", id).First(temples).Error
}

// DeleteRecordTemplates 删除模板
func (d DB) DeleteRecordTemplates(plan *record.RecordPlan, planID int) error {
	return d.db.Transaction(func(tx *gorm.DB) error {
		if err := tx.Clauses(clause.Returning{}).Where("id=?", planID).Delete(plan).Error; err != nil {
			return err
		}
		return tx.Where("plan_id = ?", planID).Delete(&record.RecordWithChannels{}).Error
	})
}

func (d DB) UpdataRecordPlan(recordPlan *record.RecordPlan) error {
	return d.db.Model(recordPlan).Where("id=?", recordPlan.ID).Updates(recordPlan).Error
}

func (d DB) EditRecordPlan(plan *record.RecordPlan2) error {
	return d.db.Model(&record.RecordPlan2{}).Where("channel_id=?", plan.ChannelID).Save(plan).Error
}

func (d DB) UpdateRecordPlanEnabledByID(channelID string, enabled bool) error {
	return d.db.Model(&record.RecordPlan2{}).Where("channel_id = ?", channelID).Updates(map[string]interface{}{
		"enabled":       enabled,
		"cloud_enabled": enabled,
		"notified_at":   nil,
	}).Error
}

// EditCouldStorage 编辑云存
func (d DB) EditCouldStorage(storage *record.CloudStorage) error {
	// 更新云存
	if err := d.db.Model(&record.CloudStorage{}).Where("id=?", storage.ID).Save(storage).Error; err != nil {
		return err
	}
	// 设置录像计划为"尚未通知"
	return d.db.Model(&record.RecordPlan2{}).Where("storage_id=?", storage.ID).Update("notified_at", nil).Error
}

func (d DB) SaveCouldStorage(storage *record.CloudStorage) error {
	return d.db.Model(storage).Where("id=?", storage.ID).Save(storage).Error
}

func (d DB) FindCouldStorage(storages *[]*record.CloudStorage, limit, offset int) (int64, error) {
	var total int64
	if err := d.db.Model(&record.CloudStorage{}).Count(&total).Error; err != nil {
		return 0, err
	}

	return total, d.db.Model(&record.CloudStorage{}).Limit(limit).Offset(offset).Order("id DESC").Find(storages).Error
}

// GetCouldStoregeByID 通过ID查询云存
func (d DB) GetCouldStoregeByID(storage *record.CloudStorage, id int) error {
	return d.db.Where("id = ?", id).First(storage).Error
}

// DeleteCouldStorege 删除云存
func (d DB) DeleteCouldStorege(id int, fn func() error) error {
	// return db.db.Model(&record.CloudStorage{}).Where("id=?", id).Delete(&record.CloudStorage{}).Error
	var num int64
	// 查询是否被引用
	err := d.db.Model(&record.RecordPlan2{}).Where("storage_id=?", id).Count(&num).Error
	if err != nil {
		return err
	}

	if num > 0 {
		return record.ErrUsingNotDelete
	}

	db := d.db.Model(&record.CloudStorage{})
	return db.Transaction(func(tx *gorm.DB) error {
		if err := tx.Model(&record.CloudStorage{}).Where("id=?", id).Delete(&record.CloudStorage{}).Error; err != nil {
			return err
		}
		return fn()
	})
}

// EditCloudStrategy 编辑策略
func (d DB) EditCloudStrategy(s *record.CloudStrategy) error {
	return d.db.Model(&record.CloudStrategy{}).Where("id=?", s.ID).Save(s).Error
}

// func (d DB) SaveCouldStrategy(s *record.CloudStrategy) error {
// 	return d.db.Model(s).Save(s).Error
// }

func (d DB) FindCloudStrategy(Strategies *[]*record.CloudStrategy, limit, offset int) (int64, error) {
	var total int64
	if err := d.db.Model(&record.CloudStrategy{}).Count(&total).Error; err != nil {
		return 0, err
	}
	return total, d.db.Model(Strategies).Limit(limit).Offset(offset).Order("id DESC").Find(Strategies).Error
}

// GetCloudStrategyByID 通过ID查询策略
func (d DB) GetCloudStrategyByID(s *record.CloudStrategy, id int) error {
	return d.db.Where("id = ?", id).First(s).Error
}

// DelCloudStrategy 删除云存服务
func (d DB) DelCloudStrategy(cloudStrategy *record.CloudStrategy, id int) error {
	// return d.db.Model(&record.CloudStrategy{}).Where("id=?", id).First(cloudStrategy).Delete(&record.CloudStrategy{}).Error
	var num int64
	// 查询是否被引用
	err := d.db.Model(&record.RecordPlan2{}).Where("strategy_id=?", id).Count(&num).Error
	if err != nil {
		return err
	}

	if num > 0 {
		return record.ErrUsingNotDelete
	}

	return d.db.Model(&record.CloudStrategy{}).Where("id=?", id).First(cloudStrategy).Delete(&record.CloudStrategy{}).Error
}

func (d DB) CreateChannelRecord(r *record.RecordPlan2) error {
	return d.db.Model(&r).Create(r).Error
}

// DeleteChannelRecord 删除录像计划
func (d DB) DeleteChannelRecord(plan *record.RecordPlan2, channelID string) error {
	return d.db.Model(&record.RecordPlan2{}).Where("channel_id=?", channelID).Find(plan).Delete(&record.RecordPlan2{}).Error
}

func (d DB) GetChannelRecordPlan(records *record.RecordPlan2, id string) error {
	return d.db.Model(records).Where("channel_id=?", id).First(records).Error
}

func (d DB) UpdateNotifiedAtForRecordPlan(channelID string, date time.Time) error {
	return d.db.Model(&record.RecordPlan2{}).Where("channel_id = ?", channelID).Update("notified_at", date).Error
}

func (d DB) FindChannelIDsByStorager(storagerID int) ([]string, error) {
	const sql = `SELECT COALESCE(a.channel_id,'') FROM record_plans a LEFT JOIN cloud_storages b ON a.storage_id=b.id WHERE b.id=?`
	chs := make([]string, 0, 5)
	err := d.db.Raw(sql, storagerID).Scan(&chs).Error
	return chs, err
}

func (d DB) FindChannelIDsByStrategy(stratrgyID int) ([]string, error) {
	const sql = `SELECT COALESCE(a.channel_id,'') FROM record_plans a LEFT JOIN cloud_strategies b ON a.strategy_id=b.id WHERE b.id=?`
	chs := make([]string, 0, 5)
	err := d.db.Raw(sql, stratrgyID).Scan(&chs).Error
	return chs, err
}

// FirstOrCreateRecordTemplates ...
func (d DB) FirstOrCreateRecordTemplates(plan *record.RecordPlan) error {
	return d.db.Where("id=?", plan.ID).FirstOrCreate(plan).Error
}

func (d DB) EditRecordPlans(bs []*record.RecordPlan2) error {
	db := d.db.Model(&record.RecordPlan2{})
	return db.Transaction(func(tx *gorm.DB) error {
		for i := range bs {
			v := bs[i]
			if err := tx.Where("channel_id=?", v.ChannelID).Save(bs[i]).Error; err != nil {
				return err
			}
		}
		return nil
	})
}

// FindRecordChannel 查询录像通道
func (d DB) FindRecordChannel(rc *[]*record.FindRecordChannelOutput, device string, channel []string) error {
	// var total int64
	return d.db.Table("channels").Where("device_id = ? and bid in (?)", device, channel).Find(&rc).Error

	// err := d.db.Table("channels").Select("channels.*,record_channels.*").
	// 	Joins("left join record_channels on channels.device_id = record_with_channels.channel_id").
	// 	Find(rp).Count(&total).Error
	// if err != nil {
	// 	return 0, err
	// }
	// return total, nil
}

func (d DB) FirstOrCreate(b any) error {
	return d.db.FirstOrCreate(b).Error
}
