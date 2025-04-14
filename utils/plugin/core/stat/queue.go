package stat

import (
	"sync"

	"easydarwin/lnton/pkg/orm"
)

// PercentData 时间百分比
type PercentData struct {
	Time orm.Time `json:"time"`
	Used float64  `json:"use"`
}

// CircleQueue 环形队列
type CircleQueue struct {
	idx, maxSize uint8
	over         bool
	array        []PercentData
	m            sync.RWMutex
}

// NewCircleQueue ...
func NewCircleQueue(size uint8) *CircleQueue {
	return &CircleQueue{
		maxSize: size,
		array:   make([]PercentData, size),
	}
}

// Push 入队, 当队列满时, 覆盖旧数据
func (c *CircleQueue) Push(v PercentData) {
	c.m.Lock()
	defer c.m.Unlock()

	if c.idx == c.maxSize-1 && !c.over {
		c.over = true
	}
	c.array[c.idx] = v
	c.idx = (c.idx + 1) % c.maxSize
}

func (c *CircleQueue) Last() *PercentData {
	s := c.Range()
	if l := len(s); l > 0 {
		return &s[l-1]
	}
	return nil
}

// Range 取出队列中所有数据
func (c *CircleQueue) Range() []PercentData {
	c.m.RLock()
	defer c.m.RUnlock()
	size := c.size()
	if size == 0 {
		return nil
	}

	var idx uint8
	if c.over {
		idx = c.idx
	}
	data := make([]PercentData, 0, size)
	for i := 0; i < size; i++ {
		data = append(data, c.array[idx])
		idx = (idx + 1) % c.maxSize
	}
	return data
}

func (c *CircleQueue) size() int {
	if !c.over {
		return int(c.idx)
	}
	return int(c.maxSize)
}
