package web

import (
	"expvar"
	"fmt"
	"net/http/pprof"
	"runtime"
	"strings"

	"github.com/gin-gonic/gin"
)

// debugAccess 授权指定 ip 访问
func debugAccess(ips *[]string) gin.HandlerFunc {
	return func(c *gin.Context) {
		lips := *ips
		if !strings.HasPrefix(c.Request.URL.Path, "/debug/") || len(lips) == 0 {
			c.Next()
			return
		}
		for _, v := range lips {
			if c.ClientIP() == v {
				c.Next()
				return
			}
		}
		c.AbortWithStatusJSON(400, gin.H{"msg": fmt.Sprintf("%s 无权访问", c.ClientIP())})
	}
}

func SetupPProf(r *gin.Engine, ips *[]string) {
	debug := r.Group("/debug", debugAccess(ips))
	debug.GET("/pprof/", gin.WrapF(pprof.Index))
	debug.GET("/pprof/cmdline", gin.WrapF(pprof.Cmdline))
	debug.GET("/pprof/profile", gin.WrapF(pprof.Profile))
	debug.GET("/pprof/symbol", gin.WrapF(pprof.Symbol))
	debug.POST("/pprof/symbol", gin.WrapF(pprof.Symbol))
	debug.GET("/pprof/trace", gin.WrapF(pprof.Trace))
	debug.GET("/pprof/allocs", gin.WrapH(pprof.Handler("allocs")))
	debug.GET("/pprof/block", gin.WrapH(pprof.Handler("block")))
	debug.GET("/pprof/goroutine", gin.WrapH(pprof.Handler("goroutine")))
	debug.GET("/pprof/heap", gin.WrapH(pprof.Handler("heap")))
	debug.GET("/pprof/mutex", gin.WrapH(pprof.Handler("mutex")))
	debug.GET("/pprof/threadcreate", gin.WrapH(pprof.Handler("threadcreate")))
	debug.GET("/vars", gin.WrapH(expvar.Handler()))
}

func SetupMutexProfile(rate int) {
	runtime.SetBlockProfileRate(1)
	runtime.SetMutexProfileFraction(1)
}
