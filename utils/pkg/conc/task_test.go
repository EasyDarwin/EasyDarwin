// Author: xiexu
// Date: 2024-05-01

package conc

import (
	"fmt"
	"testing"
	"time"
)

func TestNewAssemblyLine(t *testing.T) {
	a := NewAssemblyLine(5*time.Second, 10, 30)
	for range 100 {
		a.Add("1", func() error {
			fmt.Println("ok")
			return nil
		})
	}
	time.Sleep(5 * time.Second)
	a.Add("1", func() error {
		fmt.Println("1 ok")
		return nil
	})
	a.Add("2", func() error {
		fmt.Println("2 ok")
		return nil
	})
}
