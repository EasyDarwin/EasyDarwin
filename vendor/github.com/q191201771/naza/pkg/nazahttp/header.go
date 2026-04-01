// Copyright 2020, Chef.  All rights reserved.
// https://github.com/q191201771/naza
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

package nazahttp

import (
	"encoding/hex"
	"net/http"
	"strings"

	"github.com/q191201771/naza/pkg/nazaerrors"
)

type LineReader interface {
	ReadLine() (line []byte, isPrefix bool, err error)
}

// ReadHttpHeader
//
// @return firstLine: request的request line或response的status line
// @return headers:   request header fields的键值对
//
func ReadHttpHeader(r LineReader) (firstLine string, headers http.Header, err error) {
	headers = make(http.Header)

	readLineFn := func() (string, error) {
		var line string
		var bline []byte
		var isPrefix bool
		for {
			bline, isPrefix, err = r.ReadLine()
			if err != nil {
				return "", err
			}
			line += string(bline)
			if !isPrefix {
				break
			}
		}
		return line, nil
	}

	firstLine, err = readLineFn()
	if err != nil {
		err = nazaerrors.Wrap(err, firstLine)
		return
	}
	if len(firstLine) == 0 {
		err = nazaerrors.Wrap(ErrHttpHeader, firstLine)
		return
	}

	var lastKey string

	for {
		var l string
		l, err = readLineFn()
		if err != nil {
			err = nazaerrors.Wrap(err, l)
			return
		}
		if len(l) == 0 { // 读到一个空的 \r\n 表示http头全部读取完毕了
			break
		}

		pos := strings.Index(l, ":")
		if pos == -1 {
			// 兼容性处理，见单元测试TestReadHttpResponseMessage中的case1
			//
			// 如果找不到冒号，就把它算到上一个header条目的value里
			// 也即我们认为上一个value中的自身内容包含了格式错误的\r\n
			//
			if lastKey != "" {
				vs := headers.Values(lastKey)
				vs[len(vs)-1] = vs[len(vs)-1] + l
			}
			continue
		}
		lastKey = strings.Trim(l[0:pos], " ")
		headers.Add(strings.Trim(l[0:pos], " "), strings.Trim(l[pos+1:], " "))
	}
	return
}

// Request-Line = Method SP URI SP Version CRLF
func ParseHttpRequestLine(line string) (method string, uri string, version string, err error) {
	return parseFirstLine(line)
}

// Status-Line = Version SP Status-Code SP Reason CRLF
func ParseHttpStatusLine(line string) (version string, statusCode string, reason string, err error) {
	return parseFirstLine(line)
}

func parseFirstLine(line string) (item1, item2, item3 string, err error) {
	f := strings.Index(line, " ")
	if f == -1 {
		err = nazaerrors.Wrap(ErrFirstLine, hex.Dump([]byte(line)))
		return
	}
	s := strings.Index(line[f+1:], " ")
	// TODO(chef): refactor 整理此处代码，可使用split根据数量返回
	if s == -1 {
		return line[0:f], line[f+1:], "", nil
	}
	if f+1+s+1 == len(line) {
		return line[0:f], line[f+1 : f+1+s], "", nil
	}
	//if s == -1 || f+1+s+1 == len(line) {
	//	err = nazaerrors.Wrap(ErrFirstLine, line)
	//	return
	//}

	return line[0:f], line[f+1 : f+1+s], line[f+1+s+1:], nil
}
