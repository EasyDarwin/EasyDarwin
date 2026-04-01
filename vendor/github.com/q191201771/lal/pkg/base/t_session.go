// Copyright 2020, Chef.  All rights reserved.
// https://github.com/q191201771/lal
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

package base

// ----- 所有session -----
//
// server.pub:  rtmp(ServerSession), rtsp(PubSession), customize(CustomizePubSessionContext), ps(gb28181.PubSession)
// server.sub:  rtmp(ServerSession), rtsp(SubSession), flv(SubSession), ts(SubSession), 还有一个比较特殊的hls
//
// client.push: rtmp(PushSession), rtsp(PushSession)
// client.pull: rtmp(PullSession), rtsp(PullSession), flv(PullSession)
//
// other:       rtmp.ClientSession, (rtmp.ServerSession)
//              rtsp.BaseInSession, rtsp.BaseOutSession, rtsp.ClientCommandSession, rtsp.ServerCommandSession
//              base.BasicHttpSubSession

// ---------------------------------------------------------------------------------------------------------------------

type (
	SessionType int
)

const (
	SessionTypeCustomizePub      SessionType = SessionProtocolCustomize<<8 | SessionBaseTypePub
	SessionTypeRtmpServerSession SessionType = SessionProtocolRtmp<<8 | SessionBaseTypePubSub
	SessionTypeRtmpPush          SessionType = SessionProtocolRtmp<<8 | SessionBaseTypePush
	SessionTypeRtmpPull          SessionType = SessionProtocolRtmp<<8 | SessionBaseTypePull
	SessionTypeRtspPub           SessionType = SessionProtocolRtsp<<8 | SessionBaseTypePub
	SessionTypeRtspSub           SessionType = SessionProtocolRtsp<<8 | SessionBaseTypeSub
	SessionTypeRtspPush          SessionType = SessionProtocolRtsp<<8 | SessionBaseTypePush
	SessionTypeRtspPull          SessionType = SessionProtocolRtsp<<8 | SessionBaseTypePull
	SessionTypeFlvSub            SessionType = SessionProtocolFlv<<8 | SessionBaseTypeSub
	SessionTypeFlvPull           SessionType = SessionProtocolFlv<<8 | SessionBaseTypePull
	SessionTypeTsSub             SessionType = SessionProtocolTs<<8 | SessionBaseTypeSub
	SessionTypePsPub             SessionType = SessionProtocolPs<<8 | SessionBaseTypePub
	SessionTypeHlsSub            SessionType = SessionProtocolHls<<8 | SessionBaseTypeSub

	SessionProtocolCustomize = 1
	SessionProtocolRtmp      = 2
	SessionProtocolRtsp      = 3
	SessionProtocolFlv       = 4
	SessionProtocolTs        = 5
	SessionProtocolPs        = 6
	SessionProtocolHls       = 7

	SessionBaseTypePubSub = 1
	SessionBaseTypePub    = 2
	SessionBaseTypeSub    = 3
	SessionBaseTypePush   = 4
	SessionBaseTypePull   = 5

	SessionProtocolCustomizeStr = "CUSTOMIZE"
	SessionProtocolRtmpStr      = "RTMP"
	SessionProtocolRtspStr      = "RTSP"
	SessionProtocolFlvStr       = "FLV"
	SessionProtocolTsStr        = "TS"
	SessionProtocolPsStr        = "PS"
	SessionProtocolHlsStr       = "HLS"

	SessionBaseTypePubSubStr = "PUBSUB"
	SessionBaseTypePubStr    = "PUB"
	SessionBaseTypeSubStr    = "SUB"
	SessionBaseTypePushStr   = "PUSH"
	SessionBaseTypePullStr   = "PULL"
)

type ISession interface {
	ISessionUrlContext
	IObject
	ISessionStat
}

type IClientSession interface {
	// PushSession:
	// Push()
	// Write()
	// Flush()
	// PullSession:
	// Pull()

	IClientSessionLifecycle
	ISession
}

type IServerSession interface {
	IServerSessionLifecycle
	ISession
}

// ---------------------------------------------------------------------------------------------------------------------

type IClientSessionLifecycle interface {
	// Dispose 主动关闭session时调用
	//
	// 注意，只有Start（具体session的Start类型函数一般命令为Push和Pull）成功后的session才能调用，否则行为未定义
	//
	// Dispose可在任意协程内调用
	//
	// 注意，目前Dispose允许调用多次，但是未来可能不对该表现做保证
	//
	// Dispose后，调用Write函数将返回错误
	//
	// @return 可以通过返回值判断调用Dispose前，session是否已经被关闭了 TODO(chef) 这个返回值没有太大意义，后面可能会删掉
	//
	Dispose() error

	// WaitChan Start成功后，可使用这个channel来接收session结束的消息
	//
	// 注意，只有Start成功后的session才能调用，否则行为未定义
	//
	// 注意，目前WaitChan只会被通知一次，但是未来可能不对该表现做保证，业务方应该只关注第一次通知
	//
	// TODO(chef): 是否应该严格保证：获取到关闭消息后，后续不应该再有该session的回调上来
	//
	// @return 一般关闭有以下几种情况：
	//         - 对端关闭，此时error为EOF
	//         - 本端底层关闭，比如协议非法等，此时error为具体的错误值
	//         - 本端上层主动调用Dispose关闭，此时error为nil
	//
	WaitChan() <-chan error
}

type IServerSessionLifecycle interface {
	// RunLoop 开启session的事件循环，阻塞直到session结束
	//
	// 注意，rtsp的 pub和sub没有RunLoop，RunLoop是在cmd session上，所以暂时把这个函数从接口去除
	//
	//RunLoop() error

	// Dispose 主动关闭session时调用
	//
	// 如果是session通知业务方session已关闭（比如`RunLoop`函数返回错误），则不需要调用`Dispose` TODO(chef): review现状
	//
	Dispose() error
}

// ISessionStat
//
// 调用约束：对于Client类型的Session，调用Start函数并返回成功后才能调用，否则行为未定义
type ISessionStat interface {
	// UpdateStat
	//
	// 周期性调用该函数，用于计算bitrate
	//
	// @param intervalSec 距离上次调用的时间间隔，单位毫秒
	//
	UpdateStat(intervalSec uint32)

	// GetStat
	//
	// 获取session状态
	//
	// @return 注意，结构体中的`BitrateKbits`的值由最近一次`func UpdateStat`调用计算决定，其他值为当前最新值
	//
	GetStat() StatSession

	// IsAlive
	//
	// 周期性调用该函数，判断是否有读取、写入数据
	// 注意，判断的依据是，距离上次调用该函数的时间间隔内，是否有读取、写入数据
	// 注意，不活跃，并一定是链路或网络有问题，也可能是业务层没有写入数据
	//
	// @return readAlive  读取是否获取
	// @return writeAlive 写入是否活跃
	//
	IsAlive() (readAlive, writeAlive bool)
}

// ISessionUrlContext 获取和流地址相关的信息
//
// 调用约束：对于Client类型的Session，调用Start函数并返回成功后才能调用，否则行为未定义
type ISessionUrlContext interface {
	Url() string
	AppName() string
	StreamName() string
	RawQuery() string // 参数，也即 url param
}

type IObject interface {
	// UniqueKey
	//
	// 对象的全局唯一标识
	//
	UniqueKey() string
}

// TODO chef: rtmp.ClientSession修改为BaseClientSession更好些

// TODO(chef): [refactor] 整理 subsession 接口部分 IsFresh 和 ShouldWaitVideoKeyFrame

// ----- group中，session Dispose表现记录 -----
//
// Dispose结束后回调OnDel:
// rtmp.ServerSession(包含pub和sub)  1
// rtsp.PubSession和rtsp.SubSession 1
// rtmp.PullSession 2
// httpflv.SubSession 3
// httpts.SubSession 3
//
//
// 情况1: 协议正常走完回调OnAdd，在自身server的RunLoop结束后，回调OnDel
// 情况2: 在group中pull阻塞结束后，手动回调OnDel
// 情况3: 在logic中sub RunLoop结束后，手动回调OnDel

// TODO(chef): 整理所有Server类型Session的生命周期管理
//   -
//   - rtmp没有独立的Pub、Sub Session结构体类型，而是直接使用ServerSession
//   - write失败，需要反应到loop来
//   - rtsp是否也应该上层使用Command作为代理，避免生命周期管理混乱
//

// ISessionUrlContext 实际测试
//
// |                | 实际url                                               | Url()    | AppName, StreamName, RawQuery  |
// | -              | -                                                    | -        | -                              |
// | rtmp pub推流    | rtmp://127.0.0.1:1935/live/test110                   | 同实际url | live, test110,                 |
// |                | rtmp://127.0.0.1:1935/a/b/c/d/test110?p1=1&p2=2      | 同实际url | a/b, c/d/test110, p1=1&p2=2    |
// | rtsp pub推流    | rtsp://localhost:5544/live/test110                   | 同实际url | live, test110,                 |
// | rtsp pub推流    | rtsp://localhost:5544/a/b/c/d/test110?p1=1&p2=2      | 同实际url | a/b/c/d, test110, p1=1&p2=2    |
// | httpflv sub拉流  | http://127.0.0.1:8080/live/test110.flv              | 同实际url | live, test110,                 |
// |                 | http://127.0.0.1:8080/a/b/c/d/test110.flv?p1=1&p2=2 | 同实际url | a/b/c/d, test110, p1=1&p2=2    |
// | rtmp sub拉流    | 同rtmp pub                                           | .        | .                              |
// | rtsp sub拉流    | 同rtsp pub                                           | .        | .                              |
// | httpts sub拉流 | 同httpflv sub，只是末尾的.flv换成.ts，不再赘述             | .       | .                              |

// IsUseClosedConnectionError 当connection处于这些情况时，就不需要再Close了
// TODO(chef): 临时放这
// TODO(chef): 目前暂时没有使用，因为connection支持多次调用Close
//
//func IsUseClosedConnectionError(err error) bool {
//	if err == io.EOF || (err != nil && strings.Contains(err.Error(), "use of closed network connection")) {
//		return true
//	}
//	return false
//}
