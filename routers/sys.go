package routers

import (
	"fmt"
	"log"
	"net/http"
	"runtime"
	"strings"
	"time"

	"github.com/gin-gonic/gin"
	"github.com/rikugun/EasyDarwin/models"
	"github.com/rikugun/EasyDarwin/rtsp"
	"github.com/rikugun/EasyGoLib/db"
	"github.com/rikugun/EasyGoLib/sessions"
	"github.com/rikugun/EasyGoLib/utils"
	"github.com/shirou/gopsutil/cpu"
	"github.com/shirou/gopsutil/mem"
)

/**
 * @apiDefine sys 系统
 */

type APIHandler struct {
	RestartChan chan bool
}

var API = &APIHandler{
	RestartChan: make(chan bool),
}

var (
	memData    []PercentData = make([]PercentData, 0)
	cpuData    []PercentData = make([]PercentData, 0)
	pusherData []CountData   = make([]CountData, 0)
	playerData []CountData   = make([]CountData, 0)
)

func init() {
	go func() {
		ticker := time.NewTicker(2 * time.Second)
		defer ticker.Stop()
		timeSize := 30
		for {
			select {
			case <-ticker.C:
				mem, _ := mem.VirtualMemory()
				cpu, _ := cpu.Percent(0, false)
				now := utils.DateTime(time.Now())
				memData = append(memData, PercentData{Time: now, Used: mem.UsedPercent / 100})
				cpuData = append(cpuData, PercentData{Time: now, Used: cpu[0] / 100})
				pusherData = append(pusherData, CountData{Time: now, Total: uint(rtsp.Instance.GetPusherSize())})
				playerCnt := 0
				for _, pusher := range rtsp.Instance.GetPushers() {
					playerCnt += len(pusher.GetPlayers())
				}
				playerData = append(playerData, CountData{Time: now, Total: uint(playerCnt)})

				if len(memData) > timeSize {
					memData = memData[len(memData)-timeSize:]
				}
				if len(cpuData) > timeSize {
					cpuData = cpuData[len(cpuData)-timeSize:]
				}
				if len(pusherData) > timeSize {
					pusherData = pusherData[len(pusherData)-timeSize:]
				}
				if len(playerData) > timeSize {
					playerData = playerData[len(playerData)-timeSize:]
				}
			}
		}
	}()
}

func (h *APIHandler) ModifyPassword(c *gin.Context) {
	type Form struct {
		OldPassword string `form:"oldpassword" binding:"required"`
		NewPassword string `form:"newpassword" binding:"required"`
	}
	var form Form
	if err := c.Bind(&form); err != nil {
		return
	}
	sess := sessions.Default(c)
	var user models.User
	db.DB.First(&user, sess.Get("uid"))
	if user.ID != "" && strings.EqualFold(form.OldPassword, user.Password) {
		db.DB.Model(&user).Update("password", form.NewPassword)
	} else {
		c.AbortWithStatusJSON(http.StatusBadRequest, "原密码不正确")
		return
	}
	token, _ := sess.RenewID()
	c.IndentedJSON(http.StatusOK, gin.H{
		"token": token,
	})
}

/**
 * @api {get} /api/v1/getserverinfo 获取平台运行信息
 * @apiGroup sys
 * @apiName GetServerInfo
 * @apiSuccess (200) {String} Hardware 硬件信息
 * @apiSuccess (200) {String} RunningTime 运行时间
 * @apiSuccess (200) {String} StartUpTime 启动时间
 * @apiSuccess (200) {String} Server 软件信息
 */
func (h *APIHandler) GetServerInfo(c *gin.Context) {
	c.IndentedJSON(http.StatusOK, gin.H{
		"Hardware":         strings.ToUpper(runtime.GOARCH),
		"InterfaceVersion": "V1",
		"RunningTime":      utils.UpTimeString(),
		"StartUpTime":      utils.DateTime(utils.StartTime),
		"Server":           fmt.Sprintf("%s/%s,%s (Platform/%s;)", "EasyDarwin", BuildDateTime, BuildVersion, strings.Title(runtime.GOOS)),
		"memData":          memData,
		"cpuData":          cpuData,
		"pusherData":       pusherData,
		"playerData":       playerData,
	})
}

/**
 * @api {get} /api/v1/restart 重启服务
 * @apiGroup sys
 * @apiName Restart
 * @apiUse simpleSuccess
 */
func (h *APIHandler) Restart(c *gin.Context) {
	log.Println("Restart...")
	c.JSON(http.StatusOK, "OK")
	go func() {
		select {
		case h.RestartChan <- true:
		default:
		}
	}()
}

/**
 * @apiDefine userInfo
 * @apiSuccess (200) {String} id
 * @apiSuccess (200) {String} name 用户名
 * @apiSuccess (200) {String[]} [roles] 角色列表
 */

/**
 * @api {get} /api/v1/login 登录
 * @apiGroup sys
 * @apiName Login
 * @apiParam {String} username 用户名
 * @apiParam {String} password 密码(经过md5加密,32位长度,不带中划线,不区分大小写)
 * @apiSuccessExample 成功
 * HTTP/1.1 200 OK
 * Set-Cookie: token=s%3ArkyMbQE0M.5AKAOXbW8c7iP%2BOo0venPkCYiEiPK9FY31mB6AlFQak;//用着后续接口调用的 token
 */
func (h *APIHandler) Login(c *gin.Context) {
	type Form struct {
		Username string `form:"username" binding:"required"`
		Password string `form:"password" binding:"required"`
	}
	var form Form
	if err := c.Bind(&form); err != nil {
		return
	}
	var user models.User
	db.DB.Where(&models.User{Username: form.Username}).First(&user)
	if user.ID == "" {
		c.AbortWithStatusJSON(401, "用户名或密码错误")
		return
	}
	if !strings.EqualFold(user.Password, form.Password) {
		c.AbortWithStatusJSON(401, "用户名或密码错误")
		return
	}
	sess := sessions.Default(c)
	sess.Set("uid", user.ID)
	sess.Set("uname", user.Username)
	c.IndentedJSON(200, gin.H{
		"token": sessions.Default(c).ID(),
	})
}

/**
 * @api {get} /api/v1/userInfo 获取当前登录用户信息
 * @apiGroup sys
 * @apiName UserInfo
 * @apiUse userInfo
 */
func (h *APIHandler) UserInfo(c *gin.Context) {
	sess := sessions.Default(c)
	uid := sess.Get("uid")
	if uid != nil {
		c.IndentedJSON(200, gin.H{
			"id":   uid,
			"name": sess.Get("uname"),
		})
	} else {
		c.IndentedJSON(200, nil)
	}
}

/**
 * @api {get} /api/v1/logout 登出
 * @apiGroup sys
 * @apiName Logout
 * @apiUse simpleSuccess
 */
func (h *APIHandler) Logout(c *gin.Context) {
	sess := sessions.Default(c)
	sess.Destroy()
	c.IndentedJSON(200, "OK")
}

func (h *APIHandler) DefaultLoginInfo(c *gin.Context) {
	var user models.User
	sec := utils.Conf().Section("http")
	defUser := sec.Key("default_username").MustString("admin")
	defPass := sec.Key("default_password").MustString("admin")
	db.DB.First(&user, "username = ?", defUser)
	if utils.MD5(defPass) != user.Password {
		defPass = ""
	}
	c.JSON(200, gin.H{
		"username": defUser,
		"password": defPass,
	})
}
