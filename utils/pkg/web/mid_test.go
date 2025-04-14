package web

import (
	"io"
	"net/http"
	"net/http/httptest"
	"os"
	"testing"
	"time"

	"github.com/gin-gonic/gin"
)

func TestLimiter(t *testing.T) {
	r := gin.New()
	r.Use(IPRateLimiterForGin(2, 4))
	r.GET("/", func(ctx *gin.Context) {
		ctx.String(200, "OK")
	})

	for i := 0; i < 10; i++ {
		w := httptest.NewRecorder()
		req := httptest.NewRequest(http.MethodGet, "/", nil)
		r.ServeHTTP(w, req)
		_, _ = io.Copy(os.Stdout, w.Body)
		if i == 5 {
			time.Sleep(2 * time.Second)
		}
	}
}

func BenchmarkResponse(b *testing.B) {
	r := gin.New()
	r.Use(RecordResponse())
	r.GET("/", func(ctx *gin.Context) {
		ctx.String(200, "OK")
	})
	b.Run("test", func(b *testing.B) {
		for i := 0; i < b.N; i++ {
			w := httptest.NewRecorder()
			req := httptest.NewRequest(http.MethodGet, "/", nil)
			r.ServeHTTP(w, req)
		}
	})
}
