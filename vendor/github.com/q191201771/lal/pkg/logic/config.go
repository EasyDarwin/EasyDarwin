// Copyright 2019, Chef.  All rights reserved.
// https://github.com/q191201771/lal
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

package logic

import (
	"encoding/json"
	"fmt"
	"os"
	"strings"

	"github.com/q191201771/lal/pkg/base"
	"github.com/q191201771/lal/pkg/hls"
	"github.com/q191201771/lal/pkg/rtsp"
	"github.com/q191201771/naza/pkg/nazajson"
	"github.com/q191201771/naza/pkg/nazalog"
)

const (
	defaultHlsCleanupMode    = hls.CleanupModeInTheEnd
	defaultHttpflvUrlPattern = "/live/"
	defaultHttptsUrlPattern  = "/live/"
	defaultHlsUrlPattern     = "/hls/"
)

type Config struct {
	ConfVersion           string                `json:"conf_version"`
	RtmpConfig            RtmpConfig            `json:"rtmp"`
	InSessionConfig       InSessionConfig       `json:"in_session"`
	DefaultHttpConfig     DefaultHttpConfig     `json:"default_http"`
	HttpflvConfig         HttpflvConfig         `json:"httpflv"`
	HlsConfig             HlsConfig             `json:"hls"`
	HttptsConfig          HttptsConfig          `json:"httpts"`
	RtspConfig            RtspConfig            `json:"rtsp"`
	RecordConfig          RecordConfig          `json:"record"`
	RelayPushConfig       RelayPushConfig       `json:"relay_push"`
	StaticRelayPullConfig StaticRelayPullConfig `json:"static_relay_pull"`

	HttpApiConfig    HttpApiConfig    `json:"http_api"`
	ServerId         string           `json:"server_id"`
	HttpNotifyConfig HttpNotifyConfig `json:"http_notify"`
	SimpleAuthConfig SimpleAuthConfig `json:"simple_auth"`
	PprofConfig      PprofConfig      `json:"pprof"`
	LogConfig        nazalog.Option   `json:"log"`
	DebugConfig      DebugConfig      `json:"debug"`
}

type RtmpConfig struct {
	Enable               bool   `json:"enable"`
	Addr                 string `json:"addr"`
	RtmpsEnable          bool   `json:"rtmps_enable"`
	RtmpsAddr            string `json:"rtmps_addr"`
	RtmpsCertFile        string `json:"rtmps_cert_file"`
	RtmpsKeyFile         string `json:"rtmps_key_file"`
	GopNum               int    `json:"gop_num"` // TODO(chef): refactor 更名为gop_cache_num
	SingleGopMaxFrameNum int    `json:"single_gop_max_frame_num"`
	MergeWriteSize       int    `json:"merge_write_size"`
}

type InSessionConfig struct {
	AddDummyAudioEnable      bool `json:"add_dummy_audio_enable"`
	AddDummyAudioWaitAudioMs int  `json:"add_dummy_audio_wait_audio_ms"`
}

type DefaultHttpConfig struct {
	CommonHttpAddrConfig
}

type HttpflvConfig struct {
	CommonHttpServerConfig

	GopNum               int `json:"gop_num"`
	SingleGopMaxFrameNum int `json:"single_gop_max_frame_num"`
}

type HttptsConfig struct {
	CommonHttpServerConfig

	GopNum               int `json:"gop_num"`
	SingleGopMaxFrameNum int `json:"single_gop_max_frame_num"`
}

type HlsConfig struct {
	CommonHttpServerConfig

	UseMemoryAsDiskFlag bool `json:"use_memory_as_disk_flag"`
	hls.MuxerConfig
	SubSessionTimeoutMs int    `json:"sub_session_timeout_ms"`
	SubSessionHashKey   string `json:"sub_session_hash_key"`
}

type RtspConfig struct {
	Enable              bool   `json:"enable"`
	Addr                string `json:"addr"`
	RtspsEnable         bool   `json:"rtsps_enable"`
	RtspsAddr           string `json:"rtsps_addr"`
	RtspsCertFile       string `json:"rtsps_cert_file"`
	RtspsKeyFile        string `json:"rtsps_key_file"`
	OutWaitKeyFrameFlag bool   `json:"out_wait_key_frame_flag"`
	WsRtspEnable        bool   `json:"ws_rtsp_enable"`
	WsRtspAddr          string `json:"ws_rtsp_addr"`
	rtsp.ServerAuthConfig
}

type RecordConfig struct {
	EnableFlv     bool   `json:"enable_flv"`
	FlvOutPath    string `json:"flv_out_path"`
	EnableMpegts  bool   `json:"enable_mpegts"`
	MpegtsOutPath string `json:"mpegts_out_path"`
}

type RelayPushConfig struct {
	Enable   bool     `json:"enable"`
	AddrList []string `json:"addr_list"`
}

type StaticRelayPullConfig struct {
	Enable bool   `json:"enable"`
	Addr   string `json:"addr"`
}

type HttpApiConfig struct {
	Enable bool   `json:"enable"`
	Addr   string `json:"addr"`
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

type SimpleAuthConfig struct {
	Key                string `json:"key"`
	DangerousLalSecret string `json:"dangerous_lal_secret"`
	PubRtmpEnable      bool   `json:"pub_rtmp_enable"`
	SubRtmpEnable      bool   `json:"sub_rtmp_enable"`
	SubHttpflvEnable   bool   `json:"sub_httpflv_enable"`
	SubHttptsEnable    bool   `json:"sub_httpts_enable"`
	PubRtspEnable      bool   `json:"pub_rtsp_enable"`
	SubRtspEnable      bool   `json:"sub_rtsp_enable"`
	HlsM3u8Enable      bool   `json:"hls_m3u8_enable"`
}

type PprofConfig struct {
	Enable bool   `json:"enable"`
	Addr   string `json:"addr"`
}

type DebugConfig struct {
	LogGroupIntervalSec       int `json:"log_group_interval_sec"`
	LogGroupMaxGroupNum       int `json:"log_group_max_group_num"`
	LogGroupMaxSubNumPerGroup int `json:"log_group_max_sub_num_per_group"`
}

type CommonHttpServerConfig struct {
	CommonHttpAddrConfig

	Enable      bool   `json:"enable"`
	EnableHttps bool   `json:"enable_https"`
	UrlPattern  string `json:"url_pattern"`
}

type CommonHttpAddrConfig struct {
	HttpListenAddr  string `json:"http_listen_addr"`
	HttpsListenAddr string `json:"https_listen_addr"`
	HttpsCertFile   string `json:"https_cert_file"`
	HttpsKeyFile    string `json:"https_key_file"`
}

func LoadConfAndInitLog(rawContent []byte) *Config {
	var config *Config

	// 读取配置并解析原始内容
	if err := json.Unmarshal(rawContent, &config); err != nil {
		_, _ = fmt.Fprintf(os.Stderr, "unmarshal conf file failed. raw content=%s err=%+v", rawContent, err)
		base.OsExitAndWaitPressIfWindows(1)
	}

	j, err := nazajson.New(rawContent)
	if err != nil {
		_, _ = fmt.Fprintf(os.Stderr, "nazajson unmarshal conf file failed. raw content=%s err=%+v", rawContent, err)
		base.OsExitAndWaitPressIfWindows(1)
	}

	// 初始化日志模块，注意，这一步尽量提前，使得后续的日志内容按我们的日志配置输出
	//
	// 日志配置项不存在时，设置默认值
	//
	// 注意，由于此时日志模块还没有初始化，所以有日志需要打印时，我们采用先缓存后打印（日志模块初始化成功后再打印）的方式
	var cacheLog []string
	if !j.Exist("log.level") {
		config.LogConfig.Level = nazalog.LevelDebug
		cacheLog = append(cacheLog, fmt.Sprintf("log.level=%s", config.LogConfig.Level.ReadableString()))
	}
	if !j.Exist("log.filename") {
		config.LogConfig.Filename = "./logs/lalserver.log"
		cacheLog = append(cacheLog, fmt.Sprintf("log.filename=%s", config.LogConfig.Filename))
	}
	if !j.Exist("log.is_to_stdout") {
		config.LogConfig.IsToStdout = true
		cacheLog = append(cacheLog, fmt.Sprintf("log.is_to_stdout=%v", config.LogConfig.IsToStdout))
	}
	if !j.Exist("log.is_rotate_daily") {
		config.LogConfig.IsRotateDaily = true
		cacheLog = append(cacheLog, fmt.Sprintf("log.is_rotate_daily=%v", config.LogConfig.IsRotateDaily))
	}
	if !j.Exist("log.short_file_flag") {
		config.LogConfig.ShortFileFlag = true
		cacheLog = append(cacheLog, fmt.Sprintf("log.short_file_flag=%v", config.LogConfig.ShortFileFlag))
	}
	if !j.Exist("log.timestamp_flag") {
		config.LogConfig.TimestampFlag = true
		cacheLog = append(cacheLog, fmt.Sprintf("log.timestamp_flag=%v", config.LogConfig.TimestampFlag))
	}
	if !j.Exist("log.timestamp_with_ms_flag") {
		config.LogConfig.TimestampWithMsFlag = true
		cacheLog = append(cacheLog, fmt.Sprintf("log.timestamp_with_ms_flag=%v", config.LogConfig.TimestampWithMsFlag))
	}
	if !j.Exist("log.level_flag") {
		config.LogConfig.LevelFlag = true
		cacheLog = append(cacheLog, fmt.Sprintf("log.level_flag=%v", config.LogConfig.LevelFlag))
	}
	if !j.Exist("log.assert_behavior") {
		config.LogConfig.AssertBehavior = nazalog.AssertError
		cacheLog = append(cacheLog, fmt.Sprintf("log.assert_behavior=%s", config.LogConfig.AssertBehavior.ReadableString()))
	}

	if err := Log.Init(func(option *nazalog.Option) {
		*option = config.LogConfig
	}); err != nil {
		_, _ = fmt.Fprintf(os.Stderr, "initial log failed. err=%+v\n", err)
		base.OsExitAndWaitPressIfWindows(1)
	}
	Log.Info("initial log succ.")

	// 打印Logo
	Log.Info(`
    __    ___    __
   / /   /   |  / /
  / /   / /| | / /
 / /___/ ___ |/ /___
/_____/_/  |_/_____/
`)

	// 检查配置版本号是否匹配
	if config.ConfVersion != base.ConfVersion {
		Log.Warnf("config version invalid. conf version of lalserver=%s, conf version of config file=%s",
			base.ConfVersion, config.ConfVersion)
	}

	// 做个全量字段检查，缺失的字段，Go中会先设置为零值
	notExistFields, err := nazajson.CollectNotExistFields(rawContent, config,
		"log.",
		"default_http.http_listen_addr", "default_http.https_listen_addr", "default_http.https_cert_file", "default_http.https_key_file",
		"httpflv.http_listen_addr", "httpflv.https_listen_addr", "httpflv.https_cert_file", "httpflv.https_key_file",
		"hls.http_listen_addr", "hls.https_listen_addr", "hls.https_cert_file", "hls.https_key_file",
		"httpts.http_listen_addr", "httpts.https_listen_addr", "httpts.https_cert_file", "httpts.https_key_file",
	)
	if err != nil {
		Log.Warnf("config nazajson collect not exist fields failed. err=%+v", err)
	}
	if len(notExistFields) != 0 {
		Log.Warnf("config some fields do not exist which have been set to the zero value. fields=%+v", notExistFields)
	}

	// 日志字段检查，缺失的字段，打印前面设置的默认值
	if len(cacheLog) > 0 {
		Log.Warnf("config some log fields do not exist which have been set to default value. %s", strings.Join(cacheLog, ", "))
	}

	// 如果具体的HTTP应用没有设置HTTP监听相关的配置，则尝试使用全局配置
	mergeCommonHttpAddrConfig(&config.HttpflvConfig.CommonHttpAddrConfig, &config.DefaultHttpConfig.CommonHttpAddrConfig)
	mergeCommonHttpAddrConfig(&config.HttptsConfig.CommonHttpAddrConfig, &config.DefaultHttpConfig.CommonHttpAddrConfig)
	mergeCommonHttpAddrConfig(&config.HlsConfig.CommonHttpAddrConfig, &config.DefaultHttpConfig.CommonHttpAddrConfig)

	// 为缺失的字段中的一些特定字段，设置特定默认值
	if config.HlsConfig.Enable && !j.Exist("hls.cleanup_mode") {
		Log.Warnf("config hls.cleanup_mode not exist. set to default which is %d", defaultHlsCleanupMode)
		config.HlsConfig.CleanupMode = defaultHlsCleanupMode
	}
	if config.HlsConfig.Enable && !j.Exist("hls.delete_threshold") {
		Log.Warnf("config hls.delete_threshold not exist. set to default same as hls.fragment_num which is %d",
			config.HlsConfig.FragmentNum)
		config.HlsConfig.DeleteThreshold = config.HlsConfig.FragmentNum
	}
	if config.HlsConfig.SubSessionHashKey != "" && config.HlsConfig.SubSessionTimeoutMs == 0 {
		// 没有设置超时值，或者超时为0时
		Log.Warnf("config hls.sub_session_timeout_ms is 0. set to %d(which is fragment_num * fragment_duration_ms * 2)",
			config.HlsConfig.FragmentNum*config.HlsConfig.FragmentDurationMs*2)
		config.HlsConfig.SubSessionTimeoutMs = config.HlsConfig.FragmentNum * config.HlsConfig.FragmentDurationMs * 2
	}
	if (config.HttpflvConfig.Enable || config.HttpflvConfig.EnableHttps) && !j.Exist("httpflv.url_pattern") {
		Log.Warnf("config httpflv.url_pattern not exist. set to default which is %s", defaultHttpflvUrlPattern)
		config.HttpflvConfig.UrlPattern = defaultHttpflvUrlPattern
	}
	if (config.HttptsConfig.Enable || config.HttptsConfig.EnableHttps) && !j.Exist("httpts.url_pattern") {
		Log.Warnf("config httpts.url_pattern not exist. set to default which is %s", defaultHttptsUrlPattern)
		config.HttptsConfig.UrlPattern = defaultHttptsUrlPattern
	}
	if (config.HlsConfig.Enable || config.HlsConfig.EnableHttps) && !j.Exist("hls.url_pattern") {
		Log.Warnf("config hls.url_pattern not exist. set to default which is %s", defaultHlsUrlPattern)
		config.HttpflvConfig.UrlPattern = defaultHlsUrlPattern
	}

	// 对一些常见的格式错误做修复
	// 确保url pattern以`/`开始，并以`/`结束
	if urlPattern, changed := ensureStartAndEndWithSlash(config.HttpflvConfig.UrlPattern); changed {
		Log.Warnf("fix config. httpflv.url_pattern %s -> %s", config.HttpflvConfig.UrlPattern, urlPattern)
		config.HttpflvConfig.UrlPattern = urlPattern
	}
	if urlPattern, changed := ensureStartAndEndWithSlash(config.HttptsConfig.UrlPattern); changed {
		Log.Warnf("fix config. httpts.url_pattern %s -> %s", config.HttptsConfig.UrlPattern, urlPattern)
		config.HttpflvConfig.UrlPattern = urlPattern
	}
	if urlPattern, changed := ensureStartAndEndWithSlash(config.HlsConfig.UrlPattern); changed {
		Log.Warnf("fix config. hls.url_pattern %s -> %s", config.HlsConfig.UrlPattern, urlPattern)
		config.HttpflvConfig.UrlPattern = urlPattern
	}

	// 打印配置文件中的元素内容，以及解析后的最终值
	// 把配置文件原始内容中的换行去掉，使得打印日志时紧凑一些
	lines := strings.Split(string(rawContent), "\n")
	if len(lines) == 1 {
		lines = strings.Split(string(rawContent), "\r\n")
	}
	var tlines []string
	for _, l := range lines {
		tlines = append(tlines, strings.TrimSpace(l))
	}
	compactRawContent := strings.Join(tlines, " ")
	Log.Infof("load conf succ. raw content=%s parsed=%+v", compactRawContent, config)

	return config
}

// ---------------------------------------------------------------------------------------------------------------------

func mergeCommonHttpAddrConfig(dst, src *CommonHttpAddrConfig) {
	if dst.HttpListenAddr == "" && src.HttpListenAddr != "" {
		dst.HttpListenAddr = src.HttpListenAddr
	}
	if dst.HttpsListenAddr == "" && src.HttpsListenAddr != "" {
		dst.HttpsListenAddr = src.HttpsListenAddr
	}
	if dst.HttpsCertFile == "" && src.HttpsCertFile != "" {
		dst.HttpsCertFile = src.HttpsCertFile
	}
	if dst.HttpsKeyFile == "" && src.HttpsKeyFile != "" {
		dst.HttpsKeyFile = src.HttpsKeyFile
	}
}

func ensureStartWithSlash(in string) (out string, changed bool) {
	if in == "" {
		return in, false
	}
	if in[0] == '/' {
		return in, false
	}
	return "/" + in, true
}

func ensureEndWithSlash(in string) (out string, changed bool) {
	if in == "" {
		return in, false
	}
	if in[len(in)-1] == '/' {
		return in, false
	}
	return in + "/", true
}

func ensureStartAndEndWithSlash(in string) (out string, changed bool) {
	n, c := ensureStartWithSlash(in)
	n2, c2 := ensureEndWithSlash(n)
	return n2, c || c2
}
