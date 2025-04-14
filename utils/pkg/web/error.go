// 自定义错误

package web

import (
	"encoding/json"
	"errors"
	"fmt"
	"io"
	"net/http"
	"strings"
	// kerr "github.com/go-kratos/kratos/v2/errors"
)

// 常用错误
var (
	ErrUnknown           = NewError("UnKnow", "未知错误")
	ErrBadRequest        = NewError("ErrBadRequest", "请求参数有误")
	ErrDB                = NewError("ErrStore", "数据发生错误")
	ErrServer            = NewError("ErrServer", "服务器发生错误")
	ErrUnauthorizedToken = NewError("ErrUnauthorizedToken", "用户已过期或错误")
	ErrJSON              = NewError("ErrUnmarshal", "JSON 编解码出错")
	ErrNotFound          = NewError("ErrNotFound", "资源未找到")
	ErrUsedLogic         = NewError("ErrUsedLogic", "使用逻辑错误")
	ErrLoginLimiter      = NewError("ErrLoginLimiter", "触发登录限制")
	ErrPermissionDenied  = NewError("ErrPermissionDenied", "没有该资源的权限")
	ErrTimeout           = NewError("ErrTimeout", "请求超时")
	ErrDevice            = NewError("ErrDevice", "设备异常")
	ErrDeviceOffline     = NewError("ErrDeviceOffline", "设备离线")

	ErrAddNewDevice = NewError("ErrAddNewDevice", "请重新添加此设备") // 待删除
)

// 业务错误
var (
	ErrNameOrPasswd    = NewError("ErrNameOrPasswd", "用户名或密码错误")
	ErrCaptchaWrong    = NewError("ErrCaptchaWrong", "验证码错误")
	ErrAccountDisabled = NewError("ErrAccountDisabled", "登录限制")
)

var _ error = NewError("test_new_error", "")

// Error ...
type Error struct {
	reason  string   // 错误原因
	msg     string   // 错误信息，用户可读
	details []string // 错误扩展，开发可读
}

func (e Error) Error() string {
	var msg strings.Builder
	msg.WriteString(e.msg)
	for _, v := range e.details {
		msg.WriteString(";" + v)
	}
	return msg.String()
}

// E 可反序列化的 err
type E struct {
	Reason  string   `json:"reason"`
	Msg     string   `json:"msg"`
	Details []string `json:"details"`
}

func (e E) String() string {
	var msg strings.Builder
	msg.WriteString(e.Msg)
	for _, v := range e.Details {
		msg.WriteString(";" + v)
	}
	return msg.String()
}

// Unmarshal ...
func Unmarshal(b []byte) (e E) {
	_ = json.Unmarshal(b, &e)
	return
}

var codes = make(map[string]string, 8)

// NewError 创建自定义错误
func NewError(reason, msg string) *Error {
	if _, ok := codes[reason]; ok {
		panic(fmt.Sprintf("错误码 %s 已经存在，请更换一个", reason))
	}
	codes[reason] = msg
	return &Error{reason: reason, msg: msg}
}

// Unwrap 出现 err 套 err 的情况时
func (e *Error) Unwrap(err error) *Error {
	e1, ok := err.(*Error)
	if ok {
		return e1
	}
	return e
}

// Reason ..
func (e *Error) Reason() string {
	return e.reason
}

// Message ..
func (e *Error) Message() string {
	return e.msg
}

// Details 错误
func (e *Error) Details() []string {
	return e.details
}

// Map ..
func (e *Error) Map() map[string]any {
	return map[string]any{
		"msg":    e.Message(),
		"reason": e.Reason(),
	}
}

// With 错误详情
func (e *Error) With(args ...string) *Error {
	newErr := *e
	newErr.details = make([]string, 0, len(args)+len(e.details))
	newErr.details = append(append(newErr.details, e.details...), args...)
	return &newErr
}

// Withf 错误详情格式化
func (e *Error) Withf(format string, args ...any) *Error {
	newErr := *e
	newErr.details = make([]string, 0, len(e.details)+1)
	newErr.details = append(append(newErr.details, e.details...), fmt.Sprintf(format, args...))
	return &newErr
}

// Msg 提示内容
func (e *Error) Msg(s string) *Error {
	newErr := *e
	newErr.msg = s
	return &newErr
}

// HTTPCode http status code
// 权限相关错误 401
// 程序错误 500
// 其它错误 400
func (e *Error) HTTPCode() int {
	switch e.reason {
	case "":
		return http.StatusOK
	case ErrUnauthorizedToken.reason:
		return http.StatusUnauthorized
	}
	return http.StatusBadRequest
}

func HanddleJSONErr(err error) error {
	if err == nil {
		return nil
	}

	var syntaxError *json.SyntaxError
	var unmarshalTypeError *json.UnmarshalTypeError
	var invalidUnmarshalError *json.InvalidUnmarshalError

	switch {
	case errors.As(err, &syntaxError):
		return fmt.Errorf("格式错误 (位于 %d)", syntaxError.Offset)
	case errors.Is(err, io.ErrUnexpectedEOF):
		return fmt.Errorf("格式错误")
	case errors.As(err, &unmarshalTypeError):
		if unmarshalTypeError.Field != "" {
			return fmt.Errorf("正文包含不正确的格式类型 %q", unmarshalTypeError.Field)
		}
		return fmt.Errorf("正文包含不正确的格式类型 (位于 %d)", unmarshalTypeError.Offset)
	case errors.Is(err, io.EOF):
		return errors.New("正文不能为空")
	case errors.As(err, &invalidUnmarshalError):
		panic(err)
	default:
		return err
	}
}

// Message 获取错误消息
func Message(err error) string {
	if err == nil {
		return ""
	}
	// v, ok := err.(*kerr.Error)
	// if ok {
	// 	return v.Message
	// }
	v1, ok := err.(Errorer)
	if ok {
		return v1.Message()
	}
	return err.Error()
}
