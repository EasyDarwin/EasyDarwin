package api

import (
	"net/http"
	"net/http/httptest"
	"strings"
	"testing"
	"time"

	"easydarwin/utils/pkg/web"

	"github.com/gin-gonic/gin"
)

// TestAuthForceChangePassword 服务端强制改密拦截（CNVD-2026-42897）
// 默认口令 token 仅允许修改本人密码，其余接口一律 403，防止绕过前端弹窗直调 API
func TestAuthForceChangePassword(t *testing.T) {
	gin.SetMode(gin.TestMode)
	const secret = "test_secret_key"

	newRouter := func() *gin.Engine {
		r := gin.New()
		g := r.Group("/api/v1")
		g.Use(authWithWhitelist(secret))
		ok := func(c *gin.Context) { c.JSON(http.StatusOK, gin.H{"uid": web.GetUID(c)}) }
		g.GET("/live", ok)
		g.PUT("/users/:username/reset-password", ok)
		g.POST("/logout", ok)
		g.GET("/vod/get", ok)
		return r
	}

	newToken := func(username string, needChange bool) string {
		tok, err := web.NewToken(web.TokenInput{
			UID: 1, Username: username, Role: "admin", Secret: secret,
			Exires: time.Hour, NeedChangePwd: needChange,
		})
		if err != nil {
			t.Fatalf("NewToken: %v", err)
		}
		return tok
	}
	forcedToken := newToken("admin", true)
	normalToken := newToken("admin", false)

	cases := []struct {
		name     string
		method   string
		path     string
		token    string
		wantCode int
		wantBody string // 非空时断言响应包含该子串
	}{
		{"默认口令访问管理接口被拒绝", http.MethodGet, "/api/v1/live", forcedToken, http.StatusForbidden, "请先修改默认口令"},
		{"默认口令修改本人密码放行", http.MethodPut, "/api/v1/users/admin/reset-password", forcedToken, http.StatusOK, `"uid":1`},
		{"默认口令修改他人密码被拒绝", http.MethodPut, "/api/v1/users/other/reset-password", forcedToken, http.StatusForbidden, ""},
		{"默认口令登出放行（白名单）", http.MethodPost, "/api/v1/logout", forcedToken, http.StatusOK, ""},
		{"正常口令访问管理接口放行", http.MethodGet, "/api/v1/live", normalToken, http.StatusOK, `"uid":1`},
		{"正常口令修改他人密码放行（admin）", http.MethodPut, "/api/v1/users/other/reset-password", normalToken, http.StatusOK, ""},
		{"无 token 访问管理接口 401", http.MethodGet, "/api/v1/live", "", http.StatusUnauthorized, ""},
	}
	for _, tc := range cases {
		t.Run(tc.name, func(t *testing.T) {
			req := httptest.NewRequest(tc.method, tc.path, nil)
			if tc.token != "" {
				req.Header.Set("Authorization", "Bearer "+tc.token)
			}
			w := httptest.NewRecorder()
			newRouter().ServeHTTP(w, req)
			if w.Code != tc.wantCode {
				t.Fatalf("%s %s 状态码 = %d, 期望 %d, body=%s", tc.method, tc.path, w.Code, tc.wantCode, w.Body.String())
			}
			if tc.wantBody != "" && !strings.Contains(w.Body.String(), tc.wantBody) {
				t.Fatalf("%s %s 响应 = %s, 期望包含 %s", tc.method, tc.path, w.Body.String(), tc.wantBody)
			}
		})
	}
}

// TestResetPasswordRejectDefault 禁止将新密码改回默认口令
func TestResetPasswordRejectDefault(t *testing.T) {
	// sha256("admin") 的十六进制，即默认口令哈希
	const defaultHash = "8c6976e5b5410415bde908bd4dee15dfb167a9c873fc4bb8a81f6f2ab448a918"
	if err := ResetPassword("admin", "old", defaultHash); err == nil {
		t.Fatal("新密码为默认口令时应返回错误")
	}
	if err := ResetPassword("admin", "old", "short"); err == nil {
		t.Fatal("新密码过短时应返回错误")
	}
}
