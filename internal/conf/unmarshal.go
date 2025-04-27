package conf

import (
	"easydarwin/internal/gutils/efile"
	config "easydarwin/pkg/lalmax/conf"
	"fmt"
	"github.com/q191201771/lal/pkg/logic"
	"github.com/spf13/viper"
)

//func SetupConfig(v any, path string) error {
//	b, err := os.ReadFile(path)
//	if err != nil {
//		return err
//	}
//	return toml.Unmarshal(b, v)
//}

func SetupConfig(path string) (Bootstrap, error) {
	var bc Bootstrap
	vtoml := viper.New()
	vtoml.SetConfigName("config")
	vtoml.SetConfigType("toml")
	vtoml.AddConfigPath(path)
	if err := vtoml.ReadInConfig(); err != nil {
		return bc, err
	}
	err := vtoml.Unmarshal(&bc)
	if err != nil {
		return bc, err
	}

	var cfg config.Config
	var lgcConfig logic.Config
	lgcConfig.DefaultHttpConfig.HttpListenAddr = ":8080"
	lgcConfig.DefaultHttpConfig.HttpsListenAddr = ":4443"
	lgcConfig.DefaultHttpConfig.HttpsCertFile = "./config/cert.pem"
	lgcConfig.DefaultHttpConfig.HttpsKeyFile = "./config/key.pem"
	SetLogicConfig(bc, &lgcConfig)
	bc.LogicCfg = lgcConfig

	//cfg.LalSvrConfigPath = filepath.Join(Getwd(), "configs", "lalserver.conf.json")
	//rawContent, err := os.ReadFile(cfg.LalSvrConfigPath)
	//if err == nil {
	//	json.Unmarshal(rawContent, &bc.LogicCfg)
	//}

	SetConfig(bc, &cfg)
	bc.Config = &cfg

	bc.VodConfig.Dir = efile.GetRealPath(bc.VodConfig.Dir)
	bc.VodConfig.SrcDir = efile.GetRealPath(bc.VodConfig.SrcDir)

	efile.EnsureDir(bc.VodConfig.Dir)
	efile.EnsureDir(bc.VodConfig.SrcDir)

	return bc, nil
}

func SetLogicConfig(bc Bootstrap, cfg *logic.Config) {
	cfg.ConfVersion = "v0.4.1"
	cfg.LogConfig = bc.LogConfig
	cfg.RtspConfig.Addr = bc.RtspConfig.Addr
	cfg.RtspConfig.AuthEnable = bc.RtspConfig.AuthEnable
	cfg.RtspConfig.AuthMethod = bc.RtspConfig.AuthMethod
	cfg.RtspConfig.Enable = bc.RtspConfig.Enable
	cfg.RtspConfig.OutWaitKeyFrameFlag = bc.RtspConfig.OutWaitKeyFrameFlag
	cfg.RtspConfig.PassWord = bc.RtspConfig.PassWord
	cfg.RtspConfig.RtspsAddr = bc.RtspConfig.RtspsAddr
	cfg.RtspConfig.RtspsCertFile = bc.RtspConfig.RtspsCertFile
	cfg.RtspConfig.RtspsEnable = bc.RtspConfig.Enable
	cfg.RtspConfig.RtspsKeyFile = bc.RtspConfig.RtspsKeyFile
	cfg.RtspConfig.UserName = bc.RtspConfig.UserName
	cfg.RtspConfig.WsRtspAddr = bc.RtspConfig.WsRtspAddr
	cfg.RtspConfig.WsRtspEnable = bc.RtspConfig.WsRtspEnable
	cfg.RtmpConfig.Addr = bc.RtmpConfig.Addr
	cfg.RtmpConfig.Enable = bc.RtmpConfig.Enable
	cfg.RtmpConfig.MergeWriteSize = bc.RtmpConfig.MergeWriteSize
	cfg.RtmpConfig.RtmpsAddr = bc.RtmpConfig.RtmpsAddr
	cfg.RtmpConfig.RtmpsCertFile = bc.RtmpConfig.RtmpsCertFile
	cfg.RtmpConfig.RtmpsEnable = bc.RtmpConfig.RtmpsEnable
	cfg.RtmpConfig.RtmpsKeyFile = bc.RtmpConfig.RtmpsKeyFile
	cfg.InSessionConfig.AddDummyAudioEnable = bc.InSessionConfig.AddDummyAudioEnable
	cfg.InSessionConfig.AddDummyAudioWaitAudioMs = bc.InSessionConfig.AddDummyAudioWaitAudioMs
	cfg.HttpflvConfig.Enable = bc.HttpflvConfig.Enable
	cfg.HttpflvConfig.UrlPattern = bc.HttpflvConfig.UrlPattern
	cfg.HttpflvConfig.EnableHttps = bc.HttpflvConfig.EnableHttps
	cfg.HttpflvConfig.GopNum = 0
	cfg.HttpflvConfig.SingleGopMaxFrameNum = 0
	cfg.HlsConfig.CleanupMode = bc.HlsConfig.CleanupMode
	cfg.HlsConfig.DeleteThreshold = bc.HlsConfig.DeleteThreshold
	cfg.HlsConfig.Enable = bc.HlsConfig.Enable
	cfg.HlsConfig.EnableHttps = bc.HlsConfig.EnableHttps
	cfg.HlsConfig.FragmentDurationMs = bc.HlsConfig.FragmentDurationMs
	cfg.HlsConfig.FragmentNum = bc.HlsConfig.FragmentNum
	cfg.HlsConfig.OutPath = bc.HlsConfig.OutPath
	cfg.HlsConfig.SubSessionHashKey = bc.HlsConfig.SubSessionHashKey
	cfg.HlsConfig.SubSessionTimeoutMs = bc.HlsConfig.SubSessionTimeoutMs
	cfg.HlsConfig.UrlPattern = bc.HlsConfig.UrlPattern
	cfg.HlsConfig.UseMemoryAsDiskFlag = bc.HlsConfig.UseMemoryAsDiskFlag
	cfg.RecordConfig.EnableFlv = bc.RecordConfig.EnableFlv
	cfg.RecordConfig.FlvOutPath = bc.RecordConfig.FlvOutPath
	cfg.RecordConfig.EnableMpegts = bc.RecordConfig.EnableMpegts
	cfg.RecordConfig.MpegtsOutPath = bc.RecordConfig.MpegtsOutPath
	cfg.RelayPushConfig.Enable = bc.RelayPushConfig.Enable
	cfg.RelayPushConfig.AddrList = bc.RelayPushConfig.AddrList
	cfg.StaticRelayPullConfig.Enable = bc.StaticRelayPullConfig.Enable
	cfg.StaticRelayPullConfig.Addr = bc.StaticRelayPullConfig.Addr

	cfg.ServerId = "1"
	host := fmt.Sprintf("http://127.0.0.1%s", bc.DefaultHttpConfig.HttpListenAddr)
	cfg.HttpNotifyConfig.Enable = true
	cfg.HttpNotifyConfig.UpdateIntervalSec = 5
	cfg.HttpNotifyConfig.OnPubStart = fmt.Sprintf("%s/api/v1/push/on_pub_start", host)
	cfg.HttpNotifyConfig.OnPubStop = fmt.Sprintf("%s/api/v1/push/on_pub_stop", host)
	cfg.HttpNotifyConfig.OnRtmpConnect = fmt.Sprintf("%s/api/v1/push/on_rtmp_connect", host)
	cfg.PprofConfig.Enable = false
	cfg.PprofConfig.Addr = ":8084"
	cfg.LogConfig.AssertBehavior = bc.LogConfig.AssertBehavior
	cfg.LogConfig.Filename = bc.LogConfig.Filename
	cfg.LogConfig.IsRotateDaily = bc.LogConfig.IsRotateDaily
	cfg.LogConfig.IsToStdout = bc.LogConfig.IsToStdout
	cfg.LogConfig.Level = bc.LogConfig.Level
	cfg.LogConfig.LevelFlag = bc.LogConfig.LevelFlag
	cfg.LogConfig.ShortFileFlag = bc.LogConfig.ShortFileFlag
	cfg.LogConfig.TimestampFlag = bc.LogConfig.TimestampFlag
	cfg.LogConfig.TimestampWithMsFlag = bc.LogConfig.TimestampWithMsFlag
	cfg.DebugConfig.LogGroupIntervalSec = 30
	cfg.DebugConfig.LogGroupMaxGroupNum = 10
	cfg.DebugConfig.LogGroupMaxSubNumPerGroup = 10
}

func SetConfig(bc Bootstrap, cfg *config.Config) {
	cfg.HttpConfig.ListenAddr = bc.DefaultHttpConfig.HttpListenAddr
	cfg.HttpConfig.HttpsListenAddr = bc.DefaultHttpConfig.HttpsListenAddr
	cfg.HttpConfig.EnableHttps = true
	cfg.HttpConfig.HttpsKeyFile = bc.DefaultHttpConfig.HttpsKeyFile
	cfg.HttpConfig.HttpsCertFile = bc.DefaultHttpConfig.HttpsCertFile

	cfg.RtcConfig.Enable = bc.RtcConfig.Enable
	cfg.RtcConfig.ICETCPMuxPort = bc.RtcConfig.ICETCPMuxPort
	cfg.RtcConfig.ICEUDPMuxPort = bc.RtcConfig.ICEUDPMuxPort
	cfg.RtcConfig.ICEHostNATToIPs = bc.RtcConfig.ICEHostNATToIPs

	cfg.HttpNotifyConfig.Enable = bc.LogicCfg.HttpNotifyConfig.Enable
	cfg.HttpNotifyConfig.UpdateIntervalSec = bc.LogicCfg.HttpNotifyConfig.UpdateIntervalSec
	cfg.HttpNotifyConfig.OnPubStart = bc.LogicCfg.HttpNotifyConfig.OnPubStart
	cfg.HttpNotifyConfig.OnPubStop = bc.LogicCfg.HttpNotifyConfig.OnPubStop
	cfg.HttpNotifyConfig.OnRtmpConnect = bc.LogicCfg.HttpNotifyConfig.OnRtmpConnect
}
