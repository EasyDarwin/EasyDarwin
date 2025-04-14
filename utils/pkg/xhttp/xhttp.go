package xhttp

import (
	"net/http"
	"time"
)

var client = http.Client{Timeout: 10 * time.Second}

// Do 发送请求
func Do(req *http.Request, callback func(*http.Response) error) error {
	resp, err := client.Do(req)
	if err != nil {
		return err
	}
	defer func() {
		_ = resp.Body.Close()
	}()
	return callback(resp)
}
