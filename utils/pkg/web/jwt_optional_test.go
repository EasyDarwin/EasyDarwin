package web

import (
	"fmt"
	"net/http"
	"net/http/httptest"
	"testing"
	"time"

	"github.com/gin-gonic/gin"
	"github.com/golang-jwt/jwt/v4"
)

// TestOptionalAuthMiddleware 可选鉴权：合法 token 注入身份，无 token / 非法 token 按匿名放行，永不 Abort
func TestOptionalAuthMiddleware(t *testing.T) {
	gin.SetMode(gin.TestMode)
	const secret = "test_secret_key"

	newRouter := func() *gin.Engine {
		r := gin.New()
		r.GET("/vod/get", OptionalAuthMiddleware(secret), func(c *gin.Context) {
			c.JSON(http.StatusOK, gin.H{"uid": GetUID(c), "username": GetUsername(c)})
		})
		return r
	}

	validToken, err := NewToken(TokenInput{UID: 7, Username: "admin", Secret: secret, Exires: time.Hour})
	if err != nil {
		t.Fatalf("NewToken: %v", err)
	}
	// NewToken 会将非正时长回落为默认 2h，过期 token 需手工签发
	expiredClaims := Claims{
		UID:      7,
		Username: "admin",
		RegisteredClaims: jwt.RegisteredClaims{
			ExpiresAt: jwt.NewNumericDate(time.Now().Add(-time.Hour)),
			IssuedAt:  jwt.NewNumericDate(time.Now().Add(-2 * time.Hour)),
		},
	}
	expiredToken, err := jwt.NewWithClaims(jwt.SigningMethodHS256, expiredClaims).SignedString([]byte(secret))
	if err != nil {
		t.Fatalf("sign expired token: %v", err)
	}

	cases := []struct {
		name     string
		header   string
		wantCode int
		wantUID  int
	}{
		{"无 token 匿名放行", "", http.StatusOK, 0},
		{"合法 token 注入身份", "Bearer " + validToken, http.StatusOK, 7},
		{"非法 token 按匿名放行", "Bearer not.a.jwt", http.StatusOK, 0},
		{"过期 token 按匿名放行", "Bearer " + expiredToken, http.StatusOK, 0},
		{"错误密钥签发的 token 按匿名放行", "Bearer " + mustToken(t, "other_secret"), http.StatusOK, 0},
		{"非 Bearer 前缀按匿名放行", "Basic dXNlcjpwYXNz", http.StatusOK, 0},
	}
	for _, tc := range cases {
		t.Run(tc.name, func(t *testing.T) {
			req := httptest.NewRequest(http.MethodGet, "/vod/get", nil)
			if tc.header != "" {
				req.Header.Set("Authorization", tc.header)
			}
			w := httptest.NewRecorder()
			newRouter().ServeHTTP(w, req)
			if w.Code != tc.wantCode {
				t.Fatalf("状态码 = %d, 期望 %d", w.Code, tc.wantCode)
			}
			want := fmt.Sprintf(`{"uid":%d`, tc.wantUID)
			if body := w.Body.String(); len(body) < len(want) || body[:len(want)] != want {
				t.Fatalf("响应 = %s, 期望 uid=%d", body, tc.wantUID)
			}
		})
	}
}

func mustToken(t *testing.T, secret string) string {
	t.Helper()
	tok, err := NewToken(TokenInput{UID: 9, Username: "attacker", Secret: secret, Exires: time.Hour})
	if err != nil {
		t.Fatalf("NewToken: %v", err)
	}
	return tok
}