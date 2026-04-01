// Copyright 2020, Chef.  All rights reserved.
// https://github.com/q191201771/naza
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

package nazahttp

import (
	"bytes"
	"encoding/json"
	"net/http"
)

// @param url  地址
// @param info 需要序列化的结构体
// @param client 注意，如果为nil，则使用http.DefaultClient
func PostJson(url string, info interface{}, client *http.Client) (*http.Response, error) {
	j, err := json.Marshal(info)
	if err != nil {
		return nil, err
	}
	b := bytes.NewBuffer(j)
	if client == nil {
		client = http.DefaultClient
	}
	return client.Post(url, HeaderFieldContentType, b)
}
