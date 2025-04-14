package userdb

import (
	"time"

	"easydarwin/utils/plugin/core/user"
)

// ClearVcode implements log.Storer.
// 清除验证码
func (d DB) ClearVcode(expire time.Duration) error {
	// 查询创建时间早于当前时间减去过期时间的记录，并删除
	return d.db.Where("created_at < ?", time.Now().Add(-expire)).Delete(&user.Vcode{}).Error
}
