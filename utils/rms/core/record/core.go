package record

import (
	"context"
	"crypto/md5" // nolint
	"encoding/hex"
	"errors"
	"fmt"
	"log/slog"
	"os"
	"path"
	"strings"
	"time"

	"easydarwin/lnton/pkg/orm"
	"easydarwin/lnton/pkg/web"
	"github.com/redis/go-redis/v9"
)

// RecordPlansStreamName redis 消息通知
const (
	RecordPlansStreamName    = "record:plans:change"    // 录像计划变更通知
	DeviceChangeStreamName   = "device:info:change"     // 设备变更通知
	DeviceRegisterStreamName = "device:register:notice" // 设备注册通知
	MoveFilesStreamName      = "record:move:notice"     // 移动录像通知
)

// 通知动作
const (
	RecordPlansAction  = "plan"    // 录像计划
	CloudStorageAction = "storage" // 云存储服务
	// RecordStrategyAction    = "strategy"     // 策略
	RecordPlansActionDel  = "plan_del"    // 删除录像计划
	CloudStorageActionDel = "storage_del" // 删除云存储服务
	// RecordStrategyActionDel = "strategy_del" // 删除策略

	RecordPlansTemplate    = "template"     // 策略
	RecordPlansTemplateDel = "template_del" // 删除模板
)

const (
	DeviceChangeEdit = "edit"
	DeviceChangeDel  = "del"
)

const (
	DefaultStorageDays = 7
)

var ErrUsingNotDelete = errors.New("ErrUsingNotDelete")

// Storer 数据持久化
type Storer interface {
	PlanStorer
	// Storager
	// StrategyStorer
	TemplateStorer
	FindChannelIDsByStorager(id int) ([]string, error) // 查询与云存绑定的通道
	FindChannelIDsByStrategy(id int) ([]string, error) // 查询与云存策略绑定的通道
}

// Storager 云存相关持久化
// type Storager interface {
// 	EditCouldStorage(storage *CloudStorage) error                                 // 编辑云存
// 	FindCouldStorage(storages *[]*CloudStorage, limit, offset int) (int64, error) // 查询云存列表
// 	DeleteCouldStorege(id int, fn func() error) error                             // 删除云存

// 	GetCouldStoregeByID(*CloudStorage, int) error // 查询单个云存
// }

// StrategyStorer 策略相关持久化
// type StrategyStorer interface {
// 	EditCloudStrategy(*CloudStrategy) error
// 	FindCloudStrategy(strategy *[]*CloudStrategy, limit, offset int) (int64, error)
// 	DelCloudStrategy(cloudStrategy *CloudStrategy, id int) error

//		GetCloudStrategyByID(*CloudStrategy, int) error // 查询单个策略
//	}
type TemplateStorer interface {
	EditRecordTemplates(*RecordPlan) error // 编辑模板

	FirstOrCreateRecordTemplates(*RecordPlan) error                                       // 查询或创建模板
	DeleteRecordTemplates(template *RecordPlan, id int) error                             // 删除模板
	FindRecordTemplates(plan *[]*RecordPlan, in *FindRecordTemplatesInput) (int64, error) // 查询模板列表

	GetRecordTemplatesByID(*RecordPlan, int) error // 查询单个模板

	DelRecordWithChannels(channelIDs []string) error
	EditRecordWithChannels(planID int, channelIDs []string) error

	FindRecordWithChannelsByPlanID(rp *[]*RecordPlanWithChannelOutput, id int) (int64, error)

	FindPlanWithChannelsByPlanID(*[]string, int) error // 查询 plan_id 绑定的所有通道信息

	FindRecordChannel(rc *[]*FindRecordChannelOutput, device string, channel []string) error // 获取录像通道列表

	CountPlanWithChannels(planID int, channelID string) (int64, error) // 查询是否存在录像计划与通道的关联
	CountPlanWithChannel(channelID string) (int64, error)              // 是否此通道是否存在录像计划
}

// PlanStorer 录像计划持久化
type PlanStorer interface {
	EditRecordPlan(*RecordPlan2) error
	CreateChannelRecord(r *RecordPlan2) error
	DeleteChannelRecord(plan *RecordPlan2, channelID string) error
	// GetChannelRecordPlan(records *RecordPlan2, channelID string) error

	UpdateRecordPlanEnabledByID(channelID string, enabled bool) error // 更新录像计划开关，fn 要放到事务函数中
	UpdateNotifiedAtForRecordPlan(channelID string, date time.Time) error

	EditRecordPlans([]*RecordPlan2) error

	FindRecordPlan(bs *[]*RecordPlanWithBID, rmsID string) error
	FirstOrCreate(b any) error
}

// Core 业务
type Core struct {
	Store    Storer
	cache    *redis.Client
	getRMSID func() (string, error)
	Plan     *RecordPlanHandler
}

func NewCore(store Storer, cache *redis.Client, fn func() (string, error)) Core {
	if err := store.FirstOrCreate(&RecordPlan{
		Enabled: true,
		Name:    "每天",
		Model:   orm.Model{ID: 1},
		Plans:   "111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111",
		Days:    DefaultStorageDays,
	}); err != nil {
		slog.Error("FirstOrCreate", "err", err)
	}

	if err := store.FirstOrCreate(&RecordPlan{
		Enabled: true,
		Name:    "工作日",
		Model:   orm.Model{ID: 2},
		Plans:   "111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111000000000000000000000000000000000000000000000000",
		Days:    DefaultStorageDays,
	}); err != nil {
		slog.Error("FirstOrCreate", "err", err)
	}

	if err := store.FirstOrCreate(&RecordPlan{
		Enabled: true,
		Name:    "双休日",
		Model:   orm.Model{ID: 3},
		Plans:   "000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000111111111111111111111111111111111111111111111111",
		Days:    DefaultStorageDays,
	}); err != nil {
		slog.Error("FirstOrCreate", "err", err)
	}

	return Core{
		Store:    store,
		cache:    cache,
		getRMSID: fn,
		Plan:     NewRecordPlanHandler().Reload(store),
	}
}

// EditDevice 设备编辑操作
func (c Core) EditDevice(in EditDeviceInput) error {
	return c.pushDevice(context.TODO(), in)
}

func (c Core) DelRecordWithChannels(channelIDs []string) error {
	if err := c.Store.DelRecordWithChannels(channelIDs); err != nil {
		return web.ErrDB.Withf(`err[%s] := c.Store.DelRecordWithChannels(channelIDs)`, err)
	}
	c.Plan.DeleteChannels(channelIDs)
	return nil
}

func (c Core) EditRecordWithChannels(planID int, channelIDs []string) error {
	// 检查录像计划是否存在
	var plan RecordPlan
	if err := c.Store.GetRecordTemplatesByID(&plan, planID); err != nil {
		if orm.IsErrRecordNotFound(err) {
			return web.ErrBadRequest.Msg("录像计划不存在")
		}
		return web.ErrDB.Withf(`err[%s] := c.Store.GetRecordTemplatesByID`, err)
	}

	if err := c.Store.EditRecordWithChannels(planID, channelIDs); err != nil {
		return web.ErrDB.Withf(`err[%s] := c.Store.DelRecordWithChannels(channelIDs)`, err)
	}

	c.Plan.Reload(c.Store)
	return nil
}

func (c Core) FindRecordWithChannels(planID int) ([]*RecordPlanWithChannelOutput, int64, error) {
	rp := make([]*RecordPlanWithChannelOutput, 0, 8)

	total, err := c.Store.FindRecordWithChannelsByPlanID(&rp, planID)
	if err != nil {
		return nil, 0, web.ErrDB.Withf(`err[%s] := c.Store.FindRecordWithChannelsByPlanID`, err)
	}

	return rp, total, nil
}

func (c Core) FindRecordChannel(in *FindRecordChannelInput) ([]*FindRecordChannelOutput, int64, error) {
	// 打开目录
	// todo:通过配置文件获取目录
	f, err := os.Open("./record")
	if err != nil {
		// fmt.Println("Error opening directory:", err)
		slog.Error("Error opening directory", "err", err)
		return nil, 0, nil
	}
	defer f.Close()

	// 读取目录中的条目
	files, err := f.Readdir(-1)
	if err != nil {
		return nil, 0, err
	}
	// Waring:如果存在非法目录会导致统计异常
	total := len(files)
	// 对文件进行分页
	start := min(total, in.Offset())
	size := min(total-in.Offset(), in.Limit())
	files = files[start : start+size]
	list := make(map[string][]string)
	// 遍历并打印条目
	for _, file := range files {
		// "设备ID_通道ID"
		fileNames := strings.Split(file.Name(), "_")
		if len(fileNames) < 2 {
			slog.Warn("存在不规范的命名文件", "FileName", fileNames)
			continue
		}
		list[fileNames[0]] = append(list[fileNames[0]], fileNames[1])
	}
	out := make([]*FindRecordChannelOutput, 0, 8)
	o := make([]*FindRecordChannelOutput, 0, 8)
	// 查询每个设备信息
	for device, channels := range list {
		// 查询每个设备的通道信息
		err = c.Store.FindRecordChannel(&o, device, channels)
		if err != nil {
			return nil, 0, web.ErrDB.Withf(`err[%s] := c.Store.FindRecordChannel`, err)
		}
		out = append(out, o...)
	}
	return out, int64(total), nil
}

type DelRecordChannelInput struct {
	Device  string `json:"device_id"`
	Channel string `json:"channel_id"`
}

func (c Core) DelRecordChannel(in *DelRecordChannelInput) error {
	dir := "./record"
	fileName := fmt.Sprintf("%s_%s", in.Device, in.Channel)
	index := strings.Index(fileName, ".")

	if index >= 0 {
		return web.ErrBadRequest.Msg("ID不能包含特殊字符")
	}
	filePath := path.Join(dir, fileName)
	err := os.RemoveAll(filePath)
	if err != nil {
		return web.ErrBadRequest.Withf("err[%s] := os.RemoveAll(filePath)", err)
	}
	return nil
}

// // DelDevice 删除设备
// func (c Core) DelDevice(in EditDeviceInput) error {
// 	return c.pushDevice(context.TODO(), DeviceChangeDel, in)
// }

// const layout = "15:04"

// func (c *Core) checkPlan(plans Plans) error {
// 	for _, spans := range plans {
// 		for _, v := range spans {
// 			startTime, _ := time.Parse(layout, v.Start)
// 			endTime, _ := time.Parse(layout, v.End)
// 			if startTime.After(endTime) || startTime == endTime {
// 				return fmt.Errorf("录像计划时间不正确")
// 			}
// 		}
// 	}
// 	return nil
// }

func (c *Core) PushRecord(ctx context.Context, action, channelID string, objID int) error {
	return nil
	// return c.cache.XAdd(ctx, &redis.XAddArgs{
	// 	Stream: RecordPlansStreamName,
	// 	MaxLen: 5000,
	// 	Approx: true,
	// 	ID:     "*",
	// 	Values: []any{
	// 		"channel_id", channelID,
	// 		"action", action,
	// 		"obj_id", objID,
	// 	},
	// }).Err()
}

func (c *Core) PushMoveFiles(ctx context.Context, channelID string, prefix, newPrefix string) error {
	return nil
	// return c.cache.XAdd(ctx, &redis.XAddArgs{
	// 	Stream: MoveFilesStreamName,
	// 	MaxLen: 5000,
	// 	Approx: true,
	// 	ID:     "*",
	// 	Values: []any{
	// 		"channel_id", channelID,
	// 		"prefix", prefix,
	// 		"new_prefix", newPrefix,
	// 	},
	// }).Err()
}

// PushRegister 推送设备切换节点到消息队列
func (c *Core) PushRegister(ctx context.Context, deviceID, address, sk string) error {
	return nil
	// return c.cache.XAdd(ctx, &redis.XAddArgs{
	// 	Stream: DeviceRegisterStreamName,
	// 	MaxLen: 5000,
	// 	Approx: true,
	// 	ID:     "*",
	// 	Values: []any{
	// 		"sn", deviceID,
	// 		"address", address,
	// 		"sk", sk,
	// 	},
	// }).Err()
}

func (c *Core) pushDevice(ctx context.Context, in EditDeviceInput) error {
	return nil
	// return c.cache.XAdd(ctx, &redis.XAddArgs{
	// 	Stream: DeviceChangeStreamName,
	// 	MaxLen: 5000,
	// 	Approx: true,
	// 	ID:     "*",
	// 	Values: []any{
	// 		"device_id", in.DeviceID,
	// 		"protocol", in.Protocol,
	// 		"name", in.Name,
	// 		"updated_at", in.UpdatedAt,
	// 		"action", in.Action,
	// 		"ip", in.Ip,
	// 		"port", in.Port,
	// 		"username", in.UserName,
	// 		"password", in.Password,
	// 	},
	// }).Err()
}

// EditRecordTemplate 编辑模板
func (c *Core) EditRecordTemplate(ctx context.Context, plan *RecordPlan) error {
	if len(plan.Plans) != 168 {
		return web.ErrBadRequest.Withf("计划长度不规范")
	}

	if plan.ID > 0 {
		var p RecordPlan
		if err := c.Store.GetRecordTemplatesByID(&p, plan.ID); err != nil {
			slog.Error(`GetRecordTemplatesByID`, "err", err)
		} else {
			plan.CreatedAt = p.CreatedAt
		}
	}

	if err := c.Store.EditRecordTemplates(plan); err != nil {
		return web.ErrDB.Withf("EditRecordTemplate err:%s", err.Error())
	}

	c.Plan.Reload(c.Store)
	return nil
	// return c.PushRecord(ctx, RecordPlansTemplate, "", plan.ID)
}

// FirstOrCreateRecordTemplates 查询或创建模板
func (c *Core) FirstOrCreateRecordTemplates(plans string) (*RecordPlan, error) {
	if len(plans) != 168 {
		return nil, web.ErrBadRequest.Withf("计划长度不规范")
	}
	hash := md5.Sum([]byte(plans)) // nolint
	hashStr := hex.EncodeToString(hash[:])

	plan := RecordPlan{
		Name:  hashStr,
		Plans: plans,
	}
	if err := c.Store.FirstOrCreateRecordTemplates(&plan); err != nil {
		return &plan, web.ErrBadRequest.Withf("FirstOrCreateRecordTemplates err:%s", err.Error())
	}
	return &plan, nil
}

// DeleteRecordTemplate 删除模板
func (c *Core) DeleteRecordTemplate(ctx context.Context, planID int) (*RecordPlan, error) {
	// result := &RecordTemplate{}
	var result RecordPlan
	if err := c.Store.DeleteRecordTemplates(&result, planID); err != nil {
		return nil, web.ErrDB.Withf(`err[%s] := c.Store.DeleteRecordTemplates`, err)
	}
	c.Plan.DeletePlan(planID)
	// if err := c.PushRecord(ctx, RecordPlansTemplateDel, "", planID); err != nil {
	// return nil, web.ErrServer.Withf("err:%s", err)
	// }
	return &result, nil
}

func (c *Core) FindRecordTemplates(in *FindRecordTemplatesInput, isFull bool) (any, int64, error) {
	temples := make([]*RecordPlan, 0)
	result := make([]*RecordTemplateBaseOutput, 0)

	total, err := c.Store.FindRecordTemplates(&temples, in)
	if err != nil {
		return nil, 0, web.ErrDB.Withf(`plan, count, err[%s] := c.store.GetRecordPlan(limit, offset)`, err)
	}

	if isFull {
		return temples, total, nil
	}

	for _, v := range temples {
		result = append(result, &RecordTemplateBaseOutput{ID: v.ID, Name: v.Name})
	}

	return result, total, nil
}

// // EditCouldStorage 编辑云存
// func (c *Core) EditCouldStorage(ctx context.Context, storage *CloudStorage) error {
// 	if err := c.Store.EditCouldStorage(storage); err != nil {
// 		return web.ErrDB.Withf("err:%s,EditCouldStorage(storage *CloudStorage, fn func() error) error", err.Error())
// 	}
// 	return c.PushRecord(ctx, CloudStorageAction, "", storage.ID)
// }

// func (c *Core) FindCouldStorage(limit, offset int) (*[]*CloudStorage, int64, error) {
// 	storages := make([]*CloudStorage, 0)
// 	total, err := c.Store.FindCouldStorage(&storages, limit, offset)
// 	if err != nil {
// 		return nil, 0, web.ErrDB.Withf("err:%s,FindCouldStorage(&storages, limit, offset)", err.Error())
// 	}
// 	return &storages, total, nil
// }

// // ErrUsingNotDelete 资源使用中，不可删除
// // var ErrUsingNotDelete = errors.New("ErrUsingNotDelete")

// func (c *Core) DeleteCloudStorage(ctx context.Context, id int) error {
// 	err := c.Store.DeleteCouldStorege(id, func() error {
// 		return c.PushRecord(ctx, CloudStorageActionDel, "", id)
// 	})
// 	if errors.Is(err, ErrUsingNotDelete) {
// 		return ErrUsingNotDelete
// 	}
// 	if err != nil {
// 		return web.ErrDB.Withf("err:%s,DeleteCloudStorage(id int)", err.Error())
// 	}
// 	return nil
// }

// // EditCloudStrategy 编辑策略
// func (c *Core) EditCloudStrategy(ctx context.Context, s *CloudStrategy) error {
// 	if err := c.Store.EditCloudStrategy(s); err != nil {
// 		return web.ErrDB.Withf("err:%s,EditCloudStrategy(*CloudStrategy, func() error) error", err.Error())
// 	}
// 	return nil
// 	// return c.PushRecord(ctx, RecordStrategyAction, "", s.ID)
// }

// func (c *Core) FindCloudStrategy(limit, offset int) (*[]*CloudStrategy, int64, error) {
// 	strategies := make([]*CloudStrategy, 0)

// 	total, err := c.Store.FindCloudStrategy(&strategies, limit, offset)
// 	if err != nil {
// 		return nil, 0, web.ErrDB.Withf("err:%s,FindCouldStorage(&storages, limit, offset)", err.Error())
// 	}

// 	return &strategies, total, nil
// }

// // DeleteCloudStrategy 删除策略
// func (c *Core) DeleteCloudStrategy(ctx context.Context, id int) (*CloudStrategy, error) {
// 	// result := &CloudStrategy{}
// 	var result CloudStrategy
// 	err := c.Store.DelCloudStrategy(&result, id)
// 	if errors.Is(err, ErrUsingNotDelete) {
// 		return nil, ErrUsingNotDelete
// 	}
// 	if err != nil {
// 		return nil, web.ErrDB.Withf("err:%s,DeleteCloudStorage(id int)", err.Error())
// 	}
// 	return &result, nil
// }

type recordPackage struct {
	plan       RecordPlan2
	detail     *GetRecordPlanDetailOutput
	isNeedMove bool
}

// EditRecordPlan 编辑录像计划
func (c *Core) EditRecordPlan(ctx context.Context, input EditRecordPlanInput) (int, error) {
	data := make([]*recordPackage, 0, len(input.ChannelIDs))
	for _, channelID := range input.ChannelIDs {
		if channelID == "" {
			return 0, web.ErrBadRequest.Msg("channelID 必须不能为空")
		}

		isNeedMove := false
		// 查询一下是否策略发生变更
		detail, err := c.GetRecordPlanDetail(channelID)
		// 策略变更，需要移动文件夹
		// 同一个 bucket，不同的策略
		if err == nil && detail != nil && detail.Storage.ID > 0 && detail.Strategy.ID > 0 && detail.Storage.ID == input.StorageID && detail.Strategy.ID != input.StrategyID {
			isNeedMove = true
		}
		// 暂时不动旧代码，新增一个录像服务的判断
		// 建议将上面的判断，重构到下面代码块里
		rmsID := strings.TrimSpace(input.RMSID)
		if err == nil && detail != nil {
			if rmsID == "" {
				rmsID = detail.Plan.RMSID
			}
		}
		// 都没有 rmsID 的情况，从负载均衡里拿
		if rmsID == "" {
			// rmsID, err = c.getRMSID()
			// if err != nil {
			// 	return 0, web.ErrServer.Msg(err.Error())
			// }
		}

		plan := RecordPlan2{
			ChannelID:    channelID,
			StorageID:    input.StorageID,
			StrategyID:   input.StrategyID,
			Stream:       input.Stream,
			TemplateID:   input.TemplateID,
			Enabled:      input.Enabled,
			CloudEnabled: input.Enabled,
			NotifiedAt:   nil,
			RMSID:        rmsID,
			StoreType:    input.StoreType,
		}
		data = append(data, &recordPackage{
			plan:       plan,
			detail:     detail,
			isNeedMove: isNeedMove,
		})
	}
	out := make([]*RecordPlan2, 0, len(data))
	for _, v := range data {
		v := v.plan
		out = append(out, &v)
	}
	if err := c.Store.EditRecordPlans(out); err != nil {
		return 0, web.ErrDB.Withf("err[%s], EditRecordPlan(*RecordPlan, func() error) error", err)
	}
	for _, v := range data {
		v := v
		if err := c.PushRecord(ctx, RecordPlansAction, v.plan.ChannelID, 0); err != nil {
			return 0, web.ErrServer.Msg("消息推送失败").Withf(`err[%s] := c.pushRecord(ctx, RecordPlansAction, input.ChannelID, input.ChannelID)`, err)
		}
		slog.Info(">>>>>>>>>>>>>>>>>", "move", v.isNeedMove)
		if v.isNeedMove {
			if err := c.PushMoveFiles(ctx, v.plan.ChannelID, fmt.Sprintf("r/%d", v.detail.Strategy.ID), fmt.Sprintf("r/%d", v.plan.StrategyID)); err != nil {
				return 0, web.ErrServer.Msg("消息推送失败").Withf(`err[%s] := c.pushRecord(ctx, RecordPlansAction, input.ChannelID, input.ChannelID)`, err)
			}
		}
	}
	return len(data), nil
}

// UpdateNotifiedAtForRecordPlan 待实现
// 更新录像计划的通知时间，仅更新 NotifiedAt
func (c *Core) UpdateNotifiedAtForRecordPlan(channelID string, date time.Time) error {
	//timeDB := orm.Time{Time: date}
	//plan := RecordPlan{
	//	ChannelID:  channelID,
	//	NotifiedAt: &timeDB,
	//}
	if err := c.Store.UpdateNotifiedAtForRecordPlan(channelID, date); err != nil {
		return web.ErrDB.With("err:%s,c.Store.UpdateNotifiedAtForRecordPlan(channelID, date)", err.Error())
	}
	return nil
}

func (c *Core) DelChannelRecord(ctx context.Context, channelID string, delRecordPlan bool) (*RecordPlan2, error) {
	var result RecordPlan2
	if err := c.Store.DeleteChannelRecord(&result, channelID); err != nil {
		return nil, web.ErrDB.Withf("err:%s, c.store.DeleteChannelRecord(id)", err.Error())
	}
	// 是否删除设备的录像计划?
	if delRecordPlan {
		if err := c.PushRecord(ctx, RecordPlansActionDel, channelID, 0); err != nil {
			return nil, web.ErrServer.Msg("消息推送失败").Withf(`err[%s] := c.pushRecord(ctx, RecordPlansActionDel, 0, id)`, err)
		}
	}
	return &result, nil
}

// func (c *Core) GetChannelRecord(channelID string) (*RecordPlan2, error) {
// 	var records RecordPlan2
// 	if err := c.Store.GetChannelRecordPlan(&records, channelID); orm.IsErrRecordNotFound(err) {
// 		return &records, nil
// 	} else if err != nil {
// 		return nil, web.ErrDB.Withf("err:%s, GetChannelRecord(id int) ([]*RecordPlan, error", err.Error())
// 	}
// 	return &records, nil
// }

// func (c *Core) GetCouldStoregeByID(id int) (*CloudStorage, error) {
// 	var storage CloudStorage
// 	if err := c.Store.GetCouldStoregeByID(&storage, id); orm.IsErrRecordNotFound(err) {
// 		return nil, web.ErrNotFound.Msg("未找到云存储").Withf(`err[%s] := c.Store.GetCouldStoregeByID(&storage, id[%d])`, err, id)
// 	} else if err != nil {
// 		return nil, web.ErrDB.Withf("err:%s, GetCouldStoregeByID(id int) ([]*CloudStorage, error", err.Error())
// 	}
// 	return &storage, nil
// }

// func (c *Core) GetCloudStrategyByID(id int) (*CloudStrategy, error) {
// 	var strategy CloudStrategy
// 	if err := c.Store.GetCloudStrategyByID(&strategy, id); orm.IsErrRecordNotFound(err) {
// 		return nil, web.ErrNotFound.Msg("未找到云存策略").Withf(`err[%s] := c.Store.GetCloudStrategyByID(&strategy, id[%d])`, err, id)
// 	} else if err != nil {
// 		return nil, web.ErrDB.Withf("err:%s, GetCloudStrategyByID(id int) ([]*CloudStrategy, error", err.Error())
// 	}
// 	return &strategy, nil
// }

// GetRecordPlanDetail 查询录像计划详情
func (c *Core) GetRecordPlanDetail(channelID string) (*GetRecordPlanDetailOutput, error) {
	// var plan RecordPlan2
	// if err := c.Store.GetChannelRecordPlan(&plan, channelID); orm.IsErrRecordNotFound(err) {
	// 	return nil, web.ErrBadRequest.Msg("未找到录像计划")
	// } else if err != nil {
	// 	return nil, web.ErrDB.Withf("err:%s, GetChannelRecord(id int) ([]*RecordPlan, error", err.Error())
	// }

	// var storage CloudStorage
	// if err := c.Store.GetCouldStoregeByID(&storage, plan.StorageID); orm.IsErrRecordNotFound(err) {
	// 	return nil, web.ErrBadRequest.Msg("未找到录像存储")
	// } else if err != nil {
	// 	return nil, web.ErrDB.Withf("err:%s, GetCouldStoregeByID(id int) ([]*CloudStorage, error", err.Error())
	// }

	// var strategy CloudStrategy
	// if err := c.Store.GetCloudStrategyByID(&strategy, plan.StrategyID); orm.IsErrRecordNotFound(err) {
	// 	return nil, web.ErrBadRequest.Msg("未找到录像策略")
	// } else if err != nil {
	// 	return nil, web.ErrDB.Withf("err:%s, GetCloudStrategyByID(id int) ([]*CloudStrategy, error", err.Error())
	// }

	// var template RecordPlan
	// if err := c.Store.GetRecordTemplatesByID(&template, plan.TemplateID); orm.IsErrRecordNotFound(err) {
	// 	return nil, web.ErrBadRequest.Msg("未找到录像模板")
	// } else if err != nil {
	// 	return nil, web.ErrDB.Withf("err:%s, GetRecordTemplatesByID(id int) ([]*RecordTemplate, error", err.Error())
	// }

	return &GetRecordPlanDetailOutput{
		// Plan: plan,
		// Storage:  storage,
		// Strategy: strategy,
		// Template: template,
	}, nil
}

// FindReocrdPlanByCloudEnabled 查询启用中的录像计划
func (c *Core) FindReocrdPlanByCloudEnabled(rmsID string) ([]*RecordPlanDetailOutput, error) {
	list := make([]*RecordPlanWithBID, 0, 10)
	if err := c.Store.FindRecordPlan(&list, rmsID); err != nil {
		return nil, web.ErrDB.Withf("err:%s, FindReocrdPlan(rmsID string) error", err.Error())
	}
	out := make([]*RecordPlanDetailOutput, 0, 10)
	for _, plan := range list {
		detail, err := c.getPlanDetail(plan)
		if err != nil {
			slog.Error("FindReocrdPlanByCloudEnabled", slog.String("err", err.Error()))
			continue
		}
		out = append(out, detail)
	}
	return out, nil
}

func (c *Core) getPlanDetail(plan *RecordPlanWithBID) (*RecordPlanDetailOutput, error) {
	var storage CloudStorage
	// if err := c.Store.GetCouldStoregeByID(&storage, plan.StorageID); orm.IsErrRecordNotFound(err) {
	// 	return nil, web.ErrBadRequest.Msg("未找到录像存储")
	// } else if err != nil {
	// 	return nil, web.ErrDB.Withf("err:%s, GetCouldStoregeByID(id int) ([]*CloudStorage, error", err.Error())
	// }

	// var strategy CloudStrategy
	// if err := c.Store.GetCloudStrategyByID(&strategy, plan.StrategyID); orm.IsErrRecordNotFound(err) {
	// 	return nil, web.ErrBadRequest.Msg("未找到录像策略")
	// } else if err != nil {
	// 	return nil, web.ErrDB.Withf("err:%s, GetCloudStrategyByID(id int) ([]*CloudStrategy, error", err.Error())
	// }

	var template RecordPlan
	if err := c.Store.GetRecordTemplatesByID(&template, plan.TemplateID); orm.IsErrRecordNotFound(err) {
		return nil, web.ErrBadRequest.Msg("未找到录像模板")
	} else if err != nil {
		return nil, web.ErrDB.Withf("err:%s, GetRecordTemplatesByID(id int) ([]*RecordTemplate, error", err.Error())
	}
	return &RecordPlanDetailOutput{
		Plan:    *plan,
		Storage: storage,
		// Strategy: strategy,
		Template: template,
	}, nil
}

// func (c *Core) SaveRecordPlan(plan *RecordTemplate) (*RecordTemplate, error) {
// 	// if err := c.checkPlan(plan.Plans); err != nil {
// 	// 	return nil, err
// 	// }
// 	if err := c.store.SaveRecordPlan(plan); err != nil {
// 		return nil, web.ErrDB.Withf("err:%s, SaveRecordPlan(id int) ([]*RecordPlan, error", err.Error())
// 	}

// 	return plan, nil
// }

// func (c *Core) SaveCouldStorage(storage *CloudStorage) (*CloudStorage, error) {

// 	if err := c.store.SaveCouldStorage(storage); err != nil {
// 		return nil, web.ErrDB.Withf("err:%s, SaveCouldStorage ([]*RecordPlan, error", err.Error())
// 	}

// 	return storage, nil
// }

// func (c *Core) SaveCouldStrategy(s *CloudStrategy) (*CloudStrategy, error) {
// 	if err := c.store.SaveCouldStrategy(s); err != nil {
// 		return nil, web.ErrDB.Withf("err:%s, SaveCouldStrategy ([]*CloudStrategy, error", err.Error())
// 	}

// 	return s, nil
// }

// func (c *Core) GetRecordByMonth(DeviceCode string, date string) (map[string]string, error) {
// 	month := date[:6]

// 	//records, _, err := gRecordDao.GetChannelIdToDay("hls", streamID, date, 0, 0)
// 	//todo:DAO的接入
// 	// c.store.
// 	// 	dayMap := make(map[string]struct{})
// 	// for _, v := range records {
// 	// 	dayMap[v[6:8]] = struct{}{}
// 	// }
// 	// var dayStr string
// 	// for c := 0; c < 31; c++ {
// 	// 	dayNum := strconv.Itoa(c + 1)
// 	// 	if len(dayNum) == 1 {
// 	// 		dayNum = "0" + dayNum
// 	// 	}
// 	// 	if _, ok := dayMap[dayNum]; ok {
// 	// 		dayStr = dayStr + "1"
// 	// 		continue
// 	// 	}
// 	// 	dayStr = dayStr + "0"
// 	// }

// 	// if err != nil {
// 	// 	return nil, err
// 	// }

// 	// return map[string]string{month: dayStr}, nil
// }
