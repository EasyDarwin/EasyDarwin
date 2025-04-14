package statapi

import (
	"os"
	"path/filepath"

	"easydarwin/lnton/pkg/web"
	"easydarwin/lnton/plugin/core/stat"
	"github.com/gin-gonic/gin"
)

func Register(g gin.IRouter, hf ...gin.HandlerFunc) {
	stat := g.Group("/stats", hf...)
	stat.GET("", web.WarpH(findStat))
}

func findStat(_ *gin.Context, _ *struct{}) (gin.H, error) {
	dir, _ := os.Executable()
	return gin.H{
		"mem": stat.GetMemData(),
		"cpu": stat.GetCPUData(),
		"disk": []gin.H{
			{
				"name":  filepath.Dir(dir),
				"used":  stat.GetCurrentMainDisk(),
				"total": stat.GetTotalMainDisk(),
			},
		},
		"netup":   stat.GetNetUpData(),
		"netdown": stat.GetNetDownData(),
	}, nil
}
