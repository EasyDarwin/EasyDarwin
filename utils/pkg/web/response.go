package web

import (
	"context"
	"encoding/json"
	"net/http"
	"unsafe"

	"github.com/gin-gonic/gin"
	// errors "github.com/go-kratos/kratos/v2/errors"
)

var defaultDebug = true

func SetRelease() {
	defaultDebug = false
}

// Errorer ...
type Errorer interface {
	Reason() string
	HTTPCode() int
	Message() string
	Details() []string
}

// ResponseWriter ...
type ResponseWriter interface {
	JSON(code int, obj interface{})
	File(filepath string)
	Set(string, any)
	context.Context
	AbortWithStatusJSON(code int, obj interface{})
}

const responseErr = "responseErr"

type HTTPContext interface {
	JSON(int, any)
	Header(key, value string)
	context.Context
}

// Success 通用成功返回
func Success(c HTTPContext, bean any) {
	c.JSON(http.StatusOK, bean)
}

type WithData func(map[string]any)

// Fail 通用错误返回
func Fail(c ResponseWriter, err error, fn ...WithData) {
	out := make(map[string]any)
	if traceID, ok := TraceID(c); ok {
		out["trace_id"] = traceID
	}

	code := 400

	if err1, ok := err.(Errorer); ok {

		if ok {
			code = err1.HTTPCode()
			out["reason"] = err1.Reason()
			out["msg"] = err1.Message()
			d := err1.Details()
			if defaultDebug && len(d) > 0 {
				out["details"] = d
			}
		}
		for i := range fn {
			fn[i](out)
		}
		c.JSON(code, out)
		c.Set(responseErr, err.Error())
		return
	}

	// if err, ok := err.(*errors.Error); ok {
	// 	out["reason"] = err.Reason
	// 	out["msg"] = err.Message
	// 	d := err.Metadata
	// 	if defaultDebug && len(d) > 0 {
	// 		details := make([]string, 0, 3)
	// 		for k, v := range d {
	// 			details = append(details, fmt.Sprintf("%s:%v", k, v))
	// 		}
	// 		out["details"] = details
	// 	}
	// 	for i := range fn {
	// 		fn[i](out)
	// 	}
	// 	c.JSON(code, out)
	// 	c.Set(responseErr, err.Error())
	// 	return
	// }

	// if errors.Is(err, context.DeadlineExceeded) {
	// 	out["reason"] = "TIMEOUT"
	// 	out["msg"] = "请求超时"
	// 	c.JSON(code, out)
	// 	c.Set(responseErr, err.Error())
	// 	return
	// }

	c.JSON(code, out)
	c.Set(responseErr, err.Error())
}

func AbortWithStatusJSON(c ResponseWriter, err error, fn ...WithData) {
	out := make(map[string]any)

	err1, ok := err.(Errorer)

	var code int
	if ok {
		code = err1.HTTPCode()
		out["reason"] = err1.Reason()
		out["msg"] = err1.Message()
		d := err1.Details()
		if defaultDebug && len(d) > 0 {
			out["details"] = d
		}
	}
	if traceID, ok := TraceID(c); ok {
		out["trace_id"] = traceID
	}
	for i := range fn {
		fn[i](out)
	}
	c.AbortWithStatusJSON(code, out)
	c.Set(responseErr, err.Error())
}

// WarpH 让函数更专注于业务，一般入参和出参应该是指针类型
// 没有入参时，应该使用 struct{}
func WarpH[I any, O any](fn func(*gin.Context, *I) (O, error)) gin.HandlerFunc {
	return func(c *gin.Context) {
		var in I
		if unsafe.Sizeof(in) != 0 {
			switch c.Request.Method {
			case http.MethodGet:
				if err := c.ShouldBindQuery(&in); err != nil {
					Fail(c, ErrBadRequest.With(HanddleJSONErr(err).Error()))
					return
				}
			case http.MethodPost, http.MethodPut, http.MethodDelete, http.MethodPatch:
				if c.Request.ContentLength > 0 {
					if err := c.ShouldBindJSON(&in); err != nil {
						Fail(c, ErrBadRequest.With(HanddleJSONErr(err).Error()))
						return
					}
				}
			}
		}
		out, err := fn(c, &in)
		if err != nil {
			Fail(c, err)
			return
		}
		Success(c, out)
	}
}

type ResponseMsg struct {
	Msg string `json:"msg"`
}

// HandlerResponseMsg 获取响应的结果
func HandlerResponseMsg(resp http.Response) error {
	if resp.StatusCode == 200 {
		return nil
	}
	var out ResponseMsg
	if err := json.NewDecoder(resp.Body).Decode(&out); err != nil {
		return ErrServer.Msg(out.Msg)
	}
	return ErrServer.Msg(resp.Status)
}
