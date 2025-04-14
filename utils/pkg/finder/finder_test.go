// Author: xiexu
// Date: 2024-05-01

package finder

import (
	"fmt"
	"os"
	"strings"
	"testing"
	"time"

	"easydarwin/lnton/pkg/system"
)

func TestEngine(t *testing.T) {
	dir, _ := os.Getwd()
	os.Mkdir("test", os.ModePerm)

	s := NewEngine(dir+"/test", 1000*time.Second)
	s.SetDeadlineDuration(time.Second)
	// time.Sleep(time.Second * 3)
	fmt.Println(system.DiskUsagePercent(s.prefix))
	s.WriteFile("a.txt", []byte("123"))

	data := strings.Repeat("hello world", 1024*1024*10)
	for i := range 10 {
		s.WriteFile(fmt.Sprintf("a%d.txt", i), []byte(data))
	}

	s.SetDiskUsagePercentLimit(88.00)

	s.OverWrite(3)
	time.Sleep(5 * time.Second)

	fmt.Println(system.DiskUsagePercent(s.prefix))
	// out, err := s.ReadFile("a.txt")
	// if err != nil {
	// 	t.Fatal(err)
	// }
	// t.Log(string(out))
	// time.Sleep(time.Second * 6)

	s.Close()
	fmt.Println("end")
}
