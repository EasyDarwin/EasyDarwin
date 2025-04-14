package cloud

import (
	"fmt"
	"log/slog"
	"os"
	"path/filepath"
	"strconv"
	"strings"
	"time"

	"easydarwin/lnton/pkg/fn"
	"github.com/grafov/m3u8"
)

type File struct{}

// // DateRange 返回给定日期范围内的日期切片
// func DateRange(startDate, endDate time.Time) []string {
// 	var dateSlice []string
// 	if endDate.Before(startDate) {
// 		startDate, endDate = endDate, startDate
// 	}

// 	// 云存文件的位置是时按进行分割的
// 	// currentDate 是起始时间所在天的起始时间
// 	currentDate := time.Date(startDate.Year(), startDate.Month(), startDate.Day(), 0, 0, 0, 0, time.Local)
// 	for !currentDate.After(endDate) {
// 		stringcurrentDate := currentDate.Format("20060102")
// 		dateSlice = append(dateSlice, stringcurrentDate)
// 		currentDate = currentDate.AddDate(0, 0, 1)
// 	}

// 	return dateSlice
// }

// GetTsListByDay 获取指定时间范围内的ts切片
func (f File) GetTsListByDay(in DeviceParamsInput, startMs, endMs int64) ([]string, error) {
	out := make([]string, 0, 8)

	// 将时间戳转换为时间格式
	startTime := time.UnixMilli(startMs)
	endTime := time.UnixMilli(endMs)

	// 获取时间段内每天的切片
	dateRange := DateRange(startTime, endTime)

	// 存储各级目录参数
	pathDir := append([]string{}, in.PrefixDir, in.DeviceID, in.ChannelID)

	//// 获取30秒前的时间
	//pastTime := startTime.Add(-30 * time.Second)
	////获取30秒前的日期
	//pastTimeStr := pastTime.Format("20060102")
	//// 判断录像是否跨天了，是则查询昨天的最后一段录像
	//if pastTimeStr != dateRange[0] {
	//	file, err := s.getYesterdayFile(startTime, pastTimeStr, pathDir, dateRange)
	//	if err != nil {
	//		return nil, err
	//	}
	//	lists = append(lists, file...)
	//}

	// 遍历切片内每天的文件
	for _, v := range dateRange {
		paths := pathDir
		paths = append(paths, v)

		prefix := filepath.Join(in.DeviceID, in.ChannelID, v) + "/"

		prefixPathURL := strings.Join(paths, "/") + "/"

		dirList, _ := os.ReadDir(prefixPathURL)
		// if err != nil {
		// return nil, err
		// }
		flag := true

		// 筛选符合条件的
		for _, path := range dirList {
			if len(path.Name()) < 13 {
				continue
			}

			// 通过“-”进行分割，获取起始时间
			arr := strings.Split(path.Name(), "-")
			if len(arr) != 2 {
				slog.Error("分割文件出错", "err", fmt.Sprintf("len(arr) != 2 %s", path))
				continue
			}

			startString := arr[0][strings.LastIndex(arr[0], "/")+1:]
			timestamp, err := parseStart(startString)
			if err != nil {
				slog.Error("解析时间出错", "err", err)
				continue
			}

			// 获取跨天的ts，flag执行一次后会变成false
			// 假如放在外层执行不知道ts开始时间,要求开始时间为00:00:25秒时,会获取昨天一个ts
			// 实际00:00:06有ts,满足条件,导致两个ts都会被添加
			if flag {
				// 获取30秒前的时间
				pastTime := startTime.Add(-30 * time.Second)
				////获取30秒前的日期
				pastTimeStr := pastTime.Format("20060102")
				// 判断录像是否跨天了，是则查询昨天的最后一段录像
				if pastTimeStr != dateRange[0] && startMs < timestamp {
					file, err := f.getYesterdayFile(startTime, pastTimeStr, pathDir, dateRange)
					if err != nil {
						return nil, err
					}
					out = append(out, file...)
				}
				flag = false
			}

			// 获取包含起始时间的ts切片
			// time, _ := strconv.Atoi(fileTime)
			if (timestamp+30000) > startMs && timestamp < startMs {
				out = append(out, prefix+path.Name())
			}

			// 符合条件的ts切片
			if timestamp >= startMs && timestamp <= endMs {
				out = append(out, prefix+path.Name())
			}
		}
	}
	return out, nil
}

func (f File) getYesterdayFile(startTime time.Time, pastTimeStr string, path, dateRange []string) ([]string, error) {
	lists := make([]string, 0, 1)

	// 获取起始时间所在天的0时0分0秒时间
	t, err := time.ParseInLocation("20060102", startTime.Format("20060102"), time.Local)
	if err != nil {
		return nil, err
	}
	timesTampBasic := t.Unix()

	for i := 1; i < 4; i++ {
		timesTampBasic -= int64(10)
		str := strconv.Itoa(int(timesTampBasic))[0:9]
		paths := path
		paths = append(paths, pastTimeStr, str)
		PrefixPathURL := strings.Join(paths, "/")

		out, _ := os.ReadDir(PrefixPathURL)
		// if err != nil {
		// return nil, nil
		// }

		if len(out) < 1 {
			continue
		}
		// 拿获取到的最后一个
		lastFile := out[len(out)-1]

		if len(lastFile.Name()) < 13 {
			continue
		}

		filename := lastFile.Name() // filepath.Base(lastFile.)
		var dura, timestamp int64
		{
			arr := strings.Split(filename, "-")
			if len(arr) != 2 {
				slog.Error("分割文件出错", "err", fmt.Sprintf("len(arr) != 2 %s", filename))
				continue
			}
			timestamp, _ = strconv.ParseInt(arr[0], 10, 64)
			{
				arr := strings.Split(arr[1], ".")
				if len(arr) != 2 {
					slog.Error("分割文件出错", "err", fmt.Sprintf("len(arr) != 2 %s", filename))
					continue
				}
				dura, _ = strconv.ParseInt(arr[0], 10, 64)
			}
		}

		if time.UnixMilli(timestamp+dura).Format("20060102") == dateRange[0] {
			lists = append(lists, lastFile.Name())
		}
		break
	}
	return lists, nil
}

// FindTimeline implements CloudRecorder.
func (f File) FindTimeline(in DeviceParamsInput, startMs int64, endMs int64) ([]FindTimelineOutput, error) {
	if startMs > endMs {
		startMs, endMs = endMs, startMs
	}

	findTimelineOutput := make([]FindTimelineOutput, 0, 8)

	tsList, err := f.GetTsListByDay(in, startMs, endMs)
	if err != nil {
		return nil, err
	}

	//  os.ReadDir(root)

	for _, v := range tsList {
		if !fn.Any([]string{".ts", ".m3u8", "mp4", "m4s"}, func(s string) bool {
			return s == filepath.Ext(v)
		}) {
			continue
		}

		// 获取文件名
		filename := filepath.Base(v)

		var timestamp, dura int64
		// 通过“-”进行分割，获取起始时间
		arr := strings.Split(filename, "-")
		if len(arr) != 2 {
			slog.Error("分割文件出错", "err", fmt.Sprintf("len(arr) != 2 %s", filename))
			continue
		}

		// 解析开始时间
		timestamp, err := parseStart(arr[0])
		if err != nil {
			slog.Error("解析文件起始时间出错", "err", err.Error())
			continue
		}

		// 解析持续时间
		arr = strings.Split(arr[1], ".")
		if len(arr) != 2 {
			slog.Error("分割文件出错", "err", fmt.Sprintf("len(arr) != 2 %s", filename))
			continue
		}
		dura, _ = strconv.ParseInt(arr[0], 10, 64)
		// 封装时间轴数据
		findTimelineOutput = append(findTimelineOutput, FindTimelineOutput{
			Start:    timestamp,
			Duration: dura,
		})
	}
	return findTimelineOutput, nil
}

// type FindTimelineOutput struct {
// 	Start    int64 `json:"start"`    // 开始毫秒
// 	Duration int64 `json:"duration"` // 持续毫秒
// }

// GetM3u8 implements CloudRecorder.
func (f File) GetM3u8(in DeviceParamsInput, startMs int64, endMs int64, expire time.Duration) (string, error) {
	if startMs > endMs {
		startMs, endMs = endMs, startMs
	}
	// 获取指定时间范围内的ts切片
	tsList, err := f.GetTsListByDay(in, startMs, endMs)
	if err != nil {
		return "", err
	}
	// auth, err := s.SetAuthToken(tsList, s.bucket, expire)
	m3u8, err := GeneranM3u8(tsList)
	if err != nil {
		return "", err
	}
	return string(m3u8), nil
}

// var daysInMonth = map[string]int{
// 	"01": 31, "02": 28, "03": 31, "04": 30,
// 	"05": 31, "06": 30, "07": 31, "08": 31,
// 	"09": 30, "10": 31, "11": 30, "12": 31,
// }

// func isLeapYear(year int) bool {
// 	return year%4 == 0 && (year%100 != 0 || year%400 == 0)
// }

// GetRecordByMonth implements CloudRecorder.
func (f File) GetRecordByMonth(in DeviceParamsInput, yyyyMM string) (string, error) {
	if len(yyyyMM) != 6 {
		return "", fmt.Errorf("月份有误")
	}

	root := filepath.Join(in.PrefixDir, in.DeviceID, in.ChannelID)

	dirs, _ := os.ReadDir(root)
	// if err != nil {
	// return "", nil
	// }

	year, err := strconv.Atoi(yyyyMM[0:4])
	if err != nil {
		return "", err
	}

	month := daysInMonth[yyyyMM[4:6]]
	if isLeapYear(year) && yyyyMM[4:6] == "02" {
		month = 29
	}

	var binStr string
	// 遍历当天月份所有天数
	for i := 1; i <= month; i++ {
		// path := params
		path := fmt.Sprintf("%s%02d", yyyyMM, i)
		// 获取文件列表
		flag := "0"
		for _, f := range dirs {
			if f.Name() == path {
				flag = "1"
				continue
			}
		}
		binStr += flag
	}

	return binStr, nil
}

// // parseStart 解析起始时间
// //
// // 该函数接收一个文件路径[string]，返回该文件路径的[毫秒级起始时间戳] [毫秒级持续间隔] [错误]
// func parseStart(start string) (int64, error) {
// 	var timestamp int64
// 	// 解析文件起始时间
// 	switch len(start) {
// 	case 10: // 1584541242_3000.ts 解析[秒级时间戳]格式时间
// 		timestamp, _ = strconv.ParseInt(start, 10, 64)
// 		timestamp = timestamp * 1000

// 	case 13: // 1584541242000_3000.ts 解析[毫秒级时间戳]格式时间
// 		timestamp, _ = strconv.ParseInt(start, 10, 64)

// 	case 14: // 20240304231212_3000.ts 解析[秒级时间]格式
// 		startTime, err := time.ParseInLocation("20060102150405", start, time.Local)
// 		if err != nil {
// 			return 0, errors.New(fmt.Sprintf("起始时间格式错误:time = %s", start))
// 		}
// 		timestamp = startTime.UnixMilli()
// 	// case 17: //20240304231212000_3000.ts 解析[毫秒级时间]格式
// 	//	startTime, err := time.ParseInLocation("20060102150405000", arr[0], time.Local)
// 	//	if err != nil {
// 	//		slog.Error("解析文件起始时间出错", "err", fmt.Sprintf("time = %s", arr[0]))
// 	//	}
// 	//	timestamp = startTime.Unix()
// 	default:
// 		return 0, errors.New(fmt.Sprintf("起始时间格式错误:time = %s", start))
// 	}
// 	return timestamp, nil
// }

const offLineTime = 2 * 1000 // offLineTime 表示两个ts之间的间隔阈值，超过这个值就认为两个ts不连续

// GeneranM3u8 生成 m3u8
func GeneranM3u8(datas []string) (data []byte, err error) {
	l := uint(len(datas))
	// 创建m3u8播放对象
	p, e := m3u8.NewMediaPlaylist(l, l)
	if e != nil {
		return nil, err
	}
	// 存储上一个ts的开始时间和结束时间
	var lastStartAt, LastDuration int64
	for i, data := range datas {
		// 获取ts的开始时间和持续时间
		idx := strings.Index(data, ".ts")
		if idx == -1 {
			continue
		}
		path := data[:idx]
		baseName := path[strings.LastIndex(path, "/")+1:]
		s := strings.Split(baseName, "-")
		if len(s) < 2 {
			continue
		}
		startAt, duration, err := convertTime(s[0], s[1])
		if err != nil {
			continue
		}
		_ = p.Append(data, float64(duration)/1000, "")
		// 在不连续的地方加入标记
		// 当前起始时间 - (上个ts起始时间 + 上个 ts 结束时间）= 间隔时间
		if i > 0 && startAt-(lastStartAt+LastDuration) > offLineTime {
			_ = p.SetDiscontinuity()
		}
		// 更新开始时间和结束时间供下次查询
		lastStartAt, LastDuration = startAt, duration
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
