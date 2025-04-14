package alarm

import (
	"fmt"
	"log/slog"
	"strconv"
	"strings"
	"time"

	"easydarwin/lnton/pkg/conc"
	"easydarwin/lnton/pkg/finder"
	"easydarwin/lnton/pkg/orm"
	"easydarwin/lnton/pkg/web"
)

// Storer db
type Storer interface {
	alarmer
	Planer
	AlarmPlanChanneler
}

type alarmer interface {
	AddAlarm(input *Alarm) error
	FindAlarms(bs *[]*Alarm, in FindInput) (int64, error)
	FindAlarmInfoByDeviceID(bs *[]*AlarmInfoByDeviceID, in FindAlarmInfoInput) error // 按照设备ID获取报警数量
	UpdateAlarmWithLock(ap *Alarm, id string, fn func(ap *Alarm)) error
}

// Core 业务核心
type Core struct {
	storer  Storer
	log     *slog.Logger
	alarmCh chan *Alarm

	// Filer
	Filer *finder.Engine
	// 事件回调
	onAlarmIFrame func(*Alarm) (string, error)

	// 缓存 TODO:定时删除通道信息
	alarmChInfo *conc.Map[string, *AlarmChInfo]
	// alarmPlan   conc.Map[string, *AlarmPlan]
}

type AlarmChInfo struct {
	ChannelID      string
	LastImageAlarm time.Time
}

func (c *Core) GetAlarmch() chan *Alarm {
	return c.alarmCh
}

// NewCore 创建业务实体
func NewCore(storer Storer, log *slog.Logger, filer *finder.Engine, onAlarmIFrame func(alarm *Alarm) (string, error)) Core {
	c := Core{
		storer:        storer,
		log:           log,
		Filer:         filer,
		alarmCh:       make(chan *Alarm, 200),
		onAlarmIFrame: onAlarmIFrame,
		alarmChInfo:   &conc.Map[string, *AlarmChInfo]{},
	}
	for range 5 {
		go c.onAlarmNotify()
	}
	go c.alarmChannelClear() // 定时清理通道信息
	return c
}

func (c *Core) alarmChannelClear() {
	timer := time.NewTimer(2 * time.Hour)
	defer timer.Stop()

	for {
		<-timer.C
		// 定时删除通道，避免内存占用过高
		// 缺点，重新生成的通道信息第一此触发会立即获取快照，比设定的间隔要短

		// c.alarmChInfo.Clear()

		//c.alarmChInfo.Range(func(key string, s *AlarmChInfo) bool {
		//	fmt.Println(key)
		//	return true
		//})

		c.alarmChInfo.Range(func(key string, s *AlarmChInfo) bool {
			c.alarmChInfo.Delete(key)
			return true
		})
	}
}

func (c *Core) onAlarmNotify() {
	for alarm := range c.alarmCh {
		if alarm.DeviceID == "" {
			slog.Error("设备编码不能为空")
			continue
		}
		if alarm.ID == "" {
			alarm.ID = fmt.Sprintf("%s%d%s", alarm.DeviceID, time.Now().UnixMilli(), orm.GenerateRandomString(4))
		}
		alarm.CreatedAt = orm.Now()
		alarm.UpdatedAt = orm.Now()
		// 此时的Type已经被下层封装成字符串了，例如“2-0”,"5-6"
		if err := c.storer.AddAlarm(alarm); err != nil {
			slog.Error("onAlarmNotify", "err", err)
		}

		// 1根据报警预案，执行报警处理逻辑

		// 获取当前通道关联的报警预案
		// TODO: 重构ID
		alarm.ChannelID = alarm.DeviceID + "_" + alarm.ChannelID // 封装信息时通道ID使用的时20位的，所以这里这里要拼接
		plans, err := c.storer.GetAlarmPlansByChID(alarm.ChannelID)
		if err != nil {
			slog.Error("onAlarmNotify", "err", err)
			continue
		}
		// 获取报警预案
		aps := make([]*AlarmPlan, 0, len(plans))
		if err := c.storer.FindAlarmPlanByIDs(&aps, plans); err != nil {
			slog.Error("onAlarmNotify", "err", err)
			continue
		}
		// 预案匹配
		for _, ap := range aps {
			if !ap.Enabled {
				continue
			}
			// 条件匹配
			if CheackAlarm(*ap, *alarm) {
				aChInfo, exist := c.alarmChInfo.Load(alarm.ChannelID)
				if !exist {
					// 不存在就存储一下
					aChInfo = &AlarmChInfo{ChannelID: alarm.ChannelID}
					c.alarmChInfo.Store(alarm.ChannelID, aChInfo)
				}

				// 检查间隔
				if time.Now().Sub(aChInfo.LastImageAlarm) > time.Duration(ap.SnapInterval)*time.Second {
					aChInfo.LastImageAlarm = time.Now()
					path, err := c.onAlarmIFrame(alarm)
					if err != nil {
						slog.Error("onAlarmIFrame", "err", err)
						continue // 如果失败，就跳过，继续下一个预案
					}

					if err := c.storer.UpdateAlarmWithLock(alarm, alarm.ID, func(a *Alarm) {
						a.SnapPath = path
					}); err != nil {
						slog.Error("UpdateAlarmWithLock", "err", err)
						continue // 如果失败，就跳过，继续下一个预案
					}
					// 如果快照获取成功，就不再匹配后面的预案
					break
				}
			}

		}

	}
}

func CheackAlarm(ap AlarmPlan, a Alarm) bool {
	//if (ap.Priority == "" || matchType(ap.Priority, alarm.Priority)) && /* 匹配报警级别*/
	//	(ap.Method == "" || matchType(ap.Method, alarm.Method)) && /* 匹配报警方式*/
	//	(ap.Type == "" || matchType(ap.Type, alarm.Type)) /*匹配报警类型*/ {
	//}
	/* 匹配报警级别*/
	// 不是全部匹配，也没有精准匹配就返回false
	if ap.Priority != "" && !matchType(ap.Priority, strconv.Itoa(a.Priority)) {
		return false
	}

	/* 匹配报警方式*/
	// 不是全部匹配，也没有精准匹配就返回false
	if ap.Method != "" && !matchType(ap.Method, strconv.Itoa(a.Method)) {
		return false
	}
	// 1.报警类型为:2-设备防拆报警时，
	//   1.1 若不携带AlarmType参数，默认为报警设备报警
	//   1.2 若携带AlarmType参数，对应值为:1-视频丢失报警，2-设备防拆报警，3-存储设备磁盘满报警，4-设备高温报警，5-设备低温报警
	if a.Method == 2 {
		if ap.Type != "" && !matchType(ap.Type, a.Type) {
			return false
		}
	}
	// 2.报警类型为:5-视频报警时，
	//   2.1 对应值为:1-人工视频报警，2-运动目标检测报警，3-遗留物检测报警，4-物体移除检测报警，5-绊线检测报警，6-入侵检测报警，
	//   7-逆行检测报警，8-徘徊检测报警，9-流量统计报警，10-密度检测报警，11-视频异常检测报警，12-快速移动报警, 13-图像遮挡报警
	if a.Method == 5 {
		if ap.Type != "" && !matchType(ap.Type, a.Type) {
			return false
		}

		if a.Type == "5-6" {
			if ap.EventType != "" && !matchType(ap.EventType, strconv.Itoa(a.EventType)) {
				return false
			}
		}

	}
	// 3.报警类型为:6-设备故障报警时，
	//   3.1 对应值为:1-存储设备磁盘故障报警，2-存储设备风扇故障报警
	if a.Method == 6 {
		if ap.Type != "" && !matchType(ap.Type, a.Type) {
			return false
		}
	}

	return true
}

func matchType(t string, level string) bool {
	split := strings.Split(t, ",")
	for _, v := range split {
		if v == level {
			return true
		}

		// 按照国标规范，报警方式为2时，报警类型默认为报警设备报警，该处没有理解其含义
		// 所以这里的设备报警报警类型为0是，全部接受，如需修改请
		if level == "2-0" {
			return true
		}

	}
	return false
}

// AddAlarm 添加报警
func (c *Core) AddAlarm(in ModelInput) (*Alarm, error) {
	if in.SN == "" {
		return nil, web.ErrBadRequest.Msg("设备编码不能为空")
	}

	id := in.ID
	if in.ID == "" {
		id = fmt.Sprintf("%d%s", time.Now().UnixMilli(), orm.GenerateRandomString(4))
	}
	id = in.SN + id

	path := strings.Split(in.SnapPaths, ",")
	alarm := Alarm{
		DeviceID: in.SN,
		Priority: in.Priority,
		// TODO:因为报警出发规则修改了类型，不清楚输入模型有哪些逻辑引用，
		// 所以这里使用了拼接的方式，如果需要，因该把输入模型改为字符串，并处理相关错误
		Type:      fmt.Sprintf("%d-%d", in.Method, in.Type),
		Method:    in.Method,
		VideoPath: in.VideoPath,
		ChannelID: in.ChannelID,
		LogPath:   in.LogPath,
		Snapshots: path,
		ModelWithStrID: orm.ModelWithStrID{
			ID:        id,
			CreatedAt: orm.Now(),
			UpdatedAt: orm.Now(),
		},
	}
	if err := c.storer.AddAlarm(&alarm); err != nil {
		return nil, web.ErrDB.Withf(`err[%s] := c.storer.AddAlarm(&alarm)`, err)
	}
	return &alarm, nil
}

// FindAlarms 报警列表
func (c *Core) FindAlarms(in FindInput) ([]*Alarm, int64, error) {
	out := make([]*Alarm, 0, in.Limit())

	total, err := c.storer.FindAlarms(&out, in)
	if err != nil {
		return nil, 0, web.ErrDB.Withf(`err[%s] := c.storer.FindAlarms(&out, in)`, err)
	}
	return out, total, nil
}

func (c *Core) FindAlarmInfoByDeviceID(in FindAlarmInfoInput) ([]*AlarmInfoByDeviceID, error) {
	out := make([]*AlarmInfoByDeviceID, 0)
	if in.Top <= 0 {
		in.Top = 5
	}
	err := c.storer.FindAlarmInfoByDeviceID(&out, in)
	if err != nil {
		return nil, web.ErrDB.Withf("err[%s] := c.storer.FindAlarmInfoByDeviceID(&out, FindInput{})", err)
	}
	// todo: 需要进行limit
	if in.Top > 0 {
		out = out[0:min(len(out), in.Top)]
	}
	return out, nil
}

func (c *Core) FindAlarmInfo(in FindAlarmInfoInput) (*AlarmInfo, error) {
	var out AlarmInfo
	deviceTop, err := c.FindAlarmInfoByDeviceID(in)
	if err != nil {
		return nil, web.ErrDB.Withf("err[%s] := c.FindAlarmInfoByDeviceID(in)", err)
	}
	out.DeviceTop = deviceTop

	// Note:效率低，后面可以直接用Count
	_, total, err := c.FindAlarms(FindInput{
		StartAt: in.StartAt,
		EndAt:   in.EndAt,
	})
	if err != nil {
		return nil, web.ErrDB.Withf("err[%s] := c.FindAlarms()", err)
	}
	out.Total = total
	return &out, nil
}
