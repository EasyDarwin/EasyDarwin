package gb28181

import (
	"context"
	"github.com/ghettovoice/gosip"
	"net/http"
	"strings"
	"sync"
	"time"

	config "easydarwin/pkg/lalmax/conf"

	"github.com/ghettovoice/gosip/sip"
	"github.com/q191201771/naza/pkg/nazalog"
)

const TIME_LAYOUT = "2006-01-02T15:04:05"

var (
	Devices             sync.Map
	DeviceNonce         sync.Map //保存nonce防止设备伪造
	DeviceRegisterCount sync.Map //设备注册次数
)

type DeviceStatus string

const (
	DeviceRegisterStatus = "REGISTER"
	DeviceRecoverStatus  = "RECOVER"
	DeviceOnlineStatus   = "ONLINE"
	DeviceOfflineStatus  = "OFFLINE"
	DeviceAlarmedStatus  = "ALARMED"
)

type Device struct {
	ID              string
	Name            string
	Manufacturer    string
	Model           string
	Owner           string
	RegisterTime    time.Time
	UpdateTime      time.Time
	LastKeepaliveAt time.Time
	Status          DeviceStatus
	sn              int
	addr            sip.Address
	sipIP           string //设备对应网卡的服务器ip
	mediaIP         string //设备对应网卡的服务器ip
	NetAddr         string
	channelMap      sync.Map
	subscriber      struct {
		CallID  string
		Timeout time.Time
	}
	lastSyncTime time.Time
	GpsTime      time.Time //gps时间
	Longitude    string    //经度
	Latitude     string    //纬度

	observer IMediaOpObserver
	conf     config.GB28181Config

	network string
	sipSvr  gosip.Server
}

func (d *Device) WithMediaServer(observer IMediaOpObserver) {
	d.observer = observer
}

func (d *Device) WithSipSvr(sipSvr gosip.Server) *Device {
	d.sipSvr = sipSvr
	return d
}

func (d *Device) syncChannels() {
	if time.Since(d.lastSyncTime) > 2*time.Second {
		d.lastSyncTime = time.Now()
		d.Catalog(d.conf)
		//d.Subscribe(conf)
		//d.QueryDeviceInfo(conf)
	}
}

func (d *Device) UpdateChannels(list ...ChannelInfo) {
	for _, c := range list {
		//当父设备非空且存在时、父设备节点增加通道
		if c.ParentId != "" {
			path := strings.Split(c.ParentId, "/")
			parentId := path[len(path)-1]
			//如果父ID并非本身所属设备，一般情况下这是因为下级设备上传了目录信息，该信息通常不需要处理。
			// 暂时不考虑级联目录的实现
			if d.ID != parentId {
				if v, ok := Devices.Load(parentId); ok {
					parent := v.(*Device)
					parent.addOrUpdateChannel(c)
					continue
				}
			}
		}
		c.ParentId = d.ID
		//本设备增加通道
		d.addOrUpdateChannel(c)
		//channel.TryAutoInvite(&InviteOptions{}, conf)
	}
}

func (d *Device) addOrUpdateChannel(info ChannelInfo) (c *Channel) {
	if old, ok := d.channelMap.Load(info.ChannelId); ok {
		c = old.(*Channel)
		c.ChannelInfo = info
	} else {
		c = &Channel{
			device:      d,
			ChannelInfo: info,
			conf:        d.conf,
		}
		c.WithMediaServer(d.observer)
		d.channelMap.Store(info.ChannelId, c)
	}
	return
}

func (d *Device) Catalog(conf config.GB28181Config) int {
	request := d.CreateRequest(sip.MESSAGE, conf)
	expires := sip.Expires(3600)
	d.subscriber.Timeout = time.Now().Add(time.Second * time.Duration(expires))
	contentType := sip.ContentType("Application/MANSCDP+xml")

	request.AppendHeader(&contentType)
	request.AppendHeader(&expires)
	request.SetBody(BuildCatalogXML(d.sn, d.ID), true)
	// 输出Sip请求设备通道信息信令
	nazalog.Info("SIP->Catalog request:", request.String())

	resp, err := d.SipRequestForResponse(request)
	if err == nil && resp != nil {
		nazalog.Info("SIP->Catalog Response:", resp.String())
		return int(resp.StatusCode())
	} else if err != nil {
		nazalog.Error("SIP<-Catalog error:", err)
	}
	return http.StatusRequestTimeout
}

func (d *Device) CreateRequest(Method sip.RequestMethod, conf config.GB28181Config) (req sip.Request) {
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
	req = sip.NewRequest(
		"",
		Method,
		d.addr.Uri,
		"SIP/2.0",
		[]sip.Header{
			serverAddr.AsFromHeader(),
			d.addr.AsToHeader(),
			&callId,
			&userAgent,
			&cseq,
			&maxForwards,
			serverAddr.AsContactHeader(),
		},
		"",
		nil,
	)

	req.SetTransport(d.network)
	req.SetDestination(d.NetAddr)
	return
}

func (d *Device) Subscribe(conf config.GB28181Config) int {
	request := d.CreateRequest(sip.SUBSCRIBE, conf)
	if d.subscriber.CallID != "" {
		callId := sip.CallID(RandNumString(10))
		request.AppendHeader(&callId)
	}
	expires := sip.Expires(3600)
	d.subscriber.Timeout = time.Now().Add(time.Second * time.Duration(expires))
	contentType := sip.ContentType("Application/MANSCDP+xml")
	request.AppendHeader(&contentType)
	request.AppendHeader(&expires)

	request.SetBody(BuildCatalogXML(d.sn, d.ID), true)

	response, err := d.SipRequestForResponse(request)
	if err == nil && response != nil {
		if response.StatusCode() == http.StatusOK {
			callId, _ := request.CallID()
			d.subscriber.CallID = string(*callId)
		} else {
			d.subscriber.CallID = ""
		}
		return int(response.StatusCode())
	}
	return http.StatusRequestTimeout
}

func (d *Device) QueryDeviceInfo(conf config.GB28181Config) {
	for i := time.Duration(5); i < 100; i++ {

		time.Sleep(time.Second * i)
		request := d.CreateRequest(sip.MESSAGE, conf)
		contentType := sip.ContentType("Application/MANSCDP+xml")
		request.AppendHeader(&contentType)
		request.SetBody(BuildDeviceInfoXML(d.sn, d.ID), true)

		response, _ := d.SipRequestForResponse(request)
		if response != nil {
			if response.StatusCode() == http.StatusOK {
				break
			}
		}
	}
}

// UpdateChannelStatus 目录订阅消息处理：新增/移除/更新通道或者更改通道状态
func (d *Device) UpdateChannelStatus(deviceList []*notifyMessage, conf config.GB28181Config) {
	for _, v := range deviceList {
		switch v.Event {
		case "ON":
			nazalog.Debug("receive channel online notify")
			d.channelOnline(v.ChannelId)
		case "OFF":
			nazalog.Debug("receive channel offline notify")
			d.channelOffline(v.ChannelId)
		case "VLOST":
			nazalog.Debug("receive channel video lost notify")
			d.channelOffline(v.ChannelId)
		case "DEFECT":
			nazalog.Debug("receive channel video defect notify")
			d.channelOffline(v.ChannelId)
		case "ADD":
			nazalog.Debug("receive channel add notify")
			channel := ChannelInfo{
				ChannelId:    v.ChannelId,
				ParentId:     v.ParentId,
				Name:         v.Name,
				Manufacturer: v.Manufacturer,
				Model:        v.Model,
				Owner:        v.Owner,
				CivilCode:    v.CivilCode,
				Address:      v.Address,
				Port:         v.Port,
				Parental:     v.Parental,
				SafetyWay:    v.SafetyWay,
				RegisterWay:  v.RegisterWay,
				Secrecy:      v.Secrecy,
				Status:       v.Status,
				Longitude:    v.Longitude,
				Latitude:     v.Latitude,
			}
			d.addOrUpdateChannel(channel)
		case "DEL":
			//删除
			nazalog.Debug("receive channel delete notify")
			d.deleteChannel(v.ChannelId)
		case "UPDATE":
			nazalog.Debug("receive channel update notify")
			// 更新通道
			channel := ChannelInfo{
				ChannelId:    v.ChannelId,
				ParentId:     v.ParentId,
				Name:         v.Name,
				Manufacturer: v.Manufacturer,
				Model:        v.Model,
				Owner:        v.Owner,
				CivilCode:    v.CivilCode,
				Address:      v.Address,
				Port:         v.Port,
				Parental:     v.Parental,
				SafetyWay:    v.SafetyWay,
				RegisterWay:  v.RegisterWay,
				Secrecy:      v.Secrecy,
				Status:       v.Status,
				Longitude:    v.Longitude,
				Latitude:     v.Latitude,
			}
			d.UpdateChannels(channel)
		}
	}
}

func (d *Device) channelOnline(channelId string) {
	if v, ok := d.channelMap.Load(channelId); ok {
		c := v.(*Channel)
		c.Status = ChannelOnStatus
		nazalog.Debug("channel online, channelId: ", channelId)
	} else {
		nazalog.Debug("update channel status failed, not found, channelId: ", channelId)
	}
}

func (d *Device) channelOffline(channelId string) {
	if v, ok := d.channelMap.Load(channelId); ok {
		c := v.(*Channel)
		c.Status = ChannelOffStatus
		nazalog.Debug("channel offline, channelId: ", channelId)
	} else {
		nazalog.Debug("update channel status failed, not found, channelId: ", channelId)
	}
}

func (d *Device) deleteChannel(channelId string) {
	d.channelMap.Delete(channelId)
}

// UpdateChannelPosition 更新通道GPS坐标
func (d *Device) UpdateChannelPosition(channelId string, gpsTime string, lng string, lat string) {
	if v, ok := d.channelMap.Load(channelId); ok {
		c := v.(*Channel)
		c.GpsTime = time.Now() //时间取系统收到的时间，避免设备时间和格式问题
		c.Longitude = lng
		c.Latitude = lat
		nazalog.Debug("update channel position success")
	} else {
		//如果未找到通道，则更新到设备上
		d.GpsTime = time.Now() //时间取系统收到的时间，避免设备时间和格式问题
		d.Longitude = lng
		d.Latitude = lat
		nazalog.Debug("update device position success, channelId:", channelId)
	}
}

func (d *Device) SipRequestForResponse(request sip.Request) (sip.Response, error) {
	return d.sipSvr.RequestWithContext(context.Background(), request)
}
