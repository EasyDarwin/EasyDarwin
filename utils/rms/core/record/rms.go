package record

import (
	"context"
	"log/slog"
	"runtime"
	"time"

	"easydarwin/lnton/pkg/conc"
	"easydarwin/lnton/pkg/conc/pool"
	"easydarwin/lnton/pkg/web"
)

// RecordPlanHandler 录像机计划处理器
type RecordPlanHandler struct {
	plan conc.Map[int, *RecordPlan]
	data conc.Map[string, int]
}

// NewRecordPlanHandler ...
func NewRecordPlanHandler() *RecordPlanHandler {
	return &RecordPlanHandler{}
}

// Reload 从数据库重载配置
func (r *RecordPlanHandler) Reload(store Storer) *RecordPlanHandler {
	plans := make([]*RecordPlan, 0, 10)
	_, err := store.FindRecordTemplates(&plans, &FindRecordTemplatesInput{
		PagerFilter: web.PagerFilter{
			Page: 1,
			Size: 9999,
		},
	})
	if err != nil {
		slog.Error("RecordPlanHandler reload", "err", err)
		return r
	}

	for _, plan := range plans {
		r.UpdatePlan(plan)
		channelIDs := make([]string, 0, 8)
		if err := store.FindPlanWithChannelsByPlanID(&channelIDs, plan.ID); err != nil {
			slog.Error(`FindPlanWithChannelsByPlanID`, "err", err)
			continue
		}
		r.UpdateChannels(plan, channelIDs)
	}
	return r
}

func (r *RecordPlanHandler) UpdateChannels(plan *RecordPlan, channelIDs []string) {
	for _, cid := range channelIDs {
		r.data.Store(cid, plan.ID)
	}
}

func (r *RecordPlanHandler) UpdatePlan(plan *RecordPlan) {
	r.plan.Store(plan.ID, plan)
}

// IsRecording 判断是否在录像时间
func (r *RecordPlanHandler) IsRecording(channelID string) bool {
	planID, _ := r.data.Load(channelID)
	plan, ok := r.plan.Load(planID)
	if !ok {
		return false
	}
	return plan.Enabled && IsRecording(plan.Plans, time.Now())
}

// DeletePlan 删除录像计划与通道关联
func (r *RecordPlanHandler) DeletePlan(planID int) {
	r.plan.Delete(planID)
	r.data.Range(func(key string, value int) bool {
		if value == planID {
			r.data.Delete(key)
		}
		return true
	})
}

// DeleteChannels 删除通道关联
func (r *RecordPlanHandler) DeleteChannels(channelIDs []string) {
	for _, cid := range channelIDs {
		r.data.Delete(cid)
	}
}

// IsRecording 根据一周每天每小时的开关状态判断当前时间是否为开启状态
func IsRecording(schedule string, now time.Time) bool {
	if len(schedule) != 7*24 {
		slog.Error("schedule length is not 7*24", "schedule", schedule)
		return false
	}
	dayOfWeek := int(now.Weekday()+6) % 7 // 调整为0-6，对应周一至周日
	hourOfDay := now.Hour()

	// 计算位置索引
	index := (dayOfWeek * 24) + hourOfDay

	// 检查索引位置的字符是否为'1'
	if index >= 0 && index < len(schedule) && schedule[index] == '1' {
		return true
	}
	return false
}

// TickCheckLive 定时检查直播
func (r *RecordPlanHandler) TickCheckLive(ctx context.Context, fn func(channelID string, isRecord bool)) {
	ticker := time.NewTimer(5 * time.Second)
	defer ticker.Stop()
	p := pool.NewPool(runtime.NumCPU())
	for {
		select {
		case <-ctx.Done():
			return
		case <-ticker.C:
			r.data.Range(func(channelID string, _ int) bool {
				isRecord := r.IsRecording(channelID)
				p.Go(func() {
					fn(channelID, isRecord)
				})
				return true
			})
			p.Wait()
			ticker.Reset(5 * time.Second)
		}
	}
}
