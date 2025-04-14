package gb28181

import (
	"errors"
	"fmt"
	"net/http"
	"strconv"
	"strings"
	"time"

	"github.com/q191201771/naza/pkg/nazaatomic"

	config "easydarwin/pkg/lalmax/conf"
	"easydarwin/pkg/lalmax/gb28181/mediaserver"

	"github.com/ghettovoice/gosip/sip"
	"github.com/q191201771/naza/pkg/nazalog"
)

type Channel struct {
	device *Device // 所属设备
	//status  atomic.Int32 // 通道状态,0:空闲,1:正在invite,2:正在播放
	GpsTime time.Time // gps时间
	number  uint16
	ackReq  sip.Request

	observer IMediaOpObserver
	playInfo *PlayInfo

	ChannelInfo
	conf config.GB28181Config
}

// Channel 通道
type ChannelInfo struct {
	ChannelId    string        `xml:"DeviceID"`     // 设备id
	ParentId     string        `xml:"ParentID"`     //父目录Id
	Name         string        `xml:"Name"`         //设备名称
	Manufacturer string        `xml:"Manufacturer"` //制造厂商
	Model        string        `xml:"Model"`        //型号
	Owner        string        `xml:"Owner"`        //设备归属
	CivilCode    string        `xml:"CivilCode"`    //行政区划编码
	Address      string        `xml:"Address"`      //地址
	Port         int           `xml:"Port"`         //端口
	Parental     int           `xml:"Parental"`     //存在子设备，这里表明有子目录存在 1代表有子目录，0表示没有
	SafetyWay    int           `xml:"SafetyWay"`    //信令安全模式（可选）缺省为 0；0：不采用；2：S/MIME 签名方式；3：S/MIME	加密签名同时采用方式；4：数字摘要方式
	RegisterWay  int           `xml:"RegisterWay"`  //标准的认证注册模式
	Secrecy      int           `xml:"Secrecy"`      //0 表示不涉密
	Status       ChannelStatus `xml:"Status"`       // 状态  on 在线 off离线
	Longitude    string        `xml:"Longitude"`    // 经度
	Latitude     string        `xml:"Latitude"`     // 纬度
	StreamName   string        `xml:"-"`
	serial       string
	mediaserver.MediaInfo
	sn nazaatomic.Uint32
}

type ChannelStatus string

const (
	ChannelOnStatus  = "ON"
	ChannelOffStatus = "OFF"
)

func (channel *Channel) WithMediaServer(observer IMediaOpObserver) {
	channel.observer = observer
}

func (channel *Channel) TryAutoInvite(opt *InviteOptions, streamName string, playInfo *PlayInfo) {
	if channel.CanInvite(streamName) {
		go channel.Invite(opt, streamName, playInfo)
	}
}

func (channel *Channel) CanInvite(streamName string) bool {
	if len(channel.ChannelId) != 20 || channel.Status == ChannelOffStatus {
		nazalog.Info("return false,  channel.DeviceID:", len(channel.ChannelId), " channel.Status:", channel.Status)
		return false
	}
	if channel.Parental != 0 {
		return false
	}

	if channel.MediaInfo.IsInvite {
		return false
	}

	// 11～13位是设备类型编码
	typeID := channel.ChannelId[10:13]
	if typeID == "132" || typeID == "131" {
		return true
	}

	nazalog.Info("return false")

	return false
}

// Invite 发送Invite报文 invites a channel to play
// 注意里面的锁保证不同时发送invite报文，该锁由channel持有
/***
f字段： f = v/编码格式/分辨率/帧率/码率类型/码率大小a/编码格式/码率大小/采样率
各项具体含义：
    v：后续参数为视频的参数；各参数间以 “/”分割；
编码格式：十进制整数字符串表示
1 –MPEG-4 2 –H.264 3 – SVAC 4 –3GP
    分辨率：十进制整数字符串表示
1 – QCIF 2 – CIF 3 – 4CIF 4 – D1 5 –720P 6 –1080P/I
帧率：十进制整数字符串表示 0～99
码率类型：十进制整数字符串表示
1 – 固定码率（CBR）     2 – 可变码率（VBR）
码率大小：十进制整数字符串表示 0～100000（如 1表示1kbps）
    a：后续参数为音频的参数；各参数间以 “/”分割；
编码格式：十进制整数字符串表示
1 – G.711    2 – G.723.1     3 – G.729      4 – G.722.1
码率大小：十进制整数字符串
音频编码码率： 1 — 5.3 kbps （注：G.723.1中使用）
   2 — 6.3 kbps （注：G.723.1中使用）
   3 — 8 kbps （注：G.729中使用）
   4 — 16 kbps （注：G.722.1中使用）
   5 — 24 kbps （注：G.722.1中使用）
   6 — 32 kbps （注：G.722.1中使用）
   7 — 48 kbps （注：G.722.1中使用）
   8 — 64 kbps（注：G.711中使用）
采样率：十进制整数字符串表示
	1 — 8 kHz（注：G.711/ G.723.1/ G.729中使用）
	2—14 kHz（注：G.722.1中使用）
	3—16 kHz（注：G.722.1中使用）
	4—32 kHz（注：G.722.1中使用）
	注1：字符串说明
本节中使用的“十进制整数字符串”的含义为“0”～“4294967296” 之间的十进制数字字符串。
注2：参数分割标识
各参数间以“/”分割，参数间的分割符“/”不能省略；
若两个分割符 “/”间的某参数为空时（即两个分割符 “/”直接将相连时）表示无该参数值；
注3：f字段说明
使用f字段时，应保证视频和音频参数的结构完整性，即在任何时候，f字段的结构都应是完整的结构：
f = v/编码格式/分辨率/帧率/码率类型/码率大小a/编码格式/码率大小/采样率
若只有视频时，音频中的各参数项可以不填写，但应保持 “a///”的结构:
f = v/编码格式/分辨率/帧率/码率类型/码率大小a///
若只有音频时也类似处理，视频中的各参数项可以不填写，但应保持 “v/”的结构：
f = v/a/编码格式/码率大小/采样率
f字段中视、音频参数段之间不需空格分割。
可使用f字段中的分辨率参数标识同一设备不同分辨率的码流。
*/

func (channel *Channel) Invite(opt *InviteOptions, streamName string, playInfo *PlayInfo) (code int, err error) {
	d := channel.device
	s := "Play"

	//然后按顺序生成，一个channel最大999 方便排查问题,也能保证唯一性
	channel.number++
	if channel.number > 999 {
		channel.number = 1
	}
	if len(channel.serial) == 0 {
		channel.serial = RandNumString(6)
	}
	opt.CreateSSRC(channel.serial, channel.number)

	var mediaserver *mediaserver.GB28181MediaServer
	if channel.observer != nil {
		mediaserver = channel.observer.OnStartMediaServer(playInfo.NetWork, playInfo.SinglePort, channel.device.ID, channel.ChannelId)
	}
	if mediaserver == nil {
		return http.StatusNotFound, err
	}

	protocol := ""
	if playInfo.NetWork == "tcp" {
		opt.MediaPort = mediaserver.GetListenerPort()
		protocol = "TCP/"
	} else {
		opt.MediaPort = mediaserver.GetListenerPort()
	}

	sdpInfo := []string{
		"v=0",
		fmt.Sprintf("o=%s 0 0 IN IP4 %s", channel.ChannelId, d.mediaIP),
		"s=" + s,
		"c=IN IP4 " + d.mediaIP,
		opt.String(),
		fmt.Sprintf("m=video %d %sRTP/AVP 96", opt.MediaPort, protocol),
		"a=recvonly",
		"a=rtpmap:96 PS/90000",
		"y=" + opt.ssrc,
	}

	if playInfo.NetWork == "tcp" {
		sdpInfo = append(sdpInfo, "a=setup:passive", "a=connection:new")
	}

	invite := channel.CreateRequst(sip.INVITE, channel.conf)
	contentType := sip.ContentType("application/sdp")
	invite.AppendHeader(&contentType)

	contentLength := sip.ContentLength(len(sdpInfo))
	invite.AppendHeader(&contentLength)

	invite.SetBody(strings.Join(sdpInfo, "\r\n")+"\r\n", true)

	subject := sip.GenericHeader{
		HeaderName: "Subject", Contents: fmt.Sprintf("%s:%s,%s:0", channel.ChannelId, opt.ssrc, channel.conf.Serial),
	}
	invite.AppendHeader(&subject)
	inviteRes, err := d.SipRequestForResponse(invite)
	if err != nil {
		nazalog.Error("invite failed, err:", err, " invite msg:", invite.String())

		//jay 在media端口监听成功后，但是sip发送失败时
		if channel.observer != nil {
			if err = channel.observer.OnStopMediaServer(playInfo.NetWork, playInfo.SinglePort, channel.device.ID, channel.ChannelId, ""); err != nil {
				nazalog.Errorf("gb28181 MediaServer stop err:%s", err.Error())
			}
		}

		return http.StatusInternalServerError, err
	}
	code = int(inviteRes.StatusCode())
	if code == http.StatusOK {
		ds := strings.Split(inviteRes.Body(), "\r\n")
		for _, l := range ds {
			if ls := strings.Split(l, "="); len(ls) > 1 {
				if ls[0] == "y" && len(ls[1]) > 0 {
					if _ssrc, err := strconv.ParseInt(ls[1], 10, 0); err == nil {
						opt.SSRC = uint32(_ssrc)
					} else {
						nazalog.Error("parse invite response y failed, err:", err)
					}
				}
				if ls[0] == "m" && len(ls[1]) > 0 {
					netinfo := strings.Split(ls[1], " ")
					if strings.ToUpper(netinfo[2]) == "TCP/RTP/AVP" {
						nazalog.Info("Device support tcp")
					} else {
						nazalog.Info("Device not support tcp")
					}
				}
			}
		}
		channel.MediaInfo.IsInvite = true
		channel.MediaInfo.Ssrc = opt.SSRC
		channel.MediaInfo.StreamName = streamName
		channel.MediaInfo.MediaKey = fmt.Sprintf("%s%d", playInfo.NetWork, mediaserver.GetListenerPort())

		ackReq := sip.NewAckRequest("", invite, inviteRes, "", nil)
		//保存一下播放信息
		channel.ackReq = ackReq
		channel.playInfo = playInfo

		err = channel.device.sipSvr.Send(ackReq)
	} else {

		if channel.observer != nil {
			if err = channel.observer.OnStopMediaServer(playInfo.NetWork, playInfo.SinglePort, channel.device.ID, channel.ChannelId, ""); err != nil {
				nazalog.Errorf("gb28181 MediaServer stop err:%s", err.Error())
			}
		}

	}
	return
}
func (channel *Channel) GetCallId() string {
	if channel.ackReq != nil {
		if callId, ok := channel.ackReq.CallID(); ok {
			return callId.Value()
		}
	}
	return ""
}
func (channel *Channel) stopMediaServer() (err error) {
	if channel.playInfo != nil {
		if channel.observer != nil {
			if err = channel.observer.OnStopMediaServer(channel.playInfo.NetWork, channel.playInfo.SinglePort, channel.device.ID, channel.ChannelId, channel.playInfo.StreamName); err != nil {
				nazalog.Errorf("gb28181 MediaServer stop err:%s", err.Error())
			}
		}
	}
	return
}
func (channel *Channel) byeClear() (err error) {
	err = channel.stopMediaServer()
	channel.ackReq = nil
	channel.MediaInfo.Clear()
	return
}
func (channel *Channel) Bye(streamName string) (err error) {
	if channel.ackReq != nil {
		byeReq := channel.ackReq
		channel.ackReq = nil
		byeReq.SetMethod(sip.BYE)
		seq, _ := byeReq.CSeq()
		seq.SeqNo += 1
		channel.device.sipSvr.Send(byeReq)
	} else {
		err = errors.New("channel has been closed")
	}
	channel.stopMediaServer()
	return err
}
func (channel *Channel) CreateRequst(Method sip.RequestMethod, conf config.GB28181Config) (req sip.Request) {
	d := channel.device
	d.sn++

	callId := sip.CallID(RandNumString(10))
	userAgent := sip.UserAgentHeader("LALMax")
	maxForwards := sip.MaxForwards(70) //增加max-forwards为默认值 70
	cseq := sip.CSeq{
		SeqNo:      uint32(d.sn),
		MethodName: Method,
	}
	port := sip.Port(conf.SipPort)
	serverAddr := sip.Address{
		Uri: &sip.SipUri{
			FUser: sip.String{Str: conf.Serial},
			FHost: d.sipIP,
			FPort: &port,
		},
		Params: sip.NewParams().Add("tag", sip.String{Str: RandNumString(9)}),
	}
	//非同一域的目标地址需要使用@host
	host := conf.Realm
	if channel.ChannelId[0:9] != host {
		if channel.Port != 0 {
			deviceIp := d.NetAddr
			deviceIp = deviceIp[0:strings.LastIndex(deviceIp, ":")]
			host = fmt.Sprintf("%s:%d", deviceIp, channel.Port)
		} else {
			host = d.NetAddr
		}
	}

	channelAddr := sip.Address{
		Uri: &sip.SipUri{FUser: sip.String{Str: channel.ChannelId}, FHost: host},
	}
	req = sip.NewRequest(
		"",
		Method,
		channelAddr.Uri,
		"SIP/2.0",
		[]sip.Header{
			serverAddr.AsFromHeader(),
			channelAddr.AsToHeader(),
			&callId,
			&userAgent,
			&cseq,
			&maxForwards,
			serverAddr.AsContactHeader(),
		},
		"",
		nil,
	)

	req.SetTransport(channel.device.network)
	req.SetDestination(d.NetAddr)
	return req
}
func (channel *Channel) PtzDirection(direction *PtzDirection) error {
	ptz := Ptz{
		ZoomOut: false,
		ZoomIn:  false,
		Up:      direction.Up,
		Down:    direction.Down,
		Left:    direction.Left,
		Right:   direction.Right,
		Speed:   direction.Speed,
	}
	msgPtz := &MessagePtz{
		CmdType:  DeviceControl,
		DeviceID: direction.ChannelId,
		SN:       int(channel.sn.Add(1)),
		PTZCmd:   ptz.Pack(),
	}
	xml, err := XmlEncode(msgPtz)
	if err != nil {
		return err
	}
	return channel.sipMessage(xml)
}
func (channel *Channel) PtzZoom(zoom *PtzZoom) error {
	ptz := Ptz{
		ZoomOut: zoom.ZoomOut,
		ZoomIn:  zoom.ZoomIn,
		Speed:   zoom.Speed,
	}
	msgPtz := &MessagePtz{
		CmdType:  DeviceControl,
		DeviceID: zoom.ChannelId,
		SN:       int(channel.sn.Add(1)),
		PTZCmd:   ptz.Pack(),
	}
	xml, err := XmlEncode(msgPtz)
	if err != nil {
		return err
	}
	return channel.sipMessage(xml)
}
func (channel *Channel) PtzFi(fi *PtzFi) error {
	ptzFi := Fi{
		IrisIn:    fi.IrisIn,
		IrisOut:   fi.IrisOut,
		FocusNear: fi.FocusNear,
		FocusFar:  fi.FocusFar,
		Speed:     fi.Speed,
	}
	msgPtz := &MessagePtz{
		CmdType:  DeviceControl,
		DeviceID: fi.ChannelId,
		SN:       int(channel.sn.Add(1)),
		PTZCmd:   ptzFi.Pack(),
	}
	xml, err := XmlEncode(msgPtz)
	if err != nil {
		return err
	}
	return channel.sipMessage(xml)
}
func (channel *Channel) PtzPreset(ptzPreset *PtzPreset) error {
	cmd := byte(PresetSet)
	switch ptzPreset.Cmd {
	case PresetEditPoint:
		cmd = PresetSet
	case PresetDelPoint:
		cmd = PresetDel
	case PresetCallPoint:
		cmd = PresetCall
	default:
		return errors.New(fmt.Sprintf("ptz preset cmd error:%d", ptzPreset.Cmd))
	}
	preset := Preset{
		CMD:   cmd,
		Point: ptzPreset.Point,
	}
	msgPtz := &MessagePtz{
		CmdType:  DeviceControl,
		DeviceID: ptzPreset.ChannelId,
		SN:       int(channel.sn.Add(1)),
		PTZCmd:   preset.Pack(),
	}
	xml, err := XmlEncode(msgPtz)
	if err != nil {
		return err
	}
	return channel.sipMessage(xml)
}
func (channel *Channel) PtzStop(stop *PtzStop) error {
	ptz := Ptz{}
	msgPtz := &MessagePtz{
		CmdType:  DeviceControl,
		DeviceID: stop.ChannelId,
		SN:       int(channel.sn.Add(1)),
		PTZCmd:   ptz.Pack(),
	}
	xml, err := XmlEncode(msgPtz)
	if err != nil {
		return err
	}
	return channel.sipMessage(xml)
}
func (channel *Channel) sipMessage(xml string) error {
	d := channel.device
	msg := channel.CreateRequst(sip.MESSAGE, channel.conf)
	msg.AppendHeader(&sip.GenericHeader{HeaderName: "Content-Type", Contents: "Application/MANSCDP+xml"})
	msg.SetBody(xml, true)
	msgRes, err := d.SipRequestForResponse(msg)
	if err != nil {
		return err
	}

	code := int(msgRes.StatusCode())
	if code == http.StatusOK {
		return nil
	} else {
		return errors.New(fmt.Sprintf("sip message ptz fail,code:%d", code))
	}
}
