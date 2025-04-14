package queue

import (
	"fmt"
	"testing"

	"github.com/stretchr/testify/require"
)

func TestPush(t *testing.T) {
	q := NewCirQueue[int](5)

	for i := 1; i <= 100; i++ {
		q.Push(i)
	}

	require.EqualValues(t, q.Size(), 5)
	fmt.Println(q.Range())
}
