package source

// 离线状态
var (
	OffLineState int = 0
	OnLineState  int = 1
	LivingState  int = 2
)

type SeepEnumType uint32

const (
	SeepEnumNORMAL SeepEnumType = iota
	SeepEnumPAUSED
	SeepEnumSLOW_X2
	SeepEnumSLOW_X4
	SeepEnumSLOW_X8
	SeepEnumSLOW_X16
	SeepEnumFAST_X2
	SeepEnumFAST_X4
	SeepEnumFAST_X8
	SeepEnumFAST_X16
)

type TransType uint32

const StreamName = "stream_"

const (
	_ TransType = iota
	TransTypeTCP
	TransTypeUDP
	TransTypeMulticast
)

func (transType TransType) String() string {
	switch transType {
	case 1:
		return "TCP"
	case 2:
		return "UDP"
	case 3:
		return "Multicast"
	}
	return "unknown"
}

type DataType uint32

const (
	DATA_TYPE_VIDEO_FRAME      DataType = 0x00000001
	DATA_TYPE_AUDIO_FRAME      DataType = 0x00000002
	DATA_TYPE_EVENT_FRAME      DataType = 0x00000004
	DATA_TYPE_RTP_FRAME        DataType = 0x00000008
	DATA_TYPE_SDP_FRAME        DataType = 0x00000010
	DATA_TYPE_MEDIA_INFO       DataType = 0x00000020
	DATA_TYPE_SNAP_FRAME       DataType = 0x00000040
	EASY_SDK_BITRATE_INFO_FLAG DataType = 0x00000080
)

func (dataType DataType) String() string {
	switch dataType {
	case DATA_TYPE_VIDEO_FRAME:
		return "video_frame"
	case DATA_TYPE_AUDIO_FRAME:
		return "audio_frame"
	case DATA_TYPE_EVENT_FRAME:
		return "event_frame"
	case DATA_TYPE_RTP_FRAME:
		return "rtp_frame"
	case DATA_TYPE_SDP_FRAME:
		return "sdp_frame"
	case DATA_TYPE_MEDIA_INFO:
		return "media_info"
	case DATA_TYPE_SNAP_FRAME:
		return "snap_frame"
	}
	return "unknown"
}

type StreamClientState uint32

const (
	STREAM_CLIENT_STATE_CONNECTING StreamClientState = 1 + iota
	STREAM_CLIENT_STATE_CONNECTED
	STREAM_CLIENT_STATE_CONNECT_FAILED
	STREAM_CLIENT_STATE_CONNECT_ABORT
	STREAM_CLIENT_STATE_PUSHING
	STREAM_CLIENT_STATE_DISCONNECTED
	STREAM_CLIENT_STATE_EXIT
	STREAM_CLIENT_STATE_ERROR
)

func (state StreamClientState) String() string {
	switch state {
	case STREAM_CLIENT_STATE_CONNECTING:
		return "connecting"
	case STREAM_CLIENT_STATE_CONNECTED:
		return "connected"
	case STREAM_CLIENT_STATE_CONNECT_FAILED:
		return "failed"
	case STREAM_CLIENT_STATE_CONNECT_ABORT:
		return "abort"
	case STREAM_CLIENT_STATE_PUSHING:
		return "pushing"
	case STREAM_CLIENT_STATE_DISCONNECTED:
		return "disconnected"
	case STREAM_CLIENT_STATE_EXIT:
		return "exit"
	case STREAM_CLIENT_STATE_ERROR:
		return "error"
	}
	return "unknown"
}

type StreamStatus uint32

const (
	STREAM_STOPED StreamStatus = iota
	STREAM_OPENING
	STREAM_OPENED
	STREAM_STOPING
)

func (ss StreamStatus) String() string {
	switch ss {
	case STREAM_STOPED:
		return "stoped"
	case STREAM_OPENING:
		return "opening"
	case STREAM_OPENED:
		return "opened"
	case STREAM_STOPING:
		return "stoping"
	}
	return "unknown"
}
