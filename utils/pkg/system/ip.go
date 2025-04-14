// Author: xiexu
// Date: 2022-09-20

package system

import (
	"crypto/tls"
	"encoding/json"
	"fmt"
	"net"
	"net/http"
	"strconv"
	"strings"
	"time"

	"golang.org/x/text/encoding/simplifiedchinese"
	"golang.org/x/text/transform"
)

// LocalIP 获取本地IP地址
func LocalIP() string {
	conn, err := net.DialTimeout("udp", "8.8.8.8:53", 3*time.Second)
	if err != nil {
		return ""
	}
	host, _, _ := net.SplitHostPort(conn.LocalAddr().(*net.UDPAddr).String())
	if host != "" {
		return host
	}
	iip := strings.Split(localIP()+"/", "/")
	if len(iip) >= 2 {
		return iip[0]
	}
	return ""
}

// localIP 获取本地 IP，遇到虚拟 IP 有概率不准确
func localIP() string {
	addrs, err := net.InterfaceAddrs()
	if err != nil {
		return ""
	}

	ip := ""
	for _, v := range addrs {
		net, ok := v.(*net.IPNet)
		if !ok {
			continue
		}
		if net.IP.IsMulticast() || net.IP.IsLoopback() || net.IP.IsLinkLocalMulticast() || net.IP.IsLinkLocalUnicast() {
			continue
		}
		if net.IP.To4() == nil {
			continue
		}

		ip = v.String()
	}
	return ip
}

// PortUsed 检测端口是否已用 true:已使用;false:未使用
func PortUsed(mode string, port int) bool {
	if port > 65535 || port < 0 {
		return true
	}

	switch strings.ToLower(mode) {
	case "tcp":
		if err := TCPPortUsed(port); err != nil {
			return true
		}
	default:
		if err := UDPPortUsed(port); err != nil {
			return true
		}
	}
	return false
}

// TCPPortUsed 检测 TCP 端口
// 返回监听失败的错误原因
func TCPPortUsed(port int) error {
	interfaces, err := net.Interfaces()
	if err != nil {
		return err
	}

	for _, iface := range interfaces {
		// 先检测通配网卡端口
		ln, err := net.Listen("tcp", ":"+strconv.Itoa(port))
		if err != nil {
			return err
		}
		ln.Close()

		// 如果没有启用，跳过
		if iface.Flags&net.FlagUp == 0 {
			continue
		}

		// 获取网卡的IP列表
		addrs, err := iface.Addrs()
		if err != nil {
			continue
		}

		for _, addr := range addrs {
			var ip string
			switch v := addr.(type) {
			case *net.IPNet:
				ip = v.IP.String()
			case *net.IPAddr:
				ip = v.IP.String()
			}

			// 只检测 IPv4
			if net.ParseIP(ip).To4() == nil {
				continue
			}

			ln, err := net.Listen("tcp4", ip+":"+strconv.Itoa(port))
			if err != nil {
				return fmt.Errorf("[%s]:%s", iface.Name, err.Error())
			}
			ln.Close()
		}
	}
	return nil
}

// UDPPortUsed 检测 UDP 端口
// 返回监听失败的错误原因
func UDPPortUsed(port int) error {
	// 先检测通配网卡端口
	addr, _ := net.ResolveUDPAddr("udp", ":"+strconv.Itoa(port))
	conn, err := net.ListenUDP("udp", addr)
	if err != nil {
		return err
	}
	conn.Close()

	interfaces, err := net.Interfaces()
	if err != nil {
		return err
	}

	for _, iface := range interfaces {
		// 如果没有启用，跳过
		addrs, err := iface.Addrs()
		if err != nil {
			continue
		}

		// 获取网卡的IP列表
		for _, addr := range addrs {
			var ip string
			switch v := addr.(type) {
			case *net.IPNet:
				ip = v.IP.String()
			case *net.IPAddr:
				ip = v.IP.String()
			}

			// 只检测 IPv4
			if net.ParseIP(ip).To4() == nil {
				continue
			}

			addr, _ := net.ResolveUDPAddr("udp", ip+":"+strconv.Itoa(port))
			conn, err := net.ListenUDP("udp", addr)
			if err != nil {
				return fmt.Errorf("[%s]:%s", iface.Name, err.Error())
			}
			conn.Close()
		}
	}
	return nil
}

// ExternalIP 获取公网 IP
func ExternalIP() (string, error) {
	c := http.Client{
		Timeout: 5 * time.Second,
		Transport: &http.Transport{
			TLSClientConfig: &tls.Config{
				InsecureSkipVerify: true, // nolint
			},
		},
	}

	resp, err := c.Get("https://api.live.bilibili.com/client/v1/Ip/getInfoNew")
	if err != nil {
		return "", err
	}
	defer func() {
		_ = resp.Body.Close()
	}()

	var v bilibiliResponse
	err = json.NewDecoder(resp.Body).Decode(&v)
	return v.Data.Addr, err
}

type bilibiliResponse struct {
	Code    int          `json:"code"`
	Msg     string       `json:"msg"`
	Message string       `json:"message"`
	Data    bilibiliData `json:"data"`
}

type bilibiliData struct {
	Addr      string `json:"addr"`
	Country   string `json:"country"`
	Province  string `json:"province"`
	City      string `json:"city"`
	ISP       string `json:"isp"`
	Latitude  string `json:"latitude"`
	Longitude string `json:"longitude"`
}

var cli = http.Client{
	Timeout: 5 * time.Second,
	Transport: &http.Transport{
		Proxy:             http.ProxyFromEnvironment,
		ForceAttemptHTTP2: true,
		MaxIdleConns:      5,
		MaxConnsPerHost:   10,
		IdleConnTimeout:   10 * time.Second,
	},
}

const url = "http://whois.pconline.com.cn/ipJson.jsp?json=true&ip="

type Info struct {
	IP          string `json:"ip"`
	Pro         string `json:"pro"`        // 省;安徽省
	ProCode     string `json:"proCode"`    // 省区域代码;340000
	City        string `json:"city"`       // 城市;合肥市
	CityCode    string `json:"cityCode"`   // 城市代码;340100
	Region      string `json:"region"`     // 区域;蜀山区
	RegionCode  string `json:"regionCode"` // 区域代码;340104
	Addr        string `json:"addr"`       // 完整地址;安徽省合肥市蜀山区 电
	RegionNames string `json:"regionNames"`
	Err         string `json:"err"`
}

// IP2Info 不建议判断 Info 的 err，因为有可能发生 nocity 错误
// 只能获取省份，拿不到城市
func IP2Info(ip string) (Info, error) {
	netip := net.ParseIP(ip)
	if netip.IsLoopback() || netip.IsPrivate() {
		return Info{IP: ip, Addr: "内网 IP"}, nil
	}

	resp, err := cli.Get(url + ip)
	if err != nil {
		return Info{}, err
	}
	defer resp.Body.Close()

	if resp.StatusCode != 200 {
		return Info{}, fmt.Errorf(resp.Status)
	}
	var out Info
	reader := transform.NewReader(resp.Body, simplifiedchinese.GB18030.NewDecoder())
	err = json.NewDecoder(reader).Decode(&out)
	return out, err
}

// CompareVersionFunc 比较 ip 或 版本号是否一致
func CompareVersionFunc(a, b string, f func(a, b string) bool) bool {
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
