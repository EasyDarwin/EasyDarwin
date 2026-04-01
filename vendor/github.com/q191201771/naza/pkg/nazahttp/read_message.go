// Copyright 2020, Chef.  All rights reserved.
// https://github.com/q191201771/naza
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

package nazahttp

import (
	"io"
	"net/http"
	"strconv"
)

// e.g. bufio.Reader
type HttpReader interface {
	LineReader
	io.Reader
}

type HttpMsgCtx struct {
	ReqMethodOrRespVersion string
	ReqUriOrRespStatusCode string
	ReqVersionOrRespReason string
	Headers                http.Header
	Body                   []byte
}

type HttpReqMsgCtx struct {
	Method  string
	Uri     string
	Version string
	Headers http.Header
	Body    []byte
}

type HttpRespMsgCtx struct {
	Version    string
	StatusCode string
	Reason     string
	Headers    http.Header
	Body       []byte
}

func ReadHttpRequestMessage(r HttpReader) (ctx HttpReqMsgCtx, err error) {
	msgCtx, err := ReadHttpMessage(r)
	if err != nil {
		return
	}
	ctx.Method = msgCtx.ReqMethodOrRespVersion
	ctx.Uri = msgCtx.ReqUriOrRespStatusCode
	ctx.Version = msgCtx.ReqVersionOrRespReason
	ctx.Headers = msgCtx.Headers
	ctx.Body = msgCtx.Body
	return
}

func ReadHttpResponseMessage(r HttpReader) (ctx HttpRespMsgCtx, err error) {
	msgCtx, err := ReadHttpMessage(r)
	if err != nil {
		return
	}
	ctx.Version = msgCtx.ReqMethodOrRespVersion
	ctx.StatusCode = msgCtx.ReqUriOrRespStatusCode
	ctx.Reason = msgCtx.ReqVersionOrRespReason
	ctx.Headers = msgCtx.Headers
	ctx.Body = msgCtx.Body
	return
}

// ReadHttpMessage
//
// 注意，如果HTTP Header中不包含`Content-Length`，则不会读取HTTP Body，并且err返回值为nil
//
func ReadHttpMessage(r HttpReader) (ctx HttpMsgCtx, err error) {
	var requestLine string
	requestLine, ctx.Headers, err = ReadHttpHeader(r)
	if err != nil {
		return ctx, err
	}
	ctx.ReqMethodOrRespVersion, ctx.ReqUriOrRespStatusCode, ctx.ReqVersionOrRespReason, err = ParseHttpRequestLine(requestLine)
	if err != nil {
		return ctx, err
	}

	contentLength := ctx.Headers.Get(HeaderFieldContentLength)
	if len(contentLength) == 0 {
		return ctx, nil
	}
	cl, err := strconv.Atoi(contentLength)
	if err != nil {
		return ctx, err
	}
	ctx.Body = make([]byte, cl)
	_, err = io.ReadFull(r, ctx.Body)

	return ctx, err
}
