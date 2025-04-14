package web

import (
	"context"

	"github.com/gin-gonic/gin"
)

const traceIDKey = "TRACE_ID_KEY"

func MustTraceID(ctx context.Context) string {
	v := ctx.Value(traceIDKey)
	return v.(string)
}

func TraceID(ctx context.Context) (string, bool) {
	v := ctx.Value(traceIDKey)
	if v == nil {
		return "", false
	}
	return v.(string), true
}

func SetTraceID(ctx *gin.Context, id string) {
	ctx.Set(traceIDKey, id)
}
