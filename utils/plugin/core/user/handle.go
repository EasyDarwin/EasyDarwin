package user

import (
	"log/slog"
	"strings"
	"unicode"

	"easydarwin/utils/pkg/web"
)

// 记录重复的模型

// BatchHandleOutput 批量处理的输出
type BatchHandleOutput[T int | string] struct {
	Success int              `json:"success"`
	Failure int              `json:"failure"`
	Result  []BatchHandle[T] `json:"result"`
}

// BatchHandle 单项的信息
type BatchHandle[T int | string] struct {
	ID    T      `json:"id"`
	Error string `json:"error,omitempty"`
}

// ToSuccess 获取所有操作成功的 id
func (b *BatchHandleOutput[T]) ToSuccess() []T {
	out := make([]T, 0, 5)
	for _, v := range b.Result {
		if v.Error == "" {
			out = append(out, v.ID)
		}
	}
	return out
}

// DoBatchHandle 通用批处理函数
func DoBatchHandle[T int | string](out *BatchHandleOutput[T], id T, fn func() error) {
	success := 1
	var failure int
	var errMsg string
	if err := fn(); err != nil {
		success = 0
		failure = 1
		slog.Error("批量操作失败", "err", err)
		errMsg = web.Message(err)
	}
	out.Success += success
	out.Failure += failure
	out.Result = append(out.Result, BatchHandle[T]{
		ID:    id,
		Error: errMsg,
	})
}

// Options 权限选项
type Options struct {
	CanModify bool `json:"can_modify"`
	CanDelete bool `json:"can_delete"`
}

// CheckName 检查名称是否符合规则
func CheckName(name string) (string, error) {
	name = strings.TrimSpace(name)
	if name == "" {
		return "", web.ErrBadRequest.Msg("名称必填")
	}
	for _, char := range name {
		if char == '_' {
			continue
		}
		if unicode.IsSpace(char) || unicode.IsPunct(char) {
			return "", web.ErrBadRequest.Msg("名称不能包含特殊字符").Withf("name[%s] contain[%c]", name, char)
		}
	}
	return name, nil
}
