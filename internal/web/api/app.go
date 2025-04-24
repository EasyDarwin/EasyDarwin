package api

import (
	"easydarwin/internal/data"
	"easydarwin/utils/pkg/orm"
	"easydarwin/utils/pkg/web"
	"easydarwin/utils/plugin/core/user"
	"errors"
	"expvar"
	"fmt"
	"github.com/gin-gonic/gin"
	"gorm.io/gorm"
	"runtime"
	"strings"
	"time"
)

type login struct {
	database *gorm.DB
}

func (l login) Login(c *gin.Context, input *user.LoginInput) (*data.User, error) {
	var u data.User
	if err := l.database.Where("username=?", input.Username).First(&u).Error; err != nil {
		if errors.Is(err, orm.ErrRevordNotFound) {
			return nil, web.ErrNameOrPasswd.With("账号不存在")
		} else {
			return nil, web.ErrDB.With(err.Error())
		}
	}
	if input.Username == u.Username && input.Password == u.Password {
		return &u, nil
	}
	return nil, web.ErrDB.With("密码错误")
}

func (u login) logout(c *gin.Context) {

	web.Success(c, gin.H{
		"url": "/cloud",
	})
}

func (u login) resetPassword(c *gin.Context) {
	// 获取目标用户ID
	username := c.Param("username")
	// 定义输入结构体
	var input struct {
		Password    string `json:"password" `
		NewPassword string `json:"new_password"`
	}

	// 绑定JSON数据到输入结构体
	if err := c.ShouldBindJSON(&input); err != nil {
		// 如果绑定失败，返回错误信息
		web.Fail(c, web.ErrBadRequest.With(
			web.HanddleJSONErr(err).Error(),
			fmt.Sprintf("请检查请求类型 %s", c.GetHeader("content-type"))),
		)
		return
	}

	// 创建验证器
	v := web.NewValidator()
	// 验证密码不能为空
	v.Check(input.Password != "", "password", "不能为空")
	// 验证密码长度不能小于8
	v.Check(len(input.Password) >= 8, "password", "长度不能小于8")
	// 验证新密码长度不能小于8
	v.Check(len(input.NewPassword) >= 8, "new_password", "长度不能小于8")
	// 验证目标用户ID是否大于0
	// v.Check(targetUID > 0, "uid", "uid 参数错误")
	// 如果验证不通过，返回错误信息
	if !v.Valid() {
		web.Fail(c, web.ErrBadRequest.With(v.List()...))
		return
	}

	// 获取当前用户ID
	// handleUID := web.GetUID(c)
	// 调用核心模块重置密码
	if err := ResetPassword(username, input.Password, input.NewPassword); err != nil {
		// 如果重置密码失败，返回错误信息
		web.Fail(c, err)
		return
	}
	// 返回成功信息
	web.Success(c, gin.H{
		"id": username,
	})
}

func ResetPassword(username string, password, newPassword string) error {
	// 判断新密码长度是否小于8
	if len(newPassword) < 8 {
		return web.ErrBadRequest.Msg("密码不合法").With(newPassword)
	}
	var hUser data.User
	// 获取操作用户信息
	{
		// 根据操作用户ID获取用户信息
		if err := data.GetDatabase().Model(data.User{}).Where("username=?", username).First(&hUser).Error; err != nil {
			return web.ErrDB.Msg("用户不存在").Withf("err[%s] := c.Store.GetUserByID(&hUser, handleUID)", err)
		}
		// 判断操作用户密码是否正确
		if hUser.Password != password {
			return fmt.Errorf("数据库密码和输入的密码不一致")
		}
	}
	// 更新用户密码
	hUser.Password = newPassword
	if err := data.GetDatabase().Select("password").Save(&hUser).Error; err != nil {
		return web.ErrDB.With(err.Error())
	}
	return nil
}

// 自定义配置目录
var (
	startRuntime = time.Now()
	version      = "8.3.1"
	darwinName   = "EasyDarwin"
)

func getVersion(c *gin.Context) {
	startTime := time.Unix(startRuntime.Unix(), 0)
	timestamp := time.Now()
	diff := timestamp.Sub(startTime)
	hours := diff / time.Hour
	diff -= hours * time.Hour
	minutes := diff / time.Minute
	diff -= minutes * time.Minute
	seconds := diff / time.Second
	timeStr := fmt.Sprintf("%01d时%01d分%01d秒", hours, minutes, seconds)
	if hours == 0 {
		timeStr = fmt.Sprintf("%01d分%01d秒", minutes, seconds)
	}
	if hours == 0 && minutes == 0 {
		timeStr = fmt.Sprintf("%01d秒", seconds)
	}
	serverTime := time.Now().Format(time.DateTime)
	c.IndentedJSON(200, gin.H{
		"name":       darwinName,
		"version":    version,
		"buildTime":  strings.Trim(expvar.Get("build_time").String(), `"`),
		"startTime":  startRuntime.Format(time.DateTime),
		"serverTime": serverTime,
		"server":     runtime.GOOS,
		"hardware":   runtime.GOARCH,
		"runtime":    timeStr,
	})
}
