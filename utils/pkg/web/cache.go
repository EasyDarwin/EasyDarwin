package web

import (
	"bytes"
	"crypto/sha1"
	"encoding/hex"
	"log/slog"
	"net/http"
	"strconv"

	"github.com/gin-gonic/gin"
)

type EtagWriter struct {
	gin.ResponseWriter
	body bytes.Buffer
}

func (w *EtagWriter) Unwrap() http.ResponseWriter {
	return w.ResponseWriter
}

func (w *EtagWriter) Write(b []byte) (int, error) {
	return w.body.Write(b)
}

// WebCache 主要用于缓存静态资源
// Cache-Control: max-age=3600    # 缓存1小时
// Cache-Control: no-cache        # 每次都需要验证
// Cache-Control: no-store        # 完全不缓存
// Cache-Control: private         # 只允许浏览器缓存
// Cache-Control: public          # 允许中间代理缓存
func CacheControlMaxAge(millisecond int) gin.HandlerFunc {
	age := strconv.Itoa(millisecond)
	return func(ctx *gin.Context) {
		if ctx.Request.Method == "GET" {
			ctx.Header("Cache-Control", "max-age="+age)
		}
		ctx.Next()
	}
}

func EtagHandler() gin.HandlerFunc {
	return func(ctx *gin.Context) {
		bw := EtagWriter{
			ResponseWriter: ctx.Writer,
		}
		ctx.Writer = &bw
		ctx.Next()

		hash := sha1.New()
		buf := bw.body.Bytes()
		hash.Write(buf)
		etag := `"` + hex.EncodeToString(hash.Sum(nil)) + `"`
		if match := ctx.GetHeader("If-None-Match"); match != "" && match == etag {
			ctx.Writer.WriteHeader(http.StatusNotModified)
			return
		}
		ctx.Header("ETag", etag)
		if _, err := bw.ResponseWriter.Write(buf); err != nil {
			slog.Error("write err", "err", err)
		}
	}
}
