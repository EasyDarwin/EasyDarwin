package routers

import (
	"bytes"
	"fmt"
	"log"
	"math"
	"net/http"
	"os"
	"os/exec"
	"path/filepath"
	"regexp"
	"strconv"
	"strings"
	"time"

	"github.com/EasyDarwin/EasyDarwin/rtsp"
	"github.com/gin-gonic/gin"
	"github.com/penggy/EasyGoLib/utils"
)

/**
 * @apiDefine pull 拉流转推
 */

/**
 * @api {get} /api/vi/stream/start 启动拉流
 * @apiGroup pull
 * @apiName StreamStart
 * @apiParam {String} url RTSP源地址
 * @apiParam {String} [customPath] 转推时的推送PATH
 * @apiParam {Number} [IdleTimeout] 拉流时的超时时间
 * @apiParam {Number} [HeartbeatInterval] 拉流时的心跳间隔，毫秒为单位。如果心跳间隔不为0，那拉流时会向源地址以该间隔发送OPTION请求用来心跳保活
 * @apiSuccess (200) {String} ID	拉流的ID。后续可以通过该ID来停止拉流
 */
func (h *APIHandler) StreamStart(c *gin.Context) {
	type Form struct {
		URL               string `form:"url" binding:"required"`
		CustomPath        string `form:"customPath"`
		IdleTimeout       int    `form:"idleTimeout"`
		HeartbeatInterval int    `form:"heartbeatInterval"`
	}
	var form Form
	err := c.Bind(&form)
	if err != nil {
		log.Printf("Pull to push err:%v", err)
		return
	}
	client, err := rtsp.NewRTSPClient(rtsp.GetServer(), form.URL, int64(form.HeartbeatInterval)*1000)
	if err != nil {
		c.AbortWithStatusJSON(http.StatusBadRequest, err.Error())
		return
	}
	if form.CustomPath != "" && !strings.HasPrefix(form.CustomPath, "/") {
		form.CustomPath = "/" + form.CustomPath
	}
	client.CustomPath = form.CustomPath

	pusher := rtsp.NewClientPusher(client)
	if rtsp.GetServer().GetPusher(pusher.Path()) != nil {
		c.AbortWithStatusJSON(http.StatusBadRequest, fmt.Sprintf("Path %s already exists", client.Path))
		return
	}
	err = client.Start(time.Duration(form.IdleTimeout) * time.Second)
	if err != nil {
		log.Printf("Pull stream err :%v", err)
		c.AbortWithStatusJSON(http.StatusBadRequest, fmt.Sprintf("Pull stream err: %v", err))
		return
	}
	log.Printf("Pull to push %v success ", form)
	rtsp.GetServer().AddPusher(pusher)
	c.IndentedJSON(200, pusher.ID())
}

/**
 * @api {get} /api/vi/stream/stop 停止拉流
 * @apiGroup pull
 * @apiName StreamStop
 * @apiParam {String} id 拉流的ID
 * @apiUse simpleSuccess
 */
func (h *APIHandler) StreamStop(c *gin.Context) {
	type Form struct {
		ID string `form:"id" binding:"required"`
	}
	var form Form
	err := c.Bind(&form)
	if err != nil {
		log.Printf("stop pull to push err:%v", err)
		return
	}
	pushers := rtsp.GetServer().GetPushers()
	for _, v := range pushers {
		if v.ID() == form.ID {
			v.Stop()
			c.IndentedJSON(200, "OK")
			log.Printf("Stop %v success ", v)
			return
		}
	}
	c.AbortWithStatusJSON(http.StatusBadRequest, fmt.Sprintf("Pusher[%s] not found", form.ID))
}

/**
 * @apiDefine record 录像 Record
 */

/**
 * @apiDefine fileInfo
 * @apiSuccess (200) {String} duration	格式化好的录像时长
 * @apiSuccess (200) {Number} durationMillis	录像时长，毫秒为单位
 * @apiSuccess (200) {String} path 录像文件的相对路径,其绝对路径为：http[s]://host:port/record/[path]。
 * @apiSuccess (200) {String} folder 录像文件夹，录像文件夹以推流路径命名。
 */

/**
 * @api {get} /api/vi/record/folders 获取所有录像文件夹
 * @apiGroup record
 * @apiName RecordFolders
 * @apiParam {Number} [start] 分页开始,从零开始
 * @apiParam {Number} [limit] 分页大小
 * @apiParam {String} [sort] 排序字段
 * @apiParam {String=ascending,descending} [order] 排序顺序
 * @apiParam {String} [q] 查询参数
 * @apiSuccess (200) {Number} total 总数
 * @apiSuccess (200) {Array} rows 文件夹列表
 * @apiSuccess (200) {String} rows.folder	录像文件夹名称
 */
func (h *APIHandler) RecordFolders(c *gin.Context) {
	mp4Path := utils.Conf().Section("rtsp").Key("m3u8_dir_path").MustString("")
	form := utils.NewPageForm()
	if err := c.Bind(form); err != nil {
		log.Printf("record folder bind err:%v", err)
		return
	}
	var files = make([]interface{}, 0)
	if mp4Path != "" {
		visit := func(files *[]interface{}) filepath.WalkFunc {
			return func(path string, info os.FileInfo, err error) error {
				if err != nil {
					return err
				}
				if path == mp4Path {
					return nil
				}
				if !info.IsDir() {
					return nil
				}
				*files = append(*files, map[string]interface{}{"folder": info.Name()})
				return filepath.SkipDir
			}
		}
		err := filepath.Walk(mp4Path, visit(&files))
		if err != nil {
			log.Printf("Query RecordFolders err:%v", err)
		}
	}
	pr := utils.NewPageResult(files)
	if form.Sort != "" {
		pr.Sort(form.Sort, form.Order)
	}
	pr.Slice(form.Start, form.Limit)
	c.IndentedJSON(200, pr)

}

/**
 * @api {get} /api/vi/record/files 获取所有录像文件
 * @apiGroup record
 * @apiName RecordFiles
 * @apiParam {Number} folder 录像文件所在的文件夹
 * @apiParam {Number} [start] 分页开始,从零开始
 * @apiParam {Number} [limit] 分页大小
 * @apiParam {String} [sort] 排序字段
 * @apiParam {String=ascending,descending} [order] 排序顺序
 * @apiParam {String} [q] 查询参数
 * @apiSuccess (200) {Number} total 总数
 * @apiSuccess (200) {Array} rows 文件列表
 * @apiSuccess (200) {String} rows.duration	格式化好的录像时长
 * @apiSuccess (200) {Number} rows.durationMillis	录像时长，毫秒为单位
 * @apiSuccess (200) {String} rows.path 录像文件的相对路径,录像文件为m3u8格式，将其放到video标签中便可直接播放。其绝对路径为：http[s]://host:port/record/[path]。
 */
func (h *APIHandler) RecordFiles(c *gin.Context) {
	type Form struct {
		utils.PageForm
		Folder  string `form:"folder" binding:"required"`
		StartAt int    `form:"beginUTCSecond"`
		StopAt  int    `form:"endUTCSecond"`
	}
	var form = Form{}
	form.Limit = math.MaxUint32
	err := c.Bind(&form)
	if err != nil {
		log.Printf("record file bind err:%v", err)
		return
	}

	files := make([]interface{}, 0)
	mp4Path := utils.Conf().Section("rtsp").Key("m3u8_dir_path").MustString("")
	if mp4Path != "" {
		ffmpeg_path := utils.Conf().Section("rtsp").Key("ffmpeg_path").MustString("")
		ffmpeg_folder, executable := filepath.Split(ffmpeg_path)
		split := strings.Split(executable, ".")
		suffix := ""
		if len(split) > 1 {
			suffix = split[1]
		}
		ffprobe := ffmpeg_folder + "ffprobe" + suffix
		folder := filepath.Join(mp4Path, form.Folder)
		visit := func(files *[]interface{}) filepath.WalkFunc {
			return func(path string, info os.FileInfo, err error) error {
				if err != nil {
					return err
				}
				if path == folder {
					return nil
				}
				if info.IsDir() {
					return nil
				}
				if info.Size() == 0 {
					return nil
				}
				if info.Name() == ".DS_Store" {
					return nil
				}
				if !strings.HasSuffix(info.Name(), ".m3u8") {
					return filepath.SkipDir
				}
				cmd := exec.Command(ffprobe, "-i", path)
				cmdOutput := &bytes.Buffer{}
				//cmd.Stdout = cmdOutput
				cmd.Stderr = cmdOutput
				err = cmd.Run()
				bytes := cmdOutput.Bytes()
				output := string(bytes)
				//log.Printf("%v result:%v", cmd, output)
				var average = regexp.MustCompile(`Duration: ((\d+):(\d+):(\d+).(\d+))`)
				result := average.FindStringSubmatch(output)
				duration := time.Duration(0)
				durationStr := ""
				if len(result) > 0 {
					durationStr = result[1]
					h, _ := strconv.Atoi(result[2])
					duration += time.Duration(h) * time.Hour
					m, _ := strconv.Atoi(result[3])
					duration += time.Duration(m) * time.Minute
					s, _ := strconv.Atoi(result[4])
					duration += time.Duration(s) * time.Second
					millis, _ := strconv.Atoi(result[5])
					duration += time.Duration(millis) * time.Millisecond
				}

				*files = append(*files, map[string]interface{}{
					"path":           path[len(mp4Path):],
					"durationMillis": duration / time.Millisecond,
					"duration":       durationStr})
				return nil
			}
		}
		err = filepath.Walk(folder, visit(&files))
		if err != nil {
			log.Printf("Query RecordFolders err:%v", err)
		}
	}

	pr := utils.NewPageResult(files)
	if form.Sort != "" {
		pr.Sort(form.Sort, form.Order)
	}
	pr.Slice(form.Start, form.Limit)
	c.IndentedJSON(200, pr)
}
