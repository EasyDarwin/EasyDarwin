package recordapi

import (
	"errors"
	"fmt"
	"strconv"
	"strings"

	"easydarwin/lnton/pkg/orm"
	"easydarwin/lnton/pkg/web"
	"easydarwin/lnton/rms/core/record"
	"github.com/gin-gonic/gin"
)

type Loger interface {
	RecordLog(remark string) gin.HandlerFunc
}
type RecordConfig struct {
	core record.Core
}

const (
	findRecordTemplatesBase = "BASE"
	findRecordTemplatesFull = "FULL"
)

func Register(g gin.IRouter, core record.Core, l Loger, gf ...gin.HandlerFunc) {
	c := RecordConfig{
		core: core,
	}
	// 录像计划模板
	plans := g.Group("/records/plans", gf...)
	plans.POST("", web.WarpH(c.editRecordPlan), l.RecordLog("编辑录像计划"))         // 编辑模板
	plans.DELETE("/:id", web.WarpH(c.deleteRecordPlan), l.RecordLog("删除录像计划")) // 删除模板
	plans.GET("", web.WarpH(c.findRecordTemplates))                            // 查询模板列表(包含模板详情)

	plans.DELETE("/:id/channels", web.WarpH(c.delRecordWithChannels)) // 批量删除关联
	plans.POST("/:id/channels", web.WarpH(c.editRecordWithChannels))  // 批量增加关联
	plans.GET("/:id/channels", web.WarpH(c.findRecordWithChannels))   // 获取录像计划关联的通道

	records := g.Group("/records", gf...)
	records.GET("/channels", web.WarpH(c.findRecordChannel)) // 获取录像通道
	records.DELETE("/channels", web.WarpH(c.delRecordChannel))

	// 云存管理
	oss := g.Group("/storages", gf...)
	// oss.POST("", c.editStorage, l.RecordLog("编辑云存"))      // 编辑云存
	oss.GET("", c.findStorages) // 云存列表
	// oss.DELETE("/:id", c.delStorage, l.RecordLog("删除云存")) // 删除云存

	// 云存策略
	strategy := g.Group("/strategies", gf...)
	// strategy.POST("", c.editCloudStrategy, l.RecordLog("编辑策略"))         // 编辑策略
	strategy.GET("", c.findCloudStrategy) // 获取策略列表
	// strategy.DELETE("/:id", c.deleteCloudStrategy, l.RecordLog("删除策略")) // 删除策略

	// 录像计划设置
	// plans := g.Group("/records/plans", gf...)
	// plans.POST("", c.editChannelRecord, l.RecordLog("设置录像计划"))                      // 为通道开启录像计划
	// g.POST("/records/plans2", append(gf, c.editRecord, l.RecordLog("设置录像计划 2"))...) // 设置录像计划 2
	// plans.GET("/:id", c.getChannelRecordPlan)                                       // 获取通道的录像计划
}

// func (d *RecordConfig) editRecord(c *gin.Context) {
// 	var input struct {
// 		ChannelIDs   []string `json:"channel_ids"`
// 		Enabled      bool     `json:"enabled"`
// 		Plans        string   `json:"plans"`
// 		CloudEnabled bool     `json:"cloud_enabled"`
// 		StorageID    int      `json:"storage_id,string"`
// 		StrategyID   int      `json:"strategy_id,string"`
// 		Stream       string   `json:"stream"`
// 		RMSID        string   `json:"rms_id"`
// 	}
// 	if err := c.ShouldBindJSON(&input); err != nil {
// 		web.Fail(c, web.ErrBadRequest.With(
// 			web.HanddleJSONErr(err).Error(),
// 			fmt.Sprintf("请检查请求类型%s", c.GetHeader("Content-Type"))),
// 		)
// 		return
// 	}

// 	p, err := d.core.FirstOrCreateRecordTemplates(input.Plans)
// 	if err != nil {
// 		web.Fail(c, err)
// 		return
// 	}
// 	if input.CloudEnabled {
// 		// 确认 StorageID 与 StrategyID 存在
// 		_, err := d.core.GetCouldStoregeByID(input.StorageID)
// 		if err != nil {
// 			web.Fail(c, err)
// 			return
// 		}
// 		_, err = d.core.GetCloudStrategyByID(input.StrategyID)
// 		if err != nil {
// 			web.Fail(c, err)
// 			return
// 		}
// 	}
// 	if input.Stream == "" {
// 		input.Stream = "MAIN"
// 	}

// 	total, err := d.core.EditRecordPlan(c.Request.Context(), record.EditRecordPlanInput{
// 		StorageID:    input.StorageID,
// 		StrategyID:   input.StrategyID,
// 		Stream:       strings.ToUpper(input.Stream),
// 		TemplateID:   p.ID,
// 		Enabled:      input.Enabled,
// 		CloudEnabled: input.CloudEnabled,
// 		ChannelIDs:   input.ChannelIDs,
// 		RMSID:        input.RMSID,
// 	})
// 	if err != nil {
// 		web.Fail(c, err)
// 		return
// 	}
// 	web.Success(c, gin.H{"total": total})
// }

// editRecordPlan 编辑模板
func (r *RecordConfig) editRecordPlan(c *gin.Context, in *record.CreateRecordPlanInput) (record.RecordPlan, error) {
	createPlan := record.RecordPlan{
		Name:    in.Name,
		Plans:   in.Plans,
		Enabled: in.Enabled,
		// Days:    in.StorageDays,
	}
	// 默认7天
	if createPlan.Days <= 0 {
		createPlan.Days = 7
	}
	createPlan.ID = in.ID
	createPlan.CreatedAt = orm.Now()
	err := r.core.EditRecordTemplate(c, &createPlan)
	return createPlan, err
}

func (r *RecordConfig) findRecordTemplates(c *gin.Context, in *record.FindRecordTemplatesInput) (any, error) {
	isFull := strings.ToUpper(in.Fields) == findRecordTemplatesFull
	recordPlans, total, err := r.core.FindRecordTemplates(in, isFull)
	return gin.H{
		"total": total,
		"items": recordPlans,
	}, err
}

type recordWithChannelsInput struct {
	ChannelIDs []string `json:"channel_ids"`
}

func (r *RecordConfig) delRecordWithChannels(c *gin.Context, in *recordWithChannelsInput) (any, error) {
	err := r.core.DelRecordWithChannels(in.ChannelIDs)
	return gin.H{"channel_ids": in.ChannelIDs}, err
}

func (r *RecordConfig) editRecordWithChannels(c *gin.Context, in *recordWithChannelsInput) (any, error) {
	planID, _ := strconv.Atoi(c.Param("id"))
	err := r.core.EditRecordWithChannels(planID, in.ChannelIDs)

	return gin.H{"channel_ids": in.ChannelIDs}, err
}

func (r *RecordConfig) findRecordWithChannels(c *gin.Context, _ *struct{}) (any, error) {
	planID, _ := strconv.Atoi(c.Param("id"))
	channels, total, err := r.core.FindRecordWithChannels(planID)
	if err != nil {
		return nil, err
	}
	return gin.H{
		"total": total,
		"items": channels,
	}, nil
}

func (r *RecordConfig) deleteRecordPlan(c *gin.Context, _ *struct{}) (any, error) {
	planID, _ := strconv.Atoi(c.Param("id"))
	if planID == 0 {
		return nil, web.ErrBadRequest.Withf("planID 不正确")
	}
	result, err := r.core.DeleteRecordTemplate(c, planID)
	if errors.Is(err, record.ErrUsingNotDelete) {
		return nil, web.ErrBadRequest.Msg("使用中不可删除")
	}
	return result, err
}

func (r *RecordConfig) findRecordChannel(c *gin.Context, in *record.FindRecordChannelInput) (any, error) {
	channel, total, err := r.core.FindRecordChannel(in)
	if err != nil {
		return nil, err
	}
	return gin.H{
		"total": total,
		"items": channel,
	}, nil
}

func (r *RecordConfig) delRecordChannel(c *gin.Context, in *record.DelRecordChannelInput) (any, error) {
	err := r.core.DelRecordChannel(in)
	if err != nil {
		return nil, err
	}
	return in, nil
}

// func (r *RecordConfig) getRecordByMonth(ctx *gin.Context) {

// 	input := queryRecordByMonth{}
// 	if err := ctx.Bind(&input); err != nil {
// 		slog.Error("绑定参数失败", err)
// 		web.Fail(ctx, err)
// 		return
// 	}
// 	flagMap, err := r.core.GetRecordMonthBy(input.DeviceCode, time.Now().Format("20060102"))
// 	if err != nil {
// 		AbortWithCodeJson(c, http.StatusOK, http.StatusBadRequest, err.Error())
// 		return
// 	}

// 	AbortWithCodeJson(c, http.StatusOK, http.StatusOK, flagMap)
// }

func (r *RecordConfig) editChannelRecord(c *gin.Context) {
	var input record.EditRecordPlanInput
	if err := c.ShouldBindJSON(&input); err != nil {
		web.Fail(c, web.ErrBadRequest.With(
			web.HanddleJSONErr(err).Error(),
			fmt.Sprintf("请检查请求类型%s", c.GetHeader("Content-Type"))),
		)
		return
	}
	if input.Enabled && (input.StorageID <= 0 || input.StrategyID <= 0 && input.TemplateID <= 0) {
		web.Fail(c, web.ErrBadRequest.Msg("录像计划参数不正确"))
		return
	}
	total, err := r.core.EditRecordPlan(c.Request.Context(), input)
	if err != nil {
		web.Fail(c, err)
		return
	}
	web.Success(c, gin.H{"total": total})
}

// func (r *RecordConfig) delChannelRecord(ctx *gin.Context) {
// 	id, _ := strconv.Atoi(ctx.Param("id"))
// 	if id <= 0 {
// 		web.Fail(ctx, fmt.Errorf("id 参数错误"))
// 		return
// 	}

// 	result, err := r.core.DelChannelRecord(ctx, id)
// 	if err != nil {
// 		web.Fail(ctx, err)
// 		return
// 	}

// 	web.Success(ctx, result)
// }

// func (r *RecordConfig) getChannelRecordPlan(c *gin.Context) {
// 	channelID := c.Param("id")
// 	if channelID == "" {
// 		// web.Fail(c, fmt.Errorf("id 参数错误"))
// 		web.Fail(c, web.ErrBadRequest.Withf("id 参数错误 id[%d]", channelID))
// 		return
// 	}
// 	out, err := r.core.GetChannelRecord(channelID)
// 	if err != nil {
// 		web.Fail(c, err)
// 		return
// 	}
// 	web.Success(c, out)
// }

// // editStorage 编辑云存
// func (r *RecordConfig) editStorage(c *gin.Context) {
// 	var input record.EditStorageInput

// 	if err := c.ShouldBindJSON(&input); err != nil {
// 		web.Fail(c, web.ErrBadRequest.With(
// 			web.HanddleJSONErr(err).Error(),
// 			fmt.Sprintf("请检查请求类型%s", c.GetHeader("Content-Type"))),
// 		)
// 		return
// 	}

// 	v := record.CloudStorage{
// 		Name:     input.Name,
// 		Type:     input.Type,
// 		EndPoint: input.EndPoint,
// 		Bucket:   input.Bucket,
// 		KeyID:    input.KeyID,
// 		Secret:   input.Secret,
// 		Region:   input.Region,
// 	}
// 	err := r.core.EditCouldStorage(c, &v)
// 	if err != nil {
// 		web.Fail(c, err)
// 		return
// 	}
// 	web.Success(c, v)
// }

func (r *RecordConfig) findStorages(c *gin.Context) {
	// 	var input record.FindRecordStoragesInput

	// 	if err := c.ShouldBind(&input); err != nil {
	// 		web.Fail(c, web.ErrBadRequest.With(
	// 			web.HanddleJSONErr(err).Error(),
	// 			fmt.Sprintf("请检查请求类型%s", c.GetHeader("Content-Type"))),
	// 		)
	// 		return
	// 	}

	// 	isFull := strings.ToUpper(input.Fields) == findRecordTemplatesFull

	// 	storages, total, err := r.core.FindCouldStorage(input.Limit(), input.Offset())
	// 	if err != nil {
	// 		web.Fail(c, err)
	// 		return
	// 	}

	// 	if !isFull {
	// 		outs := make([]record.BaseSotrageOutput, len(*storages))
	// 		for i := range *storages {
	// 			outs[i].ID = (*storages)[i].ID
	// 			outs[i].Name = (*storages)[i].Name
	// 			outs[i].Type = (*storages)[i].Type
	// 		}

	// 		web.Success(c, gin.H{
	// 			"total": total,
	// 			"items": outs,
	// 		})

	// 		return
	// 	}

	// 	outs := make([]record.FindStoragesOutput, len(*storages))
	// 	for i := range *storages {
	// 		outs[i].ID = (*storages)[i].ID
	// 		outs[i].Name = (*storages)[i].Name
	// 		outs[i].Type = (*storages)[i].Type
	// 		outs[i].EndPoint = (*storages)[i].EndPoint
	// 		outs[i].Bucket = (*storages)[i].Bucket
	// 		outs[i].KeyID = (*storages)[i].KeyID
	// 		outs[i].Secret = (*storages)[i].Secret
	// 		outs[i].Region = (*storages)[i].Region
	// 	}

	// web.Success(c, gin.H{
	// 	"total": total,
	// 	"items": outs,
	// })
	web.Success(c, gin.H{
		"total": 0,
		"items": []string{},
	})
}

// func (r *RecordConfig) delStorage(c *gin.Context) {
// 	id, _ := strconv.Atoi(c.Param("id"))
// 	if id <= 0 {
// 		// web.Fail(c, fmt.Errorf("id 参数错误"))
// 		web.Fail(c, web.ErrBadRequest.Withf("id 参数错误 id[%d]", id))
// 		return
// 	}

// 	err := r.core.DeleteCloudStorage(c, id)

// 	if errors.Is(err, record.ErrUsingNotDelete) {
// 		// web.Fail(ctx, web.NewError("ErrBadMessage", "使用中不可删除"))
// 		web.Fail(c, web.ErrBadRequest.Msg("使用中不可删除"))
// 		return
// 	}

// 	if err != nil {
// 		web.Fail(c, err)
// 		return
// 	}

// 	web.Success(c, gin.H{"id": id})
// }

// // editCloudStrategy 编辑策略
// func (r *RecordConfig) editCloudStrategy(c *gin.Context) {
// 	var in record.EditCloudStrategyInput
// 	if err := c.ShouldBind(&in); err != nil {
// 		web.Fail(c, web.ErrBadRequest.With(
// 			web.HanddleJSONErr(err).Error(),
// 			fmt.Sprintf("请检查请求类型%s", c.GetHeader("Content-Type"))),
// 		)
// 		return
// 	}

// 	if in.Value < 1 {
// 		web.Fail(c, web.ErrBadRequest.Msg("策略值小于1"))
// 		return
// 	}

// 	in.Type = strings.ToUpper(in.Type)

// 	if in.Type != "DAYS" && in.Type != "CAPS" {
// 		web.Fail(c, web.ErrBadRequest.Withf("策略类型有误 type[%s]", in.Type))
// 		return
// 	}

// 	if in.Type == "CAPS" {
// 		web.Fail(c, web.ErrBadRequest.Msg("暂不支持容量策略"))
// 		return
// 	}

// 	v := record.CloudStrategy{
// 		Name:  in.Name,
// 		Type:  in.Type,
// 		Value: in.Value,
// 	}

// 	err := r.core.EditCloudStrategy(c, &v)
// 	if err != nil {
// 		web.Fail(c, err)
// 		return
// 	}
// 	web.Success(c, v)
// }

func (r *RecordConfig) findCloudStrategy(c *gin.Context) {
	// 	var input web.PagerFilter

	// 	if err := c.ShouldBindQuery(&input); err != nil {
	// 		web.Fail(c, web.ErrBadRequest.With(
	// 			web.HanddleJSONErr(err).Error(),
	// 			fmt.Sprintf("请检查请求类型%s", c.GetHeader("Content-Type"))),
	// 		)
	// 		return
	// 	}

	// 	strategies, total, err := r.core.FindCloudStrategy(input.Limit(), input.Offset())
	// 	if err != nil {
	// 		web.Fail(c, err)
	// 		return
	// 	}

	// 	out := make([]record.FindCloudStrategyOutput, len(*strategies))

	// 	for i := range *strategies {
	// 		out[i].ID = (*strategies)[i].ID
	// 		out[i].Name = (*strategies)[i].Name
	// 		out[i].Type = (*strategies)[i].Type
	// 		out[i].Value = (*strategies)[i].Value
	// 	}

	// 	web.Success(c, gin.H{
	// 		"total": total,
	// 		"items": out,
	// 	})
	// }

	// func (r *RecordConfig) deleteCloudStrategy(c *gin.Context) {
	// 	id, _ := strconv.Atoi(c.Param("id"))

	// 	if id <= 0 {
	// 		web.Fail(c, web.ErrBadRequest.Withf("ID 不正确"))
	// 		return
	// 	}

	// 	result, err := r.core.DeleteCloudStrategy(c, id)

	// 	if errors.Is(err, record.ErrUsingNotDelete) {
	// 		web.Fail(c, web.ErrBadRequest.Msg("使用中不可删除"))
	// 		return
	// 	}

	// 	if err != nil {
	// 		web.Fail(c, err)
	// 		return
	// 	}

	// web.Success(c, result)
	web.Success(c, gin.H{})
}
