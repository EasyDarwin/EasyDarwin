package userapi

import (
	"fmt"
	"log/slog"
	"strconv"
	"time"

	"easydarwin/lnton/pkg/web"
	nw "easydarwin/lnton/pkg/web"
	"easydarwin/lnton/plugin/core/user"
	"github.com/gin-gonic/gin"
)

const (
	TokenExpiredTime = 6 * time.Hour // Token过期时间
)

// User 用户
type User struct {
	core      user.Core
	debug     bool
	jwtSecret string
	// AuthFailedRedirect string `json:"auth_failed_redirect"`
}

type cptchaOutput struct {
	CaptchaID int    `json:"captcha_id"`
	Base64    string `json:"base64"`
	Expired   int    `json:"expired"`
}

// RegisterUser
// debug 会延迟 token 的有效期，验证码支持 "test" 关键字验证
// jwtSecret 生成 token 的密码
func RegisterUser(
	g gin.IRouter,
	usr user.Core,
	debug bool, // 调试模式，会延迟 token 的有效期，验证码支持 "test" 关键字验证
// appName, 					// 软件名称，普通产品会有不同的逻辑
// authFailedRedirect, // 鉴权失败或者用户退出登录时返回的地址
	jwtSecret string,      // 生成 token 的密码
	hf ...gin.HandlerFunc, // 中间件
) {
	u := User{
		core:      usr,
		debug:     debug,
		jwtSecret: jwtSecret,
		// AuthFailedRedirect: authFailedRedirect,
	}

	g.POST("/captcha", u.captcha)
	g.POST("/login", web.WarpH(u.login))
	g.POST("/logout", u.logout)
	ug := g.Group("/refresh-token", hf...)
	ug.POST("", web.WarpH(u.refreshToken))

	users := g.Group("/users", hf...)
	users.GET("", web.WarpH(u.findUsers))
	users.POST("", web.AuthLevel(2), web.WarpH(u.create)) // 创建用户
	users.PUT("/:username", web.WarpH(u.update))
	users.DELETE("/:username", web.WarpH(u.DelUser))
	users.PUT("/:username/reset-password", u.resetPassword)
	users.PUT("/:username/reset-account", web.WarpH(u.resetUsername)) // 由于gin框架限制，username存的是ID

	// users.PUT("/:id/reset-password",u.resetUsername)
	// 开发者账户
	users.POST("/apps", u.createApp)
	users.PUT("/apps/:id/reset", u.resetSecret)
	users.PUT("/apps/:id", u.editApp)
	users.GET("/apps", u.findApps)
	users.DELETE("/apps/:id", u.deleteApp)
}

func (u User) create(c *gin.Context, in *user.CreateUserInput) (any, error) {
	in.GroupID = 111 // 系统默认的用户组，如果启用用户组功能，注释改行
	user, err := u.core.CreateUser(in, web.GetUID(c))
	return user, err
}

func (u User) update(c *gin.Context, in *user.UpdateUserInput) (any, error) {
	in.UserName = c.Param("username")
	if err := u.core.UpdateUser(web.GetUID(c), in); err != nil {
		return nil, err
	}
	return in, nil
}

func (u User) findUsers(c *gin.Context, in *user.FindUsersInput) (any, error) {
	in.GroupID = 111 // 没有使用用户组，暂时写死
	users, total, err := u.core.FindUsers(web.GetUID(c), in)
	if err != nil {
		return nil, err
	}
	return gin.H{
		"total": total,
		"items": users,
	}, nil
}

func (u User) DelUser(c *gin.Context, in *user.DelUserInput) (any, error) {
	username := c.Param("username")

	in.UserName = username
	in.GroupID = 111
	if err := u.core.DelUser(web.GetUID(c), in); err != nil {
		return nil, err
	}
	return in.UserName, nil
}

// captcha函数用于生成验证码
func (u User) captcha(c *gin.Context) {
	// 定义一个结构体，用于接收JSON数据
	var in struct {
		Username string `json:"username"`
	}
	// 解析JSON数据
	if err := c.ShouldBindJSON(&in); err != nil {
		// 如果解析失败，返回错误信息
		web.Fail(c, web.ErrBadRequest.With(web.HanddleJSONErr(err).Error()))
		return
	}
	// 生成验证码
	out, err := u.core.CreateCaptcha(in.Username, c.ClientIP())
	if err != nil {
		// 如果生成失败，返回错误信息
		web.Fail(c, err)
		return
	}
	// 返回验证码信息
	web.Success(c, out)
}

// 重置密码函数
func (u User) resetPassword(c *gin.Context) {
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
	v := nw.NewValidator()
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
	if err := u.core.ResetPassword(username, input.Password, input.NewPassword); err != nil {
		// 如果重置密码失败，返回错误信息
		web.Fail(c, err)
		return
	}
	// 返回成功信息
	web.Success(c, gin.H{
		"id": username,
	})
}

// login函数用于处理用户登录请求
func (u User) login(c *gin.Context, in *user.LoginInput) (*loginOutput, error) {
	// 将用户配置中的Debug值赋给input变量
	in.Debug = u.debug
	// 将客户端IP赋给input变量
	in.IP = c.ClientIP()

	// 调用core.Login函数进行登录验证
	usr, err := u.core.Login(in)
	if err != nil {
		return nil, err
	}
	// 定义一个user.UserGroup类型的变量group
	var group user.UserGroup
	if err := u.core.Store.GetGroupByID(&group, usr.GroupID); err != nil {
		return nil, web.ErrDB.Msg(err.Error())
	}
	// 定义过期时间
	expires := TokenExpiredTime
	// 如果是Debug模式，过期时间设置为5天
	if u.debug {
		expires = 5 * 24 * time.Hour
	}
	// 生成Token
	token, err := web.NewToken(web.TokenInput{
		UID:      usr.ID,
		GroupID:  usr.GroupID,
		Exires:   expires,
		Secret:   u.jwtSecret,
		Username: usr.Username,
		// GroupLevel: group.Level,
		Level: usr.Level,
		Role:  usr.Role,
	})
	if err != nil {
		// 如果生成Token失败，记录错误信息并返回错误信息
		slog.Error("NewToken", "err", err)
		return nil, web.ErrServer
	}
	// 返回登录成功的信息
	return &loginOutput{
		Token:     token,
		ExpiredAt: time.Now().Add(expires).Unix(),
		TimeoutS:  int64(TokenExpiredTime / time.Second),
		// ResetPassword:  usr.FirstLoginInfo.ResetPW,
		ISResetAccount: usr.FirstLoginInfo.ResetPW && usr.Level == 1,
		User: loginUserOutput{
			ID:       usr.ID,
			Username: usr.Username,
			Name:     usr.Name,
			Role:     usr.Role,
			Leve:     usr.Level,
		},
	}, nil
}

// type RefreshTokenOutput loginOutput
type RefreshTokenOutput struct {
	Token     string `json:"token"`
	ExpiredAt int64  `json:"expired_at"` // 到期时间
	TimeoutS  int64  `json:"timeout_s"`  // 有效期时长(秒)
}

func (u User) refreshToken(c *gin.Context, in *user.LoginInput) (*RefreshTokenOutput, error) {
	// 重新生成Token
	input := web.TokenInput{
		UID:      web.GetUID(c),
		GroupID:  c.GetInt("groupID"),
		Exires:   TokenExpiredTime,
		Secret:   u.jwtSecret,
		Username: web.GetUsername(c),
		// GroupLevel: web.GetGroupLevel(c),
		Level: c.GetInt("level"),
		Role:  web.GetRole(c),
	}
	token, err := web.NewToken(input)
	if err != nil {
		// 如果生成Token失败，记录错误信息并返回错误信息
		slog.Error("NewToken", "err", err)
		return nil, web.ErrServer
	}
	// 返回登录成功的信息
	return &RefreshTokenOutput{
		Token:     token,
		ExpiredAt: time.Now().Add(TokenExpiredTime).Unix(),
		TimeoutS:  int64(TokenExpiredTime / time.Second),
		// ResetPassword:  usr.FirstLoginInfo.ResetPW,
		//ISResetAccount: usr.FirstLoginInfo.ResetPW && usr.Level == 1,
		//User: loginUserOutput{
		//	ID:       web.GetUID(c),
		//	Username: web.GetUsername(c),
		//	Name:     usr.Name,
		//	Role:     usr.Role,
		//	Leve:     usr.Level,
		//},
	}, nil
}

func (u User) logout(c *gin.Context) {
	// if u.AuthFailedRedirect != "" {
	// 	web.Success(c, gin.H{
	// 		"url": u.AuthFailedRedirect,
	// 	})
	// 	return
	// }
	web.Success(c, gin.H{
		"url": "/cloud",
	})
	// return
}

// 定义登录输出结构体
type loginOutput struct {
	Token     string          `json:"token"`      // 登录令牌
	ExpiredAt int64           `json:"expired_at"` // 到期时间
	TimeoutS  int64           `json:"timeout_s"`  // 有效期时长(秒)
	User      loginUserOutput `json:"user"`       // 用户信息
	// ResetPassword  bool            `json:"reset_password"`   // 是否需要重置密码
	ISResetAccount bool `json:"is_reset_account"` // 是否重置密码
}

// 定义登录用户输出结构体
type loginUserOutput struct {
	ID       int    `json:"id"`        // 用户ID
	Username string `json:"username"`  // 用户名
	RGroupID int    `json:"rgroup_id"` // 用户组ID
	Name     string `json:"name"`      // 用户姓名
	Role     string `json:"role"`      // 用户角色
	Leve     int    `json:"level"`     // 用户等级
}

// editApp 函数用于编辑应用
func (u User) editApp(c *gin.Context) {
	// 从URL参数中获取id
	id, _ := strconv.Atoi(c.Param("id"))
	// 定义一个结构体用于接收JSON数据
	var input struct {
		IPs []string `json:"ips"`
	}
	// 绑定JSON数据到结构体
	if err := c.ShouldBindJSON(&input); err != nil {
		// 如果绑定失败，返回错误信息
		web.Fail(c, web.ErrBadRequest.With(
			web.HanddleJSONErr(err).Error(),
			fmt.Sprintf("请检查请求类型 %s", c.GetHeader("content-type"))),
		)
		return
	}
	// 获取用户ID
	uid := web.GetUID(c)
	// 调用core.EditApp函数编辑应用
	if err := u.core.EditApp(uid, id, input.IPs); err != nil {
		// 如果编辑失败，返回错误信息
		web.Fail(c, err)
		return
	}
	// 编辑成功，返回成功信息
	web.Success(c, gin.H{"id": id})
}

// 添加开发者
// createApp 函数用于创建应用
func (u User) createApp(c *gin.Context) {
	// 定义输入结构体
	var input struct {
		AppID string   `json:"app_id"`
		IPs   []string `json:"ips"`
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
	// 如果AppID长度小于5，返回错误信息
	if len(input.AppID) < 5 {
		web.Fail(c, web.ErrBadRequest.Msg("AppID 长度不能小于 5 "))
		return
	}
	// 调用core模块的CreateApp函数创建应用，并返回secret
	secret, err := u.core.CreateApp(input.AppID, input.IPs)
	if err != nil {
		// 如果创建失败，返回错误信息
		web.Fail(c, err)
		return
	}
	// 返回成功信息
	web.Success(c, gin.H{"secret": secret})
}

// 重置密码
func (u User) resetSecret(c *gin.Context) {
	// 获取当前用户ID
	uid := web.GetUID(c)
	// 获取目标用户ID
	targetUID, _ := strconv.Atoi(c.Param("id"))
	// 如果目标用户ID为223，则不允许操作
	// if targetUID == 223 {
	// web.Fail(c, web.ErrPermissionDenied.Msg("此账号不允许操作"))
	// return
	// }

	// 调用core包中的ResetSecret方法重置密码
	secret, err := u.core.ResetSecret(targetUID, uid)
	if err != nil {
		web.Fail(c, err)
		return
	}
	// 返回重置后的密码
	web.Success(c, gin.H{"secret": secret})
}

// findApps 函数用于查找应用
func (u User) findApps(c *gin.Context) {
	// 定义一个 PagerFilter 结构体变量
	var input web.PagerFilter
	// 绑定查询参数到 input 变量
	if err := c.ShouldBindQuery(&input); err != nil {
		// 如果绑定失败，返回错误信息
		web.Fail(c, web.ErrBadRequest.With(
			web.HanddleJSONErr(err).Error(),
			fmt.Sprintf("请检查请求类型 %s", c.GetHeader("content-type"))),
		)
		return
	}
	// 调用 core 的 FindApps 方法，传入 limit 和 offset 参数
	out, total, err := u.core.FindApps(input.Limit(), input.Offset())
	if err != nil {
		// 如果查询失败，返回错误信息
		web.Fail(c, err)
		return
	}
	// 返回查询结果
	web.Success(c, gin.H{"items": out, "total": total})
}

// 删除应用
func (u User) deleteApp(c *gin.Context) {
	// 获取URL参数中的id
	id, _ := strconv.Atoi(c.Param("id"))
	// 获取当前用户的UID
	huid := web.GetUID(c)
	// 如果id为223，则不允许操作
	if id == 223 {
		web.Fail(c, web.ErrPermissionDenied.Msg("此账号不允许操作"))
		return
	}
	// 调用core包中的DeleteApp方法删除应用
	if err := u.core.DeleteApp(id, huid); err != nil {
		web.Fail(c, err)
		return
	}
	// 返回成功信息
	web.Success(c, gin.H{"id": id})
}

type resetUsernameOutput struct {
	UserID int `json:"user_id"`
}

func (u User) resetUsername(c *gin.Context, in *user.ResetUsernameInput) (*resetUsernameOutput, error) {
	// 由于gin框架限制，username存的是ID
	userID := c.Param("username")
	uid, err := strconv.Atoi(userID)
	if err != nil {
		return nil, web.ErrBadRequest.Msg("请输入正确的用户ID")
	}
	if web.GetUID(c) != uid {
		return nil, web.ErrBadRequest.Msg("没有操作资源的权限")
	}
	// new2 := web.GetUsername(c)

	// 重置账号和密码
	if err := u.core.ResetUsername(c.Request.Context(), uid, in); err != nil {
		return nil, err
	}
	return &resetUsernameOutput{UserID: uid}, nil
}
