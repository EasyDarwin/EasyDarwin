// Author: xiexu
// Date: 2024-05-01

package conc

import (
	"context"
	"errors"
	"testing"
	"time"
)

func TestG(t *testing.T) {
	g := New(nil)
	g.GoRun(func() {
		panic("test")
	})
	g.GoRun(func() {
		panic("test1")
	})
	g.Wait()
}

func TestUnsafeWaitWithContext(t *testing.T) {
	g := New(nil)
	g.GoRun(func() {
		time.Sleep(10 * time.Second)
	})
	g.GoRun(func() {
		time.Sleep(10 * time.Second)
	})

	ctx, cancel := context.WithTimeout(context.Background(), 3*time.Second)
	defer cancel()
	if err := g.UnsafeWaitWithContext(ctx); err != nil {
		if !errors.Is(err, context.DeadlineExceeded) {
			t.Fatal(err)
		}
	}
}
