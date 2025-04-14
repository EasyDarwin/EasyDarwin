package queue

// CirQueue 环形队列，不支持并发安全，应该由调用者控制并发安全
type CirQueue[T any] struct {
	over bool
	idx  uint8
	size uint8
	data []T
}

func NewCirQueue[T any](size uint8) *CirQueue[T] {
	return &CirQueue[T]{
		size: size,
		data: make([]T, size),
	}
}

func (c *CirQueue[T]) Push(t T) {
	if c.idx == c.size-1 && !c.over {
		c.over = true
	}

	c.data[c.idx] = t
	c.idx = (c.idx + 1) % c.size
}

func (c *CirQueue[T]) Range() []T {
	size := c.Size()
	v := make([]T, size)

	idx := c.idx
	if !c.over {
		idx = 0
	}
	for i := 0; i < int(size); i++ {
		v[i] = c.data[idx]
		idx = (idx + 1) % c.size
	}
	return v
}

func (c *CirQueue[T]) Size() uint8 {
	if !c.over {
		return c.idx
	}
	return c.size
}

func (c *CirQueue[T]) IsFull() bool {
	return c.over
}
