package livestream

import (
	"easydarwin/utils/pkg/web"
	"time"
)

const KICK_OUT_API = "/api/ctrl/kick_session"
const GROUPS_INFO_API = "/api/groups"

const (
	LIVE_PUSH = "push"
	LIVE_PULL = "pull"
)

type LiveStream struct {
	ID          int       `gorm:"primaryKey;" json:"id"`
	Name        string    `json:"name"`                          // 名称
	Url         string    `json:"url"`                           // 地址
	LiveType    string    `json:"liveType"`                      // 类型
	Online      int       `json:"online"`                        // 状态
	Enable      bool      `gorm:"default:true" json:"enable"`    // 启用
	OnDemand    bool      `gorm:"default:false" json:"onDemand"` // 按需
	Audio       bool      `gorm:"default:false" json:"audio"`    // 音频
	TransType   string    `json:"transType"`                     // 协议
	SnapURL     string    `json:"snapURL"`                       // 快照
	KeyFrame    string    `json:"keyFrame"`                      // i帧
	Authed      bool      `json:"authed"`                        // 是否启用推流验证
	SessionId   string    `json:"sessionId"`                     // 推流标识
	Sign        string    `json:"sign"`                          // 推流验证
	ErrorString string    `json:"errorString"`                   // 错误信息
	IsLive      bool      `json:"isLive"`                        // 拉流 在线流 文件
	SpeedEnum   int       `json:"speedEnum"`                     // 点播拉流倍数
	LastAt      time.Time `json:"last_at"`                       // 过期时间

}

// TableName ...
func (*LiveStream) TableName() string {
	return "live_stream"
}

type LiveInput struct {
	Name      string `json:"name"`                          // 名称
	Url       string `json:"url"`                           // 地址
	Enable    bool   `gorm:"default:false" json:"enable"`   // 启用
	OnDemand  bool   `gorm:"default:false" json:"onDemand"` // 按需
	Audio     bool   `gorm:"default:false" json:"audio"`    // 音频
	TransType string `json:"transType"`                     // 协议
	IsLive    bool   `json:"isLive"`                        // 拉流 在线流 文件
	SpeedEnum int    `json:"speedEnum"`                     // 点播拉流倍数

}
type PushInput struct {
	Name     string `json:"name"`                          // 名称
	Enable   bool   `gorm:"default:false" json:"enable"`   // 启用
	Authed   bool   `json:"authed"`                        // 是否启用推流验证
	OnDemand bool   `gorm:"default:false" json:"onDemand"` // 按需
}

// PagerFilter 分页过滤
type PagerFilter struct {
	Q    string `form:"q"`
	Type string `form:"type"`
	web.PagerFilter
}

type LivePlayer struct {
	Name    string `json:"name"` // 名称
	ID      int    `json:"id"`
	HttpFlv string `json:"http_flv"`
	HttpHls string `json:"http_hls"`
	WsFlv   string `json:"ws_flv"`
	WEBRTC  string `json:"webrtc"`
	RTMP    string `json:"rtmp"`
	RTSP    string `json:"rtsp"`
}

type PubInfo struct {
	ServerId      string `json:"server_id"`
	Protocol      string `json:"protocol"`
	Url           string `json:"url"`
	AppName       string `json:"app_name"`
	StreamName    string `json:"stream_name"`
	UrlParam      string `json:"url_param"`
	SessionId     string `json:"session_id"`
	RemotetAddr   string `json:"remotet_addr"`
	HasInSession  bool   `json:"has_in_session"`
	HasOutSession bool   `json:"has_out_session"`
}
type PubConnectInfo struct {
	ServerId   string `json:"server_id"`
	SessionId  string `json:"session_id"`
	RemoteAddr string `json:"remote_addr"`
	App        string `json:"app"`
	FlashVer   string `json:"flashVer"`
	TcUrl      string `json:"tcUrl"`
}

type OutSession struct {
	StreamName string `json:"stream_name"`
	SessionID  string `json:"session_id"`
}
type OutResponse struct {
	ErrorCode int    `json:"error_code"`
	Desp      string `json:"desp"`
}

type Response struct {
	Code int    `json:"code"`
	Msg  string `json:"msg"`
	Data string `json:"data"`
}
