package tvs

import (
	"fmt"
	"log/slog"
	"strings"

	"easydarwin/lnton/pkg/web"
)

type Storer interface {
	AddChannelGroups(in *GroupedTVs) error
	DeleteAllOldChannels() error
	FindWalls(bs *[]*GroupedTVs) (int64, error)
	ScanDBGetChannels(channels *[]string) error
}

// Core 业务核心
type Core struct {
	storer Storer
	log    *slog.Logger
}

// NewCore 创建业务实体
func NewCore(storer Storer, log *slog.Logger) Core {
	return Core{storer: storer, log: log}
}

// AddChannels 根据获取的channels分组存储到tvs表中
func (c *Core) AddChannels(num int) error {
	// 获取所有通道
	var channels []string
	if err := c.storer.ScanDBGetChannels(&channels); err != nil {
		return web.ErrDB.Withf(`err[%s] := c.storer.ScanDBGetChannels(&lists)`, err)
	}
	// 电视墙表内的数据
	if err := c.storer.DeleteAllOldChannels(); err != nil {
		return web.ErrDB.Withf(`err[%s] := c.storer.DeleteAllOldChannels()`, err)
	}
	// arr := make([]*Channels, 0, num)
	cnt := 1
	var group []string
	for i, v := range channels {
		group = append(group, v)
		if (i+1)%num == 0 || i+1 >= len(channels) {
			tvs := GroupedTVs{
				Name:     fmt.Sprintf("%d", cnt),
				Channels: strings.Join(group, ","),
			}
			cnt++
			if err := c.storer.AddChannelGroups(&tvs); err != nil {
				return web.ErrDB.Withf(`err[%s] :=  c.storer.AddChannelGroups(&tvs)`, err)
			}
			group = group[0:0]
		}
	}
	return nil
}

// FindWalls 查询所有channels组
func (c *Core) FindWalls() ([]*GroupedTVs, int64, error) {
	// if err := c.AddChannels(num); err != nil {
	// 	return nil, 0, web.ErrDB.Withf("err[%s]:= c.AddChannels(num)", err)
	// }
	out := make([]*GroupedTVs, 0)
	total, err := c.storer.FindWalls(&out)
	if err != nil {
		return nil, 0, web.ErrDB.Withf(`err[%s] := c.storer.FindWalls(out)`, err)
	}
	return out, total, nil
}
