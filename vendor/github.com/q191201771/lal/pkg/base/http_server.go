// Copyright 2021, Chef.  All rights reserved.
// https://github.com/q191201771/lal
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

package base

import (
	"crypto/tls"
	"net"
	"net/http"
	"reflect"

	"github.com/q191201771/naza/pkg/nazaerrors"
)

// TODO(chef): [refactor] 考虑移入naza中 202211

const (
	NetworkTcp = "tcp"
)

type LocalAddrCtx struct {
	IsHttps  bool
	Addr     string
	CertFile string
	KeyFile  string

	Network string
}

type HttpServerManager struct {
	addr2ServerCtx map[string]*ServerCtx
}

type ServerCtx struct {
	addrCtx         LocalAddrCtx
	listener        net.Listener
	httpServer      http.Server
	mux             *http.ServeMux
	pattern2Handler map[string]Handler
}

func NewHttpServerManager() *HttpServerManager {
	return &HttpServerManager{
		addr2ServerCtx: make(map[string]*ServerCtx),
	}
}

type Handler func(http.ResponseWriter, *http.Request)

// AddListen
//
// @param addrCtx:
//
//	 LocalAddrCtx.IsHttps 是否为https。
//		注意，如果要为相同的路由同时绑定http和https，那么应该调用该函数两次，分别将该参数设置为true和false。
//		LocalAddrCtx.Addr 监听地址，内部会为其建立监听。
//		http和https不能够使用相同的地址。
//		注意，多次调用，允许使用相同的地址绑定不同的`pattern`。
//		LocalAddrCtx.CertFile ...
//		LocalAddrCtx.KeyFile ...
//		LocalAddrCtx.Network 如果为空默认为NetworkTcp="tcp"。
//
// @param pattern: 必须以`/`开始，并以`/`结束。
//
//	注意，如果是`/`，则在其他所有pattern都匹配失败后，做为兜底匹配成功。
//	相同的pattern不能绑定不同的`handler`回调函数（显然，我们无法为相同的监听地址，相同的路径绑定多个回调函数）。
func (s *HttpServerManager) AddListen(addrCtx LocalAddrCtx, pattern string, handler Handler) error {
	var (
		ctx *ServerCtx
		mux *http.ServeMux
		ok  bool
	)

	if addrCtx.Addr == "" {
		return ErrAddrEmpty
	}

	// 监听地址是否已经创建过
	ctx, ok = s.addr2ServerCtx[addrCtx.Addr]
	if !ok {
		l, err := listen(addrCtx)
		if err != nil {
			return err
		}
		mux = http.NewServeMux()
		ctx = &ServerCtx{
			addrCtx:  addrCtx,
			listener: l,
			httpServer: http.Server{
				Handler: mux,
			},
			mux:             mux,
			pattern2Handler: make(map[string]Handler),
		}
		s.addr2ServerCtx[addrCtx.Addr] = ctx
	}

	// 路径相同，比较回调函数是否相同
	// 如果回调函数也相同，意味着重复绑定，这种情况是允许的，忽略掉就行了
	// 如果回调函数不同，返回错误
	if prevHandler, ok := ctx.pattern2Handler[pattern]; ok {
		if reflect.ValueOf(prevHandler).Pointer() == reflect.ValueOf(handler).Pointer() {
			return nil
		} else {
			return ErrMultiRegisterForPattern
		}
	}
	ctx.pattern2Handler[pattern] = handler

	ctx.mux.HandleFunc(pattern, handler)
	return nil
}

func (s *HttpServerManager) RunLoop() error {
	errChan := make(chan error, len(s.addr2ServerCtx))

	for _, v := range s.addr2ServerCtx {
		go func(ctx *ServerCtx) {
			errChan <- ctx.httpServer.Serve(ctx.listener)

			_ = ctx.httpServer.Close()
		}(v)
	}

	// 阻塞直到接到第一个error
	return <-errChan
}

func (s *HttpServerManager) Dispose() error {
	var es []error
	for _, v := range s.addr2ServerCtx {
		err := v.httpServer.Close()
		es = append(es, err)
	}
	return nazaerrors.CombineErrors(es...)
}

// ---------------------------------------------------------------------------------------------------------------------

// 为传入的`Addr`地址创建http或https监听
func listen(ctx LocalAddrCtx) (net.Listener, error) {
	if ctx.Network == "" {
		ctx.Network = NetworkTcp
	}

	if !ctx.IsHttps {
		return net.Listen(ctx.Network, ctx.Addr)
	}

	cert, err := tls.LoadX509KeyPair(ctx.CertFile, ctx.KeyFile)
	if err != nil {
		return nil, err
	}
	tlsConfig := &tls.Config{Certificates: []tls.Certificate{cert}}
	return tls.Listen(ctx.Network, ctx.Addr, tlsConfig)
}
