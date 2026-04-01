// Copyright 2019, Chef.  All rights reserved.
// https://github.com/q191201771/lal
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

package rtmp

import (
	"context"
	"crypto/md5"
	"crypto/tls"
	"encoding/base64"
	"encoding/hex"
	"fmt"
	"net"
	"strings"
	"sync"
	"time"

	"github.com/q191201771/lal/pkg/base"

	"github.com/q191201771/naza/pkg/bele"
	"github.com/q191201771/naza/pkg/connection"
)

// ClientSession rtmp 客户端类型连接的底层实现
// package rtmp 的使用者应该优先使用基于 ClientSession 实现的 PushSession 和 PullSession
type ClientSession struct {
	onDoResult func()

	// 只有PullSession使用
	onReadRtmpAvMsg OnReadRtmpAvMsg

	option ClientSessionOption

	packer        *MessagePacker
	chunkComposer *ChunkComposer
	urlCtx        base.UrlContext
	hc            IHandshakeClient

	conn                  connection.Connection
	doResultChan          chan struct{}
	errChan               chan error
	hasNotifyDoResultSucc bool

	sessionStat base.BasicSessionStat

	debugLogReadUserCtrlMsgCount int
	debugLogReadUserCtrlMsgMax   int

	recvLastAck uint64
	seqNum      uint32

	disposeOnce sync.Once
	authInfo    AuthInfo
}

type AuthInfo struct {
	challenge string
	salt      string
	opaque    string
}

type ClientSessionOption struct {
	// 单位毫秒，如果为0，则没有超时
	DoTimeoutMs      int // 从发起连接（包含了建立连接的时间）到收到publish或play信令结果的超时
	ReadAvTimeoutMs  int // 读取音视频数据的超时
	WriteAvTimeoutMs int // 发送音视频数据的超时

	ReadBufSize   int // io层读取音视频数据时的缓冲大小，如果为0，则没有缓冲
	WriteBufSize  int // io层发送音视频数据的缓冲大小，如果为0，则没有缓冲
	WriteChanSize int // io层发送音视频数据的异步队列大小，如果为0，则同步发送

	ReuseReadMessageBufferFlag bool // 接收Message时，是否重用内存块
	// PeerWinAckSize
	//
	// 设置发送 base.RtmpTypeIdAck 的触发阈值的默认值。
	// 如果没收到 base.RtmpTypeIdWinAckSize，则使用该默认值作为阈值；
	// 如果收到 base.RtmpTypeIdWinAckSize，则使用收到的值作为阈值。
	PeerWinAckSize int

	HandshakeComplexFlag bool // 握手是否使用复杂模式
	// TlsConfig
	// rtmps时使用。
	// 不关心可以不填。
	// 业务方可以通过这个字段自定义 tls.Config
	// 注意，如果使用rtmps并且该字段为nil，那么内部会使用 base.DefaultTlsConfigClient 生成 tls.Config
	TlsConfig *tls.Config
}

var defaultClientSessOption = ClientSessionOption{
	DoTimeoutMs:                10000,
	ReadAvTimeoutMs:            0,
	WriteAvTimeoutMs:           0,
	ReadBufSize:                0,
	WriteBufSize:               0,
	WriteChanSize:              0,
	HandshakeComplexFlag:       false,
	PeerWinAckSize:             0,
	ReuseReadMessageBufferFlag: true,
}

type ModClientSessionOption func(option *ClientSessionOption)

// NewClientSession @param t: session的类型，只能是推或者拉
func NewClientSession(sessionType base.SessionType, modOptions ...ModClientSessionOption) *ClientSession {
	option := defaultClientSessOption
	for _, fn := range modOptions {
		fn(&option)
	}

	var hc IHandshakeClient
	if option.HandshakeComplexFlag {
		hc = &HandshakeClientComplex{}
	} else {
		hc = &HandshakeClientSimple{}
	}

	if option.TlsConfig == nil {
		option.TlsConfig = base.DefaultTlsConfigClient()
	}

	cc := NewChunkComposer()
	cc.SetReuseBufferFlag(option.ReuseReadMessageBufferFlag)

	s := &ClientSession{
		onDoResult:                 defaultOnPullResult,
		onReadRtmpAvMsg:            defaultOnReadRtmpAvMsg,
		option:                     option,
		doResultChan:               make(chan struct{}, 1),
		packer:                     NewMessagePacker(),
		chunkComposer:              cc,
		sessionStat:                base.NewBasicSessionStat(sessionType, ""),
		debugLogReadUserCtrlMsgMax: 5,
		hc:                         hc,
		errChan:                    make(chan error, 1),
	}
	Log.Infof("[%s] lifecycle new rtmp ClientSession. session=%p", s.UniqueKey(), s)
	return s
}

// Do 阻塞直到收到服务端返回的 publish / play 对应结果的信令或者发生错误
func (s *ClientSession) Do(rawUrl string) error {
	Log.Debugf("[%s] Do. url=%s", s.UniqueKey(), rawUrl)

	var (
		ctx    context.Context
		cancel context.CancelFunc
	)
	if s.option.DoTimeoutMs == 0 {
		ctx, cancel = context.WithCancel(context.Background())
	} else {
		ctx, cancel = context.WithTimeout(context.Background(), time.Duration(s.option.DoTimeoutMs)*time.Millisecond)
	}
	defer cancel()

	if err := s.parseUrl(rawUrl); err != nil {
		return err
	}

	err := s.doContext(ctx)

	return err
}

func (s *ClientSession) Write(msg []byte) error {
	if s.conn == nil {
		return base.ErrSessionNotStarted
	}
	_, err := s.conn.Write(msg)
	return err
}

func (s *ClientSession) Flush() error {
	if s.conn == nil {
		return base.ErrSessionNotStarted
	}
	return s.conn.Flush()
}

// ---------------------------------------------------------------------------------------------------------------------
// IClientSessionLifecycle interface
// ---------------------------------------------------------------------------------------------------------------------

// Dispose 文档请参考： IClientSessionLifecycle interface
func (s *ClientSession) Dispose() error {
	return s.dispose(nil)
}

// WaitChan 文档请参考： IClientSessionLifecycle interface
func (s *ClientSession) WaitChan() <-chan error {
	return s.conn.Done()
}

// ---------------------------------------------------------------------------------------------------------------------

func (s *ClientSession) Url() string {
	return s.urlCtx.Url
}

func (s *ClientSession) AppName() string {
	return s.urlCtx.PathWithoutLastItem
}

func (s *ClientSession) StreamName() string {
	return s.urlCtx.LastItemOfPath
}

func (s *ClientSession) RawQuery() string {
	return s.urlCtx.RawQuery
}

func (s *ClientSession) UniqueKey() string {
	return s.sessionStat.UniqueKey()
}

// ----- ISessionStat --------------------------------------------------------------------------------------------------

func (s *ClientSession) GetStat() base.StatSession {
	return s.sessionStat.GetStatWithConn(s.conn)
}

func (s *ClientSession) UpdateStat(intervalSec uint32) {
	s.sessionStat.UpdateStatWitchConn(s.conn, intervalSec)
}

func (s *ClientSession) IsAlive() (readAlive, writeAlive bool) {
	return s.sessionStat.IsAliveWitchConn(s.conn)
}

// ---------------------------------------------------------------------------------------------------------------------

func (s *ClientSession) connect() {
	if err := s.tcpConnect(); err != nil {
		s.errChan <- err
		return
	}

	if err := s.handshake(); err != nil {
		s.errChan <- err
		return
	}

	Log.Infof("[%s] > W SetChunkSize %d.", s.UniqueKey(), LocalChunkSize)
	if err := s.packer.writeChunkSize(s.conn, LocalChunkSize); err != nil {
		s.errChan <- err
		return
	}

	Log.Infof("[%s] > W connect('%s'). tcUrl=%s", s.UniqueKey(), s.appName(), s.tcUrl())
	if err := s.packer.writeConnect(s.conn, s.appName(), s.tcUrl(), s.sessionStat.BaseType() == base.SessionBaseTypePushStr); err != nil {
		s.errChan <- err
		return
	}

	s.runReadLoop()
}

func (s *ClientSession) doContext(ctx context.Context) error {
	go s.connect()

	select {
	case <-ctx.Done():
		_ = s.dispose(nil)
		return ctx.Err()
	case err := <-s.errChan:
		_ = s.dispose(err)
		return err
	case <-s.doResultChan:
		return nil
	}
}

func (s *ClientSession) parseUrl(rawUrl string) (err error) {
	s.urlCtx, err = base.ParseRtmpUrl(rawUrl)
	if err != nil {
		return err
	}

	return
}

func (s *ClientSession) tcUrl() string {
	return fmt.Sprintf("%s://%s/%s", s.urlCtx.Scheme, s.urlCtx.StdHost, s.urlCtx.PathWithoutLastItem)
}
func (s *ClientSession) appName() string {
	return s.urlCtx.PathWithoutLastItem
}

func (s *ClientSession) streamNameWithRawQuery() string {
	if s.urlCtx.RawQuery == "" {
		return s.urlCtx.LastItemOfPath
	}
	return fmt.Sprintf("%s?%s", s.urlCtx.LastItemOfPath, s.urlCtx.RawQuery)
}

func (s *ClientSession) tcpConnect() error {
	Log.Infof("[%s] > tcp connect.", s.UniqueKey())
	var err error

	s.sessionStat.SetRemoteAddr(s.urlCtx.HostWithPort)

	var conn net.Conn
	if s.urlCtx.Scheme == "rtmps" {
		if conn, err = tls.Dial("tcp", s.urlCtx.HostWithPort, s.option.TlsConfig); err != nil {
			return err
		}
	} else {
		if conn, err = net.Dial("tcp", s.urlCtx.HostWithPort); err != nil {
			return err
		}
	}

	s.conn = connection.New(conn, func(option *connection.Option) {
		option.ReadBufSize = s.option.ReadBufSize
		option.WriteChanFullBehavior = connection.WriteChanFullBehaviorBlock
	})
	return nil
}

func (s *ClientSession) handshake() error {
	Log.Infof("[%s] > W Handshake C0+C1.", s.UniqueKey())
	if err := s.hc.WriteC0C1(s.conn); err != nil {
		return err
	}

	if err := s.hc.ReadS0S1(s.conn); err != nil {
		return err
	}
	Log.Infof("[%s] < R Handshake S0+S1.", s.UniqueKey())

	Log.Infof("[%s] > W Handshake C2.", s.UniqueKey())
	if err := s.hc.WriteC2(s.conn); err != nil {
		return err
	}

	if err := s.hc.ReadS2(s.conn); err != nil {
		return err
	}
	Log.Infof("[%s] < R Handshake S2.", s.UniqueKey())
	return nil
}

func (s *ClientSession) runReadLoop() {
	if err := s.chunkComposer.RunLoop(s.conn, s.doMsg); err != nil {
		_ = s.dispose(err)
	}
}

func (s *ClientSession) doMsg(stream *Stream) error {
	if err := s.writeAcknowledgementIfNeeded(stream); err != nil {
		return err
	}

	switch stream.header.MsgTypeId {
	case base.RtmpTypeIdWinAckSize:
		fallthrough
	case base.RtmpTypeIdBandwidth:
		fallthrough
	case base.RtmpTypeIdSetChunkSize:
		return s.doProtocolControlMessage(stream)
	case base.RtmpTypeIdCommandMessageAmf0:
		return s.doCommandMessage(stream)
	case base.RtmpTypeIdMetadata:
		return s.doDataMessageAmf0(stream)
	case base.RtmpTypeIdAck:
		return s.doAck(stream)
	case base.RtmpTypeIdUserControl:
		return s.doUserControl(stream)
	case base.RtmpTypeIdAudio:
		fallthrough
	case base.RtmpTypeIdVideo:
		s.onReadRtmpAvMsg(stream.toAvMsg())
	default:
		Log.Errorf("[%s] read unknown message. typeid=%d, %s", s.UniqueKey(), stream.header.MsgTypeId, stream.toDebugString())
		panic(0)
	}
	return nil
}

func (s *ClientSession) doAck(stream *Stream) error {
	seqNum := bele.BeUint32(stream.msg.buff.Bytes())
	Log.Infof("[%s] < R Acknowledgement. ignore. sequence number=%d.", s.UniqueKey(), seqNum)
	return nil
}
func (s *ClientSession) doUserControl(stream *Stream) error {
	userControlType := bele.BeUint16(stream.msg.buff.Bytes())
	if userControlType == uint16(base.RtmpUserControlPingRequest) {
		stream.msg.buff.Skip(2)
		timestamp := bele.BeUint32(stream.msg.buff.Bytes())
		return s.packer.writePingResponse(s.conn, timestamp)
	}

	s.debugLogReadUserCtrlMsgCount++
	if s.debugLogReadUserCtrlMsgCount <= s.debugLogReadUserCtrlMsgMax {
		Log.Warnf("[%s] read user control message, ignore. buf=%s",
			s.UniqueKey(), hex.Dump(stream.msg.buff.Peek(32)))
	}
	return nil
}

func (s *ClientSession) doDataMessageAmf0(stream *Stream) error {
	val, err := stream.msg.peekStringWithType()
	if err != nil {
		return err
	}

	switch val {
	case "|RtmpSampleAccess":
		Log.Debugf("[%s] < R |RtmpSampleAccess, ignore.", s.UniqueKey())
		return nil
	default:
	}
	s.onReadRtmpAvMsg(stream.toAvMsg())
	return nil
}

func (s *ClientSession) doCommandMessage(stream *Stream) error {
	cmd, err := stream.msg.readStringWithType()
	if err != nil {
		return err
	}

	tid, err := stream.msg.readNumberWithType()
	if err != nil {
		return err
	}

	switch cmd {
	case "onBWDone":
		Log.Warnf("[%s] < R onBWDone. ignore.", s.UniqueKey())
	case "_result":
		return s.doResultMessage(stream, tid)
	case "onStatus":
		return s.doOnStatusMessage(stream, tid)
	case "_error":
		return s.doErrorMessage(stream, tid)
	default:
		Log.Errorf("[%s] read unknown command message. cmd=%s, %s", s.UniqueKey(), cmd, stream.toDebugString())
	}

	return nil
}

func (s *ClientSession) doErrorMessage(stream *Stream, tid int) error {
	if err := stream.msg.readNull(); err != nil {
		return err
	}
	infos, err := stream.msg.readObjectWithType()
	if err != nil {
		return err
	}
	if s.sessionStat.BaseType() == base.SessionBaseTypePushStr {
		description, err := infos.FindString("description")
		if err != nil {
			return err
		}

		return s.dealErrorMessage(description)
	}

	return nil
}

func (s *ClientSession) parseAuthorityInfo(auth string) {
	// 解析salt、challenge、opaque字段
	res := strings.Split(auth, "&")
	for _, info := range res {
		if pos := strings.IndexAny(info, "="); pos > 0 {
			switch info[:pos] {
			case "salt":
				s.authInfo.salt = info[pos+1:]
			case "challenge":
				s.authInfo.challenge = info[pos+1:]
			case "opaque":
				s.authInfo.opaque = info[pos+1:]
			}
		}
	}
}

func (s *ClientSession) dealErrorMessage(description string) (err error) {
	if strings.Contains(description, "code=403 need auth") {
		// app和tcUrl需要加上streamid、authmod、user
		s.urlCtx.PathWithoutLastItem = fmt.Sprintf("%s/%s?authmod=adobe&user=%s", s.urlCtx.PathWithoutLastItem, s.urlCtx.LastItemOfPath, s.urlCtx.Username)

		//关闭上一次连接并发起新的连接
		s.conn.Close()
		s.connect()
	} else if strings.Contains(description, "?reason=needauth") {
		descriptions := strings.Split(description, ":")
		if len(descriptions) != 3 {
			err = fmt.Errorf("inavlid message: %s", description)
			return err
		} else {
			descriptions[2] = strings.Replace(descriptions[2], " ", "", -1)
			replacestr := fmt.Sprintf("?reason=needauth&user=%s&", s.urlCtx.Username)
			authinfo := strings.Replace(descriptions[2], replacestr, "", -1)

			s.parseAuthorityInfo(authinfo)

			// base64(md5(username|salt|password))作为新的salt1
			mds := md5.Sum([]byte(s.urlCtx.Username + s.authInfo.salt + s.urlCtx.Password))
			salt1 := base64.StdEncoding.EncodeToString(mds[:])

			// response = base64(md5(salt1|opaque|challenge))
			mds1 := md5.Sum([]byte(salt1 + s.authInfo.opaque + s.authInfo.challenge))
			response := base64.StdEncoding.EncodeToString(mds1[:])

			// app和tcUrl需要加上challenge、response、opaque字段
			s.urlCtx.PathWithoutLastItem = fmt.Sprintf("%s&challenge=%s&response=%s&opaque=%s", s.urlCtx.PathWithoutLastItem, s.authInfo.challenge, response, s.authInfo.opaque)

			// 关闭前一个连接并发起新的连接
			s.conn.Close()
			s.connect()
		}
	} else {
		err = fmt.Errorf("invalid errmessage: %s", description)
		return err
	}
	return nil
}

func (s *ClientSession) doOnStatusMessage(stream *Stream, tid int) error {
	if err := stream.msg.readNull(); err != nil {
		return err
	}
	infos, err := stream.msg.readObjectWithType()
	if err != nil {
		return err
	}
	code, err := infos.FindString("code")
	if err != nil {
		return err
	}
	switch s.sessionStat.BaseType() {
	case base.SessionBaseTypePushStr:
		switch code {
		case "NetStream.Publish.Start":
			Log.Infof("[%s] < R onStatus('NetStream.Publish.Start').", s.UniqueKey())
			s.notifyDoResultSucc()
		default:
			Log.Warnf("[%s] read on status message but code field unknown. code=%s", s.UniqueKey(), code)
		}
	case base.SessionBaseTypePullStr:
		switch code {
		case "NetStream.Play.Start":
			Log.Infof("[%s] < R onStatus('NetStream.Play.Start').", s.UniqueKey())
			s.notifyDoResultSucc()
		default:
			Log.Warnf("[%s] read on status message but code field unknown. code=%s", s.UniqueKey(), code)
		}
	}

	return nil
}

func (s *ClientSession) doResultMessage(stream *Stream, tid int) error {
	switch tid {
	case tidClientConnect:
		_, err := stream.msg.readObjectWithType()
		if err != nil {
			return err
		}
		infos, err := stream.msg.readObjectWithType()
		if err != nil {
			return err
		}
		code, err := infos.FindString("code")
		if err != nil {
			return err
		}
		switch code {
		case "NetConnection.Connect.Success":
			Log.Infof("[%s] < R _result(\"NetConnection.Connect.Success\").", s.UniqueKey())
			Log.Infof("[%s] > W createStream().", s.UniqueKey())
			if err := s.packer.writeCreateStream(s.conn); err != nil {
				return err
			}
		default:
			Log.Errorf("[%s] unknown code. code=%v", s.UniqueKey(), code)
		}
	case tidClientCreateStream:
		err := stream.msg.readNull()
		if err != nil {
			return err
		}
		sid, err := stream.msg.readNumberWithType()
		if err != nil {
			return err
		}
		Log.Infof("[%s] < R _result().", s.UniqueKey())
		switch s.sessionStat.BaseType() {
		case base.SessionBaseTypePushStr:
			Log.Infof("[%s] > W publish('%s').", s.UniqueKey(), s.streamNameWithRawQuery())
			if err := s.packer.writePublish(s.conn, s.appName(), s.streamNameWithRawQuery(), sid); err != nil {
				return err
			}
		case base.SessionBaseTypePullStr:
			Log.Infof("[%s] > W play('%s').", s.UniqueKey(), s.streamNameWithRawQuery())
			if err := s.packer.writePlay(s.conn, s.streamNameWithRawQuery(), sid); err != nil {
				return err
			}
		}
	default:
		Log.Errorf("[%s] unknown tid. tid=%d", s.UniqueKey(), tid)
	}
	return nil
}
func (s *ClientSession) doProtocolControlMessage(stream *Stream) error {
	if stream.msg.Len() < 4 {
		return base.NewErrRtmpShortBuffer(4, int(stream.msg.Len()), "ClientSession::doProtocolControlMessage")
	}
	val := int(bele.BeUint32(stream.msg.buff.Bytes()))

	switch stream.header.MsgTypeId {
	case base.RtmpTypeIdWinAckSize:
		s.option.PeerWinAckSize = val
		Log.Infof("[%s] < R Window Acknowledgement Size: %d", s.UniqueKey(), s.option.PeerWinAckSize)
	case base.RtmpTypeIdBandwidth:
		// TODO chef: 是否需要关注这个信令
		Log.Warnf("[%s] < R Set Peer Bandwidth. ignore.", s.UniqueKey())
	case base.RtmpTypeIdSetChunkSize:
		// composer内部会自动更新peer chunk size.
		Log.Infof("[%s] < R Set Chunk Size %d.", s.UniqueKey(), val)
	default:
		Log.Errorf("[%s] read unknown protocol control message. typeid=%d, %s", s.UniqueKey(), stream.header.MsgTypeId, stream.toDebugString())
	}
	return nil
}

func (s *ClientSession) writeAcknowledgementIfNeeded(stream *Stream) error {
	// https://github.com/q191201771/lal/pull/154
	if s.option.PeerWinAckSize <= 0 {
		return nil
	}

	currStat := s.conn.GetStat()
	delta := uint32(currStat.ReadBytesSum - s.recvLastAck)
	//此次接收小于窗口大小一半，不处理
	if delta < uint32(s.option.PeerWinAckSize/2) {
		return nil
	}
	s.recvLastAck = currStat.ReadBytesSum
	seqNum := s.seqNum + delta
	//当序列号溢出时，将其重置
	if seqNum > ackSeqMax {
		seqNum = delta
	}
	s.seqNum = seqNum
	//时间戳暂时先发0
	return s.packer.writeAcknowledgement(s.conn, seqNum)
}
func (s *ClientSession) notifyDoResultSucc() {
	// 碰上过对端服务器实现有问题，对于play信令回复了两次相同的结果，我们在这里忽略掉非第一次的回复
	if s.hasNotifyDoResultSucc {
		Log.Warnf("[%s] has notified do result succ already, ignore it", s.UniqueKey())
		return
	}
	s.hasNotifyDoResultSucc = true

	if s.option.WriteChanSize > 0 {
		s.conn.ModWriteChanSize(s.option.WriteChanSize)
	}

	//pull有可能还需要小包发送，不使用缓存
	if s.sessionStat.BaseType() == base.SessionBaseTypePushStr {
		s.conn.ModWriteBufSize(s.option.WriteBufSize)
	}

	s.conn.ModReadTimeoutMs(s.option.ReadAvTimeoutMs)
	s.conn.ModWriteTimeoutMs(s.option.WriteAvTimeoutMs)

	s.onDoResult()
	s.doResultChan <- struct{}{}
}

func (s *ClientSession) dispose(err error) error {
	var retErr error
	s.disposeOnce.Do(func() {
		Log.Infof("[%s] lifecycle dispose rtmp ClientSession. err=%+v", s.UniqueKey(), err)
		if s.conn == nil {
			retErr = base.ErrSessionNotStarted
			return
		}
		retErr = s.conn.Close()
	})
	return retErr
}

func defaultOnPullResult() {
}

func defaultOnReadRtmpAvMsg(msg base.RtmpMsg) {

}
