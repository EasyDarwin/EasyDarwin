package user

import (
	"errors"
	"fmt"
	"math"
	"strconv"
	"strings"
	"time"

	"easydarwin/utils/pkg/web"

	"easydarwin/utils/pkg/orm"
)

// LoginInput 结构体用于存储登录请求的输入参数
type LoginInput struct {
	Debug     bool   // 是否开启调试模式
	Username  string `json:"username"`   // 用户名
	Password  string `json:"password"`   // 密码
	Captcha   string `json:"captcha"`    // 验证码
	CaptchaID int    `json:"captcha_id"` // 验证码ID
	Mac       string `json:"mac"`        // MAC地址
	IP        string `json:"ip"`         // IP地址
}

// Login 函数用于处理登录请求
func (c Core) Login(in *LoginInput) (*User, error) {
	if !*c.disabledCaptcha {
		// 如果验证码为空，返回错误信息
		if in.CaptchaID <= 0 {
			return nil, web.ErrBadRequest.With("验证码不能为空")
		}
		// 检查验证码
		ok, err := c.CheckCaptcha(in.CaptchaID, in.Captcha)
		if !in.Debug && err != nil {
			return nil, err
		}
		if !((in.Debug && in.Captcha == "test") || ok) {
			return nil, web.ErrCaptchaWrong.Msg("验证码错误")
		}
		// 验证码通过，销毁验证码
		if err := c.Store.UpdateVcodeUsed(in.CaptchaID, in.Username); err != nil {
			return nil, web.ErrDB.With("验证码销毁失败", err.Error())
		}
	}

	var u User
	if err := c.Store.GetUserByUsername(&u, in.Username); errors.Is(err, orm.ErrRevordNotFound) {
		return nil, web.ErrNameOrPasswd.With("账号不存在")
	} else if err != nil {
		return nil, web.ErrDB.With(err.Error())
	}
	// 检查账号是否启用
	if !u.Enabled {
		return nil, web.ErrAccountDisabled.With("账号已停用")
	}
	if u.Type == UserTypeDeveloper {
		return nil, web.ErrAccountDisabled.Msg("开发者账户禁止登录")
	}
	// 尝试次数过多
	if date := time.Unix(u.LastLoginInfo.LimitedAt, 0); time.Now().Before(date) {
		return nil, web.ErrAccountDisabled.Msg(fmt.Sprintf("账号已锁定至 %s", date.Format(time.DateTime))).With("输入错误密码超过次数")
	}
	// 检查密码
	if err := CompareHashAndPasswordsd(u.Password, in.Password); err != nil {
		lockDuration := u.CheckPasswordAttempts()
		limitdAt := time.Now().Add(lockDuration).Unix()
		u.LastLoginInfo.LimitedAt = limitdAt
		if err := c.Store.UpdatePasswordAttempts(u.ID, limitdAt); err != nil {
			c.log.Error("UpdatePasswordAttempts", "err", err, "uid", u.ID)
		}
		if date := time.Unix(u.LastLoginInfo.LimitedAt-1, 0); time.Now().Before(date) {
			return nil, web.ErrAccountDisabled.Msg(fmt.Sprintf("账号或密码错误，已锁定 %.0f 分钟", lockDuration.Minutes())).With("输入错误密码超过次数")
		}
		if u.PasswordAttempts >= 1 {
			return nil, web.ErrNameOrPasswd.Msg(fmt.Sprintf("再错误 %d 次锁定账号", 3-u.PasswordAttempts%3-1)).With(err.Error())
		}
		return nil, web.ErrNameOrPasswd.With(err.Error())
	}

	go func(uid int) {
		// 更新用户密码
		var u User
		if err := c.Store.UpdateUser(&u, uid, func(u *User) {
			u.LastLoginInfo.LimitedAt = 0
			u.LastLoginInfo.IP = in.IP
			u.LastLoginInfo.Mac = in.Mac
			u.PasswordAttempts = 0
			u.LastLoginInfo.Time = time.Now().Format(time.DateOnly)
		}); err != nil {
			c.log.Error("UpdateUser", "err", err, "uid", u.ID)
		}
	}(u.ID)

	return &u, nil
}

// CheckCaptcha 函数用于检查验证码是否正确
func (c Core) CheckCaptcha(id int, captcha string) (bool, error) {
	var v Vcode
	if err := c.Store.GetCaptchaByID(&v, id); errors.Is(err, orm.ErrRevordNotFound) {
		return false, web.ErrCaptchaWrong.Msg("验证码不存在")
	} else if err != nil {
		return false, web.ErrDB.With(err.Error())
	}
	if v.UsedAt != nil {
		return false, web.ErrCaptchaWrong.Msg("验证码已被使用")
	}
	if time.Now().After(v.ExpiresAt) {
		return false, web.ErrCaptchaWrong.Msg("验证码已过期")
	}
	if captcha == v.Answer {
		return true, nil
	}

	c1 := strings.Split(captcha, ",")
	c2 := strings.Split(v.Answer, ",")
	if len(c1) != len(c2) {
		return false, web.ErrCaptchaWrong.Msg("验证码长度不匹配")
	}

	for i := range 2 {
		a1, err := strconv.Atoi(c1[i])
		if err != nil {
			return false, web.ErrCaptchaWrong.Msg("不支持的验证码")
		}
		a2, err := strconv.Atoi(c2[i])
		if err != nil {
			return false, web.ErrCaptchaWrong.Msg("不支持的验证码")
		}

		if math.Abs(float64(a2-a1)) > 4 {
			return false, web.ErrCaptchaWrong.Msg("验证码不匹配")
		}
	}

	return true, nil
}
