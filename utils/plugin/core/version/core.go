package version

import (
	"fmt"
	"strings"
)

// Storer ...
type Storer interface {
	First(*Version) error
	Add(*Version) error
}

// Core ...
type Core struct {
	Storer    Storer
	IsMigrate *bool
}

// NewCore ...
func NewCore(store Storer) *Core {
	return &Core{
		Storer: store,
	}
}

// IsAutoMigrate 是否需要进行表迁移?
func (c *Core) IsAutoMigrate(currentVer, remark string) bool {
	var ver Version
	if err := c.Storer.First(&ver); err != nil {
		isMigrate := true
		c.IsMigrate = &isMigrate
		return isMigrate
	}
	isMigrate := compareVersionFunc(currentVer, ver.Version, func(a, b string) bool {
		return a > b
	})
	c.IsMigrate = &isMigrate
	return isMigrate
}

// RecordVersion 记录当前版本号
func (c Core) RecordVersion(currentVer, remark string) error {
	var ver Version
	ver.Version = currentVer
	ver.Remark = remark
	return c.Storer.Add(&ver)
}

func compareVersionFunc(a, b string, f func(a, b string) bool) bool {
	s1 := versionToStr(a)
	s2 := versionToStr(b)
	if len(s1) != len(s2) {
		return true
	}
	return f(s1, s2)
}

func versionToStr(str string) string {
	var result strings.Builder
	arr := strings.Split(str, ".")
	for _, item := range arr {
		if idx := strings.Index(item, "-"); idx != -1 {
			item = item[0:idx]
		}
		result.WriteString(fmt.Sprintf("%03s", item))
	}
	return result.String()
}
