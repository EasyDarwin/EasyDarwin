package msgpush

import (
	"context"
	"easydarwin/lnton/pkg/orm"
)

// MsgPushStorer Instantiation interface
type MsgPushStorer interface {
	Find(context.Context, *[]*MsgPush, orm.Pager, ...orm.QueryOption) (int64, error)
	Get(context.Context, *MsgPush, ...orm.QueryOption) error
	Add(context.Context, *MsgPush) error
	Edit(context.Context, *MsgPush, func(*MsgPush), ...orm.QueryOption) error
	Del(context.Context, *MsgPush, ...orm.QueryOption) error
}
