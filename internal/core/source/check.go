package source

import (
	"crypto/tls"
	"easydarwin/internal/core/livestream"
	"fmt"
	"github.com/go-co-op/gocron"
	"log/slog"
	"net"
	"net/http"
	"runtime/debug"
	"strings"
	"sync"
	"time"
)

var (
	gCheckPullDeviceFlag = false
	// 检查拉流设备并发量
	gCheckConcurrent               = 30
	gCheckPullPushChannelScheduler *gocron.Scheduler
)

const (
	timeout     = 5 * time.Second
	rtspTimeout = 3 * time.Second
)

type StreamType string

const (
	StreamTypeHTTP    StreamType = "HTTP/HLS"
	StreamTypeRTMP    StreamType = "RTMP"
	StreamTypeRTMPS   StreamType = "RTMPS"
	StreamTypeRTSP    StreamType = "RTSP"
	StreamTypeRTSPS   StreamType = "RTSPS"
	StreamTypeTCP     StreamType = "TCP"
	StreamTypeUDP     StreamType = "UDP"
	StreamTypeUnknown StreamType = "UNKNOWN"
)

type CheckResult struct {
	URL       string
	Type      StreamType
	Online    bool
	Transport string
	Message   string
}

func StartScheduler() {
	gCheckPullDeviceFlag = false
	gCheckPullPushChannelScheduler = gocron.NewScheduler(time.Local)
	// 检测间隔时间
	rtspCheckOnlineTime := 20
	checkNetPullAndRtmpPushDevice()
	_, err := gCheckPullPushChannelScheduler.Every(rtspCheckOnlineTime).Seconds().Do(checkNetPullAndRtmpPushDevice)
	if err != nil {
		slog.Info("start scheduler error")
		return
	}
	gCheckPullPushChannelScheduler.StartAsync()
}
func StopScheduler() {
	if gCheckPullPushChannelScheduler != nil {
		gCheckPullPushChannelScheduler.Stop()
		gCheckPullPushChannelScheduler = nil
	}
	gCheckPullDeviceFlag = false
}

// 检查拉流设备
func checkNetPullAndRtmpPushDevice() {
	if gCheckPullDeviceFlag {
		slog.Info("正在检测中...")
		return
	}
	openStreamTimeOut := 20

	slog.Info("开始一次检测 pull 的状态")
	defer slog.Info("结束一次检测 pull 的状态")
	gCheckPullDeviceFlag = true
	defer func() {
		if err := recover(); err != nil {
			slog.Error(fmt.Sprintf("%s\n", err))
			slog.Error(fmt.Sprintln(string(debug.Stack())))
		}
		gCheckPullDeviceFlag = false
	}()

	channelsList, _, _ := LiveCore.FindLiveStreamALl()
	checkNumber := 0
	// 并发检测数量检测数量
	checkCurrentNumber := gCheckConcurrent
	rtspChannelNum := len(channelsList)

	// 如果一次性并发数量大于 pull 通道数量，并发数修改为 pull 通道数量
	if checkCurrentNumber > rtspChannelNum {
		checkCurrentNumber = rtspChannelNum
	}

	var wgChannel sync.WaitGroup
	for i := 0; i < rtspChannelNum; i++ {

		wgChannel.Add(1)
		checkNumber++
		go func(v livestream.LiveStream) {
			defer func() {
				if err := recover(); err != nil {
					slog.Error(fmt.Sprintf("%s\n", err))
					slog.Error(fmt.Sprintln(string(debug.Stack())))
				}
				wgChannel.Done()
			}()
			// 如果不启用，则直接返回
			if !v.Enable {
				return
			}
			// 开始检测
			if v.OnDemand {
				err := UpdateOnlineStream(v)
				if err != nil {
					slog.Error(fmt.Sprintf("scheduler [%d]%s\n", v.ID, err))
					return
				}
			} else {
				res := CheckStreamOnline(v.Url)
				if v.Online != 2 {
					online := 0
					if res.Online {
						online = 1
					}
					err := LiveCore.UpdateLiveStreamInt(v.ID, "online", online)
					if err != nil {
						slog.Error("check online update live stream online", v.ID, online, err.Error())
						return
					}
				}
			}
			time.Sleep(time.Duration(openStreamTimeOut) * time.Second)

		}((channelsList)[i])

		checkCurrentNumber = checkCurrentNumber - 1
		if checkCurrentNumber <= 0 {
			wgChannel.Wait()
			checkCurrentNumber = gCheckConcurrent
		}

	}
	slog.Info(fmt.Sprintf("检测完毕. 检测数量 %d", checkNumber))
}

func CheckStreamOnline(url string) CheckResult {
	if url == "" {
		return CheckResult{URL: url, Type: StreamTypeUnknown, Online: false, Message: "地址为空"}
	}

	switch {
	case strings.HasPrefix(url, "http://") || strings.HasPrefix(url, "https://"):
		return checkHTTP(url)

	case strings.HasPrefix(url, "rtmp://"):
		return checkRTMP(url)
	case strings.HasPrefix(url, "rtmps://"):
		return checkRTMPS(url)

	case strings.HasPrefix(url, "rtsp://"):
		return checkRTSP_PRECISION(url)
	case strings.HasPrefix(url, "rtsps://"):
		return checkRTSPS(url)

	case strings.HasPrefix(url, "tcp://"):
		return checkTCP(strings.TrimPrefix(url, "tcp://"))
	case strings.HasPrefix(url, "udp://"):
		return checkUDP_FAKE(url)

	default:
		return CheckResult{URL: url, Type: StreamTypeUnknown, Online: false, Message: "不支持的协议"}
	}
}

// ------------------------------
// HTTP 检测
// ------------------------------
func checkHTTP(url string) CheckResult {
	client := &http.Client{
		Timeout: timeout,
		CheckRedirect: func(req *http.Request, via []*http.Request) error {
			return http.ErrUseLastResponse
		},
		Transport: &http.Transport{
			TLSClientConfig: &tls.Config{InsecureSkipVerify: true},
		},
	}

	req, _ := http.NewRequest("HEAD", url, nil)
	resp, err := client.Do(req)
	if err != nil || resp.StatusCode >= 400 {
		resp, err = client.Get(url)
		if err != nil {
			return CheckResult{URL: url, Type: StreamTypeHTTP, Online: false, Transport: "tcp", Message: err.Error()}
		}
		defer resp.Body.Close()
	}

	if resp.StatusCode == http.StatusOK || resp.StatusCode == http.StatusPartialContent {
		return CheckResult{URL: url, Type: StreamTypeHTTP, Online: true, Transport: "tcp", Message: "正常"}
	}
	return CheckResult{URL: url, Type: StreamTypeHTTP, Online: false, Transport: "tcp", Message: fmt.Sprintf("状态码异常 %d", resp.StatusCode)}
}

// ------------------------------
// RTMP（TCP+UDP）
// ------------------------------
func checkRTMP(url string) CheckResult {
	addr := extractAddress(url)
	if addr == "" {
		return CheckResult{URL: url, Type: StreamTypeRTMP, Online: false, Message: "地址解析失败"}
	}

	// TCP
	if conn, err := net.DialTimeout("tcp", addr, timeout); err == nil {
		defer conn.Close()
		return CheckResult{URL: url, Type: StreamTypeRTMP, Online: true, Transport: "tcp", Message: "TCP 连通"}
	}

	// UDP
	if conn, err := net.DialTimeout("udp", addr, timeout); err == nil {
		defer conn.Close()
		return CheckResult{URL: url, Type: StreamTypeRTMP, Online: true, Transport: "udp", Message: "UDP 连通"}
	}

	return CheckResult{URL: url, Type: StreamTypeRTMP, Online: false, Message: "RTMP 服务离线"}
}

// ------------------------------
// RTMPS
// ------------------------------
func checkRTMPS(url string) CheckResult {
	addr := extractAddress(url)
	if addr == "" {
		return CheckResult{URL: url, Type: StreamTypeRTMPS, Online: false, Message: "地址解析失败"}
	}
	tlsCfg := &tls.Config{InsecureSkipVerify: true}
	conn, err := tls.DialWithDialer(&net.Dialer{Timeout: timeout}, "tcp", addr, tlsCfg)
	if err != nil {
		return CheckResult{URL: url, Type: StreamTypeRTMPS, Online: false, Transport: "tcp", Message: err.Error()}
	}
	defer conn.Close()
	return CheckResult{URL: url, Type: StreamTypeRTMPS, Online: true, Transport: "tcp", Message: "TLS 连通"}
}

// ------------------------------
// ✅ 精准 RTSP 检测（解决 UDP 误判在线）
// ------------------------------
func checkRTSP_PRECISION(url string) CheckResult {
	addr := extractAddress(url)
	if addr == "" {
		return CheckResult{URL: url, Type: StreamTypeRTSP, Online: false, Message: "地址解析失败"}
	}

	// 1. 优先 TCP 检测
	if err := rtspTCPProbe(addr); err == nil {
		return CheckResult{URL: url, Type: StreamTypeRTSP, Online: true, Transport: "tcp", Message: "TCP 服务正常"}
	}

	// 2. UDP 真实探测（发送OPTIONS包，必须收到回复才算在线）
	if err := rtspUDPProbe(addr); err == nil {
		return CheckResult{URL: url, Type: StreamTypeRTSP, Online: true, Transport: "udp", Message: "UDP 服务正常"}
	}

	// 都失败 → 真正离线
	return CheckResult{URL: url, Type: StreamTypeRTSP, Online: false, Transport: "", Message: "RTSP 设备完全离线"}
}

// ------------------------------
// TCP 模式：发送 RTSP OPTIONS
// ------------------------------
func rtspTCPProbe(addr string) error {
	conn, err := net.DialTimeout("tcp", addr, rtspTimeout)
	if err != nil {
		return err
	}
	defer conn.Close()
	conn.SetDeadline(time.Now().Add(rtspTimeout))

	// 标准 RTSP OPTIONS 请求
	_, err = conn.Write([]byte("OPTIONS * RTSP/1.0\r\nCSeq: 1\r\nUser-Agent: GoProbe\r\n\r\n"))
	if err != nil {
		return err
	}

	buf := make([]byte, 1024)
	n, err := conn.Read(buf)
	if err != nil || n < 10 {
		return err
	}

	resp := string(buf[:n])
	if strings.Contains(resp, "RTSP/1.0") && strings.Contains(resp, "200 OK") {
		return nil
	}
	return fmt.Errorf("无效响应")
}

// ------------------------------
// ✅ UDP 模式：真正发包+收包（不会误判）
// ------------------------------
func rtspUDPProbe(addr string) error {
	conn, err := net.DialTimeout("udp", addr, rtspTimeout)
	if err != nil {
		return err
	}
	defer conn.Close()
	conn.SetDeadline(time.Now().Add(rtspTimeout))

	_, _ = conn.Write([]byte("OPTIONS * RTSP/1.0\r\nCSeq: 1\r\n\r\n"))

	buf := make([]byte, 1024)
	n, err := conn.Read(buf)
	if err != nil || n < 10 {
		return err
	}

	resp := string(buf[:n])
	if strings.Contains(resp, "RTSP/1.0") && strings.Contains(resp, "200 OK") {
		return nil
	}
	return fmt.Errorf("无响应")
}

// ------------------------------
// RTSPS
// ------------------------------
func checkRTSPS(url string) CheckResult {
	addr := extractAddress(url)
	if addr == "" {
		return CheckResult{URL: url, Type: StreamTypeRTSPS, Online: false, Message: "地址解析失败"}
	}
	tlsCfg := &tls.Config{InsecureSkipVerify: true}
	conn, err := tls.DialWithDialer(&net.Dialer{Timeout: timeout}, "tcp", addr, tlsCfg)
	if err != nil {
		return CheckResult{URL: url, Type: StreamTypeRTSPS, Online: false, Transport: "tcp", Message: err.Error()}
	}
	defer conn.Close()
	return CheckResult{URL: url, Type: StreamTypeRTSPS, Online: true, Transport: "tcp", Message: "TLS 连通"}
}

// ------------------------------
// 通用端口检测
// ------------------------------
func checkTCP(addr string) CheckResult {
	conn, err := net.DialTimeout("tcp", addr, timeout)
	if err != nil {
		return CheckResult{URL: "tcp://" + addr, Type: StreamTypeTCP, Online: false, Transport: "tcp", Message: err.Error()}
	}
	defer conn.Close()
	return CheckResult{URL: "tcp://" + addr, Type: StreamTypeTCP, Online: true, Transport: "tcp", Message: "端口开放"}
}

// 仅用于纯 udp:// 地址，不用于 RTSP
func checkUDP_FAKE(addr string) CheckResult {
	return CheckResult{URL: "udp://" + addr, Type: StreamTypeUDP, Online: false, Message: "UDP 无法精准检测"}
}

// ------------------------------
// 地址解析（支持账号密码）
// ------------------------------
func extractAddress(url string) string {
	parts := strings.SplitN(url, "://", 2)
	if len(parts) != 2 {
		return ""
	}
	hostPart := parts[1]

	if atIdx := strings.Index(hostPart, "@"); atIdx != -1 {
		hostPart = hostPart[atIdx+1:]
	}
	if slashIdx := strings.Index(hostPart, "/"); slashIdx != -1 {
		hostPart = hostPart[:slashIdx]
	}
	return hostPart
}
