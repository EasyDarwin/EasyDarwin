package system

import (
	"log/slog"
	"os"
	"path/filepath"
	"sort"

	"github.com/shirou/gopsutil/disk"
)

// 弃用，为旧版本预留
var GetCWD = Executable

// Executable 获取可执行文件绝对路径
func Executable() string {
	bin, _ := os.Executable()
	return filepath.Dir(bin)
}

// Getwd 获取工作目录
func Getwd() string {
	dir, _ := os.Getwd()
	return dir
}

// GetDirSize 获取目录大小，单位 Bit
func GetDirSize(path string) (int64, error) {
	var size int64
	err := filepath.Walk(path, func(_ string, info os.FileInfo, err error) error {
		if err != nil {
			return err
		}
		if !info.IsDir() {
			size += info.Size()
		}
		return err
	})
	return size, err
}

type FileInfo struct {
	os.FileInfo
	Path string
}

// GlobFiles 按照升序排列所有文件
func GlobFiles(path string) ([]FileInfo, error) {
	files := make([]FileInfo, 0, 8)
	err := filepath.Walk(path, func(ppath string, info os.FileInfo, err error) error {
		if err != nil {
			return err
		}
		if info.IsDir() {
			return nil
		}
		files = append(files, FileInfo{
			FileInfo: info,
			Path:     ppath,
		})
		return nil
	})
	if err != nil {
		return nil, err
	}
	// 按照文件的修改时间升序排序
	sort.Slice(files, func(i, j int) bool {
		return files[i].ModTime().Before(files[j].ModTime())
	})
	return files, nil
}

func CleanOldFiles(files []FileInfo, count int) (int, error) {
	logs := make([]string, 0, count)

	// 删除文件
	num := count
	for _, file := range files {
		if err := os.Remove(file.Path); err != nil {
			slog.Error("文件删除失败", "err", err)
			continue
		}
		num--
		logs = append(logs, file.Path)
		if num <= 0 {
			break
		}
	}

	if len(logs) > 0 {
		slog.Info("删除旧文件", "logs", logs)
	}

	return count - num, nil
}

// DiskUsagePercent 获取磁盘使用率
func DiskUsagePercent(path string) (float64, error) {
	usage, err := disk.Usage(path)
	if err != nil {
		return 0, err
	}
	return usage.UsedPercent, nil
}

// RemoveEmptyDirs 删除空目录
func RemoveEmptyDirs(rootDir string) error {
	// 遍历目录树，按反向顺序处理目录
	return filepath.Walk(rootDir, func(path string, info os.FileInfo, err error) error {
		if err != nil {
			return err
		}

		if !info.IsDir() {
			return nil
		}
		// 检查目录是否为空
		entries, err := os.ReadDir(path)
		if err != nil {
			return err
		}
		if len(entries) == 0 {
			// 删除空目录
			if err := os.Remove(path); err != nil {
				slog.Error("删除空目录出错", "err", err)
			}
		}
		return nil
	})
}

// Abs 获取绝对目录
// 与 filepath.Abs 的区别是，这个以可执行文件目录为工作目录
func Abs(path string) (string, error) {
	if filepath.IsAbs(path) {
		return filepath.Clean(path), nil
	}
	bin, err := os.Executable()
	if err != nil {
		return "", err
	}
	return filepath.Join(filepath.Dir(bin), path), nil
}
