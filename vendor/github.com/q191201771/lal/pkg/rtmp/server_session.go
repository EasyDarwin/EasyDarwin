// Copyright 2019, Chef.  All rights reserved.
// https://github.com/q191201771/lal
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

package rtmp

import (
	"encoding/hex"
	"fmt"
	"net"
	"strings"
	"sync"

	"github.com/q191201771/naza/pkg/nazaerrors"

	"github.com/q191201771/lal/pkg/base"

	"github.com/q191201771/naza/pkg/bele"
	"github.com/q191201771/naza/pkg/connection"
)

// TODO chef: 没有进化成Pub Sub时的超时释放

type IServerSessionObserver interface {
	OnRtmpConnect(session *ServerSession, opa ObjectPairArray)

	// OnNewRtmpPubSession
	//
	// 上层代码应该在这个事件回调中注册音视频数据的监听
	//
	// @return 上层如果想关闭这个session，则回调中返回不为nil的error值
	//
	OnNewRtmpPubSession(session *ServerSession) error

	OnNewRtmpSubSession(session *ServerSession) error
}

type IPubSessionObserver interface {
	// OnReadRtmpAvMsg 注意，回调结束后，内部会复用Payload内存块
	OnReadRtmpAvMsg(msg base.RtmpMsg)
}

func (s *ServerSession) SetPubSessionObserver(observer IPubSessionObserver) {
	s.avObserver = observer
}

type ServerSessionType int

const (
	ServerSessionTypeUnknown ServerSessionType = iota // 收到客户端的publish或者play信令之前的类型状态
	ServerSessionTypePub
	ServerSessionTypeSub
)

type ServerSession struct {
	url                    string
	tcUrl                  string
	streamNameWithRawQuery string // const after set
	appName                string // const after set
	streamName             string // const after set
	rawQuery               string //const after set

	observer      IServerSessionObserver
	hs            HandshakeServer
	chunkComposer *ChunkComposer
	packer        *MessagePacker

	conn        connection.Connection
	sessionStat base.BasicSessionStat

	// only for PubSession
	avObserver IPubSessionObserver

	// IsFresh ShouldWaitVideoKeyFrame
	//
	// 只有sub类型需要
	//
	// IsFresh
	//  表示是新加入的session，需要新发送meta，vsh，ash以及gop等数据，再转发实时数据。
	//
	// ShouldWaitVideoKeyFrame
	//  表示是新加入的session，正在等待视频关键帧。
	//  注意，需要考虑没有纯音频流的场景。
	//
	IsFresh                 bool
	ShouldWaitVideoKeyFrame bool

	disposeOnce sync.Once

	DisposeByObserverFlag bool

	peerWinAckSize int
	recvLastAck    uint64
	seqNum         uint32
}

func NewServerSession(observer IServerSessionObserver, conn net.Conn) *ServerSession {
	s := &ServerSession{
		conn: connection.New(conn, func(option *connection.Option) {
			option.ReadBufSize = readBufSize
		}),
		sessionStat:             base.NewBasicSessionStat(base.SessionTypeRtmpServerSession, conn.RemoteAddr().String()),
		observer:                observer,
		chunkComposer:           NewChunkComposer(),
		packer:                  NewMessagePacker(),
		IsFresh:                 true,
		ShouldWaitVideoKeyFrame: true,
	}
	Log.Infof("[%s] lifecycle new rtmp ServerSession. session=%p, remote addr=%s", s.UniqueKey(), s, conn.RemoteAddr().String())
	return s
}

func (s *ServerSession) RunLoop() (err error) {
	if err = s.handshake(); err != nil {
		_ = s.dispose(err)
		return err
	}

	err = s.runReadLoop()
	_ = s.dispose(err)

	return err
}

func (s *ServerSession) Write(msg []byte) error {
	_, err := s.conn.Write(msg)
	return err
}

func (s *ServerSession) Writev(msgs net.Buffers) error {
	_, err := s.conn.Writev(msgs)
	return err
}

func (s *ServerSession) Flush() error {
	return s.conn.Flush()
}

// ----- IServerSessionLifecycle ---------------------------------------------------------------------------------------

func (s *ServerSession) Dispose() error {
	return s.dispose(nil)
}

// ----- ISessionUrlContext --------------------------------------------------------------------------------------------

func (s *ServerSession) Url() string {
	return s.url
}

func (s *ServerSession) AppName() string {
	return s.appName
}

func (s *ServerSession) StreamName() string {
	return s.streamName
}

func (s *ServerSession) RawQuery() string {
	return s.rawQuery
}

// ----- IObject -------------------------------------------------------------------------------------------------------

func (s *ServerSession) UniqueKey() string {
	return s.sessionStat.UniqueKey()
}

// ----- ISessionStat --------------------------------------------------------------------------------------------------

func (s *ServerSession) UpdateStat(intervalSec uint32) {
	s.sessionStat.UpdateStatWitchConn(s.conn, intervalSec)
}

func (s *ServerSession) GetStat() base.StatSession {
	return s.sessionStat.GetStatWithConn(s.conn)
}

func (s *ServerSession) IsAlive() (readAlive, writeAlive bool) {
	return s.sessionStat.IsAliveWitchConn(s.conn)
}

// ---------------------------------------------------------------------------------------------------------------------

func (s *ServerSession) runReadLoop() error {
	return s.chunkComposer.RunLoop(s.conn, s.doMsg)
}

func (s *ServerSession) handshake() error {
	if err := s.hs.ReadC0C1(s.conn); err != nil {
		return err
	}
	Log.Infof("[%s] < R Handshake C0+C1.", s.UniqueKey())

	Log.Infof("[%s] > W Handshake S0+S1+S2.", s.UniqueKey())
	if err := s.hs.WriteS0S1S2(s.conn); err != nil {
		return err
	}

	if err := s.hs.ReadC2(s.conn); err != nil {
		return err
	}
	Log.Infof("[%s] < R Handshake C2.", s.UniqueKey())
	return nil
}

func (s *ServerSession) doMsg(stream *Stream) error {
	var err error

	refForDebugLog := stream.msg.buff.Bytes()

	if err = s.writeAcknowledgementIfNeeded(stream); err != nil {
		Log.Errorf("[%s] doMsg failed. stream=%s, msg=%s", s.UniqueKey(), stream.toDebugString(), hex.EncodeToString(refForDebugLog))
		return err
	}

	switch stream.header.MsgTypeId {
	case base.RtmpTypeIdWinAckSize:
		err = s.doWinAckSize(stream)
	case base.RtmpTypeIdSetChunkSize:
		// noop
		// 因为底层的 chunk composer 已经处理过了，这里就不用处理
	case base.RtmpTypeIdCommandMessageAmf0:
		err = s.doCommandMessage(stream)
	case base.RtmpTypeIdCommandMessageAmf3:
		err = s.doCommandAmf3Message(stream)
	case base.RtmpTypeIdMetadata:
		err = s.doDataMessageAmf0(stream)
	case base.RtmpTypeIdAck:
		err = s.doAck(stream)
	case base.RtmpTypeIdUserControl:
		err = s.doUserControl(stream)
	case base.RtmpTypeIdAudio:
		fallthrough
	case base.RtmpTypeIdVideo:
		if s.sessionStat.BaseType() != base.SessionBaseTypePubStr {
			err = nazaerrors.Wrap(base.ErrRtmpUnexpectedMsg)
		}
		s.avObserver.OnReadRtmpAvMsg(stream.toAvMsg())
	default:
		Log.Warnf("[%s] read unknown message. stream=%s, msg=%s", s.UniqueKey(), stream.toDebugString(), hex.EncodeToString(refForDebugLog))

	}

	if err != nil {
		Log.Errorf("[%s] doMsg failed. stream=%s, msg=%s", s.UniqueKey(), stream.toDebugString(), hex.EncodeToString(refForDebugLog))
	}

	return err
}

func (s *ServerSession) doWinAckSize(stream *Stream) error {
	if stream.msg.Len() < 4 {
		return base.NewErrRtmpShortBuffer(4, int(stream.msg.Len()), "ServerSession::doWinAckSize")
	}

	s.peerWinAckSize = int(bele.BeUint32(stream.msg.buff.Bytes()))
	Log.Infof("[%s] < R Window Acknowledgement Size: %d", s.UniqueKey(), s.peerWinAckSize)
	return nil
}

func (s *ServerSession) doAck(stream *Stream) error {
	buf := stream.msg.buff.Bytes()
	if len(buf) < 4 {
		return base.ErrRtmpShortBuffer
	}
	seqNum := bele.BeUint32(buf)
	Log.Infof("[%s] < R Acknowledgement. ignore. sequence number=%d.", s.UniqueKey(), seqNum)
	return nil
}
func (s *ServerSession) doUserControl(stream *Stream) error {
	// TODO(chef): 检查buff长度有效性 202301
	userControlType := bele.BeUint16(stream.msg.buff.Bytes())
	if userControlType == uint16(base.RtmpUserControlPingRequest) {
		stream.msg.buff.Skip(2)
		timestamp := bele.BeUint32(stream.msg.buff.Bytes())
		return s.packer.writePingResponse(s.conn, timestamp)
	}
	return nil
}
func (s *ServerSession) doDataMessageAmf0(stream *Stream) error {
	if s.sessionStat.BaseType() != base.SessionBaseTypePubStr {
		return nazaerrors.Wrap(base.ErrRtmpUnexpectedMsg)
	}

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
	s.avObserver.OnReadRtmpAvMsg(stream.toAvMsg())
	return nil

	// TODO chef: 下面注释掉的代码包含的逻辑：
	// 1. 去除metadata中@setDataFrame
	// 2. 判断一些错误格式
	// 如果这个逻辑不是必须的，就可以删掉了
	// 另外，如果返回给上层的msg是删除了内容的buf，应该注意和header中的len保持一致
	//
	//switch val {
	//case "|RtmpSampleAccess":
	//	Log.Warnf("[%s] read data message, ignore it. val=%s", s.UniqueKey(), val)
	//	return nil
	//case "@setDataFrame":
	//	// macos obs and ffmpeg
	//	// skip @setDataFrame
	//	val, err = stream.msg.readStringWithType()
	//
	//	val, err := stream.msg.peekStringWithType()
	//	if err != nil {
	//		return err
	//	}
	//	if val != "onMetaData" {
	//		Log.Errorf("[%s] read unknown data message. val=%s, %s", s.UniqueKey(), val, stream.toDebugString())
	//		return ErrRtmp
	//	}
	//case "onMetaData":
	//	// noop
	//default:
	//	Log.Errorf("[%s] read unknown data message. val=%s, %s", s.UniqueKey(), val, stream.toDebugString())
	//	return nil
	//}
	//
	//s.avObserver.OnReadRtmpAvMsg(stream.toAvMsg())
	//return nil
}

func (s *ServerSession) doCommandMessage(stream *Stream) error {
	cmd, err := stream.msg.readStringWithType()
	if err != nil {
		return err
	}
	tid, err := stream.msg.readNumberWithType()
	if err != nil {
		return err
	}

	switch cmd {
	case "connect":
		return s.doConnect(tid, stream)
	case "createStream":
		return s.doCreateStream(tid, stream)
	case "publish":
		return s.doPublish(tid, stream)
	case "play":
		return s.doPlay(tid, stream)
	case "releaseStream":
		fallthrough
	case "FCPublish":
		fallthrough
	case "FCUnpublish":
		fallthrough
	case "getStreamLength":
		fallthrough
	case "deleteStream":
		Log.Debugf("[%s] read command message, ignore it. cmd=%s, %s", s.UniqueKey(), cmd, stream.toDebugString())
	default:
		Log.Errorf("[%s] read unknown command message. cmd=%s, %s", s.UniqueKey(), cmd, stream.toDebugString())
	}
	return nil
}

func (s *ServerSession) doCommandAmf3Message(stream *Stream) error {
	//去除前面的0就是Amf0的数据
	stream.msg.Skip(1)
	return s.doCommandMessage(stream)
}
func (s *ServerSession) writeAcknowledgementIfNeeded(stream *Stream) error {
	if s.peerWinAckSize <= 0 {
		return nil
	}

	currStat := s.conn.GetStat()
	delta := uint32(currStat.ReadBytesSum - s.recvLastAck)
	//此次接收小于窗口大小一半，不处理
	if delta < uint32(windowAcknowledgementSize/2) {
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

func (s *ServerSession) doConnect(tid int, stream *Stream) error {
	val, err := stream.msg.readObjectWithType()
	if err != nil {
		return err
	}
	s.appName, err = val.FindString("app")
	if err != nil {
		return err
	}
	s.tcUrl, err = val.FindString("tcUrl")
	if err != nil {
		Log.Warnf("[%s] tcUrl not exist.", s.UniqueKey())
	}
	Log.Infof("[%s] < R connect('%s'). tcUrl=%s", s.UniqueKey(), s.appName, s.tcUrl)

	s.observer.OnRtmpConnect(s, val)

	Log.Infof("[%s] > W Window Acknowledgement Size %d.", s.UniqueKey(), windowAcknowledgementSize)
	if err := s.packer.writeWinAckSize(s.conn, windowAcknowledgementSize); err != nil {
		return err
	}

	Log.Infof("[%s] > W Set Peer Bandwidth.", s.UniqueKey())
	if err := s.packer.writePeerBandwidth(s.conn, peerBandwidth, peerBandwidthLimitTypeDynamic); err != nil {
		return err
	}

	Log.Infof("[%s] > W SetChunkSize %d.", s.UniqueKey(), LocalChunkSize)
	if err := s.packer.writeChunkSize(s.conn, LocalChunkSize); err != nil {
		return err
	}

	Log.Infof("[%s] > W _result('NetConnection.Connect.Success').", s.UniqueKey())
	oe, err := val.FindNumber("objectEncoding")
	if oe != 0 && oe != 3 {
		oe = 0
	}
	if err := s.packer.writeConnectResult(s.conn, tid, oe); err != nil {
		return err
	}
	return nil
}

func (s *ServerSession) doCreateStream(tid int, stream *Stream) error {
	Log.Infof("[%s] < R createStream().", s.UniqueKey())
	Log.Infof("[%s] > W _result().", s.UniqueKey())
	if err := s.packer.writeCreateStreamResult(s.conn, tid); err != nil {
		return err
	}
	return nil
}

func (s *ServerSession) doPublish(tid int, stream *Stream) (err error) {
	if err = stream.msg.readNull(); err != nil {
		return err
	}
	s.streamNameWithRawQuery, err = stream.msg.readStringWithType()
	if err != nil {
		return err
	}
	ss := strings.Split(s.streamNameWithRawQuery, "?")
	s.streamName = ss[0]
	if len(ss) == 2 {
		s.rawQuery = ss[1]
	}

	s.url = fmt.Sprintf("%s/%s", s.tcUrl, s.streamNameWithRawQuery)

	pubType, err := stream.msg.readStringWithType()
	// 兼容 https://github.com/q191201771/lal/issues/280
	// 没有 pubType 时，继续走后面的流程
	if err != nil {
		Log.Warnf("[%s] read pubType failed. err=%s", s.UniqueKey(), err)
	}
	Log.Debugf("[%s] pubType=%s", s.UniqueKey(), pubType)
	Log.Infof("[%s] < R publish('%s')", s.UniqueKey(), s.streamNameWithRawQuery)

	Log.Infof("[%s] > W onStatus('NetStream.Publish.Start').", s.UniqueKey())
	if err = s.packer.writeOnStatusPublish(s.conn, Msid1); err != nil {
		return err
	}
	s.sessionStat.SetBaseType(base.SessionBaseTypePubStr)

	// 回复完信令后修改 connection 的属性
	s.modConnProps()

	err = s.observer.OnNewRtmpPubSession(s)
	if err != nil {
		s.DisposeByObserverFlag = true
	}
	return err
}

func (s *ServerSession) doPlay(tid int, stream *Stream) (err error) {
	if err = stream.msg.readNull(); err != nil {
		return err
	}
	s.streamNameWithRawQuery, err = stream.msg.readStringWithType()
	if err != nil {
		return err
	}
	ss := strings.Split(s.streamNameWithRawQuery, "?")
	s.streamName = ss[0]
	if len(ss) == 2 {
		s.rawQuery = ss[1]
	}

	s.url = fmt.Sprintf("%s/%s", s.tcUrl, s.streamNameWithRawQuery)

	Log.Infof("[%s] < R play('%s').", s.UniqueKey(), s.streamNameWithRawQuery)
	// TODO chef: start duration reset

	if err := s.packer.writeStreamIsRecorded(s.conn, Msid1); err != nil {
		return err
	}
	if err := s.packer.writeStreamBegin(s.conn, Msid1); err != nil {
		return err
	}

	Log.Infof("[%s] > W onStatus('NetStream.Play.Start').", s.UniqueKey())
	if err := s.packer.writeOnStatusPlay(s.conn, Msid1); err != nil {
		return err
	}
	s.sessionStat.SetBaseType(base.SessionBaseTypeSubStr)

	// 回复完信令后修改 connection 的属性
	s.modConnProps()

	err = s.observer.OnNewRtmpSubSession(s)
	if err != nil {
		s.DisposeByObserverFlag = true
	}
	return err
}

func (s *ServerSession) modConnProps() {
	s.conn.ModWriteChanSize(wChanSize)

	switch s.sessionStat.BaseType() {
	case base.SessionBaseTypePubStr:
		s.conn.ModReadTimeoutMs(serverSessionReadAvTimeoutMs)
	case base.SessionBaseTypeSubStr:
		s.conn.ModWriteTimeoutMs(serverSessionWriteAvTimeoutMs)
	}
}

func (s *ServerSession) dispose(err error) error {
	var retErr error
	s.disposeOnce.Do(func() {
		Log.Infof("[%s] lifecycle dispose rtmp ServerSession. err=%+v", s.UniqueKey(), err)
		if s.conn == nil {
			retErr = base.ErrSessionNotStarted
			return
		}
		retErr = s.conn.Close()
	})
	return retErr
}
