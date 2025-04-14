package web

import (
	"bytes"
	"io"
	"log/slog"
	"net/http"
	"time"

	"github.com/gin-gonic/gin"
	"github.com/google/uuid"
)

type BufferWriter struct {
	gin.ResponseWriter
	body  bytes.Buffer
	limit int
}

func (w *BufferWriter) Unwrap() http.ResponseWriter {
	return w.ResponseWriter
}

func (w *BufferWriter) Write(b []byte) (int, error) {
	l := len(b)
	if w.limit > 0 && l > w.limit {
		l = w.limit
	}
	w.body.Write(b[:l])
	return w.ResponseWriter.Write(b)
}

// Logger 第二个参数是否记录 请求与响应的 body。
func Logger(log *slog.Logger, recordBodyFn func(*gin.Context) bool) gin.HandlerFunc {
	return func(c *gin.Context) {
		var reqBody string
		var blw BufferWriter

		recordBody := recordBodyFn(c)

		if recordBody {
			// 请求参数
			raw, err := c.GetRawData()
			if err != nil {
				slog.Error("logger", "err", err)
			}
			maxL := len(raw)
			if maxL > 100 {
				maxL = 100
			}
			reqBody = string(raw[:maxL])

			c.Request.Body = io.NopCloser(bytes.NewReader(raw))
			// 响应参数
			blw = BufferWriter{
				ResponseWriter: c.Writer,
				limit:          100,
			}
			c.Writer = &blw
		}

		now := time.Now()
		// traceid := trace.SpanContextFromContext(c.Request.Context()).TraceID().String()
		SetTraceID(c, uuid.NewString())
		c.Next()

		code := c.Writer.Status()
		out := []any{
			"uid", uid,
			"method", c.Request.Method,
			"path", c.Request.URL.Path,
			"query", c.Request.URL.RawQuery,
			"remoteaddr", c.ClientIP(),
			"statuscode", code,
			"since", time.Since(now).Milliseconds(),
			"trace_id", MustTraceID(c),
		}
		if recordBody {
			out = append(out, []any{"request_body", reqBody, "response_body", blw.body.String()}...)
		}
		if code >= 200 && code < 400 {
			log.Info("OK", out...)
			return
		}
		// 约定: 返回给客户端的错误，记录的 key 为 responseErr
		errStr, _ := c.Get("responseErr")
		if !(code == 404 || code == 401) {
			out = append(out, []any{"err", errStr})
		}
		log.Warn("Bad", out...)
	}
}
