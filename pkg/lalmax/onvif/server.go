package onvif

import (
	"bytes"
	"context"
	"encoding/json"
	"fmt"
	"net/http"
	"strings"
	"time"

	"github.com/gin-gonic/gin"
	"github.com/q191201771/lal/pkg/base"
	"github.com/q191201771/naza/pkg/nazalog"
	goonvif "github.com/use-go/onvif"
	"github.com/use-go/onvif/device"
	media "github.com/use-go/onvif/media"
	sdk "github.com/use-go/onvif/sdk/device"
	sdkmedia "github.com/use-go/onvif/sdk/media"
	onvifcmd "github.com/use-go/onvif/xsd/onvif"
)

type OnvifPullRequest struct {
	Addr            string `json:"addr"`            // 摄像机IP:PORT
	Username        string `json:"username"`        // 用户名
	Password        string `json:"password"`        // 密码
	RtspMode        int    `json:"rtspmode"`        // rtsp拉流模式,0-tcp, 1-udp
	PullAllProfiles bool   `json:"pullallprofiles"` // 是否请求所有profiles
}

type OnvifServer struct {
}

func NewOnvifServer() *OnvifServer {
	return &OnvifServer{}
}

func (s *OnvifServer) HandlePull(c *gin.Context) {
	pullreq := OnvifPullRequest{}
	err := c.ShouldBind(&pullreq)
	if err != nil {
		c.Status(http.StatusBadRequest)
		return
	}

	dev, err := goonvif.NewDevice(goonvif.DeviceParams{
		Xaddr:    pullreq.Addr,
		Username: pullreq.Username,
		Password: pullreq.Password,
	})

	if err != nil {
		nazalog.Error(err)
		return
	}

	deviceInfoReq := device.GetDeviceInformation{}
	deviceInfoRes, err := sdk.Call_GetDeviceInformation(context.Background(), dev, deviceInfoReq)
	if err != nil {
		nazalog.Error(err)
		return
	}

	getCapabilities := device.GetCapabilities{Category: "All"}
	_, err = sdk.Call_GetCapabilities(context.Background(), dev, getCapabilities)
	if err != nil {
		nazalog.Error("Call_GetCapabilities failed, err:", err)
		c.Status(http.StatusInternalServerError)
		return
	}

	profilesReq := media.GetProfiles{}
	profilesRes, err := sdkmedia.Call_GetProfiles(context.Background(), dev, profilesReq)
	if err != nil {
		nazalog.Error("Call_GetProfiles failed, err:", err)
		c.Status(http.StatusInternalServerError)
		return
	}

	if len(profilesRes.Profiles) == 0 {
		nazalog.Error("profilesRes.Profiles invalid")
		c.Status(http.StatusInternalServerError)
		return
	}

	var protocol onvifcmd.TransportProtocol
	if pullreq.RtspMode == 1 {
		protocol = "UDP"
	} else {
		protocol = "TCP"
	}

	if pullreq.PullAllProfiles {
		for _, profile := range profilesRes.Profiles {
			streamUrlReq := media.GetStreamUri{
				ProfileToken: profile.Token,
				StreamSetup: onvifcmd.StreamSetup{
					Stream: "RTP-Unicast",
					Transport: onvifcmd.Transport{
						Protocol: protocol,
					},
				},
			}
			streamUrlRes, err := sdkmedia.Call_GetStreamUri(context.Background(), dev, streamUrlReq)
			if err != nil {
				nazalog.Error(err)
				return
			}

			playUrl := buildPlayUrl(string(streamUrlRes.MediaUri.Uri), pullreq.Username, pullreq.Password)
			DoPull(playUrl, fmt.Sprintf("%s-%s", deviceInfoRes.Model, profile.Name), pullreq.RtspMode)
		}
	} else {
		streamUrlReq := media.GetStreamUri{
			ProfileToken: profilesRes.Profiles[0].Token,
			StreamSetup: onvifcmd.StreamSetup{
				Stream: "RTP-Unicast",
				Transport: onvifcmd.Transport{
					Protocol: protocol,
				},
			},
		}
		streamUrlRes, err := sdkmedia.Call_GetStreamUri(context.Background(), dev, streamUrlReq)
		if err != nil {
			nazalog.Error(err)
			return
		}

		playUrl := buildPlayUrl(string(streamUrlRes.MediaUri.Uri), pullreq.Username, pullreq.Password)
		DoPull(playUrl, fmt.Sprintf("%s-%s", deviceInfoRes.Model, profilesRes.Profiles[0].Name), pullreq.RtspMode)
	}
}

func buildPlayUrl(rawurl, username, password string) string {
	if username != "" && password != "" {
		playUrl := fmt.Sprintf("rtsp://%s:%s@%s", username, password, strings.TrimLeft(rawurl, "rtsp://"))
		return playUrl
	}

	return rawurl
}

func DoPull(url, streamname string, rtspmod int) {
	request := base.ApiCtrlStartRelayPullReq{
		Url:                      url,
		StreamName:               streamname,
		RtspMode:                 rtspmod,
		AutoStopPullAfterNoOutMs: -1,
	}

	data, _ := json.Marshal(request)

	req, err := http.NewRequest("POST", "http://127.0.0.1:8083/api/ctrl/start_relay_pull", bytes.NewReader(data))
	if err != nil {
		return
	}

	req.Header.Set("Content-Type", "application/json")

	cli := &http.Client{
		Transport: http.DefaultTransport,
		Timeout:   time.Duration(5) * time.Second,
	}

	resp, err := cli.Do(req)
	if err != nil {
		return
	}

	if resp.StatusCode != 200 {
		return
	}

	resp.Body.Close()

	return
}
