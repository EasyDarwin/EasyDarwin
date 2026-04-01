package chanx

import (
	"context"
	"sync/atomic"
)

// UnboundedChan is an unbounded chan.
// In is used to write without blocking, which supports multiple writers.
// and Out is used to read, which supports multiple readers.
// You can close the in channel if you want.
type UnboundedChan[T any] struct {
	bufCount int64
	In       chan<- T       // channel for write
	Out      <-chan T       // channel for read
	buffer   *RingBuffer[T] // buffer
}

// Len returns len of In plus len of Out plus len of buffer.
// It is not accurate and only for your evaluating approximate number of elements in this chan,
// see https://github.com/smallnest/chanx/issues/7.
func (c UnboundedChan[T]) Len() int {
	return len(c.In) + c.BufLen() + len(c.Out)
}

// BufLen returns len of the buffer.
// It is not accurate and only for your evaluating approximate number of elements in this chan,
// see https://github.com/smallnest/chanx/issues/7.
func (c UnboundedChan[T]) BufLen() int {
	return int(atomic.LoadInt64(&c.bufCount))
}

// NewUnboundedChan creates the unbounded chan.
// in is used to write without blocking, which supports multiple writers.
// and out is used to read, which supports multiple readers.
// You can close the in channel if you want.
func NewUnboundedChan[T any](ctx context.Context, initCapacity int) *UnboundedChan[T] {
	return NewUnboundedChanSize[T](ctx, initCapacity, initCapacity, initCapacity)
}

// NewUnboundedChanSize is like NewUnboundedChan but you can set initial capacity for In, Out, Buffer.
func NewUnboundedChanSize[T any](ctx context.Context, initInCapacity, initOutCapacity, initBufCapacity int) *UnboundedChan[T] {
	in := make(chan T, initInCapacity)
	out := make(chan T, initOutCapacity)
	ch := UnboundedChan[T]{In: in, Out: out, buffer: NewRingBuffer[T](initBufCapacity)}

	go process(ctx, in, out, &ch)

	return &ch
}

func process[T any](ctx context.Context, in, out chan T, ch *UnboundedChan[T]) {
	defer close(out)
	drain := func() {
		for !ch.buffer.IsEmpty() {
			select {
			case out <- ch.buffer.Pop():
				atomic.AddInt64(&ch.bufCount, -1)
			case <-ctx.Done():
				return
			}
		}

		ch.buffer.Reset()
		atomic.StoreInt64(&ch.bufCount, 0)
	}
	for {
		select {
		case <-ctx.Done():
			return
		case val, ok := <-in:
			if !ok { // in is closed
				drain()
				return
			}

			// make sure values' order
			// buffer has some values
			if atomic.LoadInt64(&ch.bufCount) > 0 {
				ch.buffer.Write(val)
				atomic.AddInt64(&ch.bufCount, 1)
			} else {
				// out is not full
				select {
				case out <- val:
					continue
				default:
				}

				// out is full
				ch.buffer.Write(val)
				atomic.AddInt64(&ch.bufCount, 1)
			}

			for !ch.buffer.IsEmpty() {
				select {
				case <-ctx.Done():
					return
				case val, ok := <-in:
					if !ok { // in is closed
						drain()
						return
					}
					ch.buffer.Write(val)
					atomic.AddInt64(&ch.bufCount, 1)

				case out <- ch.buffer.Peek():
					ch.buffer.Pop()
					atomic.AddInt64(&ch.bufCount, -1)
					if ch.buffer.IsEmpty() && ch.buffer.size > ch.buffer.initialSize { // after burst
						ch.buffer.Reset()
						atomic.StoreInt64(&ch.bufCount, 0)
					}
				}
			}
		}
	}
}
