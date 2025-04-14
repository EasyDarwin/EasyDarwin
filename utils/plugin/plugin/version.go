package plugin

import (
	"log/slog"

	"easydarwin/lnton/plugin/core/version"
	"easydarwin/lnton/plugin/core/version/store/versiondb"
	"gorm.io/gorm"
)

// NewVersion ...
func NewVersion(ver, remark string) func(db *gorm.DB) *version.Core {
	return func(db *gorm.DB) *version.Core {
		vdb := versiondb.NewDB(db)
		core := version.NewCore(vdb)
		isOK := core.IsAutoMigrate(ver, remark)
		vdb.AutoMigrate(isOK)
		if isOK {
			slog.Info("更新数据库表结构")
			if err := core.RecordVersion(ver, remark); err != nil {
				slog.Error("RecordVersion", "err", err)
			}
		}
		return core
	}
}
