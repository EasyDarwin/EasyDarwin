package api

import (
	"easydarwin/internal/data"
	"fmt"
	"github.com/gin-gonic/gin"
	"github.com/gorilla/websocket"
	"net"
	"net/http"
	"net/http/httputil"
	"strings"
	"time"
)

type ShutDownTransport struct {
	Trans    *http.Transport
	response *http.Response
}

// 覆盖上层 Transport
func (t *ShutDownTransport) RoundTrip(req *http.Request) (*http.Response, error) {
	res, err := t.Trans.RoundTrip(req)
	t.response = res
	// 删除代理层的 Access-Control-Allow-Origin
	/*t.response.Header.Del("Access-Control-Allow-Origin")
	  t.response.Header.Del("Access-Control-Allow-Credentials")*/
	t.response.Header = http.Header{}
	//fmt.Println("header ======================== ", t.response.Header)
	return res, err
}

// 实现关闭方法
func (t *ShutDownTransport) ShutDown(d time.Duration) {
	time.AfterFunc(d, func() {
		res := t.response
		if res != nil {
			if res.Body != nil {
				res.Body.Close()
			}
		}
	})
}

func FlvHandler() gin.HandlerFunc {
	return func(c *gin.Context) {
		path := c.Param("path")
		if strings.HasSuffix(path, ".flv") {
			addr := data.GetConfig().LogicCfg.DefaultHttpConfig.HttpListenAddr
			target := fmt.Sprintf("127.0.0.1%v", addr)
			//target := fmt.Sprintf("127.0.0.1:%v", 8080)
			director := func(req *http.Request) {
				req.URL.Scheme = "http"
				req.URL.Host = target
				req.URL.Path = path
			}
			modifyRes := func(res *http.Response) (err error) {
				res.Header.Del("Access-Control-Allow-Credentials")
				res.Header.Del("Access-Control-Allow-Headers")
				res.Header.Del("Access-Control-Allow-Methods")
				res.Header.Del("Access-Control-Allow-Origin")
				res.Header.Del("Vary")
				res.Header.Del("Server")
				return
			}
			transport := &ShutDownTransport{
				Trans: &http.Transport{
					Proxy: http.ProxyFromEnvironment,
					DialContext: (&net.Dialer{
						Timeout:   30 * time.Second,
						KeepAlive: 30 * time.Second,
					}).DialContext,
					ForceAttemptHTTP2:     true,
					MaxIdleConns:          100,
					IdleConnTimeout:       90 * time.Second,
					TLSHandshakeTimeout:   10 * time.Second,
					ExpectContinueTimeout: 1 * time.Second,
					ResponseHeaderTimeout: 10 * time.Second,
				},
			}

			proxy := &httputil.ReverseProxy{
				Director:       director,
				Transport:      transport,
				ModifyResponse: modifyRes,
			}
			proxy.ServeHTTP(c.Writer, c.Request)
			return
		}
		c.Next()
	}
}

// websocket拉流
var Grader = websocket.Upgrader{
	CheckOrigin: func(r *http.Request) bool {
		return true
	},
}

func WSFlvHandler() gin.HandlerFunc {
	return func(c *gin.Context) {
		ws, err := Grader.Upgrade(c.Writer, c.Request, nil)
		if err != nil {
			return
		}
		defer func() {
			fmt.Printf("关闭长连接")
			ws.Close()
		}()

		path := c.Param("path")
		if strings.HasSuffix(path, ".flv") {
			addr := data.GetConfig().LogicCfg.DefaultHttpConfig.HttpListenAddr
			target := fmt.Sprintf("ws://127.0.0.1%v%s", addr, path)
			conn, _, err := websocket.DefaultDialer.Dial(target, nil)
			if err != nil {
				fmt.Printf("websocket 连接错误:%s\n", err.Error())
				return
			}
			defer conn.Close()
			writeFlag := false
			readFunc := func() {
				opCode, _, err := ws.NextReader()
				if err != nil || opCode == websocket.CloseMessage {
					writeFlag = true
				}
			}
			go readFunc()
			for {
				if writeFlag {
					break
				}
				messageType, p, err := conn.ReadMessage()
				if err != nil {
					fmt.Printf("err:--%v", err)
					break
				}
				err = ws.WriteMessage(messageType, p)
				if err != nil {
					fmt.Printf("err:--%v", err)
					break
				}
			}
			return
		}
		c.Next()
	}
}

func HlsHandler() gin.HandlerFunc {
	return func(c *gin.Context) {
		path := c.Param("path")
		isM3u8 := strings.HasSuffix(path, ".m3u8")
		isTs := strings.HasSuffix(path, ".ts")
		if isM3u8 || isTs {
			addr := data.GetConfig().LogicCfg.DefaultHttpConfig.HttpListenAddr
			target := fmt.Sprintf("127.0.0.1%v", addr)
			director := func(req *http.Request) {
				req.URL.Scheme = "http"
				req.URL.Host = target
				req.URL.RawQuery = ""
				req.URL.Path = path
			}
			tranport := &ShutDownTransport{
				Trans: &http.Transport{
					Proxy: http.ProxyFromEnvironment,
					DialContext: (&net.Dialer{
						Timeout:   30 * time.Second,
						KeepAlive: 30 * time.Second,
					}).DialContext,
					ForceAttemptHTTP2:     true,
					MaxIdleConns:          100,
					IdleConnTimeout:       90 * time.Second,
					TLSHandshakeTimeout:   10 * time.Second,
					ExpectContinueTimeout: 1 * time.Second,
					ResponseHeaderTimeout: 10 * time.Second,
				},
			}

			errHandle := func(res http.ResponseWriter, req *http.Request, err error) {
				c.AbortWithError(http.StatusVariantAlsoNegotiates, err)
			}

			modifyRes := func(res *http.Response) (err error) {
				if isM3u8 {
					//res.Header.Set("Content-Type", "application/x-mpegurl")
					res.Header.Set("Content-Type", "application/vnd.apple.mpegurl")
				}
				if isTs {
					res.Header.Set("Content-Type", "video/mp2t")
				}
				return
			}

			proxy := &httputil.ReverseProxy{
				Director:       director,
				Transport:      tranport,
				ErrorHandler:   errHandle,
				ModifyResponse: modifyRes,
			}

			proxy.ServeHTTP(c.Writer, c.Request)

			c.Next()
		} else {
			c.AbortWithStatusJSON(http.StatusBadRequest, "BadRequest")
		}
		return
	}
}
