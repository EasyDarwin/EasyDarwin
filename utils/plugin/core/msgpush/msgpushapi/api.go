package msgpushapi

import (
	"easydarwin/lnton/pkg/orm"
	"easydarwin/lnton/pkg/web"
	"easydarwin/lnton/plugin/core/msgpush"
	"easydarwin/lnton/plugin/core/msgpush/store/msgpushdb"
	"github.com/gin-gonic/gin"
	"gorm.io/gorm"
	"time"
)

type MsgPushAPI struct {
	Core *msgpush.Core
}

func NewMsgPushAPI(db *gorm.DB) MsgPushAPI {
	core := msgpush.NewCore(msgpushdb.NewDB(db).AutoMerge(orm.EnabledAutoMigrate))
	return MsgPushAPI{Core: core}
}

func RegisterMsgPush(g gin.IRouter, api MsgPushAPI, hf ...gin.HandlerFunc) {
	psuh := g.Group("/msgpush", hf...)
	psuh.POST("/", api.AddMsg)
}

// AddMsg 建立SSE连接
func (mp *MsgPushAPI) AddMsg(c *gin.Context) {
	sse := web.NewSSE(200, 365*24*time.Hour)
	mp.Core.AddSession(sse)
	defer mp.Core.DelSession(sse)
	defer sse.Close()
	sse.ServeHTTP(c.Writer, c.Request)
}
