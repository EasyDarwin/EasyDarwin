package config

import (
	"encoding/json"
	"io/ioutil"
)

var defaultConfig Config

type Config struct {
	SrtConfig        SrtConfig        `json:"srt_config"`      // srt配置
	RtcConfig        RtcConfig        `json:"rtc_config"`      // rtc配置
	HttpConfig       HttpConfig       `json:"http_config"`     // http/https配置
	HttpFmp4Config   HttpFmp4Config   `json:"httpfmp4_config"` // http-fmp4配置
	HlsConfig        HlsConfig        `json:"hls_config"`      // hls-fmp4/llhls配置
	GB28181Config    GB28181Config    `json:"gb28181_config"`  // gb28181配置
	OnvifConfig      OnvifConfig      `json:"onvif_config"`    // onvif配置
	ServerId         string           `json:"server_id"`       // http 通知唯一标识
	HttpNotifyConfig HttpNotifyConfig `json:"http_notify"`     // http 通知配置
	LalSvrConfigPath string           `json:"lal_config_path"` // lal配置目录
	HookConfig       HookConfig       `json:"hook_config"`     // gop cache配置
	RoomConfig       RoomConfig       `json:"room_config"`     // room配置
}

type SrtConfig struct {
	Enable bool   `json:"enable"` // srt服务使能配置
	Addr   string `json:"addr"`   // srt服务监听地址
}

type RtcConfig struct {
	Enable          bool     `json:"enable"`              // rtc服务使能配置
	ICEHostNATToIPs []string `json:"ice_host_nat_to_ips"` // rtc服务公网IP，未设置使用内网
	ICEUDPMuxPort   int      `json:"ice_udp_mux_port"`    // rtc udp mux port
	ICETCPMuxPort   int      `json:"ice_tcp_mux_port"`    // rtc tcp mux port
	WriteChanSize   int      `json:"write_chan_size"`
}

type HttpConfig struct {
	ListenAddr        string            `json:"http_listen_addr"`  // http服务监听地址
	EnableHttps       bool              `json:"enable_https"`      // https使能标志
	HttpsListenAddr   string            `json:"https_listen_addr"` // https监听地址
	HttpsCertFile     string            `json:"https_cert_file"`   // https cert 文件
	HttpsKeyFile      string            `json:"https_key_file"`    // https key 文件
	CtrlAuthWhitelist CtrlAuthWhitelist `json:"ctrl_auth_whitelist"`
}

// CtrlAuthWhitelist 控制类接口鉴权
type CtrlAuthWhitelist struct {
	IPs     []string // 允许访问的远程 IP，零值时不生效
	Secrets []string // 认证信息，零值时不生效
}

type HttpFmp4Config struct {
	Enable bool `json:"enable"` // http-fmp4使能标志
}

type HlsConfig struct {
	Enable          bool `json:"enable"`           // hls使能标志
	SegmentCount    int  `json:"segment_count"`    // 分片个数,llhls默认7个
	SegmentDuration int  `json:"segment_duration"` // hls分片时长,默认1s
	PartDuration    int  `json:"part_duration"`    // llhls part时长,默认200ms
	LowLatency      bool `json:"low_latency"`      // 是否开启llhls
}

type GB28181Config struct {
	Enable            bool               `json:"enable"`             // gb28181使能标志
	ListenAddr        string             `json:"listen_addr"`        // gb28181监听地址
	SipIP             string             `json:"sip_ip"`             // sip 服务器公网IP
	SipPort           uint16             `json:"sip_port"`           // sip 服务器端口，默认 5060
	Serial            string             `json:"serial"`             // sip 服务器 id, 默认 34020000002000000001
	Realm             string             `json:"realm"`              // sip 服务器域，默认 3402000000
	Username          string             `json:"username"`           // sip 服务器账号
	Password          string             `json:"password"`           // sip 服务器密码
	KeepaliveInterval int                `json:"keepalive_interval"` // 心跳包时长
	QuickLogin        bool               `json:"quick_login"`        // 快速登陆,有keepalive就认为在线
	MediaConfig       GB28181MediaConfig `json:"media_config"`       // 媒体服务器配置
}

type GB28181MediaConfig struct {
	MediaIp               string `json:"media_ip"`                 // 流媒体IP,用于在SDP中指定
	ListenPort            uint16 `json:"listen_port"`              // tcp,udp监听端口 默认启动
	MultiPortMaxIncrement uint16 `json:"multi_port_max_increment"` //多端口范围 ListenPort+1至ListenPort+MultiPortMax
}

type OnvifConfig struct {
	Enable bool `json:"enable"` // onvif使能标志
}

type HttpNotifyConfig struct {
	Enable            bool   `json:"enable"`
	UpdateIntervalSec int    `json:"update_interval_sec"`
	OnServerStart     string `json:"on_server_start"`
	OnUpdate          string `json:"on_update"`
	OnPubStart        string `json:"on_pub_start"`
	OnPubStop         string `json:"on_pub_stop"`
	OnSubStart        string `json:"on_sub_start"`
	OnSubStop         string `json:"on_sub_stop"`
	OnRelayPullStart  string `json:"on_relay_pull_start"`
	OnRelayPullStop   string `json:"on_relay_pull_stop"`
	OnRtmpConnect     string `json:"on_rtmp_connect"`
	OnHlsMakeTs       string `json:"on_hls_make_ts"`
}

type HookConfig struct {
	GopCacheNum          int `json:"gop_cache_num"`
	SingleGopMaxFrameNum int `json:"single_gop_max_frame_num"`
}

type RoomConfig struct {
	Enable    bool   `json:"enable"`     // room功能使能标志
	APIKey    string `json:"api_key"`    // livekit api key
	APISecret string `json:"api_secret"` // livekit api secret
}

func Open(filepath string) error {
	data, err := ioutil.ReadFile(filepath)
	if err != nil {
		return err
	}

	err = json.Unmarshal(data, &defaultConfig)
	if err != nil {
		return err
	}
	return nil
}

func GetConfig() *Config {
	return &defaultConfig
}
