package m3u8mannager

import (
	"log/slog"
	"path/filepath"
	"strconv"
	"strings"

	"github.com/grafov/m3u8"
)

const offLineTime = 2 * 1000 // offLineTime 表示两个ts之间的间隔阈值，超过这个值就认为两个ts不连续

func getExtIndex(s string) int {
	for _, v := range [...]string{".mp4", ".ts", ".m4s"} {
		if i := strings.Index(s, v); i != -1 {
			return i
		}
	}
	return -1
}

// GeneranM3u8 生成 m3u8
func GeneranM3u8(datas []string) (data []byte, err error) {
	l := uint(len(datas))
	// 创建m3u8播放对象
	p, e := m3u8.NewMediaPlaylist(l, l)
	if e != nil {
		return nil, err
	}
	p.SetVersion(7)
	// 存储上一个ts的开始时间和结束时间
	var lastStartAt, LastDuration int64

	var lastItem string
	for i, item := range datas {
		idx := getExtIndex(item)
		if idx == -1 {
			continue
		}
		path := item[:idx]
		baseName := path[strings.LastIndex(path, "/")+1:]
		s := strings.Split(baseName, "-")
		if len(s) < 2 {
			continue
		}
		startAt, duration, err := convertTime(s[0], s[1])
		if err != nil {
			continue
		}
		_ = p.Append(item, float64(duration)/1000, "")

		// 支持同时包含 mp4/ts 两种文件
		v1 := parseExt(item)
		v2 := parseExt(lastItem)
		// 在不连续的地方加入标记
		// 当前起始时间 - (上个ts起始时间 + 上个 ts 结束时间）= 间隔时间
		if (i > 0 && startAt-(lastStartAt+LastDuration) > offLineTime) || (lastItem != "" && v1 != v2) {
			_ = p.SetDiscontinuity()
		}
		// 更新开始时间和结束时间供下次查询
		lastStartAt, LastDuration = startAt, duration
		lastItem = item
	}
	p.Close()
	data = p.Encode().Bytes()
	return
}

func convertTime(startTime, durationTime string) (int64, int64, error) {
	startTimeInt, err := strconv.ParseInt(startTime, 10, 64)
	if err != nil {
		slog.Error("strconv.ParseInt failed,GeneranM3u8(datas)", "err", err)
		return 0, 0, err
	}
	durationTimeInt, err := strconv.ParseInt(durationTime, 10, 64)
	if err != nil {
		slog.Error("strconv.ParseInt failed,GeneranM3u8(datas)", "err", err)
		return 0, 0, err
	}
	return startTimeInt, durationTimeInt, nil
}

func parseExt(str string) string {
	idx := strings.Index(str, "?")
	if idx != -1 {
		str = str[:idx]
	}
	return filepath.Ext(str)
}
