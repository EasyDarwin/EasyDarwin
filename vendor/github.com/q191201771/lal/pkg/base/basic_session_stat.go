// Copyright 2022, Chef.  All rights reserved.
// https://github.com/q191201771/lal
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

package base

import (
	"github.com/q191201771/naza/pkg/connection"
	"github.com/q191201771/naza/pkg/nazalog"
)

type IStatable interface {
	GetStat() connection.Stat // TODO(chef): [refactor] 考虑为 nazanet.UdpConnection 实现这个接口
}

// BasicSessionStat
//
// 包含两部分功能：
// 1. 维护 StatSession 的一些静态信息
// 2. 计算带宽
//
// 计算带宽有两种方式，一种是通过外部的 connection.Connection 获取最新状态，一种是内部自己管理状态
type BasicSessionStat struct {
	stat StatSession

	prevConnStat connection.Stat
	staleStat    *connection.Stat

	currConnStat connection.StatAtomic
}

// ---------------------------------------------------------------------------------------------------------------------

// NewBasicSessionStat
//
// @param remoteAddr: 如果当前未知，填入""空字符串
func NewBasicSessionStat(sessionType SessionType, remoteAddr string) BasicSessionStat {
	var s BasicSessionStat
	s.stat.typ = sessionType
	s.stat.StartTime = ReadableNowTime()
	s.stat.RemoteAddr = remoteAddr

	// TODO(chef): [fix] 为customize pub添加 202205
	switch sessionType {
	case SessionTypeRtmpServerSession:
		s.stat.SessionId = GenUkRtmpServerSession()
		s.stat.BaseType = SessionBaseTypePubSubStr
		s.stat.Protocol = SessionProtocolRtmpStr
	case SessionTypeRtmpPush:
		s.stat.SessionId = GenUkRtmpPushSession()
		s.stat.BaseType = SessionBaseTypePushStr
		s.stat.Protocol = SessionProtocolRtmpStr
	case SessionTypeRtmpPull:
		s.stat.SessionId = GenUkRtmpPullSession()
		s.stat.BaseType = SessionBaseTypePullStr
		s.stat.Protocol = SessionProtocolRtmpStr
	case SessionTypeRtspPub:
		s.stat.SessionId = GenUkRtspPubSession()
		s.stat.BaseType = SessionBaseTypePubStr
		s.stat.Protocol = SessionProtocolRtspStr
	case SessionTypeRtspSub:
		s.stat.SessionId = GenUkRtspSubSession()
		s.stat.BaseType = SessionBaseTypeSubStr
		s.stat.Protocol = SessionProtocolRtspStr
	case SessionTypeRtspPush:
		s.stat.SessionId = GenUkRtspPushSession()
		s.stat.BaseType = SessionBaseTypePushStr
		s.stat.Protocol = SessionProtocolRtspStr
	case SessionTypeRtspPull:
		s.stat.SessionId = GenUkRtspPullSession()
		s.stat.BaseType = SessionBaseTypePullStr
		s.stat.Protocol = SessionProtocolRtspStr
	case SessionTypeFlvSub:
		s.stat.SessionId = GenUkFlvSubSession()
		s.stat.BaseType = SessionBaseTypeSubStr
		s.stat.Protocol = SessionProtocolFlvStr
	case SessionTypePsPub:
		s.stat.SessionId = GenUkPsPubSession()
		s.stat.BaseType = SessionBaseTypePubStr
		s.stat.Protocol = SessionProtocolPsStr
	case SessionTypeHlsSub:
		s.stat.SessionId = GenUkHlsSubSession()
		s.stat.BaseType = SessionBaseTypeSubStr
		s.stat.Protocol = SessionProtocolHlsStr
	case SessionTypeTsSub:
		s.stat.SessionId = GenUkTsSubSession()
		s.stat.BaseType = SessionBaseTypeSubStr
		s.stat.Protocol = SessionProtocolTsStr
	default:
		nazalog.Errorf("unknown session type: [%d]", sessionType)
	}
	return s
}

func (s *BasicSessionStat) SetBaseType(baseType string) {
	s.stat.BaseType = baseType
}

func (s *BasicSessionStat) SetRemoteAddr(addr string) {
	s.stat.RemoteAddr = addr
}

// ---------------------------------------------------------------------------------------------------------------------

func (s *BasicSessionStat) AddReadBytes(n int) {
	s.currConnStat.ReadBytesSum.Add(uint64(n))
}

func (s *BasicSessionStat) AddWriteBytes(n int) {
	s.currConnStat.WroteBytesSum.Add(uint64(n))
}

func (s *BasicSessionStat) UpdateStat(intervalSec uint32) {
	s.updateStat(s.currConnStat.ReadBytesSum.Load(), s.currConnStat.WroteBytesSum.Load(), s.stat.BaseType, intervalSec)
}

func (s *BasicSessionStat) UpdateStatWitchConn(conn IStatable, intervalSec uint32) {
	currStat := conn.GetStat()
	s.updateStat(currStat.ReadBytesSum, currStat.WroteBytesSum, s.stat.BaseType, intervalSec)
}

func (s *BasicSessionStat) GetStat() StatSession {
	s.stat.ReadBytesSum = s.currConnStat.ReadBytesSum.Load()
	s.stat.WroteBytesSum = s.currConnStat.WroteBytesSum.Load()
	return s.stat
}

func (s *BasicSessionStat) GetStatWithConn(conn IStatable) StatSession {
	if conn == nil {
		return s.stat
	}

	connStat := conn.GetStat()
	s.stat.ReadBytesSum = connStat.ReadBytesSum
	s.stat.WroteBytesSum = connStat.WroteBytesSum
	return s.stat
}

func (s *BasicSessionStat) IsAlive() (readAlive, writeAlive bool) {
	return s.isAlive(s.currConnStat.ReadBytesSum.Load(), s.currConnStat.WroteBytesSum.Load())
}

func (s *BasicSessionStat) IsAliveWitchConn(conn IStatable) (readAlive, writeAlive bool) {
	if conn == nil {
		return s.isAlive(0, 0)
	}

	currStat := conn.GetStat()
	return s.isAlive(currStat.ReadBytesSum, currStat.WroteBytesSum)
}

// ---------------------------------------------------------------------------------------------------------------------

func (s *BasicSessionStat) BaseType() string {
	return s.stat.BaseType
}

func (s *BasicSessionStat) UniqueKey() string {
	return s.stat.SessionId
}

// ---------------------------------------------------------------------------------------------------------------------

// updateStat 根据两次调用间隔计算bitrate
func (s *BasicSessionStat) updateStat(readBytesSum, wroteBytesSum uint64, typ string, intervalSec uint32) {
	rDiff := readBytesSum - s.prevConnStat.ReadBytesSum
	s.stat.ReadBitrateKbits = int(rDiff * 8 / 1024 / uint64(intervalSec))
	wDiff := wroteBytesSum - s.prevConnStat.WroteBytesSum
	s.stat.WriteBitrateKbits = int(wDiff * 8 / 1024 / uint64(intervalSec))

	switch typ {
	case SessionBaseTypePubStr, SessionBaseTypePullStr:
		s.stat.BitrateKbits = s.stat.ReadBitrateKbits
	case SessionBaseTypeSubStr, SessionBaseTypePushStr:
		s.stat.BitrateKbits = s.stat.WriteBitrateKbits
	default:
		nazalog.Errorf("invalid session base type. type=%s", typ)
	}

	s.prevConnStat.ReadBytesSum = readBytesSum
	s.prevConnStat.WroteBytesSum = wroteBytesSum
}

// isAlive 根据两次调用间隔计算是否存活
func (s *BasicSessionStat) isAlive(readBytesSum, wroteBytesSum uint64) (readAlive, writeAlive bool) {
	if s.staleStat == nil {
		s.staleStat = new(connection.Stat)
		s.staleStat.ReadBytesSum = readBytesSum
		s.staleStat.WroteBytesSum = wroteBytesSum
		return true, true
	}

	readAlive = !(readBytesSum-s.staleStat.ReadBytesSum == 0)
	writeAlive = !(wroteBytesSum-s.staleStat.WroteBytesSum == 0)
	s.staleStat.ReadBytesSum = readBytesSum
	s.staleStat.WroteBytesSum = wroteBytesSum
	return
}
