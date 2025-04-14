package sq

// // RichQueue 请求与响应 1 对多
// type RichQueue[T any] struct {
// 	streams map[string]*RichStream[T]
// 	m       sync.RWMutex
// }

// type RichStream[T any] struct {
// 	q        Deler
// 	ID       string
// 	Ctx      context.Context
// 	Response chan T
// }

// func NewRichQueue[T any]() *RichQueue[T] {
// 	return &RichQueue[T]{
// 		streams: make(map[string]*RichStream[T]),
// 	}
// }

// func NewRichStream[T any](ctx context.Context, id string) *RichStream[T] {
// 	return &RichStream[T]{
// 		ID:  id,
// 		Ctx: ctx,
// 	}
// }

// func (s *RichQueue[T]) GetStream(key string) *RichStream[T] {
// 	s.m.RLock()
// 	defer s.m.RUnlock()
// 	return s.streams[key]
// }

// func (s *RichQueue[T]) setStream(key string, stream *RichStream[T]) {
// 	if stream != nil {
// 		stream.q = s
// 		if stream.Ctx == nil {
// 			panic("stream ctx need not nil")
// 		}
// 	}

// 	s.m.Lock()
// 	defer s.m.Unlock()
// 	s.streams[key] = stream
// }

// func (s *RichQueue[T]) DelStream(key string) {
// 	s.m.Lock()
// 	defer s.m.Unlock()
// 	delete(s.streams, key)
// }

// func (s *RichStream[T]) PushReponse(t T) {
// 	s.Response <- t
// }
