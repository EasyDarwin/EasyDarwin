// Package api Copyright 2025 EasyDarwin.
// http://www.easydarwin.org
// 管理接口鉴权
// 修复 CNVD-2026-42897 / CNVD-2026-42898 未授权访问漏洞：
// /api/v1 路由组统一强制 JWT 鉴权，仅白名单接口允许匿名访问
package api

import (
	"easydarwin/utils/pkg/web"
	"net"
	"net/http"
	"strings"

	"github.com/gin-gonic/gin"
)

// authWithWhitelist /api/v1 统一鉴权中间件
// 白名单：登录/登出、分享页点播查询（处理函数内限制仅返回分享中的视频）
// 流媒体内部回调（on_pub_start 等）仅允许本机回环地址访问
func authWithWhitelist(secret string) gin.HandlerFunc {
	auth := web.AuthMiddleware(secret)
	optionalAuth := web.OptionalAuthMiddleware(secret)
	return func(c *gin.Context) {
		switch c.FullPath() {
		case "/api/v1/login", "/api/v1/logout":
			c.Next()
			return
		case "/api/v1/vod/get":
			// 分享播放页 share.html 匿名访问，get 处理函数内校验分享状态
			// 可选鉴权：携带合法 token 时注入身份，避免登录用户访问自有非分享视频被误判为匿名而 403
			optionalAuth(c)
			return
		case "/api/v1/push/on_pub_start", "/api/v1/push/on_pub_stop", "/api/v1/push/on_rtmp_connect":
			// 内部回调由流媒体服务从 127.0.0.1 发起
			// 使用 RemoteAddr 而非 ClientIP，防止 X-Forwarded-For 伪造绕过
			if isLoopbackRemote(c.Request.RemoteAddr) {
				c.Next()
				return
			}
			c.AbortWithStatusJSON(http.StatusUnauthorized, gin.H{"msg": "无权访问"})
			return
		}
		// 服务端强制改密（CNVD-2026-42897）：仍在使用默认口令的账号
		// 仅允许修改本人密码，其余管理接口一律拒绝，防止绕过前端弹窗直接调用 API
		// 必须在 auth(c) 之前判定：AuthMiddleware 内部会 c.Next() 执行处理函数，事后拦截为时已晚
		if claims := parseBearerClaims(c, secret); claims != nil && claims.NeedChangePwd {
			if c.FullPath() == "/api/v1/users/:username/reset-password" && c.Param("username") == claims.Username {
				// 走正常鉴权注入身份，resetPassword 依赖 GetUsername/GetRole 校验
				auth(c)
				return
			}
			c.AbortWithStatusJSON(http.StatusForbidden, gin.H{"msg": "请先修改默认口令"})
			return
		}
		auth(c)
	}
}

// parseBearerClaims 解析请求中的 Bearer token，缺失或非法时返回 nil
func parseBearerClaims(c *gin.Context, secret string) *web.Claims {
	auth := c.Request.Header.Get("Authorization")
	const prefix = "Bearer "
	if len(auth) <= len(prefix) || !strings.EqualFold(auth[:len(prefix)], prefix) {
		return nil
	}
	claims, err := web.ParseToken(auth[len(prefix):], secret)
	if err != nil || claims.Valid() != nil {
		return nil
	}
	return claims
}

// isLoopbackRemote 判断 TCP 连接对端是否为本机回环地址
func isLoopbackRemote(remoteAddr string) bool {
	host, _, err := net.SplitHostPort(remoteAddr)
	if err != nil {
		host = remoteAddr
	}
	ip := net.ParseIP(host)
	return ip != nil && ip.IsLoopback()
}
