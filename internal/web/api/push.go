package api

import (
	"easydarwin/internal/core/livestream"
	"easydarwin/internal/core/source"
	"easydarwin/internal/core/svr"
	"easydarwin/internal/data"
	"easydarwin/internal/gutils"
	"easydarwin/utils/pkg/web"
	"encoding/json"
	"fmt"
	"github.com/gin-gonic/gin"
	"io/ioutil"
	"log/slog"
	"net/http"
	"strconv"
	"strings"
	"time"
)

func GetUrl(url string) ([]byte, error) {
	client := http.Client{
		Timeout: 5 * time.Second,
	}
	req, err := http.NewRequest("GET", url, nil)
	if err != nil {
		return nil, fmt.Errorf("get url %s error : %s", url, err.Error())
	}

	resp, err := client.Do(req)
	if err != nil {
		return nil, fmt.Errorf("get do url %s error : %s", url, err.Error())
	}
	defer resp.Body.Close()
	body, err := ioutil.ReadAll(resp.Body)
	if err != nil {
		err = fmt.Errorf("get url %s readbody error %s", url, err.Error())
		return nil, err
	}
	return body, nil
}
func (l LiveStreamAPI) GetGroupsInfo(id int) ([]byte, error) {
	customizePubStreamName := fmt.Sprintf("%s%d", source.StreamName, id)
	v := svr.Lals.GetILalServer().StatGroup(customizePubStreamName)
	if v == nil {
		return []byte("{\"code\": 11001,\n\"msg\": \"group不存在\"\n}"), nil
	}
	body, err := json.Marshal(v)
	if err != nil {
		return nil, err
	}
	return body, err
}

func (l LiveStreamAPI) pubStart(c *gin.Context) {
	var onPubStart = &livestream.PubInfo{}
	err := c.Bind(onPubStart)
	if err != nil {
		slog.Error(fmt.Sprintf("pub start bind err:[%v]", err))
		return
	}
	var id int
	_, err = fmt.Sscanf(onPubStart.StreamName, "stream_%d", &id)
	if err != nil {
		slog.Error(fmt.Sprintf("pub start sscanf err:[%v]", err))
		return
	}

	live, err := source.LiveCore.FindInfoLiveStream(id)
	if err != nil {
		slog.Error(fmt.Sprintf("pub start find live name:[%s] err:[%v]", onPubStart.StreamName, err))
		return
	}
	if !live.Enable {
		slog.Error(fmt.Sprintf("pub start enable live out  id:[%d] name:[%s] Enable:[%v]", live.ID, onPubStart.StreamName, live.Enable))
		return
	}
	if live.Authed {
		if onPubStart.UrlParam == "" {
			slog.Error(fmt.Sprintf("pub start url param live out  id:[%d] name:[%s] UrlParam:[%v]", live.ID, onPubStart.StreamName, "fail"))
			return
		}
		countSplit := strings.Split(onPubStart.UrlParam, "=")
		sign := countSplit[1]
		if live.Sign != sign {
			slog.Error(fmt.Sprintf("pub start sign live out id:[%d] name:[%s] sign:[%v]", live.ID, onPubStart.StreamName, sign))
			return
		}
	}
	// todo: 前台播放跳过这个拦截
	if !live.OnDemand {

		err = source.LiveCore.UpdateLiveStreamInt(live.ID, "online", 1)
		if err != nil {
			slog.Error(fmt.Sprintf("pub start update session id live id:[%d] name:[%s] err:[%v]", live.ID, onPubStart.StreamName, err))
			return
		}
		err = source.LiveCore.UpdateLiveStreamString(live.ID, "session_id", onPubStart.SessionId)
		if err != nil {
			slog.Error(fmt.Sprintf("pub start update session id live id:[%d] name:[%s] err:[%v]", live.ID, onPubStart.StreamName, err))
			return
		}
		if time.Since(live.LastAt) > time.Second*15 {
			_, err = l.KickOutLive(onPubStart.StreamName, onPubStart.SessionId)
			if err != nil {
				slog.Error(fmt.Sprintf("pub start kick out live:[%s] err:[%v]", onPubStart.StreamName, err))
				return
			}
		} else {
			err = source.LiveCore.UpdateLiveStreamInt(live.ID, "online", 2)
			if err != nil {
				slog.Error(fmt.Sprintf("pub start update session id live id:[%d] name:[%s] err:[%v]", live.ID, onPubStart.StreamName, err))
				return
			}
		}

		return
	}

	err = source.LiveCore.UpdateLiveStreamString(live.ID, "session_id", onPubStart.SessionId)
	if err != nil {
		slog.Error(fmt.Sprintf("pub start update session id live id:[%d] name:[%s] err:[%v]", live.ID, onPubStart.StreamName, err))
		return
	}
	err = source.LiveCore.UpdateLiveStreamInt(live.ID, "online", 2)
	if err != nil {
		slog.Error(fmt.Sprintf("pub start update session id live id:[%d] name:[%s] err:[%v]", live.ID, onPubStart.StreamName, err))
		return
	}
}

func (l LiveStreamAPI) pubStop(c *gin.Context) {
	var onPubStart = &livestream.PubInfo{}
	err := c.Bind(onPubStart) //调用了前面代码块中封装的函数，自己封装的，不是库里的
	if err != nil {
		slog.Error(fmt.Sprintf("pub stop bind err:[%v]", err))
		return
	}
	var id int
	_, err = fmt.Sscanf(onPubStart.StreamName, "stream_%d", &id)
	if err != nil {
		slog.Error(fmt.Sprintf("pub stop sscanf err:[%v]", err))
		return
	}

	live, err := source.LiveCore.FindInfoLiveStream(id)
	if err != nil {
		slog.Error(fmt.Sprintf("pub stop find live name:[%s] err:[%v]", onPubStart.StreamName, err))
		return
	}
	err = source.LiveCore.UpdateLiveStreamString(live.ID, "session_id", "")
	if err != nil {
		slog.Error(fmt.Sprintf("pub stop update session id live id:[%d] name:[%s] err:[%v]", live.ID, onPubStart.StreamName, err))
		return
	}
	live, err = source.LiveCore.FindInfoLiveStream(id)
	if err != nil {
		slog.Error(fmt.Sprintf("pub start find live name:[%s] err:[%v]", onPubStart.StreamName, err))
		return
	}
	if live.OnDemand {
		err = source.LiveCore.UpdateLiveStreamInt(live.ID, "online", 0)
		if err != nil {
			slog.Error(fmt.Sprintf("pub stop update session id live id:[%d] name:[%s] err:[%v]", live.ID, onPubStart.StreamName, err))
			return
		}
	}
}

func (l LiveStreamAPI) StreamInfo(c *gin.Context) {
	id, err := strconv.Atoi(c.Param("id"))
	if err != nil {
		web.Fail(c, err)
		return
	}
	out, err := source.LiveCore.FindInfoLiveStream(id)
	if err != nil {
		// 如果查询失败，返回错误信息
		web.Fail(c, err)
		return
	}
	info, err := l.GetGroupsInfo(out.ID)
	if err != nil {
		web.Fail(c, err)
		return
	}
	var result map[string]interface{}

	// 将JSON字符串转换为map
	err = json.Unmarshal(info, &result)
	if err != nil {
		web.Fail(c, err)
		return
	}

	// 返回查询结果
	web.Success(c, result)
}
func (l LiveStreamAPI) playStop(c *gin.Context) {
	id, err := strconv.Atoi(c.Param("id"))
	if err != nil {
		web.Fail(c, err)
		return
	}
	out, err := source.LiveCore.FindInfoLiveStream(id)
	if err != nil {
		// 如果查询失败，返回错误信息
		web.Fail(c, err)
		return
	}
	if out.LiveType == livestream.LIVE_PUSH {
		web.Success(c, gin.H{"url": "", "id": out.ID})
		return
	}
	err = source.StopStream(out)
	if err != nil {
		web.Fail(c, web.ErrBadRequest.Msg("停流失败"))
		return
	}
	// 返回查询结果
	web.Success(c, gin.H{"url": "", "id": out.ID})
}

func (l LiveStreamAPI) delete(c *gin.Context) {
	id, err := strconv.Atoi(c.Param("id"))
	if err != nil {
		web.Fail(c, err)
		return
	}
	live, err := source.LiveCore.FindInfoLiveStream(id)
	if err != nil {
		web.Fail(c, err)
		return
	}
	if live.LiveType == livestream.LIVE_PULL {
		if !live.Enable && live.SessionId != "" {
			slog.Error(fmt.Sprintf("pub start sign live out id:[%d] name:[%s] err:[%v]", live.ID, live.SessionId, err))
			web.Fail(c, web.ErrBadRequest.Msg("删除推流失败"))
			return
		}
	}
	if live.LiveType == livestream.LIVE_PULL {
		err = source.StopStream(live)
		if err != nil {
			web.Fail(c, web.ErrBadRequest.Msg("删除流失败"))
			return
		}
		source.DelStreamClient(id)
	}
	err = source.LiveCore.DeleteLiveStream(id)
	if err != nil {
		web.Fail(c, err)
		return
	}
	web.Success(c, gin.H{
		"id": id,
	})
}

func PostUrl(url string, jsonStr string) ([]byte, error) {
	client := http.Client{
		Timeout: 5 * time.Second,
	}
	reqBody := strings.NewReader(jsonStr)
	resp, err := client.Post(url, "application/json", reqBody)
	if err != nil {
		return nil, fmt.Errorf("post url %s error : %s", url, err.Error())
	}

	defer resp.Body.Close()
	body, err := ioutil.ReadAll(resp.Body)
	if err != nil {
		err = fmt.Errorf("post url %s readbody error %s", url, err.Error())
		return nil, err
	}
	return body, nil
}

// 踢出直播流
func (l LiveStreamAPI) KickOutLive(name, id string) (bool, error) {
	httpPort := data.GetConfig().DefaultHttpConfig.HttpListenAddr
	url := fmt.Sprintf("http://127.0.0.1%s%s", httpPort, livestream.KICK_OUT_API)
	kickSess := livestream.OutSession{
		StreamName: name,
		SessionID:  id,
	}
	jsonStr, _ := json.Marshal(kickSess)

	body, err := PostUrl(url, string(jsonStr))
	if err != nil {
		return false, err
	}

	resp := livestream.OutResponse{}
	err = json.Unmarshal(body, &resp)
	fmt.Println(resp)
	if err != nil {
		err = fmt.Errorf("post url %s unmarshal json error %s", url, err.Error())
		return false, err
	}

	if resp.ErrorCode == 0 {
		return true, nil
	}
	err = fmt.Errorf("%s", resp.Desp)
	return false, err
}

func (l LiveStreamAPI) createPush(c *gin.Context) {
	var input livestream.PushInput
	// 将请求的JSON数据绑定到input变量上
	if err := c.ShouldBindJSON(&input); err != nil {
		// 如果绑定失败，返回错误信息
		web.Fail(c, web.ErrBadRequest.With(
			web.HanddleJSONErr(err).Error(),
			fmt.Sprintf("请检查请求类型 %s", c.GetHeader("content-type"))),
		)
		return
	}
	_, err := source.LiveCore.CreatePushStream(input)
	if err != nil {
		web.Fail(c, err)
		return
	}
	web.Success(c, gin.H{
		"name": input.Name,
	})
}

func (l LiveStreamAPI) updatePush(c *gin.Context) {
	id, err := strconv.Atoi(c.Param("id"))
	if err != nil {
		web.Fail(c, err)
		return
	}
	var input livestream.PushInput
	// 将请求的JSON数据绑定到input变量上
	if err := c.ShouldBindJSON(&input); err != nil {
		// 如果绑定失败，返回错误信息
		web.Fail(c, web.ErrBadRequest.With(
			web.HanddleJSONErr(err).Error(),
			fmt.Sprintf("请检查请求类型 %s", c.GetHeader("content-type"))),
		)
		return
	}

	err = source.LiveCore.UpdatePushStream(input, id)
	if err != nil {
		web.Fail(c, err)
		return
	}
	live, err := source.LiveCore.FindInfoLiveStream(id)
	if err != nil {
		web.Fail(c, err)
		return
	}
	if !live.Enable && live.SessionId != "" {
		_, errs := l.KickOutLive(fmt.Sprintf("stream_%d", live.ID), live.SessionId)
		if errs != nil {
			slog.Error(fmt.Sprintf("pub start sign live out id:[%d] name:[%s] err:[%v]", live.ID, live.SessionId, err))
			web.Fail(c, web.ErrBadRequest.Msg("更新推流停止失败"))
			return
		}
	}
	web.Success(c, gin.H{
		"name": input.Name,
	})
}

func (l LiveStreamAPI) updateOnePush(c *gin.Context) {
	id, err := strconv.Atoi(c.Param("id"))
	if err != nil {
		web.Fail(c, err)
		return
	}
	value, err := strconv.Atoi(c.Param("value"))
	if err != nil {
		web.Fail(c, err)
		return
	}
	key := ""
	switch c.Param("type") {
	case "enable":
		key = "enable"
		var live livestream.LiveStream
		live, err = source.LiveCore.FindInfoLiveStream(id)
		if err != nil {
			web.Fail(c, err)
			return
		}
		if value == 0 && live.SessionId != "" {
			_, errs := l.KickOutLive(fmt.Sprintf("stream_%d", live.ID), live.SessionId)
			if errs != nil {
				slog.Error(fmt.Sprintf("pub start sign live out id:[%d] name:[%s] err:[%v]", live.ID, live.SessionId, err))
				web.Fail(c, web.ErrBadRequest.Msg("关闭推流停止失败"))
				return
			}
		}
		err = source.LiveCore.UpdateLiveStreamInt(id, key, value)
		if err != nil {
			web.Fail(c, web.ErrBadRequest.Msg("更新enable失败"))
			return
		}
	case "onDemand":
		key = "on_demand"
		var live livestream.LiveStream
		live, err = source.LiveCore.FindInfoLiveStream(id)
		if err != nil {
			web.Fail(c, err)
			return
		}
		if value == 1 {
			live.OnDemand = true
		} else {
			live.OnDemand = false
		}
		err = source.LiveCore.UpdateLiveStreamInt(id, key, value)
		if err != nil {
			web.Fail(c, web.ErrBadRequest.Msg("更新enable失败"))
			return
		}
		if value == 0 && live.SessionId != "" && live.Authed {
			_, errs := l.KickOutLive(fmt.Sprintf("stream_%d", live.ID), live.SessionId)
			if errs != nil {
				slog.Error(fmt.Sprintf("pub start sign live out id:[%d] name:[%s] err:[%v]", live.ID, live.SessionId, err))
				web.Fail(c, web.ErrBadRequest.Msg("关闭推流停止失败"))
				return
			}
		}
	case "sign":
		key = "sign"
		var live livestream.LiveStream
		live, err = source.LiveCore.FindInfoLiveStream(id)
		if err != nil {
			web.Fail(c, err)
			return
		}
		sign := gutils.GenerateRandomString(10)
		err = source.LiveCore.UpdateLiveStreamString(live.ID, "sign", sign)
		if err != nil {
			web.Fail(c, web.ErrBadRequest.Msg("更新sign失败"))
			return
		}
		if value == 0 && live.SessionId != "" && live.Authed {
			_, errs := l.KickOutLive(fmt.Sprintf("stream_%d", live.ID), live.SessionId)
			if errs != nil {
				slog.Error(fmt.Sprintf("pub start sign live out id:[%d] name:[%s] err:[%v]", live.ID, live.SessionId, err))
				web.Fail(c, web.ErrBadRequest.Msg("关闭推流停止失败"))
				return
			}
		}
	default:
		web.Fail(c, web.ErrBadRequest.Msg("更新失败"))
		return
	}

	web.Success(c, gin.H{
		"id": id,
	})
}

func (l LiveStreamAPI) pubRtmpConnect(c *gin.Context) {
	c.IndentedJSON(200, "OK")
}
