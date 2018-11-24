package routers

import (
	"log"

	"github.com/EasyDarwin/EasyDarwin/rtsp"
	"github.com/gin-gonic/gin"
	"github.com/reactivex/rxgo/handlers"
	"github.com/reactivex/rxgo/observer"
)

func (h *APIHandler) StreamStart(c *gin.Context) {
	type Form struct {
		URL         string `form:"url" binding:"required"`
		IdleTimeout int    `form:"idleTimeout"`
	}
	var form Form
	err := c.Bind(&form)
	if err != nil {
		return
	}
	client := rtsp.NewRTSPClient(rtsp.GetServer(), form.URL, int64(form.IdleTimeout)*1000)
	pusher := rtsp.NewClientPusher(client)
	rtsp.GetServer().AddPusher(pusher)
	onNext := handlers.NextFunc(func(item interface{}) {
		log.Printf("CLIENT:RTSP拉流成功:%v", item)
	})
	onDone := handlers.DoneFunc(func() {
		log.Println("CLIENT  done")
	})
	onError := handlers.ErrFunc(func(err error) {
		log.Println("CLIENT  Error :", err.Error())
	})
	watcher := observer.New(onNext, onDone, onError)
	client.Start().Subscribe(watcher)
	c.IndentedJSON(200, "OK")
}

func (h *APIHandler) StreamStop(c *gin.Context) {

}
