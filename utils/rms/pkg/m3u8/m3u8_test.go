package m3u8mannager

import (
	"fmt"
	"testing"
)

func TestNewTimeM3u8(t *testing.T) {
	data := []string{
		"https://oos-xiongan.ctyunapi.cn/apkbao/r/1739524256681115649/34020000001320000112/34020000001320000001/20240608/20240608085919-30000.ts?X-Amz-Algorithm=AWS4-HMAC-SHA256&X-Amz-Credential=0f6ca7429abaee658d82%2F20240608%2Fxiongan%2Fs3%2Faws4_request&X-Amz-Date=20240608T054130Z&X-Amz-Expires=86400&X-Amz-SignedHeaders=host&X-Amz-Signature=61bffc1089beced8ce5fb4b72b4fe5c27747e11bb5922acc489f662c341184ef",
		"https://oos-xiongan.ctyunapi.cn/apkbao/r/1739524256681115649/34020000001320000112/34020000001320000001/20240608/20240608085949-4520.ts?X-Amz-Algorithm=AWS4-HMAC-SHA256&X-Amz-Credential=0f6ca7429abaee658d82%2F20240608%2Fxiongan%2Fs3%2Faws4_request&X-Amz-Date=20240608T054130Z&X-Amz-Expires=86400&X-Amz-SignedHeaders=host&X-Amz-Signature=b44eae1e89ac8b95deee117b0706123b2a3f2acda7ae4b72f3173e6b4bad9229",
		"https://oos-xiongan.ctyunapi.cn/apkbao/r/1739524256681115649/34020000001320000112/34020000001320000001/20240608/20240608090225-31964.ts?X-Amz-Algorithm=AWS4-HMAC-SHA256&X-Amz-Credential=0f6ca7429abaee658d82%2F20240608%2Fxiongan%2Fs3%2Faws4_request&X-Amz-Date=20240608T054130Z&X-Amz-Expires=86400&X-Amz-SignedHeaders=host&X-Amz-Signature=df1614540aa4f95ed24d649bbb79526e11c3bff40211061df623ad42d246e39e",
		"https://oos-xiongan.ctyunapi.cn/apkbao/r/1739524256681115649/34020000001320000112/34020000001320000001/20240608/20240608090257-31850.ts?X-Amz-Algorithm=AWS4-HMAC-SHA256&X-Amz-Credential=0f6ca7429abaee658d82%2F20240608%2Fxiongan%2Fs3%2Faws4_request&X-Amz-Date=20240608T054130Z&X-Amz-Expires=86400&X-Amz-SignedHeaders=host&X-Amz-Signature=528fef03ae8aac092131980036161ce9e9cc49a110fa74adb66a7114187293ec",
		"https://oos-xiongan.ctyunapi.cn/apkbao/r/1739524256681115649/34020000001320000112/34020000001320000001/20240608/20240608090257-31850.ts?X-Amz-Algorithm=AWS4.mp4",
		"https://oos-xiongan.ctyunapi.cn/apkbao/r/1739524256681115649/34020000001320000112/34020000001320000001/20240608/20240608090257-31850.mp4?",
		"1739524256681115649/34020000001320000112/34020000001320000001/20240608/20240608090257-31850.mp4",
	}
	out, err := GeneranM3u8(data)
	if err != nil {
		t.Fatal(err)
	}
	fmt.Println(string(out))
}
