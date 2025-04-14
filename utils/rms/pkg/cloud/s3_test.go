package cloud

import (
	"context"
	"fmt"
	"log"
	"log/slog"
	"os"
	"strings"
	"testing"
	"time"

	m3u8mannager "easydarwin/lnton/rms/pkg/m3u8"
	"github.com/aws/aws-sdk-go/aws"
	"github.com/aws/aws-sdk-go/service/s3"
)

// var configs = Config{
// 	AccessKey: "uuPxOWhRGsJoLFYCRvaz",
// 	SecretKey: "Xt4sSVP5uMreFfnmNGDgmHeHSxgfcd6La3JQiMlR",
// 	Region:    "hefei",
// 	EndPoint:  "http://192.168.1.10:9000",
// 	Bucker:    "bucket1",
// 	// PuffAddr:  "http://192.168.100.136:8080",
// 	// IamPoint:  "oos-xiongan-iam.ctyunapi.cn",
// }

var configs = Config{
	AccessKey: "5nJcaMNucJNEyTeN55TH",
	SecretKey: "jNHKYYL8fkaEgzx1uYKDdNfw3g1lhljXiN5fcP8P",
	Region:    "hefei",
	EndPoint:  "http://127.0.0.1:9000",
	Bucker:    "bucket1",
	// PuffAddr:  "http://192.168.100.136:8080",
	// IamPoint:  "oos-xiongan-iam.ctyunapi.cn",
}

var s3m = NewCore(configs)

func TestGetBuckAllPrefixFile(t *testing.T) {
	file, err := s3m.FindPrefixFiles("record-1d/306e5fbcb048c097a85b39835034b189/0/20231015/")
	if err != nil {
		panic(err)
	}
	log.Println(file[0])
	log.Println("文件个数", len(file))
}

func TestBuckFile(t *testing.T) {
	objects, err := s3m.BucketFiles("tysdk")
	if err != nil {
		t.Fatal(err)
		return
	}
	for _, v := range objects.Contents {
		log.Println(*(v.Key))
	}
}

// func TestGetObjectKey(t *testing.T) {
// 	s3m.GetObjectKey("tysdk")
// }

// func TestGetBukets(t *testing.T) {
// 	buckets, err := s3manager.GetBuckets()
// 	if err != nil {
// 		t.Fatalf(err.Error())
// 		return
// 	}
// 	log.Println(buckets)
//
// }

func TestAuth(t *testing.T) {
	path := []string{"record-1d/306e5fbcb048c097a85b39835034b189/0/20231013/1697178843310-30000.ts", "record-1d/306e5fbcb048c097a85b39835034b189/0/20231013/1697178923310-30000.ts"}
	urls, err := s3m.SetAuthToken(path, "tysdk", time.Hour)
	if err != nil {
		t.Fatal(err)
		return
	}
	data, err := m3u8mannager.GeneranM3u8(urls)
	if err != nil {
		t.Fatal(err)
		return
	}
	fmt.Println(string(data))
}

func TestGetBucketLifecycleConfiguration(t *testing.T) {
	configuration, err := s3m.s3.GetBucketLifecycleConfiguration(&s3.GetBucketLifecycleConfigurationInput{
		Bucket: aws.String("bucket1"),
	})
	if err != nil {
		t.Errorf(err.Error())
	}
	fmt.Println(configuration)
}

// TestPutBucketLifecycleConfiguration 测试生命周期
func TestPutBucketLifecycleConfiguration(t *testing.T) {
	configuration, err := s3m.s3.PutBucketLifecycleConfiguration(&s3.PutBucketLifecycleConfigurationInput{
		Bucket: aws.String("bucket1"), // 设置生命周期规则的存储桶
		LifecycleConfiguration: &s3.BucketLifecycleConfiguration{
			Rules: []*s3.LifecycleRule{
				{
					ID:     aws.String("delete-expired-files"), // 规则ID
					Status: aws.String("Enabled"),              // 必须是Enable或者Disable
					// Status: s3.ObjectLockEnabledEnabled,
					Filter: &s3.LifecycleRuleFilter{
						Prefix: aws.String("r"), // 指定文件夹（前缀）
					},
					Expiration: &s3.LifecycleExpiration{
						Days: aws.Int64(30), // 过期后30天删除文件
					},
				},
			},
		},
	})
	if err != nil {
		t.Errorf(err.Error())
	}
	fmt.Println(configuration)

	lifecycleConfiguration, err := s3m.s3.GetBucketLifecycleConfiguration(&s3.GetBucketLifecycleConfigurationInput{
		Bucket: aws.String("bucket1"),
	})
	if err != nil {
		t.Errorf(err.Error())
	}
	fmt.Println(lifecycleConfiguration)
}

func TestGetPrefix(t *testing.T) {
	// obecjts, err := s3manager.QueryFileList("sdbucket", []string{"record", "hls", "ck5qqav8hhvq3gpgh4bg", "20230921"})
	objects, err := s3m.QueryFileList("apkbao", []string{"records", "25f393af665cb387a54634fb30d5d43e", "1", "20231215"})
	if err != nil {
		t.Fatal(err)
		return
	}
	for _, v := range objects.Contents {
		fmt.Println(*(v.Key))
	}
	fmt.Println(len(objects.Contents))
}

// TestFindPrefixFiles 获取全部文件ts列表，加分页情况
func TestFindPrefixFiles(t *testing.T) {
	// obecjts, err := s3m.QueryFileList("sdbucket", []string{"record", "hls", "ck5qqav8hhvq3gpgh4bg", "20230921"})

	// var arg = []string{"records", "25f393af665cb387a54634fb30d5d43e", "1", "20231215"}
	// var params string
	// for _, v := range arg {
	// params += v + "/"
	// }
	files, err := s3m.FindPrefixFiles("")
	// objects, err := s3m.QueryFileList(params)
	if err != nil {
		t.Fatal(err)
		return
	}
	for _, v := range files {
		fmt.Println(v)
	}

	o, _ := s3m.s3.GetObjectRequest(&s3.GetObjectInput{
		Bucket: aws.String(s3m.bucket),
		Key:    aws.String(files[0]),
	})

	ss, _ := o.Presign(120 * time.Hour)
	ss = strings.ReplaceAll(ss, "%2F", "/")
	fmt.Println(ss)
	// fmt.Println(len(files))
}

// GetMonthRecord 查询指定月份的存储信息，
func TestGetMonthRecord(t *testing.T) {
	intput := DeviceParamsInput{
		DeviceID:  "34020000001110000002",
		ChannelID: "34020000001310000001",
		PrefixDir: "r/1739524256681115649/",
	}
	record, err := s3m.GetRecordByMonth(intput, "202408")
	if err != nil {
		panic(err)
	}
	fmt.Println(record)
}

func TestGetM3u8(t *testing.T) {
	intput := DeviceParamsInput{
		DeviceID:  "34020000001110000002",
		ChannelID: "34020000001310000001",
		PrefixDir: "r/1739524256681115649",
	}
	m3u8, _, err := s3m.GetM3u8(intput, 1723132800000, 1723219199999, time.Hour)
	if err != nil {
		panic(err)
	}
	fmt.Println(m3u8)
}

func TestGetbetweenday(t *testing.T) {
	starttime := time.Now().AddDate(0, 0, -1)
	starttime = time.Unix(1719327600, 0)
	endtime := time.Now()
	endtime = starttime.AddDate(0, 0, 1)

	fmt.Printf("%v", DateRange(starttime, endtime))
}

// Test_FindTimeline 返回加载时间轴的数据
func TestFindTimeline(t *testing.T) {
	intput := DeviceParamsInput{
		DeviceID:  "cf419def39870454721ffc6b14cfe38c",
		ChannelID: "2",
		PrefixDir: "r/1739524256681115649",
	}
	start := time.Now().Add(-12 * time.Hour).UnixMilli()
	end := time.Now().Add(time.Hour).UnixMilli()

	timeline, err := s3m.FindTimeline(intput, start, end)
	if err != nil {
		log.Println("程序出错", err)
		return
	}
	fmt.Println(timeline)
}

func TestModifyString(t *testing.T) {
	endPoint := config.EndPoint
	fmt.Println("before:", endPoint)
	url := ModifyString(endPoint)
	fmt.Println(url)
}

func TestUpload(t *testing.T) {
	url, err := s3m.PutObject(context.Background(), DeviceParamsInput{DeviceID: "test", ChannelID: "test", PrefixDir: "./"}, "test", []byte("hello world"))
	if err != nil {
		log.Println(err)
		t.Fail()
		return
	}
	slog.Info("TestUpload", "url", url)
}

// func TestSetDirExpireTime(t *testing.T) {
// 	// record-1d/306e5fbcb048c097a85b39835034b189/0"
// 	// d := DeviceParamsInput{DeviceID: "306e5fbcb048c097a85b39835034b189", ChannelID: "0", PrefixDir: "record-1"}
// 	// prefixDir := "record-1d/306e5fbcb048c097a85b39835034b189/0"
// 	// day := 30
// 	// s3m.SetDirExpireTime(d, day)
// 	// s3m.SetDirExpireTime(d, 30)
// 	// s3m.GetDirLifecycle()

// 	// out, err := s3m.s3.GetBucketLifecycleConfiguration(context.TODO(), &s3.GetBucketLifecycleConfigurationInput{})
// 	// if err != nil {
// 	// 	t.Fatal(err)
// 	// }
// 	// fmt.Println(out.ResultMetadata)
// 	// for _, v := range out.Rules {
// 	// 	fmt.Println(v)
// 	// }

// 	// 使用 s3 实例，获取云存策略配置
// 	out, err := s3m.s3.GetBucketLifecycleConfiguration(context.TODO(), &s3.GetBucketLifecycleConfigurationInput{
// 		Bucket: &s3m.bucket,
// 	})
// 	if err != nil {
// 		t.Fatal(err)
// 	}
// 	// 遍历策略，打印前缀
// 	for _, v := range out.Rules {
// 		fmt.Println(*v.Prefix, "--", v.Expiration.Days)
// 	}

// 	// 设置云存策略
// 	// {
// 	// 	// 添加一条新策略
// 	// 	out.Rules = append(out.Rules, types.LifecycleRule{
// 	// 		ID:         aws.String("123123123123"),           // 策略的 唯一 id
// 	// 		Prefix:     aws.String("/cccc"),                  // 策略指定的前缀
// 	// 		Expiration: &types.LifecycleExpiration{Days: 10}, // 策略的过期时间
// 	// 		Status:     types.ExpirationStatusEnabled,        // 策略是否启用
// 	// 	})
// 	// 	// 使用 s3 设置到 bucket
// 	// 	out, err := s3m.s3.PutBucketLifecycleConfiguration(context.TODO(), &s3.PutBucketLifecycleConfigurationInput{
// 	// 		Bucket: &s3m.bucket,
// 	// 		LifecycleConfiguration: &types.BucketLifecycleConfiguration{
// 	// 			Rules: out.Rules,
// 	// 		},
// 	// 	})
// 	// 	if err != nil {
// 	// 		t.Fatal(err)
// 	// 	}
// 	// 	fmt.Println(out.ResultMetadata)
// 	// }

// 	// {
// 	// 	// 等待一会，获取看看，是否设置成功
// 	// 	time.Sleep(time.Second * 10)
// 	// 	out, err := s3m.s3.GetBucketLifecycleConfiguration(context.TODO(), &s3.GetBucketLifecycleConfigurationInput{
// 	// 		Bucket: &s3m.bucket,
// 	// 	})
// 	// 	if err != nil {
// 	// 		fmt.Println(err)
// 	// 	}
// 	// 	for _, v := range out.Rules {
// 	// 		fmt.Println(*v.Prefix)
// 	// 	}
// 	// 	fmt.Println(out.Rules)
// 	// }

// 	// out, err := s3m.tyIamClient.GetBucketLifecycle(s3m.bucket)
// 	// if err != nil {
// 	// 	t.Fatal(err)
// 	// }
// 	// fmt.Println(out.Rules)

// 	// rule := oss.BuildLifecycleExpirRuleByDays("abc", "/124312", true, 10)
// 	// if err := s3m.tyIamClient.SetBucketLifecycle(s3m.bucket, []oss.LifecycleRule{
// 	// 	rule,
// 	// }); err != nil {
// 	// 	t.Fatal(err)
// 	// }

// 	// {
// 	// 	time.Sleep(time.Second)
// 	// 	fmt.Println(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>")
// 	// 	out, err := s3m.tyIamClient.GetBucketLifecycle(s3m.bucket)
// 	// 	if err != nil {
// 	// 		t.Fatal(err)
// 	// 	}
// 	// 	fmt.Println(out.Rules)
// 	// }

// }

// func TestCopy(t *testing.T) {
// 	prefix := "records/54ad376ddc6226f5dae6f15d4296bb22"
// 	fmt.Println("start")
// 	out, err := s3m.s3.ListObjectsV2(context.TODO(), &s3.ListObjectsV2Input{Bucket: &s3m.bucket, Prefix: aws.String(prefix)})
// 	if err != nil {
// 		t.Fatal(err)
// 	}
// 	fmt.Println("secound")
// 	for _, v := range out.Contents {
// 		fmt.Println(*v.Key)
// 		_, err := s3m.s3.CopyObject(context.TODO(), &s3.CopyObjectInput{
// 			Bucket:     &s3m.bucket,
// 			CopySource: aws.String(s3m.bucket + "/" + *v.Key),
// 			Key:        aws.String("records/abc/" + strings.TrimLeft(*v.Key, "records")),
// 		})
// 		if err != nil {
// 			fmt.Println(err)
// 			return
// 		}
// 		return
// 	}
// }

func TestMoveFiles(t *testing.T) {
	now := time.Now()
	l := slog.New(slog.NewJSONHandler(os.Stdout, &slog.HandlerOptions{Level: slog.LevelDebug}))
	slog.SetDefault(l)
	if err := s3m.MoveFiles(DeviceParamsInput{
		DeviceID:  "bddc3cc337a8a2a5d44461ec969c0c64",
		PrefixDir: "records",
		ChannelID: "2",
	}, "r/1739524256681115649"); err != nil {
		fmt.Println(err)
	}
	fmt.Println(time.Since(now))
}

func TestURL(t *testing.T) {
	req, _ := s3m.s3.GetObjectRequest(&s3.GetObjectInput{
		Bucket: aws.String("tysdk"),
		Key:    aws.String("record-1d/984df5f8929087ebb9e3c438eeac59f4/1/20231213/1702467442788-30000.ts"),
	})
	urlStr, err := req.Presign(time.Hour)
	if err != nil {
		slog.Error("SetAuthToken", "err", err)
	}
	fmt.Println(urlStr)
}
