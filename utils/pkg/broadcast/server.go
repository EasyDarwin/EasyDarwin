// 发现流程
// server 启动时应主动广播 3 次，间隔 2 秒一次。
// 1. {url:"/notify","method":"notify" , body:{}}
// client 启动时应主动查询
// 2. {url:"/discover","method":"get", data:{}}
// server 收到发现消息时，应当立即回复自己的信息
// 3. {type:"/discover", "method":"get", data:{ version, web_addr, mac  }}
// 4. 服务端停止时，应该发送一个停止消息，通知其他服务端，不再广播自己的信息
//  {type:"notify","topic":""}

package broadcast

import (
	"bufio"
	"bytes"
	"errors"
	"fmt"
	"io"
	"log/slog"
	"net"
	"net/http"
	"net/http/httptest"
	"net/http/httputil"
	"strings"
	"sync"
	"time"

	"github.com/gin-gonic/gin"
)

type Config struct {
	Version string `json:"version"`
	WebAddr string `json:"web_addr"`
	Mac     string `json:"mac"`
	Name    string `json:"name"`
}

type Conn struct {
	conn    net.PacketConn
	cfg     Config
	handler http.Handler
	to      *net.UDPAddr

	rq map[string]chan *http.Response
	m  sync.Mutex
}

func (c *Conn) WriteResponse(callID string, resp *http.Response) error {
	c.m.Lock()
	defer c.m.Unlock()

	ch, ok := c.rq[callID]
	if ok && ch != nil {
		ch <- resp
	}
	return nil
}

func (c *Conn) DelCallID(callID string) {
	c.m.Lock()
	defer c.m.Unlock()

	ch, ok := c.rq[callID]
	if ok {
		delete(c.rq, callID)
		close(ch)
	}
}

func (c *Conn) setCallID(callID string) <-chan *http.Response {
	c.m.Lock()
	defer c.m.Unlock()
	ch, ok := c.rq[callID]
	if !ok {
		ch = make(chan *http.Response, 10)
		c.rq[callID] = ch
	}
	return ch
}

func NewServer(cfg Config, handler http.Handler) (*Conn, error) {
	conn, err := net.ListenMulticastUDP("udp4", nil, &net.UDPAddr{
		IP:   broadcastIP,
		Port: broadcastPort,
	})
	if err != nil {
		return nil, err
	}
	c := Conn{
		conn:    conn,
		cfg:     cfg,
		handler: handler,
		rq:      make(map[string]chan *http.Response),
		to:      &net.UDPAddr{IP: broadcastIP, Port: broadcastPort},
	}
	go c.read()
	return &c, nil
}

func (c *Conn) read() {
	if c.conn == nil {
		return
	}
	defer func() {
		c.conn.Close()
	}()
	for {
		buf := make([]byte, 1472)
		n, raddr, err := c.conn.ReadFrom(buf)
		if err != nil {
			slog.Error(c.cfg.Name+"read", "err", err)
			break
		}
		if n < 4 {
			slog.Error("bad request, n<4", "buf", string(buf))
			continue
		}
		go func() {
			if strings.EqualFold(string(buf[0:4]), "http") {
				c.handleResponse(buf, n)
			} else {
				c.handleRequest(raddr, buf, n)
			}
		}()
	}
}

func NewServerHandler(cfg Config) http.Handler {
	gin.SetMode(gin.ReleaseMode)
	g := gin.New()
	g.GET("/discover", func(ctx *gin.Context) {
		ctx.JSON(200, cfg)
	})
	g.PUT("/ip", func(ctx *gin.Context) {
		ctx.JSON(400, gin.H{"msg": "未实现"})
	})
	g.POST("/notify/online", func(ctx *gin.Context) {
		fmt.Println("remote online")
		ctx.JSON(200, gin.H{"msg": "ok"})
	})
	g.POST("/notify/offline", func(ctx *gin.Context) {
		fmt.Println("remote offline")
		ctx.JSON(200, gin.H{"msg": "ok"})
	})
	return g
}

func NewClient(cfg Config, mux http.Handler) *Conn {
	dstAddr := &net.UDPAddr{IP: broadcastIP, Port: broadcastPort}
	conn, err := net.ListenPacket("udp4", ":0")
	if err != nil {
		slog.Error("broadcast failed", "err", err)
	}

	s := Conn{
		conn:    conn,
		cfg:     cfg,
		handler: mux,
		to:      dstAddr,
		rq:      make(map[string]chan *http.Response),
	}
	go s.read()
	return &s
}

// Notify 服务端上线/离线通知
func (s *Conn) Notify(isOnline bool) error {
	if s.conn == nil {
		return nil
	}
	link := "/notify/online"
	if !isOnline {
		link = "/notify/offline"
	}
	req, _ := http.NewRequest(http.MethodPost, link, nil)
	_, b, err := request(req)
	if err != nil {
		return err
	}
	_, err = s.conn.WriteTo(b, s.to)
	return err
}

func (s *Conn) handleResponse(buf []byte, n int) {
	resp, err := http.ReadResponse(bufio.NewReader(bytes.NewReader(buf[:n])), nil) // nolint
	if err != nil {
		slog.Error("read response err", "err", err)
		return
	}
	callID := resp.Header.Get(xRequestID)

	s.WriteResponse(callID, resp)
}

func (s *Conn) handleRequest(raddr net.Addr, buf []byte, n int) {
	req, err := http.ReadRequest(bufio.NewReader(bytes.NewReader(buf[:n])))
	if err != nil {
		slog.Error("read request err", "err", err)
		return
	}
	callID := req.Header.Get(xRequestID)

	r := httptest.NewRecorder()
	s.handler.ServeHTTP(r, req)
	resp := r.Result()
	resp.Header.Set(xRequestID, callID)
	defer resp.Body.Close()
	b, err := httputil.DumpResponse(resp, true)
	if err != nil {
		slog.Error("DumpResponse", "err", err)
		return
	}
	_, err = s.conn.WriteTo(b, raddr)
	if err != nil {
		slog.Error("write faild", "err", err)
	}
}

// Discover 作为客户端，发现环境内的服务
func (s *Conn) Discover(userAgent string) error {
	discover := "/discover"
	if userAgent != "" {
		discover = discover + "?service=" + userAgent
	}
	req, _ := http.NewRequest(http.MethodGet, discover, nil)
	err := s.request(req, func(r *http.Response) {
		fmt.Println(">>>>>>>>>>>>>>>>>> response")
		b, err := io.ReadAll(r.Body)
		if err != nil {
			slog.Error("response", "err", err)
			return
		}
		fmt.Println(string(b))
	})
	if errors.Is(err, io.EOF) {
		return nil
	}
	return err
}

// 请求封装，一次性返回所有响应
func (s *Conn) request(req *http.Request, fn func(*http.Response)) error {
	callID, b, err := request(req)
	if err != nil {
		return err
	}

	ch := s.setCallID(callID)
	defer s.DelCallID(callID)

	// 发送广播消息
	_, err = s.conn.WriteTo(b, s.to)
	if err != nil {
		return err
	}
	for {
		select {
		case resp := <-ch:
			resp.Request = req
			fn(resp)
			resp.Body.Close()
		case <-time.After(5 * time.Second):
			return nil
		}
	}
}

type Receiver[T any] struct {
	response chan T
	timeout  time.Duration
	// lastWriteAt time.Time

	timer *time.Timer
	m     sync.Mutex
	once  sync.Once
}

func NewReceiver[T any](timeout time.Duration) *Receiver[T] {
	return &Receiver[T]{
		timeout:  timeout,
		response: make(chan T, 10),
		timer:    time.NewTimer(timeout),
	}
}

func (r *Receiver[T]) Close() {
	r.once.Do(func() {
		close(r.response)
	})
}

// 只有写入时才能关闭，读取时应该也允许关闭
func (r *Receiver[T]) Write(t T) {
	r.m.Lock()
	defer r.m.Unlock()

	select {
	case <-r.timer.C:
		r.Close()
		return
	default:
		r.response <- t
		r.timer.Reset(r.timeout)
	}
}
