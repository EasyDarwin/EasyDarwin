// Author: xiexu
// Date: 2022-09-20

package server

import (
	"context"
	"net/http"
	"sync"
	"time"

	"golang.org/x/net/http2"
	"golang.org/x/net/http2/h2c"
)

const (
	defaultReadTimeout     = 10 * time.Second
	defaultWriteTimeout    = 10 * time.Second
	defaultAddr            = ":8080"
	defaultShutdownTimeout = 3 * time.Second
)

// Server HTTP 服务
type Server struct {
	server          *http.Server
	notify          chan error
	shutdownTimeout time.Duration
	once            sync.Once
}

// New 初始化并启动路由
func New(handler http.Handler, opts ...Option) *Server {
	httpSer := http.Server{
		Addr: defaultAddr,
		Handler: h2c.NewHandler(handler, &http2.Server{
			IdleTimeout: time.Minute,
		}),
		ReadTimeout:  defaultReadTimeout,
		WriteTimeout: defaultWriteTimeout,
	}

	s := &Server{
		server:          &httpSer,
		notify:          make(chan error, 1),
		shutdownTimeout: defaultShutdownTimeout,
	}

	_ = Raise(65535)
	for _, opt := range opts {
		opt(s)
	}
	return s
}

func (s *Server) Start() {
	s.once.Do(func() {
		s.notify <- s.server.ListenAndServe()
		close(s.notify)
	})
}

func (s *Server) StartTLS(certFile, keyFile string) {
	s.once.Do(func() {
		s.notify <- s.server.ListenAndServeTLS(certFile, keyFile)
		close(s.notify)
	})
}

// Notify .
func (s *Server) Notify() <-chan error {
	return s.notify
}

// Shutdown 关闭服务
func (s *Server) Shutdown() error {
	ctx, cancel := context.WithTimeout(context.Background(), s.shutdownTimeout)
	defer cancel()
	return s.server.Shutdown(ctx)
}
