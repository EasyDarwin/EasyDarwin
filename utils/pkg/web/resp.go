package web

import (
	"bytes"
	"sync"

	"github.com/gin-gonic/gin"
)

type ResponseWriterWrapper struct {
	gin.ResponseWriter
	Body *bytes.Buffer // 缓存
}

func (w ResponseWriterWrapper) Write(b []byte) (int, error) {
	w.Body.Write(b)
	return w.ResponseWriter.Write(b)
}

func (w ResponseWriterWrapper) WriteString(s string) (int, error) {
	w.Body.WriteString(s)
	return w.ResponseWriter.WriteString(s)
}

func RecordResponse() gin.HandlerFunc {
	pool := sync.Pool{New: func() any {
		return bytes.NewBuffer(make([]byte, 0, 50))
	}}

	return func(c *gin.Context) {
		b := pool.Get().(*bytes.Buffer)
		b.Reset()
		c.Writer = &ResponseWriterWrapper{
			Body:           b,
			ResponseWriter: c.Writer,
		}
		c.Next()
		// fmt.Println(b.String())

		if b.Len() <= 1024*5 {
			pool.Put(b)
		}
	}
}

func AddHead() gin.HandlerFunc {
	return func(c *gin.Context) {
		// https://developer.mozilla.org/en-US/docs/Web/HTTP/Headers/Vary
		// 这表示任何缓存响应可能因请求头中 Authorization 而异
		c.Header("Vary", "Authorization")
		c.Next()
	}
}
