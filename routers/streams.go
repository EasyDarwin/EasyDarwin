package routers

import (
	"bytes"
	"fmt"
	"github.com/EasyDarwin/EasyDarwin/rtsp"
	"github.com/gin-gonic/gin"
	"github.com/penggy/EasyGoLib/utils"
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
)

func (h *APIHandler) StreamStart(c *gin.Context) {
	type Form struct {
		URL         string `form:"url" binding:"required"`
		IdleTimeout int    `form:"idleTimeout"`
	}
	var form Form
	err := c.Bind(&form)
	if err != nil {
		log.Printf("Pull to push err:%v", err)
		return
	}
	client := rtsp.NewRTSPClient(rtsp.GetServer(), form.URL, 0)
	pusher := rtsp.NewClientPusher(client)
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

func (h *APIHandler) RecordFolders(c *gin.Context) {
	mp4Path := utils.Conf().Section("rtsp").Key("mp4_dir_path").MustString("")
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
	mp4Path := utils.Conf().Section("rtsp").Key("mp4_dir_path").MustString("")
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
					"name":           info.Name(),
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
