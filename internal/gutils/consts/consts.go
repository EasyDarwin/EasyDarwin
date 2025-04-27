package consts

const (
	EmptyString     = ""
	ErrorString     = "error"
	ParamID         = "id"
	ParamSession    = "session"
	ParamProduct    = "p"
	ParamToken      = "token"
	ParamShareToken = "shareToken"
)

const (
	SplitComma = ","
	SplitBlank = " "
)

const (
	VodStatusWaiting  = "waiting"
	VodStatusTransing = "transing"
	VodStatusDone     = "done"
	VodStatusError    = "error"
)

const (
	VideoH265  = "H.265"
	VideoH264  = "H.264"
	VideoHevc  = "HEVC"
	VideoVp9   = "VP9"
	VideoVp8   = "VP8"
	VideoMpeg4 = "MPEG4"

	AudioAac  = "AAC"
	AudioMp3  = "MP3"
	AudioOpus = "Opus"
)

// 多清晰转码
const (
	//DefinitionSD 标清
	DefinitionSD = "sd"
	//DefinitionHD 高清
	DefinitionHD = "hd"
	//DefinitionFHD 超清
	DefinitionFHD = "fhd"
	//DefinitionYH 原画，视频的原始分辨率
	DefinitionYH = "yh"
)

const (
	RouteStaticVOD = "/fvod"
)

const (
	MsgSuccess         = "Success"
	MsgErrorBadRequest = "Bad Request Params"
)

const (
	SqlWhereID           = "id = ?"
	SqlWhereIDIn         = "id in (?)"
	SqlWhereIDNotIn      = "id not in (?)"
	SqlOrderCreateAtDesc = "create_at desc"
	SqlWhereUserID       = "user_id = ?"
	SqlShared            = "shared = ?"
)

const (
	Separator = "/"
)
