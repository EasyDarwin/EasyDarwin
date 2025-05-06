package api

import (
	"easydarwin/internal/core/video"
	"easydarwin/internal/data"
	"easydarwin/internal/gutils"
	"easydarwin/internal/gutils/consts"
	"easydarwin/internal/gutils/efile"
	"easydarwin/internal/gutils/estring"
	"easydarwin/internal/gutils/etime"
	"fmt"
	"github.com/gin-gonic/gin"
	melody "gopkg.in/olahol/melody.v1"
	"log"
	"log/slog"
	"net/http"
	"net/url"
	"os"
	"path/filepath"
	"regexp"
	"strings"
	"sync"
	"time"
)

type VODRouter struct {
	ws        *melody.Melody
	TransChan chan *video.TVod
}

type IDForm struct {
	ID string `form:"id" binding:"required"`
}

type IDSForm struct {
	IDS []string `form:"ids[]"`
}

var (
	gVodRouter = &VODRouter{
		ws:        melody.New(),
		TransChan: make(chan *video.TVod, 200),
	}
)

const (
	ACCEPT = ".mp3,.wav,.mp4,.mpg,.mpeg,.wmv,.avi,.rmvb,.mkv,.flv,.mov,.3gpp,.3gp,.webm,.m4v,.mng,.vob"
)

// AbortWithString 终止
func AbortWithString(c *gin.Context, status int, msg string) {
	c.String(status, msg)
	c.Abort()
	return
}

// CodeWithMsg 返回对应的数据
func CodeWithMsg(c *gin.Context, code int, msg string) {
	c.AbortWithStatusJSON(code, gin.H{"code": code, "msg": msg})
	return
}

// SuccessWithMsg 成功调用返回
func SuccessWithMsg(c *gin.Context, msg string) {
	c.AbortWithStatusJSON(http.StatusOK, gin.H{"code": http.StatusOK, "msg": msg})
	return
}

// DefaultValue post or get key value
func DefaultValue(c *gin.Context, key string, defaultValue string) string {
	if c.Request.Method == "POST" {
		return c.DefaultPostForm(key, defaultValue)
	}
	return c.DefaultQuery(key, defaultValue)
}

type PageForm struct {
	Start uint   `form:"start" binding:"gte=0"`
	Limit uint   `form:"limit" binding:"gte=0"`
	Q     string `form:"q"`
	Sort  string `form:"sort"`
	Order string `form:"order"`
}

func NewPageForm() *PageForm {
	return &PageForm{
		Start: 0,
		Limit: 10,
	}
}

// 根据 vodID 获取直播服务
func GetVod(vodID string) (*video.TVod, error) {
	vod := &video.TVod{}
	err := data.GetDatabase().First(vod, consts.SqlWhereID, vodID).Error

	return vod, err
}

// 更新点播服务的所有数据库字段，返回更新后的数据
func UpdateVod(vod *video.TVod) (*video.TVod, error) {
	err := data.GetDatabase().Save(vod).Error

	if err != nil {
		return nil, err
	}

	return GetVod(vod.ID)
}

// Success 成功调用返回
func Success(c *gin.Context) {
	c.AbortWithStatusJSON(http.StatusOK, gin.H{"code": http.StatusOK, "msg": consts.MsgSuccess})
	return
}

func initVod() {

	gutils.Go(func() {
		db := data.GetDatabase()

		wg := sync.WaitGroup{}
		currNumber := 0

		for lgvod := range gVodRouter.TransChan {

			wg.Add(1)
			currNumber++
			go func(vod *video.TVod) {

				defer wg.Done()

				_progress := -1
				vod.Status = consts.VodStatusTransing
				fileTsFolder := efile.GetRealPath(filepath.Join(gCfg.VodConfig.Dir, vod.Folder))
				// 如果 ts 目录和视频目录相等，直接跳过
				if fileTsFolder == gCfg.VodConfig.Dir || fileTsFolder == gCfg.VodConfig.SrcDir {
					return
				}

				db.Save(vod)
				os.RemoveAll(fileTsFolder)
				slog.Info("remove init : ", fileTsFolder)

				if vod.RealPath == consts.EmptyString {
					vod.RealPath = filepath.Join(gCfg.VodConfig.SrcDir, vod.Path)
				}

				VODSnap(vod.RealPath, DefaultSnapTime, DefaultSnapDest(fileTsFolder))

				VODTrans(vod, func(progress int) {
					if _progress == progress {
						return
					}
					//save progress in memory
					video.TransProgress.Set(vod.ID, progress)

					if progress == 100 {
						vod.Status = consts.VodStatusDone
						//trans done
						video.TransProgress.Delete(vod.ID)
						db.Save(vod)
					}
					vodProgressNotify(vod.ID, progress)
					_progress = progress
				})
				if vod.Status != consts.VodStatusDone {
					video.TransProgress.Delete(vod.ID)
					vod.Status = consts.VodStatusError
					db.Save(vod)
				}

			}(lgvod)

			if currNumber == int(gCfg.VodConfig.SysTranNumber) {
				wg.Wait()
				currNumber = 0
			}
		}
	})

	onceVODWaitingProc()
}

func onceVODWaitingProc() {
	gutils.Go(func() {
		// 停止 3s，防止 gVodRouter.TransChan  还没有被监听,导致缺少转码
		time.Sleep(3 * time.Second)

		var vods []video.TVod
		data.GetDatabase().Where("status = ?", "waiting").Or("status = ?", "transing").Find(&vods)
		for _, vod := range vods {
			//go func(video video.TVod) {
			//	defer func() {
			//		if err := recover(); err != nil {
			//			gErrorLogger.Error(fmt.Sprintf("panic %s\n", err))
			//			gErrorLogger.Error(fmt.Sprint(string(debug.Stack())))
			//		}
			//	}()

			lvod := vod

			gVodRouter.TransChan <- &lvod
			//}(video)
		}
	})
}

/**
 * @apiDefine vodInfo
 * @apiSuccess (200) {String} rows.id
 * @apiSuccess (200) {String} rows.name 名称
 * @apiSuccess (200) {String} rows.size 文件大小
 * @apiSuccess (200) {String} rows.type 文件类型
 * @apiSuccess (200) {String=transing,waiting,done,error} rows.status 转码状态:(转码中-transing、等待转码-waiting、转码完成-done、转码失败-error)
 * @apiSuccess (200) {String} rows.duration 时长
 * @apiSuccess (200) {String} rows.videoCodec 视频编码
 * @apiSuccess (200) {String} rows.audioCodec 音频编码
 * @apiSuccess (200) {String} rows.aspect 宽高
 * @apiSuccess (200) {String} rows.error 错误信息
 * @apiSuccess (200) {String} rows.sharedLink 分享链接
 * @apiSuccess (200) {String} rows.snapUrl 封面图片链接
 * @apiSuccess (200) {String} rows.videoUrl 点播播放链接
 * @apiSuccess (200) {Integer} rows.playNum 点播次数
 * @apiSuccess (200) {Integer} rows.flowNum 点播总流量(B)
 */

func (r *VODRouter) BroadcastProgress(msg []byte) {
	r.ws.Broadcast(msg)
}

func (r *VODRouter) WSProgress(c *gin.Context) {
	r.ws.HandleRequest(c.Writer, c.Request)
}
func (r *VODRouter) accept(c *gin.Context) {
	c.AbortWithStatusJSON(http.StatusOK, ACCEPT)
}
func (r *VODRouter) uploadoptions(c *gin.Context) {
	c.Header("access-control-allow-headers", "Content-Type")
	c.Header("access-control-allow-methods", "OPTIONS, POST")
	c.Header("access-control-allow-origin", "*")
	c.Status(http.StatusOK)
}

/**
 * @api {post} /vod/upload 01 上传点播文件
 * @apiGroup 02vod
 * @apiParam {File} file 上传文件
 * @apiParam {String} [path] 上传子目录
 * @apiParam {String} [resolution] 多清晰度转码 如 yh,hfd,hd,sd  其中yh:原始视频(必须包含)，hfd:超清，hd:高清，sd:标清
 * @apiUse vodInfo
 * @apiUse timeInfo
 */
func (r *VODRouter) upload(c *gin.Context) {
	file, _ := c.FormFile("file")
	ext := filepath.Ext(file.Filename)
	reg := regexp.MustCompile("(?i)(" + strings.Join(strings.Split(ACCEPT, ","), "|") + ")$")
	if !reg.Match([]byte(ext)) {
		AbortWithString(c, http.StatusBadRequest, "not accept")
		return
	}
	name := filepath.Base(file.Filename)[:strings.LastIndex(filepath.Base(file.Filename), ".")]
	contentType := ""
	if f, err := file.Open(); err == nil {
		defer f.Close()
		buffer := make([]byte, 512)
		if _, err := f.Read(buffer); err == nil {
			contentType = http.DetectContentType(buffer)
			if contentType == "application/octet-stream" && len(ext) > 1 {
				contentType = "video/" + ext[1:]
			}
		}
	}
	resolution := DefaultValue(c, "resolution", consts.EmptyString)
	if resolution != consts.EmptyString && !strings.Contains(resolution, consts.DefinitionYH) {
		AbortWithString(c, http.StatusBadRequest, "resolution must contain yh")
		return
	}
	if resolution == consts.EmptyString {
		resolution = gCfg.VodConfig.TransDefinition
	}
	resolution = sortTransDefinition(resolution)
	vod := video.TVod{
		Resolution: resolution,
		Name:       name,
		Size:       int(file.Size),
		Type:       contentType, Status: consts.VodStatusWaiting,
	}
	db := data.GetDatabase()
	vodSrcDir := gCfg.VodConfig.SrcDir

	vod.ID = estring.ShortID()

	folder := DefaultValue(c, "path", consts.EmptyString)
	efile.EnsureDir(filepath.Join(vodSrcDir, folder))

	folder = estring.FormatPath(filepath.Join(folder, vod.ID))
	if strings.HasPrefix(folder, "/") || strings.HasSuffix(folder, "/") {
		AbortWithString(c, http.StatusBadRequest, "子目录格式错误")
		return
	}
	vod.Path = fmt.Sprintf("%s%s", folder, ext)
	vod.Folder = folder

	db.Create(&vod)

	dest := filepath.Join(vodSrcDir, fmt.Sprintf("%s%s", folder, ext))
	if err := efile.SaveUploadedFile(file, dest); err != nil {
		slog.Error("上传文件失败", "err", err)
		c.AbortWithStatusJSON(http.StatusInternalServerError, err.Error())
		return
	}
	info, _ := Info(dest, false)
	vod.Duration = info.Duration
	switch info.VideoDecodec {
	case "h265":
		vod.VideoCodec = consts.VideoH265
	case "h264":
		vod.VideoCodec = consts.VideoH264
	case "hevc":
		vod.VideoCodec = consts.VideoHevc
	case "vp9":
		vod.VideoCodec = consts.VideoVp9
	case "vp8":
		vod.VideoCodec = consts.VideoVp8
	case "mpeg4":
		vod.VideoCodec = consts.VideoMpeg4
	default:
		vod.VideoCodec = info.VideoDecodec
	}
	switch info.AudioDecodec {
	case "aac":
		vod.AudioCodec = consts.AudioAac
	case "mp3":
		vod.AudioCodec = consts.AudioMp3
	case "opus":
		vod.AudioCodec = consts.AudioOpus
	default:
		vod.AudioCodec = info.AudioDecodec
	}
	vod.Aspect = info.Aspect
	vod.Rotate = info.Rotate
	vod.VidioCodecOriginal = vod.VideoCodec
	vod.AudioCodecOriginal = vod.AudioCodec

	vod.RealPath = dest
	if !strings.Contains(info.Aspect, "x") ||
		strings.HasSuffix(strings.ToLower(dest), ".mp3") ||
		strings.HasSuffix(strings.ToLower(dest), ".wav") {
		vod.Resolution = consts.EmptyString
	}

	video.TransProgress.Set(vod.ID, 0)
	gutils.Go(func() {
		r.TransChan <- &vod
	})
	var ids []string
	ids = append(ids, vod.ID)

	c.AbortWithStatusJSON(http.StatusOK, newVODRow(c, vod))
}

func sortTransDefinition(definition string) string {
	if !gCfg.VodConfig.OpenDefinition {
		return consts.EmptyString
	}
	df := []string{}
	for _, s := range []string{"yh", "fhd", "hd", "sd"} {
		if strings.Contains(definition, s) {
			df = append(df, s)
		}
	}
	return strings.Join(df, consts.SplitComma)
}

/**
 * @api {get|post} /vod/list 02 点播列表
 * @apiGroup 02vod
 * @apiUse pageParam
 * @apiParam {String} [folder] 子文件夹
 * @apiUse pageSuccess
 * @apiUse vodInfo
 * @apiUse timeInfo
 */
func (r *VODRouter) list(c *gin.Context) {
	type formdata struct {
		Start uint   `form:"start" binding:"gte=0"`
		Limit uint   `form:"limit" binding:"gte=0"`
		Q     string `form:"q"`
		Sort  string `form:"sort"`
		Order string `form:"order"`
		Foler string `form:"folder"`
	}
	form := &formdata{Start: 0, Limit: 10}
	if err := c.Bind(form); err != nil {
		slog.Error("解析参数失败", "err", err)
		return
	}
	db := data.GetDatabase().Model(&video.TVod{})
	var total int64

	if form.Q != "" {
		db = db.Where("name like ?", "%"+form.Q+"%").Or("id like ?", "%"+form.Q+"%")
	}

	//get Subcatalog
	if form.Foler != consts.EmptyString {
		switch form.Foler {
		case "full":
		case "other":
			db = db.Where("folder not like ?", "%/%").Where("folder not like ?", "%\\%")
		default:
			db = db.Where("folder like ?", form.Foler+"/%")
		}
	}
	db.Count(&total)
	if form.Sort != "" {
		db = db.Order(fmt.Sprintf("%s %s", "sort", strings.TrimSuffix(form.Order, "ending")))
	}
	var rows []video.TVod

	db.Limit(int(form.Limit)).Offset(int(form.Start)).Find(&rows)
	list := make([]video.VodView, len(rows))

	for i, row := range rows {
		list[i] = newVODRow(c, row)
	}
	result := gin.H{
		"total": total,
		"rows":  list,
	}

	c.AbortWithStatusJSON(http.StatusOK, result)
}

/**
 * @api {post} /vod/sharelist 03 获取分享点播列表
 * @apiGroup 02vod
 * @apiParam {String} [folder] 子文件夹
 * @apiUse pageParam
 * @apiUse pageSuccess
 * @apiUse vodInfo
 * @apiUse timeInfo
 */
func (r *VODRouter) sharelist(c *gin.Context) {
	type formdata struct {
		Start uint   `form:"start" binding:"gte=0"`
		Limit uint   `form:"limit" binding:"gte=0"`
		Q     string `form:"q"`
		Sort  string `form:"sort"`
		Order string `form:"order"`
		Foler string `form:"folder"`
	}
	form := &formdata{Start: 0, Limit: 10}
	if err := c.Bind(form); err != nil {
		AbortWithString(c, http.StatusBadRequest, consts.MsgErrorBadRequest)
		return
	}
	if !gCfg.VodConfig.OpenSquare {
		c.AbortWithStatus(http.StatusBadRequest)
		return
	}
	db := data.GetDatabase().Model(&video.TVod{})
	db = db.Where("shared = ?", true)

	if form.Q != "" {
		db = db.Where("name like ?", "%"+form.Q+"%")
	}

	var total int64
	db.Count(&total)

	if form.Sort != "" {
		db = db.Order(fmt.Sprintf("%s %s", "sort", strings.TrimSuffix(form.Order, "ending")))
	}
	var rows []video.TVod
	db.Limit(int(form.Limit)).Offset(int(form.Start)).Find(&rows)
	list := make([]video.VodView, len(rows))

	for i, row := range rows {
		list[i] = newVODRow(c, row)
	}
	c.AbortWithStatusJSON(http.StatusOK, gin.H{
		"total": total,
		"rows":  list,
	})
}

/**
 * @api {post} /vod/get 04 获取单条点播信息
 * @apiGroup 02vod
 * @apiParam {String} id
 * @apiUse vodInfo
 * @apiUse timeInfo
 */
func (r *VODRouter) get(c *gin.Context) {
	var form IDForm
	if err := c.Bind(&form); err != nil {
		return
	}
	var vod video.TVod
	data.GetDatabase().First(&vod, consts.SqlWhereID, form.ID)
	if vod.ID == "" {
		AbortWithString(c, http.StatusBadRequest, "未查找到对应的视频")
		return
	}

	c.AbortWithStatusJSON(http.StatusOK, newVODRow(c, vod))
}

/**
 * @api {post} /vod/save 05 编辑点播
 * @apiGroup 02vod
 * @apiParam {String} id
 * @apiParam {String} name 名称
 * @apiParam {Boolean} shared 鉴权开关
 * @apiUse simpleSuccess
 */
func (r *VODRouter) save(c *gin.Context) {
	type formdata struct {
		ID             string `form:"id" binding:"required"`
		Name           string `form:"name" binding:"required"`
		Shared         bool   `form:"shared"`
		ShareBeginTime string `form:"shareBeginTime"`
		ShareEndTime   string `form:"shareEndTime"`
	}
	form := formdata{
		Shared: false,
	}
	if err := c.Bind(&form); err != nil {
		c.AbortWithStatus(http.StatusBadRequest)
		return
	}

	vod, err := GetVod(form.ID)
	if err != nil {
		AbortWithString(c, http.StatusBadRequest, "未查找到对应的视频")
		return
	}

	vod.Name = form.Name
	vod.Shared = form.Shared
	vod.ShareBeginTime = etime.StrToDateTime(form.ShareBeginTime)
	vod.ShareEndTime = etime.StrToDateTime(form.ShareEndTime)

	UpdateVod(vod)

	Success(c)
}

/**
 * @api {post} /vod/snap 06 设置点播封面
 * @apiGroup 02vod
 * @apiParam {String} id
 * @apiParam {String} time 时间, HH:mm:ss
 * @apiParam {File} cover 封面
 * @apiUse simpleSuccess
 */
func (r *VODRouter) snap(c *gin.Context) {
	type formdata struct {
		ID   string `form:"id" binding:"required"`
		Time string `form:"time"`
	}
	var form formdata
	if err := c.Bind(&form); err != nil {
		c.AbortWithStatus(http.StatusBadRequest)
		return
	}

	var vod video.TVod
	data.GetDatabase().First(&vod, "id = ?", form.ID)
	if vod.ID == "" {
		c.AbortWithStatus(http.StatusNotFound)
		return
	}

	fileSrcPath := filepath.Join(gCfg.VodConfig.SrcDir, vod.Path)
	fileTsPath := filepath.Join(gCfg.VodConfig.Dir, vod.Folder)
	if form.Time != "" {
		VODSnap(fileSrcPath, form.Time, DefaultSnapDest(fileTsPath))
	}

	// 读取 cover 字段，必须是文件
	file, _ := c.FormFile("cover")
	if file != nil {
		ext := filepath.Ext(file.Filename)
		srcPic := filepath.Join(gCfg.VodConfig.SrcDir, fmt.Sprintf("%s%s", form.ID, ext))
		if err := c.SaveUploadedFile(file, srcPic); err != nil {
			slog.Error("上传封面文件失败", "err", err)
			c.AbortWithStatusJSON(http.StatusInternalServerError, err.Error())
			return
		}

		destPic := filepath.Join(fileTsPath, consts.VodCover)
		if err := VodCoverSnap(srcPic, destPic); err != nil {
			slog.Error("转换为封面失败", "err", err)
			c.AbortWithStatusJSON(http.StatusInternalServerError, err.Error())
			return
		}
	}

	Success(c)
}

/**
 * @api {post} /vod/turn/shared 07 分享开关
 * @apiGroup 02vod
 * @apiParam {String} id
 * @apiParam {Boolean} shared
 * @apiUse simpleSuccess
 */
func (r *VODRouter) shared(c *gin.Context) {
	type formdata struct {
		ID     string `form:"id"`
		Shared bool   `form:"shared"`
	}
	var form formdata
	if err := c.Bind(&form); err != nil {
		c.AbortWithStatus(http.StatusBadRequest)
		return
	}
	data.GetDatabase().Model(video.TVod{}).Where("id = ?", form.ID).Update("shared", form.Shared)
	Success(c)
}

/**
 * @api {post} /vod/remove 08 删除点播
 * @apiGroup 02vod
 * @apiParam {String} id
 * @apiUse simpleSuccess
 */
func (r *VODRouter) remove(c *gin.Context) {
	form := IDForm{}
	if err := c.Bind(&form); err != nil {
		c.AbortWithStatus(http.StatusBadRequest)
		return
	}
	db := data.GetDatabase()

	var vod video.TVod
	db.First(&vod, consts.SqlWhereID, form.ID)
	if vod.ID != "" {
		if vod.Status == consts.VodStatusTransing {
			AbortWithString(c, http.StatusBadRequest, fmt.Sprintf("%s 正在转码", vod.Name))
			return
		}

		conf := gCfg.VodConfig
		// 必须添加 efile.GetRealPath 才能对比。要不然 video.Folder 为空，会导致整个最外层文件夹被删除
		tsFolder := efile.GetRealPath(filepath.Join(conf.Dir, vod.Folder))
		srcFile := efile.GetRealPath(filepath.Join(conf.SrcDir, vod.Path))

		if tsFolder != conf.Dir && srcFile != conf.SrcDir {
			slog.Info("remove tsFole : ", tsFolder)
			slog.Info("remove srcFile : ", srcFile)
			os.RemoveAll(tsFolder)
			os.RemoveAll(srcFile)
		}

		db.Delete(vod)
	}
	Success(c)
}

/**
 * @api {post} /vod/removeBatch 09 批量删除点播文件
 * @apiGroup 02vod
 * @apiParam {Array} ids
 * @apiUse simpleSuccess
 */
func (r *VODRouter) removeBatch(c *gin.Context) {
	form := IDSForm{}
	if err := c.Bind(&form); err != nil {
		c.AbortWithStatus(http.StatusBadRequest)
		return
	}
	var vods []video.TVod
	db := data.GetDatabase()

	tx := db.Begin()
	tx.Find(&vods, consts.SqlWhereIDIn, form.IDS)
	for _, vod := range vods {
		if vod.Status == consts.VodStatusTransing {
			tx.Rollback()
			AbortWithString(c, http.StatusBadRequest, fmt.Sprintf("%s 正在转码", vod.Name))
			return
		}

		tx.Delete(vod)
	}
	tx.Commit()

	conf := gCfg.VodConfig
	for _, vod := range vods {
		tsFolder := efile.GetRealPath(filepath.Join(conf.Dir, vod.Folder))
		srcFile := efile.GetRealPath(filepath.Join(conf.SrcDir, vod.Path))
		if tsFolder != conf.Dir && srcFile != conf.SrcDir {
			slog.Info("remove batch video : ", tsFolder)
			slog.Info("remove batch video : ", srcFile)
			os.RemoveAll(tsFolder)
			os.RemoveAll(srcFile)
		} else {
			CodeWithMsg(c, http.StatusInternalServerError, "内部错误，删除失败。")
			return
		}
	}
	Success(c)
}

/**
 * @api {get} /vod/download/:id 10 下载点播文件
 * @apiGroup 02vod
 * @apiParam {String} id 点播ID
 * @apiUse simpleSuccess
 */
func (r *VODRouter) download(c *gin.Context) {
	id := c.Param("id")
	var vod video.TVod
	data.GetDatabase().First(&vod, consts.SqlWhereID, id)
	if vod.ID == consts.EmptyString {
		c.AbortWithStatus(http.StatusNotFound)
		return
	}
	var dest string
	if strings.HasSuffix(strings.ToLower(vod.Path), ".mp3") ||
		strings.HasSuffix(strings.ToLower(vod.Path), ".wav") ||
		strings.Contains(strings.ToLower(vod.Type), "audio") {
		if efile.Exisit(filepath.Join(gCfg.VodConfig.Dir, vod.Path)) {
			dest = filepath.Join(gCfg.VodConfig.Dir, vod.Path)
		}
	}
	if dest == consts.EmptyString && vod.Folder != consts.EmptyString {
		dest = M3U8ToMP4(filepath.Join(gCfg.VodConfig.Dir, vod.Folder, "video.m3u8"))
		defer os.RemoveAll(dest)
		slog.Info("remove download : ", dest)
	}
	filename := strings.TrimSuffix(vod.Name, filepath.Ext(vod.Name)) + filepath.Ext(dest)
	header := c.Writer.Header()
	header["Content-type"] = []string{"application/octet-stream"}
	header["Content-Disposition"] = []string{"attachment; filename=" + filename}
	c.File(dest)
}

/**
 * @api {get|post} /vod/progress 11 获取正在转码进度
 * @apiGroup 02vod
 * @apiParam {String} [id] 点播ID
 * @apiSuccess (200) {String} id 点播ID
 * @apiSuccess (200) {String} progress 进度, [0-100]
 * @apiSuccessExample 成功
 * [{"id":"ioehMD8iR","progress":89}]
 */
type progress struct {
	ID       string `json:"id"`
	Progress int    `json:"progress"`
}

func (r *VODRouter) progress(c *gin.Context) {
	type formdata struct {
		ID string `form:"id"`
	}
	form := &formdata{}
	if err := c.Bind(form); err != nil {
		AbortWithString(c, http.StatusBadRequest, consts.MsgErrorBadRequest)
		return
	}

	result := []progress{}
	if form.ID != consts.EmptyString {
		if v, OK := video.TransProgress.Get(form.ID); OK {
			result = append(result, progress{
				ID:       form.ID,
				Progress: v})
		}
	} else {
		for _, k := range video.TransProgress.Keys() {
			if v, OK := video.TransProgress.Get(k); OK {
				result = append(result, progress{
					ID:       k,
					Progress: v})
			}
		}
	}
	c.AbortWithStatusJSON(http.StatusOK, result)
	return
}

func vodProgressNotify(id string, progress int) {
	gutils.Go(func() {
		if gCfg.VodConfig.ProgressNotifyURL != consts.EmptyString {
			v := url.Values{}
			v.Add("id", id)
			v.Add("progress", fmt.Sprintf("%v", progress))
			url := fmt.Sprintf(`%s?%s`, gCfg.VodConfig.ProgressNotifyURL, v.Encode())

			client := &http.Client{}
			client.Timeout = 5 * time.Second
			resp, err := client.Get(url)
			if err != nil {
				log.Printf("video notify error %v", err)
			}
			if resp == nil || resp.StatusCode != http.StatusOK {
				slog.Info("video notify status code not 200")
			}
		}
	})
}

/**
 * @api {post} /vod/retran 19 重新转码点播文件
 * @apiGroup 02vod
 * @apiParam {String} id
 * @apiUse simpleSuccess
 */
func (r *VODRouter) retran(c *gin.Context) {
	form := IDForm{}
	if err := c.Bind(&form); err != nil {
		c.AbortWithStatus(http.StatusBadRequest)
		return
	}

	// 查找数据库
	vod := video.TVod{}
	err := data.GetDatabase().Where(consts.SqlWhereID, form.ID).First(&vod).Error
	if err != nil {
		CodeWithMsg(c, http.StatusInternalServerError, err.Error())
		return
	}
	if vod.ID != form.ID {
		CodeWithMsg(c, http.StatusBadRequest, "未查找到对应 id 视频文件")
		return
	}

	// 找到数据库数据，设置为转码
	video.TransProgress.Set(vod.ID, 0)
	gutils.Go(func() {
		r.TransChan <- &vod
	})

	Success(c)
}
