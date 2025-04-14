package gb28181

import (
	"sync"

	"github.com/gin-gonic/gin"
)

type GbLogic struct {
	s *GB28181Server
}

var gbLogic *GbLogic
var once sync.Once

func NewGbLogic(s *GB28181Server) *GbLogic {
	once.Do(func() {
		gbLogic = &GbLogic{
			s: s,
		}
	})
	return gbLogic
}

func (g *GbLogic) GetDeviceInfos(c *gin.Context) {
	deviceInfos := g.s.getDeviceInfos()
	ResponseSuccess(c, deviceInfos)
}

func (g *GbLogic) StartPlay(c *gin.Context) {
	var reqPlay ReqPlay
	if err := c.ShouldBindJSON(&reqPlay); err != nil {
		ResponseErrorWithMsg(c, CodeInvalidParam, CodeInvalidParam.Msg())
	} else {
		ch := g.s.FindChannel(reqPlay.DeviceId, reqPlay.ChannelId)
		if ch == nil {
			ResponseErrorWithMsg(c, CodeDeviceNotRegister, CodeDeviceNotRegister.Msg())
		} else {
			streamName := reqPlay.StreamName
			if len(streamName) == 0 {
				streamName = reqPlay.ChannelId
			}
			if len(reqPlay.NetWork) == 0 || !(reqPlay.NetWork == "udp" || reqPlay.NetWork == "tcp") {
				reqPlay.NetWork = "udp"
			}

			ch.TryAutoInvite(&InviteOptions{}, streamName, &reqPlay.PlayInfo)
			respPlay := &RespPlay{
				StreamName: streamName,
			}
			ResponseSuccess(c, respPlay)
		}
	}

}
func (g *GbLogic) StopPlay(c *gin.Context) {
	var reqStop ReqStop
	if err := c.ShouldBindJSON(&reqStop); err != nil {
		ResponseErrorWithMsg(c, CodeInvalidParam, CodeInvalidParam.Msg())
	} else {
		ch := g.s.FindChannel(reqStop.DeviceId, reqStop.ChannelId)
		if ch == nil {
			ResponseErrorWithMsg(c, CodeDeviceNotRegister, CodeDeviceNotRegister.Msg())
		} else {
			streamName := reqStop.StreamName
			if len(streamName) == 0 {
				streamName = reqStop.ChannelId
			}
			if err = ch.Bye(streamName); err != nil {
				ResponseErrorWithMsg(c, CodeDeviceStopError, err.Error())
			} else {
				ResponseSuccess(c, nil)
			}
		}
	}
}
func (g *GbLogic) PtzDirection(c *gin.Context) {
	var reqDirection PtzDirection
	if err := c.ShouldBindJSON(&reqDirection); err != nil {
		ResponseErrorWithMsg(c, CodeInvalidParam, CodeInvalidParam.Msg())
	} else {
		if !(reqDirection.Speed > 0 && reqDirection.Speed <= 8) {
			ResponseErrorWithMsg(c, CodeInvalidParam, SpeedParamError)
		}
		ch := g.s.FindChannel(reqDirection.DeviceId, reqDirection.ChannelId)
		if ch == nil {
			ResponseErrorWithMsg(c, CodeDeviceNotRegister, CodeDeviceNotRegister.Msg())
		} else {
			reqDirection.Speed = reqDirection.Speed * 25
			if err = ch.PtzDirection(&reqDirection); err != nil {
				ResponseErrorWithMsg(c, CodeDeviceStopError, err.Error())
			} else {
				ResponseSuccess(c, nil)
			}
		}
	}
}
func (g *GbLogic) PtzZoom(c *gin.Context) {
	var reqZoom PtzZoom
	if err := c.ShouldBindJSON(&reqZoom); err != nil {
		ResponseErrorWithMsg(c, CodeInvalidParam, CodeInvalidParam.Msg())
	} else {
		if !(reqZoom.Speed > 0 && reqZoom.Speed <= 8) {
			ResponseErrorWithMsg(c, CodeInvalidParam, SpeedParamError)
		}
		ch := g.s.FindChannel(reqZoom.DeviceId, reqZoom.ChannelId)
		if ch == nil {
			ResponseErrorWithMsg(c, CodeDeviceNotRegister, CodeDeviceNotRegister.Msg())
		} else {
			reqZoom.Speed = reqZoom.Speed * 25
			if err = ch.PtzZoom(&reqZoom); err != nil {
				ResponseErrorWithMsg(c, CodeDeviceStopError, err.Error())
			} else {
				ResponseSuccess(c, nil)
			}
		}
	}
}
func (g *GbLogic) PtzFi(c *gin.Context) {
	var reqFi PtzFi
	if err := c.ShouldBindJSON(&reqFi); err != nil {
		ResponseErrorWithMsg(c, CodeInvalidParam, CodeInvalidParam.Msg())
	} else {
		if !(reqFi.Speed > 0 && reqFi.Speed <= 8) {
			ResponseErrorWithMsg(c, CodeInvalidParam, SpeedParamError)
		}
		ch := g.s.FindChannel(reqFi.DeviceId, reqFi.ChannelId)
		if ch == nil {
			ResponseErrorWithMsg(c, CodeDeviceNotRegister, CodeDeviceNotRegister.Msg())
		} else {
			reqFi.Speed = reqFi.Speed * 25
			if err = ch.PtzFi(&reqFi); err != nil {
				ResponseErrorWithMsg(c, CodeDeviceStopError, err.Error())
			} else {
				ResponseSuccess(c, nil)
			}
		}
	}
}
func (g *GbLogic) PtzPreset(c *gin.Context) {
	var reqPreset PtzPreset
	if err := c.ShouldBindJSON(&reqPreset); err != nil {
		ResponseErrorWithMsg(c, CodeInvalidParam, CodeInvalidParam.Msg())
	} else {
		if !(reqPreset.Point > 0 && reqPreset.Point <= 50) {
			ResponseErrorWithMsg(c, CodeInvalidParam, PointParamError)
		}
		ch := g.s.FindChannel(reqPreset.DeviceId, reqPreset.ChannelId)
		if ch == nil {
			ResponseErrorWithMsg(c, CodeDeviceNotRegister, CodeDeviceNotRegister.Msg())
		} else {
			if err = ch.PtzPreset(&reqPreset); err != nil {
				ResponseErrorWithMsg(c, CodeDeviceStopError, err.Error())
			} else {
				ResponseSuccess(c, nil)
			}
		}
	}
}
func (g *GbLogic) PtzStop(c *gin.Context) {
	var reqStop PtzStop
	if err := c.ShouldBindJSON(&reqStop); err != nil {
		ResponseErrorWithMsg(c, CodeInvalidParam, CodeInvalidParam.Msg())
	} else {
		ch := g.s.FindChannel(reqStop.DeviceId, reqStop.ChannelId)
		if ch == nil {
			ResponseErrorWithMsg(c, CodeDeviceNotRegister, CodeDeviceNotRegister.Msg())
		} else {
			if err = ch.PtzStop(&reqStop); err != nil {
				ResponseErrorWithMsg(c, CodeDeviceStopError, err.Error())
			} else {
				ResponseSuccess(c, nil)
			}
		}
	}
}
func (g *GbLogic) UpdateAllNotify(c *gin.Context) {
	g.s.GetAllSyncChannels()
	ResponseSuccess(c, nil)
}
func (g *GbLogic) UpdateNotify(c *gin.Context) {
	var reqUpdateNotify ReqUpdateNotify
	if err := c.ShouldBindJSON(&reqUpdateNotify); err != nil {
		ResponseErrorWithMsg(c, CodeInvalidParam, CodeInvalidParam.Msg())
	} else {
		if g.s.GetSyncChannels(reqUpdateNotify.DeviceId) {
			ResponseSuccess(c, nil)
		} else {
			ResponseErrorWithMsg(c, CodeDeviceNotRegister, CodeDeviceNotRegister.Msg())
		}

	}

}
