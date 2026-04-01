package gohlslib

import (
	"context"
	"sync"
	"time"
)

type segmentData struct {
	dateTime *time.Time
	payload  []byte
}

type clientSegmentQueue struct {
	mutex   sync.Mutex
	queue   []*segmentData
	didPush chan struct{}
	didPull chan struct{}
}

func (q *clientSegmentQueue) initialize() {
	q.didPush = make(chan struct{})
	q.didPull = make(chan struct{})
}

func (q *clientSegmentQueue) push(seg *segmentData) {
	q.mutex.Lock()

	queueWasEmpty := (len(q.queue) == 0)
	q.queue = append(q.queue, seg)

	if queueWasEmpty {
		close(q.didPush)
		q.didPush = make(chan struct{})
	}

	q.mutex.Unlock()
}

func (q *clientSegmentQueue) waitUntilSizeIsBelow(ctx context.Context, n int) bool {
	q.mutex.Lock()

	for len(q.queue) > n {
		q.mutex.Unlock()

		select {
		case <-q.didPull:
		case <-ctx.Done():
			return false
		}

		q.mutex.Lock()
	}

	q.mutex.Unlock()
	return true
}

func (q *clientSegmentQueue) pull(ctx context.Context) (*segmentData, bool) {
	q.mutex.Lock()

	for len(q.queue) == 0 {
		didPush := q.didPush
		q.mutex.Unlock()

		select {
		case <-didPush:
		case <-ctx.Done():
			return nil, false
		}

		q.mutex.Lock()
	}

	var seg *segmentData
	seg, q.queue = q.queue[0], q.queue[1:]

	close(q.didPull)
	q.didPull = make(chan struct{})

	q.mutex.Unlock()
	return seg, true
}
