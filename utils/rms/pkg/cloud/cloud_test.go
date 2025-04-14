package cloud

import (
	"fmt"
	"log"
	"os"
	"testing"
)

var config = Config{
	AccessKey: "0f6ca7429abaee658d82",
	SecretKey: "7628ce52ec8f905603002d16d58a84a58f2a23e0",
	Region:    "xiongan",
	EndPoint:  "http://oos-xiongan.ctyunapi.cn",
	Bucker:    "apkbao",
	PuffAddr:  "http://192.168.100.136:8080",
	// IamPoint:  "oos-xiongan-iam.ctyunapi.cn",
}

var s3manager1 = NewCore(config)

func TestGetTsListByDay(t *testing.T) {
	list, err := s3manager1.getTsListByDay(DeviceParamsInput{
		DeviceID:  "34020000001110000002",
		ChannelID: "34020000001310000001",
		PrefixDir: "r/1739524522033758209",
	}, 1723167928449, 1723175956543)
	if err != nil {
		t.Fatal(err)
		return
	}
	for _, v := range list {
		fmt.Println(v)
	}
	fmt.Println(len(list))
}

func TestGetTsListByInterval(t *testing.T) {
	str := []string{
		"r/1739524256681115649/54ad376ddc6226f5dae6f15d4296bb22/1/20231229/1703779266971-30067.ts",
		"r/1739524256681115649/54ad376ddc6226f5dae6f15d4296bb22/1/20231229/1703779297024-30066.ts",
		"r/1739524256681115649/54ad376ddc6226f5dae6f15d4296bb22/1/20231229/1703779327096-30067.ts",
		"r/1739524256681115649/54ad376ddc6226f5dae6f15d4296bb22/1/20231229/1703779357153-30067.ts",
	}
	list, _ := GetTsListByInterval(str, int64(1703779295024), int64(1703779328096))
	for _, v := range list {
		fmt.Println(v)
	}
}

func TestDownload(t *testing.T) {
	in := DeviceParamsInput{
		DeviceID:  "8e624f914779e295d06c7dbc09f7e1fe",
		ChannelID: "1",
		PrefixDir: "r/1739524522033758209",
	}
	file, err := os.Create("./output.txt")
	if err != nil {
		log.Fatal("错误")
	}
	defer file.Close()
	err = s3manager1.DownloadFiles(in, 1704952800000, 1704953394000, file)
	if err != nil {
		log.Fatal("错误：", err)
	}
}

func TestDownloadFiles2(t *testing.T) {
	// file, err := os.Create("./output1.ts")
	// if err != nil {
	// 	log.Fatal("错误")
	// }
	// defer file.Close()
	// err = DownloadFiles2("#EXTM3U\n#EXT-X-VERSION:3\n#EXT-X-MEDIA-SEQUENCE:0\n#EXT-X-TARGETDURATION:12\n#EXTINF:12.000,\nhttps://apkbao.oos-xiongan.ctyunapi.cn/r/1739524522033758209/8e624f914779e295d06c7dbc09f7e1fe/1/20240110/1704902314554-30066.ts?X-Amz-Algorithm=AWS4-HMAC-SHA256&X-Amz-Credential=0f6ca7429abaee658d82%2F20240111%2Fxiongan%2Fs3%2Faws4_request&X-Amz-Date=20240111T094153Z&X-Amz-Expires=3600&X-Amz-SignedHeaders=host&X-Amz-Signature=2fcaf03a76ef12070fd8d250972849d2064e69b5b4ec8113e368dfe8f0d38a55\n#EXTINF:12.000,\nhttps://apkbao.oos-xiongan.ctyunapi.cn/r/1739524522033758209/8e624f914779e295d06c7dbc09f7e1fe/1/20240110/1704902344617-30067.ts?X-Amz-Algorithm=AWS4-HMAC-SHA256&X-Amz-Credential=0f6ca7429abaee658d82%2F20240111%2Fxiongan%2Fs3%2Faws4_request&X-Amz-Date=20240111T094153Z&X-Amz-Expires=3600&X-Amz-SignedHeaders=host&X-Amz-Signature=d67a8665efe85d88d75fba5c59ddadd2eb33e2f6dc7aa6247ec75045523792bf\n#EXTINF:12.000,\nhttps://apkbao.oos-xiongan.ctyunapi.cn/r/1739524522033758209/8e624f914779e295d06c7dbc09f7e1fe/1/20240110/1704902374680-30067.ts?X-Amz-Algorithm=AWS4-HMAC-SHA256&X-Amz-Credential=0f6ca7429abaee658d82%2F20240111%2Fxiongan%2Fs3%2Faws4_request&X-Amz-Date=20240111T094153Z&X-Amz-Expires=3600&X-Amz-SignedHeaders=host&X-Amz-Signature=f05091df62db75716b3128e2f0f1c74da5e458b0e14ac5359c854e635cb0db3f\n", 1719367579000, 1719626798000, file)
	// if err != nil {
	// 	log.Fatal("错误", err)
	// }
}
