// Simple Queue
// 一个简单的事务队列，用于将响应匹配上请求
package sq

import (
	"context"
	"fmt"
	"sync"
)

// SimpleQueue 请求与响应一对一
type SimpleQueue[T any] struct {
	streams map[string]*Stream[T]
	m       sync.RWMutex
}

func NewSimpleQueue[T any]() *SimpleQueue[T] {
	return &SimpleQueue[T]{
		streams: make(map[string]*Stream[T]),
	}
}

func (s *SimpleQueue[T]) GetStream(key string) *Stream[T] {
	s.m.RLock()
	defer s.m.RUnlock()
	return s.streams[key]
}

func (s *SimpleQueue[T]) Request(key string, stream *Stream[T], fn func()) (T, error) {
	s.setStream(key, stream)
	fn()
	if stream != nil {
		return stream.wait()
	}
	var t T
	return t, fmt.Errorf("stream is nil")
}

func (s *SimpleQueue[T]) setStream(key string, stream *Stream[T]) {
	if stream != nil {
		stream.q = s
		if stream.Ctx == nil {
			panic("stream ctx need not nil")
		}
	}

	s.m.Lock()
	defer s.m.Unlock()
	s.streams[key] = stream
}

func (s *SimpleQueue[T]) DelStream(key string) {
	s.m.Lock()
	defer s.m.Unlock()
	delete(s.streams, key)
}

type Deler interface {
	DelStream(string)
}

// 一个双向流的会话
type Stream[T any] struct {
	q        Deler
	ID       string
	Ctx      context.Context
	active   chan struct{}
	response T
}

func NewStream[T any](ctx context.Context, id string) *Stream[T] {
	return &Stream[T]{
		ID:     id,
		Ctx:    ctx,
		active: make(chan struct{}),
	}
}

func (s *Stream[T]) SetReponse(t T) {
	s.response = t
	close(s.active)
}

func (s *Stream[T]) wait() (t T, err error) {
	select {
	case <-s.Ctx.Done():
		err = fmt.Errorf("ctx timeout")
	case <-s.active:
	}
	s.q.DelStream(s.ID)
	t = s.response
	return
}
