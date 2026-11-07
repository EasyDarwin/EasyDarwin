package api

import (
	"strings"
	"testing"
)

// TestMaskURLCredentials 拉流地址凭据脱敏，重点覆盖 url.Parse 失败时的兜底分支（CNVD-2026-42898）
func TestMaskURLCredentials(t *testing.T) {
	cases := []struct {
		name     string
		raw      string
		want     string // 非空时精确比对
		secret   string // 非空时断言结果不含该明文
		contains string // 非空时断言结果包含该子串
	}{
		{
			name:     "常规 RTSP 带凭据",
			raw:      "rtsp://admin:pass123@192.168.1.1:554/path",
			want:     "rtsp://****:****@192.168.1.1:554/path",
			secret:   "pass123",
			contains: "192.168.1.1:554/path",
		},
		{
			name: "无凭据原样返回",
			raw:  "rtsp://192.168.1.1:554/path",
			want: "rtsp://192.168.1.1:554/path",
		},
		{
			name:     "口令含空格导致解析失败",
			raw:      "rtsp://admin:pa ss@192.168.1.1:554/path",
			secret:   "pa ss",
			contains: "****:****@192.168.1.1:554/path",
		},
		{
			name:     "口令含非法百分号编码导致解析失败",
			raw:      "rtsp://admin:p%zzword@192.168.1.1:554/path",
			secret:   "p%zzword",
			contains: "****:****@192.168.1.1:554/path",
		},
		{
			name:     "口令含中文导致解析失败",
			raw:      "rtsp://admin:密码@192.168.1.1:554/path",
			secret:   "密码",
			contains: "****:****@192.168.1.1:554/path",
		},
		{
			name:     "口令含未编码 @",
			raw:      "rtsp://admin:pa@ss@192.168.1.1:554/path",
			secret:   "pa@ss",
			contains: "****:****@192.168.1.1:554/path",
		},
		{
			name:   "解析失败且无法定位凭据时返回占位符",
			raw:    "not a url %zz",
			want:   "****",
			secret: "not a url",
		},
		{
			name: "空字符串",
			raw:  "",
			want: "",
		},
	}
	for _, tc := range cases {
		t.Run(tc.name, func(t *testing.T) {
			got := maskURLCredentials(tc.raw)
			if tc.want != "" && got != tc.want {
				t.Fatalf("maskURLCredentials(%q) = %q, 期望 %q", tc.raw, got, tc.want)
			}
			if tc.secret != "" && strings.Contains(got, tc.secret) {
				t.Fatalf("maskURLCredentials(%q) = %q, 仍含明文凭据 %q", tc.raw, got, tc.secret)
			}
			if tc.contains != "" && !strings.Contains(got, tc.contains) {
				t.Fatalf("maskURLCredentials(%q) = %q, 期望包含 %q", tc.raw, got, tc.contains)
			}
		})
	}
}