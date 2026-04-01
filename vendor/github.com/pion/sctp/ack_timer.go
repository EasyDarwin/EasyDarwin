// SPDX-FileCopyrightText: 2023 The Pion community <https://pion.ly>
// SPDX-License-Identifier: MIT

package sctp

import (
	"math"
	"sync"
	"time"
)

const (
	ackInterval time.Duration = 200 * time.Millisecond
)

// ackTimerObserver is the inteface to an ack timer observer.
type ackTimerObserver interface {
	onAckTimeout()
}

type ackTimerState int

const (
	ackTimerStopped ackTimerState = iota
	ackTimerStarted
	ackTimerClosed
)

// ackTimer provides the retnransmission timer conforms with RFC 4960 Sec 6.3.1
type ackTimer struct {
	observer ackTimerObserver
	mutex    sync.RWMutex
	state    ackTimerState
	timer    *time.Timer
}

// newAckTimer creates a new acknowledgement timer used to enable delayed ack.
func newAckTimer(observer ackTimerObserver) *ackTimer {
	t := &ackTimer{observer: observer}
	t.timer = time.AfterFunc(math.MaxInt64, t.timeout)
	t.timer.Stop()
	return t
}

func (t *ackTimer) timeout() {
	t.mutex.Lock()
	if t.state == ackTimerStarted {
		t.state = ackTimerStopped
		defer t.observer.onAckTimeout()
	}
	t.mutex.Unlock()
}

// start starts the timer.
func (t *ackTimer) start() bool {
	t.mutex.Lock()
	defer t.mutex.Unlock()

	// this timer is already closed or already running
	if t.state != ackTimerStopped {
		return false
	}

	t.state = ackTimerStarted
	t.timer.Reset(ackInterval)
	return true
}

// stops the timer. this is similar to stop() but subsequent start() call
// will fail (the timer is no longer usable)
func (t *ackTimer) stop() {
	t.mutex.Lock()
	defer t.mutex.Unlock()

	if t.state == ackTimerStarted {
		t.timer.Stop()
		t.state = ackTimerStopped
	}
}

// closes the timer. this is similar to stop() but subsequent start() call
// will fail (the timer is no longer usable)
func (t *ackTimer) close() {
	t.mutex.Lock()
	defer t.mutex.Unlock()

	if t.state == ackTimerStarted {
		t.timer.Stop()
	}
	t.state = ackTimerClosed
}

// isRunning tests if the timer is running.
// Debug purpose only
func (t *ackTimer) isRunning() bool {
	t.mutex.RLock()
	defer t.mutex.RUnlock()

	return t.state == ackTimerStarted
}
