// Copyright 2022, Chef.  All rights reserved.
// https://github.com/q191201771/lal
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

package logic

import (
	"github.com/q191201771/lal/pkg/hls"
	"github.com/q191201771/lal/pkg/httpflv"
	"github.com/q191201771/lal/pkg/httpts"
	"github.com/q191201771/lal/pkg/rtmp"
	"github.com/q191201771/lal/pkg/rtsp"
)

func (group *Group) AddRtmpSubSession(session *rtmp.ServerSession) {
	Log.Debugf("[%s] [%s] add SubSession into group.", group.UniqueKey, session.UniqueKey())
	group.mutex.Lock()
	defer group.mutex.Unlock()
	group.rtmpSubSessionSet[session] = struct{}{}
	// 加入时，如果上行还没有推过视频（比如还没推流，或者是单音频流），就不需要等待关键帧了
	// 也即我们假定上行肯定是以关键帧为开始进行视频发送，假设不是，那么我们按上行的流正常发，而不过滤掉关键帧前面的不包含关键帧的非完整GOP
	// TODO(chef):
	//   1. 需要仔细考虑单音频无视频的流的情况
	//   2. 这里不修改标志，让这个session继续等关键帧也可以
	if group.stat.VideoCodec == "" {
		session.ShouldWaitVideoKeyFrame = false
	}

	group.addSub()
}

func (group *Group) AddHttpflvSubSession(session *httpflv.SubSession) {
	Log.Debugf("[%s] [%s] add httpflv SubSession into group.", group.UniqueKey, session.UniqueKey())
	session.WriteHttpResponseHeader()
	session.WriteFlvHeader()

	group.mutex.Lock()
	defer group.mutex.Unlock()
	group.httpflvSubSessionSet[session] = struct{}{}
	// 加入时，如果上行还没有推流过，就不需要等待关键帧了
	if group.stat.VideoCodec == "" {
		session.ShouldWaitVideoKeyFrame = false
	}

	group.addSub()
}

// AddHttptsSubSession ...
func (group *Group) AddHttptsSubSession(session *httpts.SubSession) {
	Log.Debugf("[%s] [%s] add httpts SubSession into group.", group.UniqueKey, session.UniqueKey())
	session.WriteHttpResponseHeader()

	group.mutex.Lock()
	defer group.mutex.Unlock()
	group.httptsSubSessionSet[session] = struct{}{}

	group.addSub()
}

// AddHlsSubSession ...
func (group *Group) AddHlsSubSession(session *hls.SubSession) {
	Log.Debugf("[%s] [%s] add hls SubSession into group.", group.UniqueKey, session.UniqueKey())

	group.mutex.Lock()
	defer group.mutex.Unlock()
	group.hlsSubSessionSet[session] = struct{}{}

	group.addSub()
}

func (group *Group) HandleNewRtspSubSessionDescribe(session *rtsp.SubSession) (ok bool, sdp []byte) {
	Log.Debugf("[%s] [%s] rtsp sub describe.", group.UniqueKey, session.UniqueKey())

	group.mutex.Lock()
	defer group.mutex.Unlock()

	group.rtspSubSessionSet[session] = struct{}{}

	if group.sdpCtx == nil {
		Log.Warnf("[%s] [%s] rtsp subSession describe but sdp not exist.", group.UniqueKey, session.UniqueKey())

		return true, nil
	}
	return true, group.sdpCtx.RawSdp
}

func (group *Group) HandleNewRtspSubSessionPlay(session *rtsp.SubSession) {
	Log.Debugf("[%s] [%s] rtsp sub play.", group.UniqueKey, session.UniqueKey())

	group.mutex.Lock()
	defer group.mutex.Unlock()

	if group.stat.VideoCodec == "" {
		session.ShouldWaitVideoKeyFrame = false
	}

	group.addSub()
}

func (group *Group) DelRtmpSubSession(session *rtmp.ServerSession) {
	group.mutex.Lock()
	defer group.mutex.Unlock()
	group.delRtmpSubSession(session)
}

func (group *Group) DelHttpflvSubSession(session *httpflv.SubSession) {
	group.mutex.Lock()
	defer group.mutex.Unlock()
	group.delHttpflvSubSession(session)
}

func (group *Group) DelHttptsSubSession(session *httpts.SubSession) {
	group.mutex.Lock()
	defer group.mutex.Unlock()
	group.delHttptsSubSession(session)
}

func (group *Group) DelRtspSubSession(session *rtsp.SubSession) {
	group.mutex.Lock()
	defer group.mutex.Unlock()
	group.delRtspSubSession(session)
}

func (group *Group) DelHlsSubSession(session *hls.SubSession) {
	group.mutex.Lock()
	defer group.mutex.Unlock()
	group.delHlsSubSession(session)
}

// ---------------------------------------------------------------------------------------------------------------------

func (group *Group) delRtmpSubSession(session *rtmp.ServerSession) {
	Log.Debugf("[%s] [%s] del rtmp SubSession from group.", group.UniqueKey, session.UniqueKey())
	delete(group.rtmpSubSessionSet, session)
}

func (group *Group) delHttpflvSubSession(session *httpflv.SubSession) {
	Log.Debugf("[%s] [%s] del httpflv SubSession from group.", group.UniqueKey, session.UniqueKey())
	delete(group.httpflvSubSessionSet, session)
}

func (group *Group) delHttptsSubSession(session *httpts.SubSession) {
	Log.Debugf("[%s] [%s] del httpts SubSession from group.", group.UniqueKey, session.UniqueKey())
	delete(group.httptsSubSessionSet, session)
}

func (group *Group) delRtspSubSession(session *rtsp.SubSession) {
	Log.Debugf("[%s] [%s] del rtsp SubSession from group.", group.UniqueKey, session.UniqueKey())

	delete(group.rtspSubSessionSet, session)
}

func (group *Group) delHlsSubSession(session *hls.SubSession) {
	Log.Debugf("[%s] [%s] del hls SubSession from group.", group.UniqueKey, session.UniqueKey())
	delete(group.hlsSubSessionSet, session)
}

// ---------------------------------------------------------------------------------------------------------------------

func (group *Group) addSub() {
	group.pullIfNeeded()
}
