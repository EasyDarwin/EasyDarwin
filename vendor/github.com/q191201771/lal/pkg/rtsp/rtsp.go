// Copyright 2020, Chef.  All rights reserved.
// https://github.com/q191201771/lal
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

package rtsp

import (
	"fmt"
	"net"
	"strconv"
	"strings"

	"github.com/q191201771/naza/pkg/nazaerrors"

	"github.com/q191201771/lal/pkg/base"

	"github.com/q191201771/lal/pkg/rtprtcp"
	"github.com/q191201771/naza/pkg/nazanet"
)

// TODO chef
// - out缺少主动发送sr
// - pull session回调有observer interface和on func回调两种方式，是否需要统一
// - [refactor] BaseInSession和BaseOutSession有不少重复内容
// - [refactor] PullSession和PushSession有不少重复内容

const (
	MethodOptions      = "OPTIONS"
	MethodAnnounce     = "ANNOUNCE"
	MethodDescribe     = "DESCRIBE"
	MethodSetup        = "SETUP"
	MethodRecord       = "RECORD"
	MethodPlay         = "PLAY"
	MethodTeardown     = "TEARDOWN"
	MethodGetParameter = "GET_PARAMETER"
)

const (
	// HeaderAccept header key
	HeaderAccept          = "Accept"
	HeaderUserAgent       = "User-Agent"
	HeaderCSeq            = "CSeq"
	HeaderContentLength   = "Content-Length"
	HeaderTransport       = "Transport"
	HeaderSession         = "Session"
	HeaderRange           = "Range"
	HeaderWwwAuthenticate = "WWW-Authenticate"
	HeaderAuthorization   = "Authorization"
	HeaderPublic          = "Public"

	// HeaderAcceptApplicationSdp header value
	HeaderAcceptApplicationSdp         = "application/sdp"
	HeaderRangeDefault                 = "npt=0.000-"
	HeaderTransportClientPlayTmpl      = "RTP/AVP/UDP;unicast;client_port=%d-%d" // localRtpPort, localRtcpPort
	HeaderTransportClientPlayTcpTmpl   = "RTP/AVP/TCP;unicast;interleaved=%d-%d" // rtpChannel, rtcpChannel
	HeaderTransportClientRecordTmpl    = "RTP/AVP/UDP;unicast;client_port=%d-%d;mode=record"
	HeaderTransportClientRecordTcpTmpl = "RTP/AVP/TCP;unicast;interleaved=%d-%d;mode=record"
	HeaderTransportServerPlayTmpl      = "RTP/AVP/UDP;unicast;client_port=%d-%d;server_port=%d-%d"

	//HeaderTransportServerPlayTCPTmpl   = "RTP/AVP/TCP;unicast;interleaved=%d-%d"

	HeaderTransportServerRecordTmpl = "RTP/AVP/UDP;unicast;client_port=%d-%d;server_port=%d-%d;mode=record"

	//HeaderTransportServerRecordTCPTmpl = "RTP/AVP/TCP;unicast;interleaved=%d-%d;mode=record"
)

const (
	TransportFieldClientPort  = "client_port"
	TransportFieldServerPort  = "server_port"
	TransportFieldInterleaved = "interleaved"
)

const (
	Interleaved = uint8(0x24)
)

var (
	// TODO chef: 参考协议标准，不要使用固定值
	sessionId = "191201771"

	minServerPort = uint16(30000)
	maxServerPort = uint16(60000)

	unpackerItemMaxSize = 1024

	serverCommandSessionReadBufSize   = 256
	serverCommandSessionWriteChanSize = 1024

	dummyRtpPacket = []byte{
		0x80, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00,
	}

	dummyRtcpPacket = []byte{
		0x80, 0xc9, 0x00, 0x01,
		0x00, 0x00, 0x00, 0x00,
	}
)

type IInterleavedPacketWriter interface {
	WriteInterleavedPacket(packet []byte, channel int) error
}

var availUdpConnPool *nazanet.AvailUdpConnPool

// 传入远端IP，RtpPort，RtcpPort，创建两个对应的RTP和RTCP的UDP连接对象，以及对应的本端端口
func initConnWithClientPort(rHost string, rRtpPort, rRtcpPort uint16) (rtpConn, rtcpConn *nazanet.UdpConnection, lRtpPort, lRtcpPort uint16, err error) {
	// NOTICE
	// 处理Pub时，
	// 一路流的rtp端口和rtcp端口必须不同。
	// 我尝试给ffmpeg返回rtp和rtcp同一个端口，结果ffmpeg依然使用rtp+1作为rtcp的端口。
	// 又尝试给ffmpeg返回rtp:a和rtcp:a+2的端口，结果ffmpeg依然使用a和a+1端口。
	// 也即是说，ffmpeg默认认为rtcp的端口是rtp的端口+1。而不管SETUP RESPONSE的rtcp端口是多少。
	// 我目前在Acquire2这个函数里做了保证，绑定两个可用且连续的端口。

	var rtpc, rtcpc *net.UDPConn
	rtpc, lRtpPort, rtcpc, lRtcpPort, err = availUdpConnPool.Acquire2()
	if err != nil {
		return
	}

	rtpConn, err = nazanet.NewUdpConnection(func(option *nazanet.UdpConnectionOption) {
		option.Conn = rtpc
		option.RAddr = net.JoinHostPort(rHost, fmt.Sprintf("%d", rRtpPort))
		option.MaxReadPacketSize = rtprtcp.MaxRtpRtcpPacketSize
	})
	if err != nil {
		return
	}
	rtcpConn, err = nazanet.NewUdpConnection(func(option *nazanet.UdpConnectionOption) {
		option.Conn = rtcpc
		option.RAddr = net.JoinHostPort(rHost, fmt.Sprintf("%d", rRtcpPort))
		option.MaxReadPacketSize = rtprtcp.MaxRtpRtcpPacketSize
	})
	return
}

// 从setup消息的header中解析rtp rtcp channel
func parseRtpRtcpChannel(setupTransport string) (rtp, rtcp uint16, err error) {
	return parseTransport(setupTransport, TransportFieldInterleaved)
}

// 从setup消息的header中解析rtp rtcp 端口
func parseClientPort(setupTransport string) (rtp, rtcp uint16, err error) {
	return parseTransport(setupTransport, TransportFieldClientPort)
}

func parseServerPort(setupTransport string) (rtp, rtcp uint16, err error) {
	return parseTransport(setupTransport, TransportFieldServerPort)
}

func parseTransport(setupTransport string, key string) (first, second uint16, err error) {
	var clientPort string
	items := strings.Split(setupTransport, ";")
	for _, item := range items {
		if strings.HasPrefix(item, key) {
			kv := strings.Split(item, "=")
			if len(kv) != 2 {
				continue
			}
			clientPort = kv[1]
		}
	}
	items = strings.Split(clientPort, "-")
	if len(items) != 2 {
		return 0, 0, nazaerrors.Wrap(base.ErrRtsp)
	}
	iFirst, err := strconv.Atoi(items[0])
	if err != nil {
		return 0, 0, err
	}
	iSecond, err := strconv.Atoi(items[1])
	if err != nil {
		return 0, 0, err
	}
	return uint16(iFirst), uint16(iSecond), err
}

func makeSetupUri(urlCtx base.UrlContext, aControl string) string {
	if strings.HasPrefix(aControl, "rtsp://") {
		return aControl
	}
	return fmt.Sprintf("%s/%s", urlCtx.RawUrlWithoutUserInfo, aControl)
}

func init() {
	availUdpConnPool = nazanet.NewAvailUdpConnPool(minServerPort, maxServerPort)
}

// ---------------------------------------------------------------------------------------------------------------------
// PUB
// ffmpeg -re -stream_loop -1 -i /Volumes/Data/tmp/wontcry.flv -acodec copy -vcodec copy -f rtsp rtsp://localhost:5544/live/test110

// read http request. method=OPTIONS, uri=rtsp://localhost:5544/live/test110, headers=map[CSeq:1 User-Agent:Lavf57.83.100], body= - server.go:95
// read http request. method=ANNOUNCE, uri=rtsp://localhost:5544/live/test110, headers=map[CSeq:2 Content-Length:490 Content-Type:application/sdp User-Agent:Lavf57.83.100], body=v=0
// o=- 0 0 IN IP4 127.0.0.1
// s=No Name
// c=IN IP4 127.0.0.1
// t=0 0
// a=tool:libavformat 57.83.100
// m=video 0 RTP/AVP 96
// a=rtpmap:96 H264/90000
// a=fmtp:96 packetization-mode=1; sprop-parameter-sets=Z2QAFqyyAUBf8uAiAAADAAIAAAMAPB4sXJA=,aOvDyyLA; profile-level-id=640016
// a=control:streamid=0
// m=audio 0 RTP/AVP 97
// b=AS:128
// a=rtpmap:97 MPEG4-GENERIC/44100/2
// a=fmtp:97 profile-level-id=1;mode=AAC-hbr;sizelength=13;indexlength=3;indexdeltalength=3; config=121056E500
// a=control:streamid=1
// - server.go:95
// read http request. method=SETUP, uri=rtsp://localhost:5544/live/test110/streamid=0, headers=map[CSeq:3 Transport:RTP/AVP/UDP;unicast;client_port=32182-32183;mode=record User-Agent:Lavf57.83.100], body= - server.go:95
// read http request. method=SETUP, uri=rtsp://localhost:5544/live/test110/streamid=1, headers=map[CSeq:4 Session:191201771 Transport:RTP/AVP/UDP;unicast;client_port=32184-32185;mode=record User-Agent:Lavf57.83.100], body= - server.go:95
// read http request. method=RECORD, uri=rtsp://localhost:5544/live/test110, headers=map[CSeq:5 Range:npt=0.000- Session:191201771 User-Agent:Lavf57.83.100], body= - server.go:95
// read http request. method=TEARDOWN, uri=rtsp://localhost:5544/live/test110, headers=map[CSeq:6 Session:191201771 User-Agent:Lavf57.83.100], body= - server.go:95
// ---------------------------------------------------------------------------------------------------------------------

// ---------------------------------------------------------------------------------------------------------------------
// PUB(rtp over tcp)
// ffmpeg -re -stream_loop -1 -i /Volumes/Data/tmp/wontcry.flv -acodec copy -vcodec copy -rtsp_transport tcp -f rtsp rtsp://localhost:5544/live/test110
//
// read http request. method=OPTIONS, uri=rtsp://localhost:5544/live/test110, headers=map[CSeq:1 User-Agent:Lavf57.83.100], body= - server.go:137
// read http request. method=ANNOUNCE, uri=rtsp://localhost:5544/live/test110, headers=map[CSeq:2 Content-Length:478 Content-Type:application/sdp User-Agent:Lavf57.83.100], body=v=0
// o=- 0 0 IN IP6 ::1
// s=No Name
// c=IN IP6 ::1
// t=0 0
// a=tool:libavformat 57.83.100
// m=video 0 RTP/AVP 96
// a=rtpmap:96 H264/90000
// a=fmtp:96 packetization-mode=1; sprop-parameter-sets=Z2QAFqyyAUBf8uAiAAADAAIAAAMAPB4sXJA=,aOvDyyLA; profile-level-id=640016
// a=control:streamid=0
// m=audio 0 RTP/AVP 97
// b=AS:128
// a=rtpmap:97 MPEG4-GENERIC/44100/2
// a=fmtp:97 profile-level-id=1;mode=AAC-hbr;sizelength=13;indexlength=3;indexdeltalength=3; config=121056E500
// a=control:streamid=1
// - server.go:137
// read http request. method=SETUP, uri=rtsp://localhost:5544/live/test110/streamid=0, headers=map[CSeq:3 Transport:RTP/AVP/TCP;unicast;interleaved=0-1;mode=record User-Agent:Lavf57.83.100], body= - server.go:137
// read http request. method=SETUP, uri=rtsp://localhost:5544/live/test110/streamid=1, headers=map[CSeq:4 Session:191201771 Transport:RTP/AVP/TCP;unicast;interleaved=2-3;mode=record User-Agent:Lavf57.83.100], body= - server.go:137
// read http request. method=RECORD, uri=rtsp://localhost:5544/live/test110, headers=map[CSeq:5 Range:npt=0.000- Session:191201771 User-Agent:Lavf57.83.100], body= - server.go:137
// ---------------------------------------------------------------------------------------------------------------------

// ---------------------------------------------------------------------------------------------------------------------
// SUB
//
// read http request. method=OPTIONS, uri=rtsp://localhost:5544/live/test110, headers=map[CSeq:1 User-Agent:Lavf57.83.100], body= - server.go:108
// read http request. method=DESCRIBE, uri=rtsp://localhost:5544/live/test110, headers=map[Accept:application/sdp CSeq:2 User-Agent:Lavf57.83.100], body= - server.go:108
// read http request. method=SETUP, uri=rtsp://localhost:5544/live/test110/streamid=0, headers=map[CSeq:3 Transport:RTP/AVP/UDP;unicast;client_port=15690-15691 User-Agent:Lavf57.83.100], body= - server.go:108
// read http request. method=SETUP, uri=rtsp://localhost:5544/live/test110/streamid=1, headers=map[CSeq:4 Session:191201771 Transport:RTP/AVP/UDP;unicast;client_port=15692-15693 User-Agent:Lavf57.83.100], body= - server.go:108
// read http request. method=PLAY, uri=rtsp://localhost:5544/live/test110, headers=map[CSeq:5 Range:npt=0.000- Session:191201771 User-Agent:Lavf57.83.100], body= - server.go:108
// ---------------------------------------------------------------------------------------------------------------------

// ---------------------------------------------------------------------------------------------------------------------
// SUB(rtp over tcp)
//
// read http request. method=OPTIONS, uri=rtsp://localhost:5544/live/test110, headers=map[CSeq:1 User-Agent:Lavf57.83.100], body= - server_command_session.go:136
// read http request. method=DESCRIBE, uri=rtsp://localhost:5544/live/test110, headers=map[Accept:application/sdp CSeq:2 User-Agent:Lavf57.83.100], body= - server_command_session.go:136
// read http request. method=SETUP, uri=rtsp://localhost:5544/live/test110/streamid=0, headers=map[CSeq:3 Transport:RTP/AVP/TCP;unicast;interleaved=0-1 User-Agent:Lavf57.83.100], body= - server_command_session.go:136
// read http request. method=SETUP, uri=rtsp://localhost:5544/live/test110/streamid=1, headers=map[CSeq:4 Session:191201771 Transport:RTP/AVP/TCP;unicast;interleaved=2-3 User-Agent:Lavf57.83.100], body= - server_command_session.go:136
// read http request. method=PLAY, uri=rtsp://localhost:5544/live/test110, headers=map[CSeq:5 Range:npt=0.000- Session:191201771 User-Agent:Lavf57.83.100], body= - server_command_session.go:136
// ---------------------------------------------------------------------------------------------------------------------

// 8000 video rtp
// 8001 video rtcp
// 8002 audio rtp
// 8003 audio rtcp
