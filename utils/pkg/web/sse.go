package web

import (
	"bytes"
	"context"
	"encoding/json"
	"fmt"
	"io"
	"net/http"
	"strings"
	"time"

	"github.com/gin-gonic/gin"
)

// SSE 发送事件
/*
	使用案例

	http.HandleFunc("/stream", func(w http.ResponseWriter, r *http.Request) {
		sse := web.NewSSE(1024, time.Minute)

		go func(){
			defer sse.Close()
			for range 3 {
				sse.Publish(web.Event{
					ID:    uuid.New().String(),
					Event: "ping",
					Data: []byte("pong"),
				})
				time.Sleep(time.Second)
			}
		}()
		sse.ServeHTTP(w, r)
	})


*/
type SSE struct {
	Headers map[string]string
	stream  chan Event
	timeout time.Duration
	cancel  context.CancelFunc
}

type Event struct {
	ID    string
	Event string
	Data  []byte
}

func NewSSE(length int, timeout time.Duration) *SSE {
	if length <= 0 {
		length = 1024
	}
	return &SSE{
		stream:  make(chan Event, length),
		timeout: timeout,
	}
}

func (s *SSE) Publish(v Event) {
	s.stream <- v
}

func (s *SSE) Close() {
	cancel := s.cancel
	if cancel != nil {
		s.cancel()
	}
	close(s.stream)
}

func (s *SSE) ServeHTTP(w http.ResponseWriter, req *http.Request) {
	rc := http.NewResponseController(w) // nolint
	_ = rc.SetWriteDeadline(time.Now().Add(s.timeout))

	w.Header().Set("Content-Type", "text/event-stream")
	w.Header().Set("Cache-Control", "no-cache")
	w.Header().Set("Connection", "keep-alive")

	for k, v := range s.Headers {
		w.Header().Set(k, v)
	}

	ctx, cancel := context.WithCancel(req.Context())
	s.cancel = cancel

	for {
		select {
		case <-ctx.Done():
			return
		case ev := <-s.stream:
			if len(ev.Data) == 0 {
				continue
			}
			if len(ev.ID) > 0 {
				fmt.Fprintf(w, "id: %s\n", ev.ID)
			}
			if len(ev.Event) > 0 {
				fmt.Fprintf(w, "event: %s\n", ev.Event)
			}
			fmt.Fprintf(w, "data: %s\n", ev.Data)
			fmt.Fprint(w, "\n")
			rc.Flush()
		}
	}
}

type EventMessage struct {
	id    string
	event string
	data  string
}

func (m *EventMessage) prepareMessage() []byte {
	var data bytes.Buffer
	if len(m.id) > 0 {
		data.WriteString(fmt.Sprintf("id: %s\n", strings.Replace(m.id, "\n", "", -1)))
	}
	if len(m.event) > 0 {
		data.WriteString(fmt.Sprintf("event: %s\n", strings.Replace(m.event, "\n", "", -1)))
	}
	if len(m.data) > 0 {
		lines := strings.Split(m.data, "\n")
		for _, line := range lines {
			data.WriteString(fmt.Sprintf("data: %s\n", line))
		}
	}
	data.WriteString("\n")
	return data.Bytes()
}

func NewEventMessage(event string, data map[string]any) *EventMessage {
	b, _ := json.Marshal(data)
	return &EventMessage{
		event: event,
		data:  string(b),
	}
}

func SendSSE(ch <-chan EventMessage, c *gin.Context) {
	c.Header("Cache-Control", "no-store")
	c.Header("Content-Type", "text/event-stream")
	tick := time.NewTicker(40 * time.Millisecond)
	defer tick.Stop()
	var last *EventMessage
	var zero EventMessage
	for {
		select {
		case <-tick.C:
			if last != nil {
				_, _ = io.WriteString(c.Writer, fmt.Sprintf("%v\n", *last))
				c.Writer.Flush()
				last = nil
			}
		case v := <-ch:
			if v != zero {
				last = &v
				continue
			}
			if last != nil {
				_, _ = io.WriteString(c.Writer, fmt.Sprintf("%v\n", *last))
				c.Writer.Flush()
			}
			return
		}
	}
}

type Chunk struct {
	Total   int    `json:"total"`
	Current int    `json:"current"`
	Success int    `json:"success"`
	Failure int    `json:"failure"`
	Err     string `json:"err,omitempty"`
}

// SendChunkPro 高性能版
func SendChunkPro(ch <-chan Chunk, c *gin.Context) {
	if c == nil || c.Writer == nil {
		return
	}
	tick := time.NewTicker(40 * time.Millisecond)
	defer tick.Stop()
	var last *Chunk
	var zero Chunk
	var i int
	for {
		if i == 1 {
			c.Header("Cache-Control", "no-store")
			c.Header("Transfer-Encoding", "chunked")
			c.Header("Content-Type", "text/plain")
		}
		select {
		case <-tick.C:
			if last != nil {
				b, _ := json.Marshal(last)
				_, err := c.Writer.Write(append(b, '\n'))
				if err != nil {
					return
				}
				c.Writer.Flush()
				last = nil
			}
		case v := <-ch:
			i++
			if v != zero {
				last = &v
				continue
			}
			if last != nil {
				b, _ := json.Marshal(last)
				_, err := c.Writer.Write(append(b, '\n'))
				if err != nil {
					return
				}
				c.Writer.Flush()
			}
			return
		}
	}
}

// SendChunk 发送分块数据
func SendChunk(ch <-chan Chunk, c *gin.Context) {
	if c == nil || c.Writer == nil {
		return
	}
	c.Header("Cache-Control", "no-store")
	c.Header("Transfer-Encoding", "chunked")
	c.Header("Content-Type", "text/plain")
	var zero Chunk
	var i int
	for {
		i++
		v := <-ch
		if v == zero {
			return
		}
		b, _ := json.Marshal(v)
		_, err := c.Writer.Write(append(b, '\n'))
		if err != nil {
			return
		}
		c.Writer.Flush()
	}
}
