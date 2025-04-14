package sms

import (
	"crypto/tls"
	"encoding/json"
	"fmt"
	"log/slog"
	"net"
	"net/http"
	"time"

	"golang.org/x/net/http2"
)

var callbackSMSHTTPCli = &http.Client{
	Timeout: 5 * time.Second,
	Transport: &http2.Transport{
		AllowHTTP: true,
		DialTLS: func(network, addr string, cfg *tls.Config) (net.Conn, error) {
			return net.DialTimeout(network, addr, 10*time.Second)
		},
		IdleConnTimeout: time.Hour,
	},
}

type SMSResponse struct {
	Msg  string `json:"msg"`
	Data []byte `json:"data"`
}

// GetIFrame //api/groups/{streamName}/snapshot 获取指定设备在指定通道上指定时间戳的关键帧图像数据
func GetIFrame(addr string, streamName string, timestamp int64) ([]byte, error) {
	// 只有正在播放的设备才能拿到其sms的交互地址
	// 是否关键帧信息会持久保存

	// 检查是否存在正在播放的流信息
	// 构造获取关键帧图像的URL
	url := fmt.Sprintf("http://%s/api/groups/%s/snapshot?timestamp=%d", addr, streamName, timestamp)

	// 发起HTTP GET请求获取数据
	resp, err := callbackSMSHTTPCli.Get(url)
	if err != nil {
		slog.Error("GetIFrame NewRequest", "err", err)
		return nil, err
	}
	defer resp.Body.Close()
	// 解析响应体
	var out SMSResponse
	if err := json.NewDecoder(resp.Body).Decode(&out); err != nil {
		slog.Error("GetIFrame json.NewDecoder(resp.Body).Decode(&out)", "err", err)
		return nil, fmt.Errorf("数据解析异常 %w", err)
	}
	// 检查响应状态码
	if resp.StatusCode == 200 {
		return out.Data, nil
	}
	// 如果有错误消息，返回相应错误
	if out.Msg != "" {
		return nil, fmt.Errorf(out.Msg)
	}
	// 否则返回HTTP状态错误
	return nil, fmt.Errorf(resp.Status)
}
