package cloud

import (
	"bytes"
	"context"
	"fmt"
	"io"
	"log/slog"
	"net/http"
	"net/url"
	"os"
	"path/filepath"
	"runtime"
	"slices"
	"strconv"
	"strings"
	"time"

	"github.com/goccy/go-json"

	"easydarwin/lnton/pkg/finder"
	"easydarwin/lnton/pkg/fn"
	"easydarwin/lnton/pkg/orm"
	"easydarwin/lnton/pkg/system"
	m3u8mannager "easydarwin/lnton/rms/pkg/m3u8"
	"github.com/aws/aws-sdk-go/aws"
	"github.com/aws/aws-sdk-go/aws/credentials"
	"github.com/aws/aws-sdk-go/aws/session"
	"github.com/aws/aws-sdk-go/service/s3"
	"github.com/sourcegraph/conc/pool"
)

type DeviceParamsInput struct {
	DeviceID  string
	ChannelID string
	PrefixDir string
	SMSURL    string // 支持 sms 生成访问前缀，如 http://127.0.0.1:28080
}

// FindTimelineOutput 对文件名做处理，取开始时间戳和持续时间
// /records/deviceID/channelID/yyyyMMDD/timestamp-duration.ts
type FindTimelineOutput struct {
	Start    int64 `json:"start"`    // 开始毫秒
	Duration int64 `json:"duration"` // 持续毫秒
}

type CloudRecorder interface {
	// GetRecordByMonth 获取某个月哪些天存在录像
	GetRecordByMonth(in DeviceParamsInput, yyyymm string) (string, error) // 返回指定月份，哪些天存在录像，例如 010101001 ;0:没有;1:存在
	// GetM3u8 根据开始时间和结束时间获取m3u8文件
	GetM3u8(in DeviceParamsInput, startMs, endMs int64, expire time.Duration) (string, []string, error) // 返回 m3u8 文本内容
	// FindTimeline 获取事件轴
	FindTimeline(in DeviceParamsInput, startMs, endMs int64) ([]FindTimelineOutput, error) // 返回加载时间轴的数据
	// M3u8Handler
	// MoveFiles 移植文件，当更改过期变化时，需要移植文件到指定的目录下
	MoveFiles(in DeviceParamsInput, newPrefix string) error
	// DownloadFiles 下载视频，由客户端下载
	// DownloadFiles(in DeviceParamsInput, startMs, endMs int64, w io.Writer) error
	// DownloadFiles2(m3u8 string, startMs, endMs int64, w io.Writer) error // 分析 m3u8 文件，下载所有录像(应禁止调用)
}

type M3u8Handler interface {
	// CutM3u8 截取 m3u8 ，重新生成包含符合时间区间的 ts ，返回该 m3u8 文本内容
	CutM3u8(m3u8 string, startMs, endMs int64) (string, error)
	// SetDirExpireTime 设置指定目录，例如 /records/deviceID/channelID，3 天后过期，则之前还是未来上传的文件，只有 3 天的生命周期，然后自动删除
	// SetDirExpireTime(in DeviceParamsInput, expireDays int) error
	// WriteM3u8ToRedis 写入 redis，key 命名规范:    <deviceID>:<channelID>:<date>:m3u8
	// WriteM3u8ToRedis(in DeviceParamsInput, re *redis.Client, m3u8 string, startMs, endMs int64) error
}

var _ CloudRecorder = (*S3Mannager)(nil)

type S3Mannager struct {
	s3     *s3.S3
	bucket string

	PuffAddr string
}

var CloudRecord = finder.NewEngine(filepath.Join(system.Getwd(), "temporary", "cloud_record_download"), 8*time.Hour)

// DownloadFiles2 implements CloudRecorder.
// func DownloadFiles2(m3u8 string, startMs int64, endMs int64, fn func(total, current int64, id, err string)) error {
// 	if startMs > endMs {
// 		startMs, endMs = endMs, startMs
// 	}
// 	if endMs-startMs > 3*24*60*60*1000 {
// 		slog.Warn("下载时间超过3天，禁止下载")
// 		return "", errors.New("下载时间超过3天，禁止下载")
// 	}
// 	// 解析带http协议标识和参数的m3u8
// 	links, err := GetLinksByM3u8(m3u8)
// 	if err != nil {
// 		return "", err
// 	}

// 	var isMP4 bool
// 	var isTs bool
// 	for _, link := range links {
// 		if strings.Contains(link, ".mp4") {
// 			isMP4 = true
// 		} else {
// 			isTs = true
// 		}

// 		if isMP4 && isTs {
// 			return "", fmt.Errorf("MP4/TS 混合流禁止下载")
// 		}
// 	}

// 	if isTs {
// 		for _, link := range links {
// 			if err := getFileByLink(link, w); err != nil {
// 				slog.Error("下载文件失败", "link", link, "err", err)
// 				break
// 			}
// 		}
// 		return "", nil
// 	}

// 	// mp4 的逻辑
// 	// 1. 文件全部下载过来
// 	// 2. 合并成最终 mp4
// 	// 3. 无论是超时还是怎样，自动清理
// 	// 4. 任务完成后，3 天不操作自动清理
// 	dir := fmt.Sprintf("%d_%s", time.Now().UnixMicro(), orm.GenerateRandomString(6))

// 	defer func() {
// 		for i := range links {
// 			path := filepath.Join(cloudRecord.Prefix(), fmt.Sprintf("%s/%d.m4s", dir, i))
// 			os.RemoveAll(path)
// 		}
// 	}()

// 	// 合并文件
// 	outName := fmt.Sprintf("%s/%d_%d_out.mp4", dir, startMs, endMs)
// 	outFile, err := cloudRecord.MkdirAll(dir).CreateFile(outName)
// 	if err != nil {
// 		return "", err
// 	}
// 	fmp4 := NewFmp4(outFile)
// 	for i, link := range links {
// 		file, err := cloudRecord.CreateFile(fmt.Sprintf("%s/%d.m4s", dir, i))
// 		if err != nil {
// 			return "", err
// 		}
// 		if err := getFileByLink(link, file); err != nil {
// 			return "", err
// 		}
// 		if err := fmp4.ProcessMP4(file); err != nil {
// 			return "", err
// 		}
// 		file.Close()
// 	}
// 	return filepath.Join(cloudRecord.Prefix(), outName), nil
// }

// DownloadFiles2 implements CloudRecorder.
func DownloadFiles2(m3u8 string, startMs int64, endMs int64, callback func(total, current int64, id, err, link string)) error {
	if startMs > endMs {
		startMs, endMs = endMs, startMs
	}

	// 解析带http协议标识和参数的m3u8
	links, err := GetLinksByM3u8(m3u8)
	if err != nil {
		callback(100, 0, "0.1", err.Error(), "")
		return err
	}

	var isMP4 bool
	var isTs bool
	for _, link := range links {
		if strings.Contains(link, ".mp4") {
			isMP4 = true
		} else {
			isTs = true
		}

		if isMP4 && isTs {
			callback(100, 0, "0.2", "MP4/TS 混合流禁止下载", "")
			return fmt.Errorf("MP4/TS 混合流禁止下载")
		}
	}

	if isTs {
		callback(100, 0, "0.3", "TS 不支持下载", "")
		return fmt.Errorf("TS 不支持下载")
	}

	// mp4 的逻辑
	// 1. 文件全部下载过来
	// 2. 合并成最终 mp4
	// 3. 无论是超时还是怎样，自动清理
	// 4. 任务完成后，3 天不操作自动清理
	dir := fmt.Sprintf("%d_%s", time.Now().UnixMicro(), orm.GenerateRandomString(6))

	defer func() {
		for i := range links {
			path := filepath.Join(CloudRecord.Prefix(), fmt.Sprintf("%s/%d.m4s", dir, i))
			os.RemoveAll(path)
		}
	}()

	if len(links) <= 0 {
		callback(100, 0, "0.4", "此时间段没有录像", "")
		return fmt.Errorf("此时间段没有录像")
	}

	// 合并文件
	outName := fmt.Sprintf("%s/%d_%d_out.mp4", dir, startMs, endMs)
	outFile, err := CloudRecord.MkdirAll(dir).CreateFile(outName)
	if err != nil {
		callback(100, 0, "0.5", err.Error(), "")
		return err
	}
	fmp4 := NewFmp4(outFile)

	total := len(links) + 1
	var current int
	for i, link := range links {
		current = i
		file, err := CloudRecord.CreateFile(fmt.Sprintf("%s/%d.m4s", dir, i))
		if err != nil {
			callback(int64(total), int64(current), "", err.Error(), "")
			return err
		}
		if err := getFileByLink(link, file); err != nil {
			callback(int64(total), int64(current), "", err.Error(), "")
			return err
		}
		if err := fmp4.ProcessMP4(file); err != nil {
			callback(int64(total), int64(current), "", err.Error(), "")
			return err
		}
		file.Close()
		callback(int64(total), int64(current), "", "", "")
	}

	callback(int64(total), int64(total), "", "", outName)
	return nil
}

var cli = http.Client{Timeout: 10 * time.Minute}

// getFileByLink 通过连接获取文件内容并写入io.Writer中
//
// 函数的抽离为了防止在for循环中进行defer出现问题
func getFileByLink(link string, w io.Writer) error {
	resp, err := cli.Get(link)
	if err != nil {
		return err
	}
	defer resp.Body.Close()
	_, err = io.Copy(w, resp.Body)
	if err != nil {
		if err == io.EOF {
			return nil
		}
		return fmt.Errorf("写入出错: %s", err)
	}
	return nil
}

const (
	Enable  = "Enabled"  // 启用
	Disable = "Disabled" // 禁用
)

// SetBucketLifecycleConfiguration 设置云存录像的生命周期规则
func (s *S3Mannager) SetBucketLifecycleConfiguration(id string, status string, prefix string, expirationDay int64) error {
	_, err := s.s3.PutBucketLifecycleConfiguration(&s3.PutBucketLifecycleConfigurationInput{
		Bucket: aws.String(s.bucket), // 设置生命周期规则的存储桶
		LifecycleConfiguration: &s3.BucketLifecycleConfiguration{
			Rules: []*s3.LifecycleRule{
				{
					ID:     aws.String(id),     // 规则ID
					Status: aws.String(status), // 必须是Enable或者Disable
					// Status: s3.ObjectLockEnabledEnabled,
					Filter: &s3.LifecycleRuleFilter{
						Prefix: aws.String(prefix), // 指定文件夹（前缀）
					},
					Expiration: &s3.LifecycleExpiration{
						Days: aws.Int64(expirationDay), // 过期后30天删除文件
					},
				},
			},
		},
	})
	if err != nil {
		return err
	}
	return nil
}

func (s *S3Mannager) GetGetBucketLifecycleConfiguration() ([]*s3.LifecycleRule, error) {
	configuration, err := s.s3.GetBucketLifecycleConfiguration(&s3.GetBucketLifecycleConfigurationInput{
		Bucket: aws.String(s.bucket),
	})
	if err != nil {
		return nil, err
	}
	return configuration.Rules, nil

}

// DownloadFiles implements CloudRecorder.
func (s *S3Mannager) DownloadFiles(in DeviceParamsInput, startMs int64, endMs int64, w io.Writer) error {
	if startMs > endMs {
		startMs, endMs = endMs, startMs
	}
	// 获取指定时间范围内的ts切片
	tsList, err := s.getTsListByDay(in, startMs, endMs)
	if err != nil {
		return err
	}

	for _, v := range tsList {
		out, err := s.s3.GetObject(&s3.GetObjectInput{
			Bucket: aws.String(s.bucket),
			Key:    aws.String(v),
		})
		if err != nil {
			continue
		}
		_, err = io.Copy(w, out.Body)
		if err == io.EOF {
			continue
		}
		if err != nil {
			return fmt.Errorf("写入流失败")
		}
	}

	return nil
	// auth, err := s.SetAuthToken(tsList, s.bucket)
	// m3u8, err := m3u8mannager.GeneranM3u8(auth)
	// if err != nil {
	//	return err
	// }
	// return string(m3u8), nil
	//
	//
	// for _, key := range list {
	//	out, err := s.s3.GetObject(&s3.GetObjectInput{Key: key})
	//	_, err := io.Copy(w, out.Body)
	//	if err == io.EOF {
	//		continue
	//	}
	//	if err != nil {
	//		return fmt.Errorf("sadasdasd ")
	//	}
	//
	// }
	// panic("unimplemented")
}

// MoveFiles implements CloudRecorder.
func (s *S3Mannager) MoveFiles(in DeviceParamsInput, newPrefix string) error {
	prefix := fmt.Sprintf("%s/%s/%s", in.PrefixDir, in.DeviceID, in.ChannelID)

	input := &s3.ListObjectsInput{
		Bucket:  aws.String(s.bucket),
		Prefix:  aws.String(prefix),
		MaxKeys: aws.Int64(1000),
	}
	// 分页参考: https://github.com/aws/aws-sdk-php/issues/1428

	p := pool.New().WithMaxGoroutines(runtime.NumCPU())
	for {
		out, err := s.s3.ListObjects(input)
		if err != nil {
			return err
		}
		contents := out.Contents
		p.Go(func() {
			s.moveFiles(contents, in.PrefixDir, newPrefix)
		})
		if !*out.IsTruncated {
			break
		}
		input.Marker = out.NextMarker
	}
	p.Wait()

	// 防止在迁移过程中，又上传了文件，再查找迁移一次
	time.Sleep(time.Second * 10)
	out, err := s.s3.ListObjects(input)
	if err == nil && len(out.Contents) > 0 {
		s.moveFiles(out.Contents, in.PrefixDir, newPrefix)
	}
	return nil
}

func (s *S3Mannager) moveFiles(contents []*s3.Object, prefix, newPrefix string) {
	for _, content := range contents {
		key := *content.Key
		if !fn.Any([]string{".ts", ".m3u8", "mp4", "m4s"}, func(s string) bool {
			return strings.EqualFold(s, filepath.Ext(key))
		}) {
			continue
		}
		newKey := filepath.Join(newPrefix, strings.TrimPrefix(key, prefix))
		slog.Debug("移动s3文件", "source", key, "target", newKey, "prefix", prefix, "new_prefix", newPrefix)
		// 迁移文件
		if _, err := s.s3.CopyObject(&s3.CopyObjectInput{
			Bucket:     aws.String(s.bucket),
			CopySource: aws.String(s.bucket + "/" + key),
			Key:        aws.String(newKey),
		}); err != nil {
			slog.Error("CopyObject", "err", err)
		} else {
			// 删除文件
			if _, err := s.s3.DeleteObject(&s3.DeleteObjectInput{
				Bucket: aws.String(s.bucket),
				Key:    aws.String(*content.Key),
			}); err != nil {
				slog.Error("DeleteObject", "err", err)
			}
		}
	}
}

type Config struct {
	AccessKey string
	SecretKey string
	Region    string
	EndPoint  string
	Bucker    string

	PuffAddr string // ip:port   此参数不为空串时，应当走本地存储，而非 s3 协议
}

// FindTimeline 返回加载时间轴的数据
func (s *S3Mannager) FindTimeline(in DeviceParamsInput, startMs, endMs int64) ([]FindTimelineOutput, error) {
	// var timeStampLen = 13
	// var findTimelineOutput []FindTimelineOutput
	// 判断起始时间和结束时间关系，进行纠正
	if startMs > endMs {
		startMs, endMs = endMs, startMs
	}
	findTimelineOutput := make([]FindTimelineOutput, 0, 8)
	// 获取指定时间范围内的ts切片
	tsList, err := s.getTsListByDay(in, startMs, endMs)
	if err != nil {
		return nil, err
	}

	// 查询所有文件列表
	for _, v := range tsList {
		ext := [...]string{".ts", ".m3u8", ".mp4", ".m4s"}
		if !slices.Contains(ext[:], filepath.Ext(v)) {
			continue
		}

		// 获取文件名
		filename := filepath.Base(v)

		var timestamp, dura int64
		// 通过“-”进行分割，获取起始时间
		arr := strings.Split(filename, "-")
		if len(arr) != 2 {
			slog.Error("分割文件出错", "err", fmt.Sprintf("len(arr) != 2 %s", filename))
			continue
		}

		// 解析开始时间
		timestamp, err := parseStart(arr[0])
		if err != nil {
			slog.Error("解析文件起始时间出错", "err", err)
			continue
		}

		// 解析持续时间
		arr = strings.Split(arr[1], ".")
		if len(arr) != 2 {
			slog.Error("分割文件出错", "err", fmt.Sprintf("len(arr) != 2 %s", filename))
			continue
		}
		dura, _ = strconv.ParseInt(arr[0], 10, 64)
		// 封装时间轴数据
		findTimelineOutput = append(findTimelineOutput, FindTimelineOutput{
			Start:    timestamp,
			Duration: dura,
		})
	}

	return findTimelineOutput, err
}

// parseStart 解析起始时间
//
// 该函数接收一个文件路径[string]，返回该文件路径的[毫秒级起始时间戳] [毫秒级持续间隔] [错误]
func parseStart(start string) (int64, error) {
	var timestamp int64
	// 解析文件起始时间
	switch len(start) {
	case 10: // 1584541242_3000.ts 解析[秒级时间戳]格式时间
		timestamp, _ = strconv.ParseInt(start, 10, 64)
		timestamp = timestamp * 1000

	case 13: // 1584541242000_3000.ts 解析[毫秒级时间戳]格式时间
		timestamp, _ = strconv.ParseInt(start, 10, 64)

	case 14: // 20240304231212_3000.ts 解析[秒级时间]格式
		startTime, err := time.ParseInLocation("20060102150405", start, time.Local)
		if err != nil {
			return 0, fmt.Errorf("起始时间格式错误:time = %s", start)
		}
		timestamp = startTime.UnixMilli()
	// case 17: //20240304231212000_3000.ts 解析[毫秒级时间]格式
	//	startTime, err := time.ParseInLocation("20060102150405000", arr[0], time.Local)
	//	if err != nil {
	//		slog.Error("解析文件起始时间出错", "err", fmt.Sprintf("time = %s", arr[0]))
	//	}
	//	timestamp = startTime.Unix()
	default:
		return 0, fmt.Errorf("起始时间格式错误:time = %s", start)
	}
	return timestamp, nil
}

func NewCore(config Config) *S3Mannager {
	return &S3Mannager{
		s3:       NewSession(config),
		bucket:   config.Bucker,
		PuffAddr: config.PuffAddr,
	}
}

func ModifyString(originalString string) string {
	parts := strings.Split(originalString, ".")
	if len(parts) > 0 {
		parts[0] += "-iam"
	}
	return strings.Join(parts, ".")
}

// addHTTPSPrefix 增加 https:// 前缀
// func addHTTPSPrefix(s string) string {
// 	return "https://" + strings.TrimLeft(strings.TrimLeft(s, "http://"), "https://")
// }

var httpClient = http.Client{
	Transport: &http.Transport{
		MaxIdleConns:        20,
		MaxIdleConnsPerHost: 20,
		MaxConnsPerHost:     50,
		IdleConnTimeout:     3 * time.Minute,
	},
	Timeout: time.Minute,
}

func NewSession(config Config) *s3.S3 {
	s3 := s3.New(session.Must(session.NewSession(&aws.Config{
		Endpoint:         aws.String(config.EndPoint),
		Region:           aws.String(config.Region),
		Credentials:      credentials.NewStaticCredentials(config.AccessKey, config.SecretKey, ""),
		S3ForcePathStyle: aws.Bool(true),
		HTTPClient:       &httpClient,
	})))
	return s3
}

func (s *S3Mannager) GetBuckets() (*s3.ListBucketsOutput, error) {
	buckets, err := s.s3.ListBuckets(&s3.ListBucketsInput{})
	if err != nil {
		return nil, err
	}
	return buckets, nil
}

func (s *S3Mannager) BucketFiles(bucketName string) (*s3.ListObjectsOutput, error) {
	params := &s3.ListObjectsInput{
		Bucket:  aws.String(bucketName),
		MaxKeys: aws.Int64(10),
	}

	return s.s3.ListObjects(params)
}

// func (s *S3Mannager) GetObjectKey(bucketName string) *s3.ListObjectsV2Output {
// 	input := &s3.ListObjectsV2Input{
// 		Bucket: aws.String(bucketName),
// 	}

// 	v2, err := s.s3.ListObjectsV2(input, func(options *s3.Options) {})
// 	if err != nil {
// 		return &s3.ListObjectsV2Output{}
// 	}
// 	for _, item := range v2.Contents {
// 		fmt.Println(*item.LastModified, *item.Key)
// 	}
// 	return v2
// }

func (s *S3Mannager) SetAuthToken(paths []string, bucketName string, expire time.Duration) (result []string, err error) {
	for _, v := range paths {
		if v == "" {
			slog.Error("SetAuthToken paths contains empty path")
			continue
		}
		var urlStr string
		if s.PuffAddr == "" {
			req, _ := s.s3.GetObjectRequest(&s3.GetObjectInput{
				Bucket: aws.String(bucketName),
				Key:    aws.String(v),
			})
			urlStr, err = req.Presign(expire)
			if err != nil {
				slog.Error("SetAuthToken", "err", err)
			}
		} else {
			urlStr = v
		}

		result = append(result, urlStr)
	}

	return result, nil
}

func (s *S3Mannager) QueryFileList(bucketName string, arg []string) (*s3.ListObjectsOutput, error) {
	var params string
	for _, v := range arg {
		params += v + "/"
	}
	return s.s3.ListObjects(&s3.ListObjectsInput{Bucket: aws.String(bucketName), Prefix: aws.String(params)})
}

// func (s *S3Mannager) CreateNewFoldWithExpericeTime(time string) error {
// 	return nil
// }

// func (s *S3Mannager) SetDirExpireTime(in DeviceParamsInput, expireDays int) error {
// 	dirURL := fmt.Sprintf("%s/%s/%s/", in.PrefixDir, in.DeviceID, in.ChannelID)
// 	if err := s.PutDirLifecycle(dirURL, expireDays); err != nil {
// 		slog.Error("设置对象声明周期失败！", err)
// 		return err
// 	}

// 	return nil
// }

// func (s *S3Mannager) UpdateObjectsPolicyByPrefix(objectKeys string, day int32) error {
// 	params := &s3.PutBucketLifecycleConfigurationInput{
// 		Bucket: aws.String(s.bucket),
// 		LifecycleConfiguration: &types.BucketLifecycleConfiguration{
// 			Rules: []types.LifecycleRule{
// 				{
// 					ID:         aws.String("Rule1"),
// 					Status:     types.ExpirationStatusEnabled,
// 					Filter:     &types.LifecycleRuleFilterMemberPrefix{Value: objectKeys},
// 					Expiration: &types.LifecycleExpiration{Days: day},
// 				},
// 			}},
// 	}

// 	_, err := s.s3.PutBucketLifecycleConfiguration(context.TODO(), params)
// 	if err != nil {
// 		slog.Error("更换策略失败！")
// 		return err
// 	}
// 	fmt.Println("更换策略成功！")
// 	return nil
// }

// GetFileByPrefix https://oos-xiongan-iam.ctyunapi.cn
// record-1d/306e5fbcb048c097a85b39835034b189/0/
//
//	获取天翼云符合条件的文件列表
// func (s *S3Mannager) GetFileByPrefix(prefix string) []string {
// 	bucket, err := s.tyIamClient.Bucket(s.bucket)
// 	if err != nil {
// 		slog.Error("获取天翼云bucket出错！")
// 		return []string{}
// 	}

// 	list := make([]string, 0)
// 	// 创建一个字符变量来接收标记
// 	var maker string
// 	for {
// 		res, err := bucket.ListObjects(oos.Prefix(prefix), oos.MaxKeys(1000), oos.Marker(maker))
// 		if err != nil {
// 			slog.Error("获取天翼云所有文件出错！", err)
// 			return list
// 		}
// 		for _, v := range res.Objects {
// 			list = append(list, v.Key)
// 		}
// 		if !res.IsTruncated {
// 			return list
// 		}
// 		// 将标记后移1000
// 		maker = res.NextMarker
// 	}
// }

// // PutDirLifecycle 设置生命周期
// func (s *S3Mannager) PutDirLifecycle(prefix string, days int) error {
// 	rule1 := oos.BuildLifecycleExpirRuleByDays("", prefix, true, days)
// 	s.tyIamClient.Config.IsEnableMD5 = true
// 	rules := []oos.LifecycleRule{rule1}
// 	// rule := oos.LifecycleRule{
// 	// 	ID:     "",
// 	// 	Prefix: prefix,
// 	// 	Status: "true",
// 	// 	Expiration: &oos.LifecycleExpiration{Days: days},
// 	// }
// 	// rules := []oos.LifecycleRule{rule}
// 	//
// 	// var lxml oos.LifecycleXML
// 	// lxml.Rules = rules
// 	// bs, err := xml.Marshal(lxml)
// 	// if err != nil {
// 	// 	return err
// 	// }
// 	// buffer := new(bytes.Buffer)
// 	// buffer.Write(bs)
// 	//
// 	// params := map[string]interface{}{}
// 	// params["lifecycle"] = nil
// 	//
// 	// headers := map[string]string{
// 	// 	"Content-Type": "application/xml",
// 	// }
// 	//
// 	err := s.tyIamClient.SetBucketLifecycle(s.bucket, rules)
// 	// _, err = s.tyIamClient.Conn.Do(string(oos.HTTPPut), s.bucket, prefix, params, headers, buffer, nil)
// 	if err != nil {
// 		slog.Error("设置生命周期策略失败！", err)
// 		return err
// 	}
// 	slog.Info("设置对象生命周期成功！")
// 	return nil
// }

// func (s *S3Mannager) GetDirLifecycle() {
// 	gbl, err := s.tyIamClient.GetBucketLifecycle(s.bucket)
// 	if err != nil {
// 		slog.Error("获取生命周期失败！", err)
// 		return
// 	}

// 	slog.Info("rules:", "info:", gbl.Rules)
// 	for _, v := range gbl.Rules {
// 		slog.Info("到期时间", slog.Int("id", v.Expiration.Days))
// 	}
// }

// func (s *S3Mannager) DeleteDirLifecycle() {
// 	err := s.tyIamClient.DeleteBucketLifecycle(s.bucket)
// 	if err != nil {
// 		slog.Error("err", err)
// 		return
// 	}
// }

var daysInMonth = map[string]int{
	"01": 31, "02": 28, "03": 31, "04": 30,
	"05": 31, "06": 30, "07": 31, "08": 31,
	"09": 30, "10": 31, "11": 30, "12": 31,
}

type RespRecordsMonthOutput struct {
	Data string `json:"data"`
}

// GetMonthRecord 查询指定月份的存储信息，返回000101010101....
func (s *S3Mannager) GetRecordByMonth(in DeviceParamsInput, yyyyMM string) (string, error) {
	if len(yyyyMM) != 6 {
		return "", fmt.Errorf("月份有误")
	}

	if s.PuffAddr != "" {
		// formData := url.Values{
		//	"stream_name": {in.DeviceID + "_" + in.ChannelID},
		//	"yyyymm":      {yyyyMM},
		// }

		// strings.NewReader(formData.Encode())
		params := url.Values{}
		params.Add("stream_name", in.DeviceID+"_"+in.ChannelID)
		params.Add("yyyymm", yyyyMM)
		req, err := http.NewRequest(http.MethodGet, fmt.Sprintf("%s/api/records/months?%s", s.PuffAddr, params.Encode()), nil)
		if err != nil {
			// fmt.Println("Error creating request:", err)
			return "", err
		}
		resp, err := httpClient.Do(req)
		if err != nil {
			return "", err
		}
		defer resp.Body.Close()

		var out RespRecordsMonthOutput
		if err := json.NewDecoder(resp.Body).Decode(&out); err != nil {
			return "", err
		}
		return out.Data, nil
	}

	// 存储各级目录参数
	path := make([]string, 0, 4)
	path = append(path, in.PrefixDir, in.DeviceID, in.ChannelID, yyyyMM)

	// 存储前缀匹配参数
	// 拼接要匹配的前缀
	params := strings.Join(path, "/")

	year, err := strconv.Atoi(yyyyMM[0:4])
	if err != nil {
		return "", err
	}
	// log.Println("年份", year)

	var binStr string
	// 获取当前月份天数
	month := daysInMonth[yyyyMM[4:6]]
	if isLeapYear(year) && yyyyMM[4:6] == "02" {
		month = 29
	}

	// 遍历当天月份所有天数
	for i := 1; i <= month; i++ {
		path := params
		if i < 10 {
			path += "0" + strconv.Itoa(i)
		} else {
			path += strconv.Itoa(i)
		}
		// 获取文件列表
		fileList, err := s.GetBuckPrefixFileByNum2(path)
		if err != nil {
			return "", err
		}
		if len(fileList) > 1 {
			binStr += "1"
			continue
		}
		binStr += "0"
	}
	return binStr, nil
}

// GetBuckPrefixFileByNum2 通过前缀获取2个文件列表
func (s *S3Mannager) GetBuckPrefixFileByNum2(prefix string) ([]string, error) {
	out, err := s.s3.ListObjects(&s3.ListObjectsInput{
		Bucket:  aws.String(s.bucket),
		Prefix:  aws.String(prefix),
		MaxKeys: aws.Int64(2),
	})
	if err != nil {
		return nil, err
	}
	list := make([]string, 0, 2)
	// 将对象转换为string切片
	for _, v := range out.Contents {
		list = append(list, *(v.Key))
	}
	return list, nil
}

func isLeapYear(year int) bool {
	return year%4 == 0 && (year%100 != 0 || year%400 == 0)
}

func (s *S3Mannager) GetM3u8(in DeviceParamsInput, startMs, endMs int64, expire time.Duration) (string, []string, error) {
	if startMs > endMs {
		startMs, endMs = endMs, startMs
	}
	// 获取指定时间范围内的ts切片
	tsList, err := s.getTsListByDay(in, startMs, endMs)
	if err != nil {
		return "", nil, err
	}
	// var auth []string
	// var m3u8 []byte
	// if s.PuffAddr == "" {
	auth, err := s.SetAuthToken(tsList, s.bucket, expire)
	if err != nil {
		return "", nil, err
	}

	// }
	m3u8, err := m3u8mannager.GeneranM3u8(auth)
	if err != nil {
		return "", nil, err
	}
	return string(m3u8), auth, nil
}

// getTsListByDay 获取指定时间范围内的ts切片
type RespRecordsOutput struct {
	Items []string `json:"items"`
}

// getTsListByDay 获取指定时间范围内的ts切片
func (s *S3Mannager) getTsListByDay(in DeviceParamsInput, startMs, endMs int64) ([]string, error) {
	if s.PuffAddr != "" {
		// formData := url.Values{
		//	"stream_name": {input.DeviceID + "_" + input.ChannelID},
		//	"start_ms":    {strconv.FormatInt(startMs, 10)},
		//	"end_ms":      {strconv.FormatInt(endMs, 10)},
		//	"expire":      {"6000"},
		//	"network":     {"WAN"},
		// }
		params := url.Values{}
		params.Add("stream_name", in.DeviceID+"_"+in.ChannelID)
		params.Add("start_ms", strconv.FormatInt(startMs, 10))
		params.Add("end_ms", strconv.FormatInt(endMs, 10))
		params.Add("expire", "6000")
		params.Add("network", "WAN")
		params.Add("url", in.SMSURL)
		req, err := http.NewRequest(http.MethodGet, fmt.Sprintf("%s/api/records?%s", s.PuffAddr, params.Encode()), nil)
		if err != nil {
			// fmt.Println("Error creating request:", err)
			return nil, err
		}
		resp, err := httpClient.Do(req)
		if err != nil {
			return nil, err
		}
		defer resp.Body.Close()

		var out RespRecordsOutput
		err = json.NewDecoder(resp.Body).Decode(&out)
		return out.Items, err
	}

	lists := make([]string, 0, 8)

	// 将时间戳转换为时间格式
	startTime := time.UnixMilli(startMs)
	endTime := time.UnixMilli(endMs)

	// 获取时间段内每天的切片
	dateRange := DateRange(startTime, endTime)

	// 存储各级目录参数
	pathDir := append([]string{}, in.PrefixDir, in.DeviceID, in.ChannelID)

	// // 获取30秒前的时间
	// pastTime := startTime.Add(-30 * time.Second)
	// //获取30秒前的日期
	// pastTimeStr := pastTime.Format("20060102")
	// // 判断录像是否跨天了，是则查询昨天的最后一段录像
	// if pastTimeStr != dateRange[0] {
	//	file, err := s.getYesterdayFile(startTime, pastTimeStr, pathDir, dateRange)
	//	if err != nil {
	//		return nil, err
	//	}
	//	lists = append(lists, file...)
	// }

	// 遍历切片内每天的文件
	for _, v := range dateRange {
		paths := pathDir
		paths = append(paths, v)

		prefixPathURL := strings.Join(paths, "/") + "/"

		out, err := s.FindPrefixFiles(prefixPathURL)
		if err != nil {
			return nil, err
		}
		flag := true
		// 筛选符合条件的
		for _, path := range out {
			if len(path) < len(prefixPathURL)+13 {
				continue
			}

			// 通过“-”进行分割，获取起始时间
			arr := strings.Split(path, "-")
			if len(arr) != 2 {
				slog.Error("分割文件出错", "err", fmt.Sprintf("len(arr) != 2 %s", path))
				continue
			}

			startString := arr[0][strings.LastIndex(arr[0], "/")+1:]
			timestamp, err := parseStart(startString)
			if err != nil {
				slog.Error("解析时间出错", "err", err)
				continue
			}

			// 获取跨天的ts，flag执行一次后会变成false
			// 假如放在外层执行不知道ts开始时间,要求开始时间为00:00:25秒时,会获取昨天一个ts
			// 实际00:00:06有ts,满足条件,导致两个ts都会被添加
			if flag {
				// 获取30秒前的时间
				pastTime := startTime.Add(-30 * time.Second)
				// //获取30秒前的日期
				pastTimeStr := pastTime.Format("20060102")
				// 判断录像是否跨天了，是则查询昨天的最后一段录像
				if pastTimeStr != dateRange[0] && startMs < timestamp {
					file, err := s.getYesterdayFile(startTime, pastTimeStr, pathDir, dateRange)
					if err != nil {
						return nil, err
					}
					lists = append(lists, file...)
				}
				flag = false
			}

			// 获取包含起始时间的ts切片
			// time, _ := strconv.Atoi(fileTime)
			if (timestamp+30000) > startMs && timestamp < startMs {
				lists = append(lists, path)
			}

			// 符合条件的ts切片
			if timestamp >= startMs && timestamp <= endMs {
				lists = append(lists, path)
			}
		}
	}
	return lists, nil
}

func (s *S3Mannager) getYesterdayFile(startTime time.Time, pastTimeStr string, path, dateRange []string) ([]string, error) {
	lists := make([]string, 0, 1)

	// 获取起始时间所在天的0时0分0秒时间
	t, err := time.ParseInLocation("20060102", startTime.Format("20060102"), time.Local)
	if err != nil {
		return nil, err
	}
	timesTampBasic := t.Unix()

	for i := 1; i < 4; i++ {
		timesTampBasic -= int64(10)
		str := strconv.Itoa(int(timesTampBasic))[0:9]
		paths := path
		paths = append(paths, pastTimeStr, str)
		PrefixPathURL := strings.Join(paths, "/")
		out, err := s.FindPrefixFile(PrefixPathURL)
		if err != nil {
			return nil, err
		}

		if len(out) < 1 {
			continue
		}
		// 拿获取到的最后一个
		lastFile := out[len(out)-1]

		if len(lastFile) < len(PrefixPathURL)+13 {
			continue
		}

		filename := filepath.Base(lastFile)
		var dura, timestamp int64
		{
			arr := strings.Split(filename, "-")
			if len(arr) != 2 {
				slog.Error("分割文件出错", "err", fmt.Sprintf("len(arr) != 2 %s", filename))
				continue
			}
			timestamp, _ = strconv.ParseInt(arr[0], 10, 64)
			{
				arr := strings.Split(arr[1], ".")
				if len(arr) != 2 {
					slog.Error("分割文件出错", "err", fmt.Sprintf("len(arr) != 2 %s", filename))
					continue
				}
				dura, _ = strconv.ParseInt(arr[0], 10, 64)
			}
		}

		if time.UnixMilli(timestamp+dura).Format("20060102") == dateRange[0] {
			lists = append(lists, lastFile)
		}
		break
	}
	return lists, nil
}

// CutM3u8 截取 m3u8 ，重新生成包含符合时间区间的 ts ，返回该 m3u8 文本内容
func (s *S3Mannager) CutM3u8(m3u8 string, startMs, endMs int64) (string, error) {
	// 给定m3u8文件、开始时间start、结束时间end三个参数，遍历m3u8返回包括start和end的最小m3u8文件
	// 例如开始时间为01:20，结束时间为02:00，返回的开始时间应该是小于01:20，大于02:00的最小时间段 ，1:10~02:20

	// 开始时间大于结束时间，交换内容
	if startMs > endMs {
		startMs, endMs = endMs, startMs
	}
	// 提取m3u8的ts列表
	tsList, err := GetTsListByM3u8(m3u8)
	if err != nil {
		return "", err
	}

	// 通过指定时间筛选符合条件的ts列表
	cutTsList, err := GetTsListByInterval(tsList, startMs, endMs)
	if err != nil {
		return "", err
	}

	// 生成m3u8文本
	cutM3u8, err := m3u8mannager.GeneranM3u8(cutTsList)
	if err != nil {
		return "", err
	}
	return string(cutM3u8), nil
}

// DateRange 返回给定日期范围内的日期切片
func DateRange(startDate, endDate time.Time) []string {
	var dateSlice []string
	if endDate.Before(startDate) {
		startDate, endDate = endDate, startDate
	}

	// 云存文件的位置是时按进行分割的
	// currentDate 是起始时间所在天的起始时间
	currentDate := time.Date(startDate.Year(), startDate.Month(), startDate.Day(), 0, 0, 0, 0, time.Local)
	for !currentDate.After(endDate) {
		stringcurrentDate := currentDate.Format("20060102")
		dateSlice = append(dateSlice, stringcurrentDate)
		currentDate = currentDate.AddDate(0, 0, 1)
	}

	return dateSlice
}

const RecordDefaultPrefix = "r"

func (c *S3Mannager) PutObject(ctx context.Context, in DeviceParamsInput, name string, body []byte) (string, error) {
	prefix := fmt.Sprintf("%s/%s/%s", in.PrefixDir, in.DeviceID, in.ChannelID)
	path := filepath.Join(prefix, name)
	_, err := c.s3.PutObjectWithContext(ctx, &s3.PutObjectInput{
		Bucket: aws.String(c.bucket),
		Key:    aws.String(path),
		Body:   bytes.NewReader(body),
	})
	if err != nil {
		return "", err
	}
	objectURL, err := url.JoinPath(c.s3.Endpoint, c.bucket, path)
	if err != nil {
		return "", err
	}
	return objectURL, err
}

// PutObjectWithContext 上传文件，弃用 PutObject
func (c *S3Mannager) PutObjectWithContext(ctx context.Context, in DeviceParamsInput, name string, body io.ReadSeeker) (string, error) {
	prefix := fmt.Sprintf("%s/%s/%s", in.PrefixDir, in.DeviceID, in.ChannelID)
	path := filepath.Join(prefix, name)

	// Windows操作系统中路径为"/",minIO标准的分隔符为"/",需要替换
	// 注意文件和文件夹名字中不允许有"\"会导致存储异常
	path = strings.Replace(path, "\\", "/", -1)
	_, err := c.s3.PutObjectWithContext(ctx, &s3.PutObjectInput{
		Bucket: aws.String(c.bucket),
		Key:    aws.String(path),
		Body:   body,
	})
	if err != nil {
		return "", err
	}
	objectURL, err := url.JoinPath(c.s3.Endpoint, c.bucket, path)
	if err != nil {
		return "", err
	}
	return objectURL, err
}

// WriteM3u8ToRedis 写入 redis，key 命名规范:    <deviceID>:<channelID>:<date>:m3u8
// func (s *S3Mannager) WriteM3u8ToRedis(in DeviceParamsInput, client *redis.Client, m3u8 string, startMs, endMs int64) error { // 将时间戳转换为时间格式
// 	startTime := time.Unix(startMs/1000, 0)

// 	// 将时间戳转换为时间格式
// 	endTime := time.Unix(endMs/1000, 0)

// 	// log.Println(startTime, "------", endTime)

// 	// 获取时间段内每天的切片
// 	dateRange := common.DateRange(startTime, endTime)
// 	if len(dateRange) > 2 {
// 		return errors.New("日期间隔不在同一天")
// 	}

// 	key := fmt.Sprintf("%s:%d:%s:m3u8", in.DeviceID, in.ChannelID, dateRange[0])

// 	// todo:是否要设置超时时间
// 	ctx, cancel := context.WithTimeout(context.Background(), 500*time.Millisecond)
// 	defer cancel()
// 	// 直接执行命令获取错误
// 	err := client.Do(ctx, "set", key, m3u8, "EX", 300).Err()
// 	if err != nil {
// 		return err
// 	}
// 	return nil
// }
