package api

import (
	"bufio"
	"bytes"
	"easydarwin/internal/core/video"
	"easydarwin/internal/data"
	"easydarwin/internal/gutils"
	"easydarwin/internal/gutils/consts"
	"easydarwin/internal/gutils/efile"
	"easydarwin/internal/gutils/estring"
	"fmt"
	"io"
	"log"
	"log/slog"
	"math"
	"os"
	"os/exec"
	"path/filepath"
	"regexp"
	"runtime"
	"strconv"
	"strings"
	"time"

	"github.com/teris-io/shortid"
)

// EasyTrans 返回EasyTrans的执行文件
func EasyTrans() string {
	switch runtime.GOOS {
	case "windows":
		return filepath.Join(efile.GetRealPath(""), "ffmpeg.exe")
	case "linux":
		path := filepath.Join(efile.GetRealPath(""), "ffmpeg")
		os.Chmod(path, 0755)
		return path
	default:
	}

	return ""
}

// printCmd 构造并打印命令字符串
func printCmd(name string, args ...string) {
	cmdStr := name
	for _, arg := range args {
		// 简单处理，实际应用中可能需要更复杂的转义逻辑
		cmdStr += " " + fmt.Sprintf("%q", arg)
	}
	fmt.Println("Running command:", cmdStr)
}

// MediaInfo 封装流媒体信息
type MediaInfo struct {
	Duration     int
	VideoDecodec string
	AudioDecodec string
	Aspect       string
	Rotate       int
	Yuvj420p     bool
}

// Info 获取指定资源的信息
func Info(file string, isRtsp bool) (*MediaInfo, error) {
	ret := &MediaInfo{}
	var cmd *exec.Cmd
	if isRtsp {
		cmd = exec.Command(EasyTrans(), "-rtsp_transport", "tcp", "-hide_banner", "-i", file)
	} else {
		cmd = exec.Command(EasyTrans(), "-hide_banner", "-i", file)
	}

	out := bytes.NewBuffer(nil)
	cmd.Stderr = out
	if err := cmd.Start(); err != nil {
		log.Println(err)
		return nil, err
	}
	timeOutChan := make(chan string)
	//设置超时时间
	gutils.Go(func() {
		for {
			select {
			case _, ok := <-timeOutChan:
				if !ok {
					return
				}
			case <-time.After(time.Second * 30):
				if cmd != nil {
					cmd.Process.Kill()
					return
				}
			}
		}
	})
	cmd.Wait()
	close(timeOutChan)
	if out.String() == "" {
		return nil, fmt.Errorf("获取信息超时或失败")
	}
	reg := regexp.MustCompile("Duration:\\s+(.+?),\\s+")
	matchs := reg.FindStringSubmatch(out.String())
	if matchs != nil {
		ret.Duration = TimeSeconds(matchs[1])
	}
	reg = regexp.MustCompile("Video:\\s+(.+?)\\s+")
	matchs = reg.FindStringSubmatch(out.String())
	if matchs != nil {
		// 去除最后的 ,
		ret.VideoDecodec = strings.TrimSuffix(matchs[1], ",")
	}
	reg = regexp.MustCompile("Audio:\\s+(.+?)\\s+")
	matchs = reg.FindStringSubmatch(out.String())
	if matchs != nil {
		// 去除最后的 ,
		ret.AudioDecodec = strings.TrimSuffix(matchs[1], ",")
	}
	reg = regexp.MustCompile("\\s+(\\d+x\\d+)(\\s+|,\\s+)")
	matchs = reg.FindStringSubmatch(out.String())
	if matchs != nil {
		ret.Aspect = matchs[1]
	}
	reg = regexp.MustCompile("\\s+rotate\\s*:\\s+(\\d+)\\s+")
	matchs = reg.FindStringSubmatch(out.String())
	if matchs != nil {
		ret.Rotate, _ = strconv.Atoi(matchs[1])
	}

	reg = regexp.MustCompile("yuvj420p")
	matchs = reg.FindStringSubmatch(out.String())
	if matchs != nil {
		ret.Yuvj420p = true
	} else {
		ret.Yuvj420p = false
	}
	return ret, nil
}

// DefaultSnapTime 默认的截图时间
const DefaultSnapTime = "00:00:01"

// DefaultSnapDest 默认截图地址
func DefaultSnapDest(file string) string {
	return filepath.Join(filepath.Dir(file), strings.TrimSuffix(filepath.Base(file), filepath.Ext(file)), "snap.jpg")
}

// VODSnap 截图操作
func VODSnap(file, time, dest string) {
	if file == "" {
		return
	}
	ext := filepath.Ext(file)
	if time == "" {
		time = DefaultSnapTime
	}
	if dest == "" {
		dest = DefaultSnapDest(file)
	}
	efile.EnsureDir(filepath.Dir(dest))
	params := []string{"-ss", time, "-hide_banner", "-i", file, "-y", "-f", "image2", "-vframes", "1", dest}
	if strings.ToLower(ext) == ".mp3" {
		params = []string{"-hide_banner", "-i", file, "-y", "-an", "-vcodec", "copy", dest}
	}

	cmd := exec.Command(EasyTrans(), params...)

	//printCmd(cmd.Path, params...)

	if err := cmd.Run(); err != nil {
		slog.Error("run snap err", err)
	}
}

// VODTrans  点播转码
// h264 转换为 h265
// ffmpeg -i "h264.mp4" -c:a copy -c:v libx265 "h265_2.mp4"
// h264 转换成 HLS
// ffmpeg.exe -fflags +genpts -hide_banner -i input.mp4 -vcodec copy -acodec copy -ac -2 -strict -2 -f hls -hls_time 8 -hls_list_size 0 output.m3u8
// 其他格式转换成 HLS
// ffmpeg.exe -fflags +genpts -hide_banner -i input.mp4 -vcodec libx264 -acodec copy -strict -2 -f hls -hls_time 8 -hls_list_size 0 output.m3u8
func VODTrans(vod *video.TVod, callback func(progress int)) bool {
	if vod == nil {
		return false
	}

	sysConfig := gCfg.VodConfig
	hlsTime := sysConfig.HlsTime
	videoTranWay := sysConfig.TranWay
	h265VideoTranWay := sysConfig.TranHevcWay
	vcodec := "libx264"
	acodec := "aac"

	// 转换成 h264 有两种参数操作 libx264、h264_nvenc
	// copy 为默认原数据编码
	if vod.Rotate == 0 {
		switch vod.VideoCodec {
		case "H.264":
			vcodec = "copy"
		case "VP9":
			// vp9 编码，仅能够转换成 h265 编码， nvenc 代表设置编码
			vcodec = "hevc_nvenc"
		case "HEVC":
			// hevc 编码为 h265 编码，转换成 h264,有两种 h264_nvenc、libx264、copy
			// libx264 会导致 cpu 使用率 为 100%
			vcodec = h265VideoTranWay
			if vcodec == "h264_nvenc" || vcodec == "libx264" {
				vod.VideoCodec = "H.264"
			}
		case "MPEG4":
			//MPEG4无法使用硬件转码
		default:
			vcodec = videoTranWay
		}
	}

	if strings.EqualFold(vod.AudioCodec, "AAC") {
		acodec = "copy"
	}
	dest := filepath.Join(gCfg.VodConfig.Dir, vod.Folder, "video.m3u8")
	efile.EnsureDir(filepath.Dir(dest))
	//-ac 2 设置双声道的
	args := []string{"-fflags", "+genpts", "-hide_banner", "-i", vod.RealPath, "-vcodec", vcodec, "-acodec", acodec, "-ac", "2"}
	if vod.Aspect != "" {
		if sizes := strings.SplitN(vod.Aspect, "x", 2); len(sizes) == 2 {
			h, _ := strconv.Atoi(sizes[1])
			if h%2 != 0 {
				args = append(args, "-vf", "scale=iw:trunc(ow/a/2)*2")
			}
		}
	}
	argHLS := []string{"-strict", "-2", "-f", "hls", "-hls_time", strconv.Itoa(hlsTime), "-hls_list_size", "0"}
	args = append(args, argHLS...)
	args = append(args, dest)
	filetype := strings.ToLower(filepath.Ext(vod.RealPath))
	if filetype == ".mp3" {
		args = []string{"-hide_banner", "-i", vod.RealPath, "-vn", "-acodec", acodec}
		args = append(args, "-strict", "-2", "-f", "hls", "-hls_time", strconv.Itoa(hlsTime), "-hls_list_size", "0", dest)
	}

	//判断有没有开启多清晰度
	if sysConfig.OpenDefinition && filetype != ".mp3" && filetype != ".wav" {
		for _, defType := range strings.Split(sysConfig.TransDefinition, consts.SplitComma) {
			//默认会转码原始视频分辨率
			if defType != consts.DefinitionYH {
				args = getDefinitionCommand(args, vod.Aspect, defType, vcodec, acodec)
				args = append(args, argHLS...)
				args = append(args, filepath.Join(gCfg.VodConfig.Dir, vod.Folder, fmt.Sprintf("video_%s.m3u8", defType)))
			}
		}
	}

	cmd := exec.Command(EasyTrans(), args...)

	cmd.Stdout = os.Stdout
	stderr, err := cmd.StderrPipe()
	if err != nil {
		log.Println("run trans err", err)
		return false
	}
	cmd.Start()
	reader := bufio.NewReader(stderr)
	reg := regexp.MustCompile(`frame=.+\s+time=(.+?)\s+`)
	for {
		if line, err := reader.ReadString('\r'); err != nil {

			if err == io.EOF && callback != nil {
				callback(100)
			}
			break
		} else {
			// elog.DevInfo(line)
			matchs := reg.FindStringSubmatch(line)
			if matchs != nil && vod.Duration != 0 {
				progress := float64(TimeSeconds(matchs[1])) / float64(vod.Duration) * 100
				if callback != nil {
					callback(int(math.Floor(progress)))
				}
			}
		}
	}
	cmd.Wait()
	data.GetDatabase().Save(vod)
	return true
}

func getDefinitionCommand(args []string, aspect string, defType string, vcodec string, acodec string) []string {
	if aspect != "" {
		if sizes := strings.SplitN(aspect, "x", 2); len(sizes) == 2 {
			w, _ := strconv.ParseFloat(sizes[0], 64)
			h, _ := strconv.ParseFloat(sizes[1], 64)
			switch defType {
			case consts.DefinitionSD: //640x360
				nh := math.Ceil(640 / w * h)
				if int(nh)%2 != 0 {
					nh++
				}
				args = append(args, "-acodec", acodec, "-s", fmt.Sprintf("640x%v", nh), "-b", "300k")
			case consts.DefinitionHD: //1280x720
				nh := math.Ceil(1280 / w * h)
				if int(nh)%2 != 0 {
					nh++
				}
				args = append(args, "-acodec", acodec, "-s", fmt.Sprintf("1280x%v", nh), "-b", "500k")
			case consts.DefinitionFHD: //1920x1080
				nh := math.Ceil(1920 / w * h)
				if int(nh)%2 != 0 {
					nh++
				}
				args = append(args, "-acodec", acodec, "-s", fmt.Sprintf("1920x%v", nh), "-b", "1000k")
			}
		}
	}
	return args
}

// M3U8ToMP4 将m3u8转为mp4
func M3U8ToMP4(m3u8Path string) string {
	m3u8Path = estring.FormatPath(m3u8Path)
	dir := filepath.Dir(m3u8Path)
	dest := filepath.Join(dir, shortid.MustGenerate()+".mp4")
	args := []string{"-i", m3u8Path, "-vcodec", "copy", "-acodec", "copy", "-y", dest}
	cmd := exec.Command(EasyTrans(), args...)
	cmd.Run()
	return dest
}

// TimeSeconds 解析时间
func TimeSeconds(t string) int {
	reg := regexp.MustCompile("(\\d+):(\\d+):(\\d+)\\.(\\d+)")
	matchs := reg.FindStringSubmatch(t)
	if matchs != nil {
		timeWithUnit := strings.Join([]string{matchs[1], "h", matchs[2], "m", matchs[3], ".", matchs[4], "s"}, "")
		d, err := time.ParseDuration(timeWithUnit)
		if err == nil {
			return int(d.Seconds())
		}

	}
	return 0
}
