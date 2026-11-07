package web

import (
	"fmt"
	"strings"
	"time"

	"github.com/gin-gonic/gin"
	"github.com/golang-jwt/jwt/v4"
)

// Claims ...
type Claims struct {
	UID        int
	Username   string
	GroupID    int
	GroupLevel int8
	Role       string
	Level      int
	// NeedChangePwd 仍在使用默认口令，服务端强制仅允许修改本人密码（CNVD-2026-42897）
	NeedChangePwd bool
	jwt.RegisteredClaims
}

const (
	uid           = "uid"
	token         = "token"
	username      = "username"
	groupLevel    = "group_level"
	role          = "role"
	needChangePwd = "need_change_pwd"
)

// AuthMiddleware 鉴权
func AuthMiddleware(secret string) gin.HandlerFunc {
	// var errResp = gin.H{
	// "msg": "身份验证失败",
	// }
	return func(c *gin.Context) {
		auth := c.Request.Header.Get("Authorization")
		const prefix = "Bearer "
		if len(auth) <= len(prefix) || !strings.EqualFold(auth[:len(prefix)], prefix) {
			AbortWithStatusJSON(c, ErrUnauthorizedToken.Msg("身份验证失败"))
			return
		}
		claims, err := ParseToken(auth[len(prefix):], secret)
		if err != nil {
			AbortWithStatusJSON(c, ErrUnauthorizedToken.Msg("身份验证失败"))
			return
		}
		if err := claims.Valid(); err != nil {
			AbortWithStatusJSON(c, ErrUnauthorizedToken.Msg("身份验证失败"))
			return
		}

		setClaims(c, auth, claims)
		c.Next()
	}
}

// OptionalAuthMiddleware 可选鉴权：携带合法 token 时注入用户身份，
// 无 token 或 token 非法时按匿名继续，永不 Abort
// 用于白名单中允许匿名访问、但登录用户需要身份的接口（如分享页点播查询）
func OptionalAuthMiddleware(secret string) gin.HandlerFunc {
	return func(c *gin.Context) {
		auth := c.Request.Header.Get("Authorization")
		const prefix = "Bearer "
		if len(auth) <= len(prefix) || !strings.EqualFold(auth[:len(prefix)], prefix) {
			c.Next()
			return
		}
		claims, err := ParseToken(auth[len(prefix):], secret)
		if err != nil || claims.Valid() != nil {
			c.Next()
			return
		}
		setClaims(c, auth, claims)
		c.Next()
	}
}

// setClaims 将 token 中的用户身份写入请求上下文
func setClaims(c *gin.Context, auth string, claims *Claims) {
	c.Set(uid, claims.UID)
	c.Set(username, claims.Username)
	c.Set(token, auth)
	c.Set(groupLevel, claims.GroupLevel)
	c.Set(role, claims.Role)
	c.Set("level", claims.Level)
	c.Set(needChangePwd, claims.NeedChangePwd)
}

// GetUID 获取用户 ID
func GetUID(c *gin.Context) int {
	return c.GetInt(uid)
}

// GetUsername 获取用户名
func GetUsername(c *gin.Context) string {
	return c.GetString(username)
}

// GetRole 获取用户角色
func GetRole(c *gin.Context) string {
	return c.GetString(role)
}

// GetNeedChangePwd 获取是否仍在使用默认口令（强制改密标记）
func GetNeedChangePwd(c *gin.Context) bool {
	return c.GetBool(needChangePwd)
}

func GetGroupLevel(c *gin.Context) int8 {
	v, exist := c.Get(groupLevel)
	if exist {
		return v.(int8)
	}
	return 12
}

func AuthLevel(level int) gin.HandlerFunc {
	// 等级从1开始，等级越小，权限越大
	return func(c *gin.Context) {
		l := c.GetInt("level")
		if l > level || l == 0 {
			Fail(c, ErrBadRequest.Msg("权限不足"))
			c.Abort()
			return
		}
		c.Next()
	}
}

// ParseToken 解析 token
func ParseToken(tokenString string, secret string) (*Claims, error) {
	var claims Claims
	token, err := jwt.ParseWithClaims(tokenString, &claims, func(token *jwt.Token) (interface{}, error) {
		return []byte(secret), nil
	})
	if err != nil {
		return nil, err
	}
	if token == nil {
		return nil, fmt.Errorf("解析失败")
	}
	c, ok := token.Claims.(*Claims)
	if !ok {
		return nil, fmt.Errorf("令牌类型错误")
	}
	return c, nil
}

type TokenInput struct {
	UID        int
	GroupID    int
	GroupLevel int8
	Username   string
	Secret     string
	Role       string
	Level      int
	Exires     time.Duration
	// NeedChangePwd 仍在使用默认口令，随 token 下发供服务端强制改密拦截
	NeedChangePwd bool
}

// NewToken 创建 token
func NewToken(input TokenInput) (string, error) {
	if input.Exires <= 0 {
		input.Exires = 2 * time.Hour
	}
	now := time.Now()
	claims := Claims{
		UID:           input.UID,
		Username:      input.Username,
		GroupID:       input.GroupID,
		GroupLevel:    input.GroupLevel,
		Level:         input.Level,
		NeedChangePwd: input.NeedChangePwd,
		RegisteredClaims: jwt.RegisteredClaims{
			ExpiresAt: jwt.NewNumericDate(now.Add(input.Exires)), // 失效时间
			IssuedAt:  jwt.NewNumericDate(now),                   // 签发时间
			Issuer:    "xx@golang.space",                         // 签发人
			// Subject:   "login",                                    // 主题
		},
		Role: input.Role, // 角色
	}
	tokenClaims := jwt.NewWithClaims(jwt.SigningMethodHS256, claims)
	return tokenClaims.SignedString([]byte(input.Secret))
}
