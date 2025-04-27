// Copyright 2020 TSINGSEE.
// http://www.tsingsee.com
// 针对 hsl 的 m3u8 文件封装的一些数据
// Creat By Sam
// History (Name, Time, Desc)
// (Sam, 20210331, 创建文件)
package efile

import (
	"bufio"
	"fmt"
	"io"
	"io/ioutil"
	"math"
	"os"
	"regexp"
	"strconv"
	"strings"
)

// 生成 hls 对应的 m3u8 文件
func GenerateHls(hlsPath string, timeAll, timeMax float64, tsRoot string, tsKey *[]string, keyValue *map[string]float64) error {
	// 创建 m3u8 文件
	mfile, err := os.Create(hlsPath)
	if err != nil {
		return err
	}
	defer mfile.Close()

	w := bufio.NewWriter(mfile)
	fmt.Fprintln(w, "#EXTM3U")
	fmt.Fprintln(w, "#EXT-X-VERSION:3")
	fmt.Fprintln(w, "#EXT-X-MEDIA-SEQUENCE:0")
	fmt.Fprintln(w, fmt.Sprintf("#EXT-X-TARGETDURATION:%s", fmt.Sprintf("%v", int(math.Ceil(timeMax)))))
	fmt.Fprintln(w, fmt.Sprintf("#EXT_X_TOTAL_DURATION:%s", fmt.Sprintf("%v", timeAll)))

	for _, tsname := range *tsKey {
		value, ok := (*keyValue)[tsname]
		if !ok {
			continue
		}
		fmt.Fprintln(w, fmt.Sprintf("#EXTINF:%s,", fmt.Sprintf("%v", value)))
		//fmt.Fprintln(w, tsname)
		if tsRoot != "" {
			ts := tsRoot + "/" + tsname
			fmt.Fprintln(w, ts)
		} else {
			fmt.Fprintln(w, tsname)
		}
	}
	fmt.Fprintln(w, "#EXT-X-ENDLIST")

	return w.Flush()
}

// 获取对应的 Ts 数据，为了加快速度，只解析一次，返回出所有的时间数据，后期全部查询这个map数据
func GetTsTimes(m3u8Path string) (*map[string]float64, error) {
	f, err := os.Open(m3u8Path)
	if err != nil {
		return nil, err
	}
	defer f.Close()

	tsMap := make(map[string]float64, 0)
	rd := bufio.NewReader(f)
	nextKey := false
	preValue := -1.00
	for {
		line, err := rd.ReadString('\n') //以'\n'为结束符读入一行
		if err != nil {
			if io.EOF == err {
				break
			} else {
				return nil, err
			}
		}
		line = strings.TrimSpace(line)

		if nextKey {
			nextKey = false
			tsMap[line] = preValue
		}

		timeStr := "#EXTINF:"
		if strings.Contains(line, timeStr) {
			preValue, err = strconv.ParseFloat(strings.Split(strings.TrimPrefix(line, timeStr), ",")[0], 64)
			if err != nil {
				continue
			}
			nextKey = true
		}
	}

	return &tsMap, nil
}

func GetM3u8Duration(m3u8Path string) (d float64) {
	buf, err := ioutil.ReadFile(m3u8Path)
	if err != nil {
		return
	}
	m3u8 := string(buf)
	reg := regexp.MustCompile(`#EXT_X_TOTAL_DURATION:\s*(\d+[\\.]?\d*)`)
	if matchs := reg.FindStringSubmatch(m3u8); matchs != nil {
		fmt.Sscanf(matchs[1], "%f", &d)
	} else {
		reg = regexp.MustCompile(`#EXTINF:\s*(\d+[\\.]?\d*)`)
		if _matchs := reg.FindAllStringSubmatch(m3u8, -1); _matchs != nil {
			for _, _match := range _matchs {
				var _d float64
				fmt.Sscanf(_match[1], "%f", &_d)
				d += _d
			}
		}
	}
	return
}

// 获取 m3u8 文件中第 1 个 ts 文件名称
func GetFirstTsName(m3u8File string) string {
	f, err := os.Open(m3u8File)
	if err != nil {
		return ""
	}
	defer f.Close()

	rd := bufio.NewReader(f)
	for {
		line, err := rd.ReadString('\n') //以'\n'为结束符读入一行
		if err != nil {
			if io.EOF == err {
				break
			} else {
				return ""
			}
		}
		line = strings.TrimSpace(line)

		if strings.HasSuffix(line, ".ts") {
			return line
		}
	}

	return ""
}

// 获取 m3u8 文件中最后一个 ts 文件名称
func GetLastTsName(m3u8File string) string {
	f, err := os.Open(m3u8File)
	if err != nil {
		return ""
	}
	defer f.Close()

	rd := bufio.NewReader(f)
	lastTSName := ""
	for {
		line, err := rd.ReadString('\n') //以'\n'为结束符读入一行
		if err != nil {
			if io.EOF == err {
				break
			} else {
				return ""
			}
		}
		line = strings.TrimSpace(line)

		if strings.HasSuffix(line, ".ts") {
			lastTSName =  line
		}
	}

	return lastTSName
}
