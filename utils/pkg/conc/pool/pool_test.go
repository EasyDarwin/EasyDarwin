// Author: xiexu
// Date: 2024-05-01

package pool

import (
	"fmt"
	"testing"
)

func TestPool(t *testing.T) {
	pool := NewPool(10)
	for i := range 100 {
		pool.Go(func() {
			fmt.Println(i)
		})
	}
	pool.Wait()
	fmt.Println("continue")
	for i := range 10 {
		pool.Go(func() {
			fmt.Println(i)
		})
	}
	pool.Wait()
}
