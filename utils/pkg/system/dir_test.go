package system

import (
	"fmt"
	"os"
	"path/filepath"
	"testing"
)

func TestFile(t *testing.T) {
	_ = os.MkdirAll("./test", 0o744)
	for i := range 20 {
		os.WriteFile(filepath.Join("./test", fmt.Sprintf("%d.txt", i)), []byte("123"), os.ModeAppend|os.ModePerm)
	}
	size, err := GetDirSize("./test")
	if err != nil {
		t.Fatal(err)
	}
	fmt.Println(size, "===", 3*20)
	// if err := CleanOldFiles("./test", 10); err != nil {
	// 	t.Fatal(err)
	// }
	// if err := CleanOldFiles("./test", 10); err != nil {
	// 	t.Fatal(err)
	// }
	// if err := CleanOldFiles("./test", 10); err != nil {
	// 	t.Fatal(err)
	// }
	RemoveEmptyDirs("./test")
}

func TestDiskUsagePercent(t *testing.T) {
	got, err := DiskUsagePercent("./")
	fmt.Println(got, err)
}
