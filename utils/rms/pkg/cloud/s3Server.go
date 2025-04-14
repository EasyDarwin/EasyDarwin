package cloud

import (
	"bufio"
	"strconv"
	"strings"

	"github.com/aws/aws-sdk-go/aws"
	"github.com/aws/aws-sdk-go/service/s3"
)

var s3manager = NewSession(Config{
	AccessKey: "0f6ca7429abaee658d82",
	SecretKey: "7628ce52ec8f905603002d16d58a84a58f2a23e0",
	Region:    "xiongan",
	EndPoint:  "",
	Bucker:    "",
})

// GetMonthRecord 查询某一天的视频内容

// getBuckFile 获取文件列表
func (s *S3Mannager) getBuckFile(bucket string) ([]string, error) {
	out, err := s.BucketFiles(bucket)
	if err != nil {
		return nil, err
	}
	list := make([]string, 0, 8)
	for _, v := range out.Contents {
		list = append(list, *(v.Key))
	}
	return list, nil
}

// GetTsListByM3u8 m3u8转换为ts列表
func GetTsListByM3u8(m3u8 string) ([]string, error) {
	tsFiles := make([]string, 0, 8)
	reader := bufio.NewReader(strings.NewReader(m3u8))
	for {
		line, err := reader.ReadString('\n')
		if err != nil && err.Error() == "EOF" {
			break
		}
		if err != nil {
			return nil, err
		}
		if strings.HasPrefix(line, "#") {
			continue
		}
		tsFiles = append(tsFiles, line)
	}
	return tsFiles, nil
}

// GetLinksByM3u8 传入带鉴权的m3u8文件，获取所有ts文件的http链接
//
// 最初获取ts在S3中的路径，由于S3的访问链接规范，桶名可以在域名中，也可以在链接中，因此需要判断
// 如果仍然去解析m3u8中的文件路径会导致逻辑比较负责，因此该函数直接获取S3的访问链接
func GetLinksByM3u8(m3u8 string) ([]string, error) {
	links := make([]string, 0, 8)
	reader := bufio.NewReader(strings.NewReader(m3u8))
	for {
		// 对m3u8进行逐行读取
		line, err := reader.ReadString('\n')
		// 如果读取到文件末尾，则退出循环
		if err != nil && err.Error() == "EOF" {
			break
		}
		// 如果读取出错，则返回错误
		if err != nil {
			return nil, err
		}

		// 如果是#开头的行，是m3u8的描述行，进行忽略
		if strings.HasPrefix(line, "#") {
			continue
		}

		// 如果是标准的链接是存在?的，这里的校验可以删除
		// TODO: 如果 s3 出现鉴权问题? 会不会因为这里
		// split := strings.Split(line, "?")
		// if len(split) < 2 {
		// continue
		// }
		line = strings.TrimSuffix(line, "\n")
		// 将扫描的链接添加到链接列表中
		links = append(links, line)
	}
	return links, nil
}

func GetTsListByHTTPM3u8(m3u8 string) ([]string, error) {
	tsFiles := make([]string, 0, 8)
	reader := bufio.NewReader(strings.NewReader(m3u8))
	for {
		line, err := reader.ReadString('\n')
		if err != nil && err.Error() == "EOF" {
			break
		}
		if err != nil {
			return nil, err
		}
		if strings.HasPrefix(line, "#") {
			continue
		}
		split := strings.Split(line, "?")
		if len(split) < 2 {
			continue
		}

		line = split[0][strings.Index(split[0], "//")+2:]
		line = line[strings.Index(line, "/"):]
		tsFiles = append(tsFiles, line)
	}
	return tsFiles, nil
}

// GetTsListByInterval 通过一段时间获取Ts列表中符合条件的Ts列表
func GetTsListByInterval(list []string, start, end int64) ([]string, error) {
	// 给定m3u8文件、开始时间start、结束时间end三个参数，遍历m3u8返回包括start和end的最小m3u8文件
	// 例如开始时间为01:20，结束时间为02:00，返回的开始时间应该是小于01:20，大于02:00的最小时间段 ，1:10~02:20
	TimeStampLen := 13

	tsList := make([]string, 0, 8)
	for _, v := range list {
		// 获取文件名
		lastSlash := strings.LastIndex(v, "/") + 1

		// 获取文件名中的时间戳
		fileTime := v[lastSlash : lastSlash+TimeStampLen]
		fileTimeInt, _ := strconv.ParseInt(fileTime, 10, 64)

		// 获取包含起始时间的ts切片
		// time, _ := strconv.Atoi(fileTime)
		if (fileTimeInt+30000) > start && fileTimeInt < start {
			tsList = append(tsList, v)
		}

		//// 获取包含结束时间的ts切片
		//if (time+30000) > endInt && time < endInt {
		//	tsList = append(tsList, v)
		//}

		if fileTimeInt >= start && fileTimeInt <= end {
			tsList = append(tsList, v)
		}
	}
	return tsList, nil
}

// FindPrefixFiles 通过前缀获取全部文件列表
func (s *S3Mannager) FindPrefixFiles(prefix string) ([]string, error) {
	input := &s3.ListObjectsInput{
		Bucket:  aws.String(s.bucket),
		Prefix:  aws.String(prefix),
		MaxKeys: aws.Int64(1000),
	}
	list := make([]string, 0, 10)
	// 分页参考: https://github.com/aws/aws-sdk-php/issues/1428
	for {
		out, err := s.s3.ListObjects(input)
		if err != nil {
			return nil, err
		}
		for _, v := range out.Contents {
			list = append(list, *(v.Key))
		}
		if !*out.IsTruncated {
			return list, nil
		}
		input.Marker = out.NextMarker
	}
}

func (s *S3Mannager) FindPrefixFile(prefix string) ([]string, error) {
	parms := &s3.ListObjectsInput{Bucket: aws.String(s.bucket), Prefix: aws.String(prefix)}
	out, err := s.s3.ListObjects(parms)
	if err != nil {
		return nil, err
	}
	list := make([]string, 0, 8)
	// 将对象转换为string切片
	for _, v := range out.Contents {
		list = append(list, *(v.Key))
	}
	return list, nil
}
