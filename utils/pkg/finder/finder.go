// Author: xiexu
// Date: 2024-05-01

package finder

import (
	"context"
	"fmt"
	"io"
	"log/slog"
	"os"
	"path/filepath"
	"strings"
	"time"

	"easydarwin/lnton/pkg/conc"
	"easydarwin/lnton/pkg/system"
)

// Engine 文件管理器
type Engine struct {
	prefix string
	ctx    context.Context
	cancel context.CancelFunc
	g      *conc.G

	limit chan struct{}

	deadlineDurationTimer time.Duration // 遍历时间
	diskUsagePercentLimit float32       // 磁盘使用率限制
	deleteElements        int           // 达到限制后删除几个元素
	expire                time.Duration // 文件过期时间
}

// SetDiskUsagePercentLimit 设置磁盘使用率
func (e *Engine) SetDiskUsagePercentLimit(limit float32) *Engine {
	e.diskUsagePercentLimit = limit
	return e
}

// SetDeleteElements 设置删除元素数量
func (e *Engine) SetDeleteElements(num int) *Engine {
	e.deleteElements = num
	return e
}

// SetDeadlineDuration 设置遍历等待时间
func (e *Engine) SetDeadlineDuration(d time.Duration) *Engine {
	e.deadlineDurationTimer = d
	return e
}

// SetExpire 设置文件过期删除时间
func (e *Engine) SetExpire(d time.Duration) *Engine {
	e.expire = d
	return e
}

// SetPrefix 设置文件夹
func (e *Engine) SetPrefix(prefix string) *Engine {
	e.prefix = prefix
	return e
}

// NewEngine 传入受管理的文件夹，最好是绝对路径，以及文件过期时间
func NewEngine(prefix string, expire time.Duration) *Engine {
	_ = os.MkdirAll(prefix, os.ModePerm)
	ctx, cancel := context.WithCancel(context.Background())
	e := Engine{
		prefix:                prefix,
		expire:                expire,
		ctx:                   ctx,
		cancel:                cancel,
		g:                     conc.New(nil),
		limit:                 make(chan struct{}, 1),
		deadlineDurationTimer: time.Minute * 10,
		diskUsagePercentLimit: 85, // 保守一点
		deleteElements:        5,
	}
	e.g.GoRun(e.deadline)
	e.g.GoRun(e.overWrite)
	return &e
}

func (e *Engine) Close() {
	e.cancel()
	e.g.Wait()
}

func (e *Engine) Prefix() string {
	return e.prefix
}

// deadline 超时删除
func (e *Engine) deadline() {
	ticker := time.NewTimer(e.deadlineDurationTimer)
	defer ticker.Stop()
	for {
		select {
		case <-e.ctx.Done():
			return
		case <-ticker.C:
			if err := filepath.Walk(e.prefix, func(path string, info os.FileInfo, _ error) error {
				if info == nil || info.IsDir() {
					return nil
				}
				if time.Since(info.ModTime()) > e.expire {
					_ = os.RemoveAll(path)
				}
				return nil
			}); err != nil {
				slog.Error("deadline", "err", err)
			}
			ticker.Reset(e.deadlineDurationTimer)
		}
	}
}

func (e *Engine) OverWrite(limit int) {
	select {
	case e.limit <- struct{}{}:
	default:

	}
}

func (e *Engine) overWrite() {
	for {
		select {
		case <-e.ctx.Done():
			return
		case <-e.limit:
			var files []system.FileInfo
			for {
				got, err := system.DiskUsagePercent(e.prefix)
				if err != nil {
					slog.Error("DiskUsagePercent", "err", err)
					continue
				}
				if float32(got) < e.diskUsagePercentLimit {
					// 如果磁盘使用率低于限制很多，可以延缓
					s := e.diskUsagePercentLimit - float32(got)
					switch true {
					case s > 30:
						time.Sleep(5 * time.Minute)
					case s > 10:
						time.Sleep(time.Minute)
					case s > 6:
						time.Sleep(10 * time.Second)
					}
					break
				}

				if files == nil {
					files, err = system.GlobFiles(e.prefix)
					if err != nil {
						slog.Error("GlobFiles", "err", err)
						continue
					}
				}

				count, err := system.CleanOldFiles(files, e.deleteElements)
				if err != nil {
					slog.Error("CleanOldFiles", "err", err)
				}
				if count <= len(files) {
					files = files[count:]
				}
				if count < e.deleteElements {
					break
				}
			}
		}
	}
}

// // DeleteFile 删除文件
// func (e *Engine) DeleteFile(path string) error {
// 	return os.RemoveAll(filepath.Join(e.prefix, path))
// }

// WriteFile 保存文件
func (e *Engine) WriteFile(path string, b []byte) error {
	return os.WriteFile(filepath.Join(e.prefix, path), b, 0o600)
}

// ReadFile 读取文件
func (e *Engine) ReadFile(path string) ([]byte, error) {
	if path == "" {
		return nil, fmt.Errorf("文件不存在")
	}
	return os.ReadFile(filepath.Join(e.prefix, path)) // nolint
}

func (e *Engine) MkdirAll(name string) *Engine {
	_ = os.MkdirAll(filepath.Join(e.prefix, name), 0o755)
	return e
}

// CreateFile 创建文件
func (e *Engine) CreateFile(name string) (*os.File, error) {
	return os.Create(filepath.Join(e.prefix, name)) // nolint
}

func (e *Engine) OpenFile(name string) (*os.File, error) {
	return os.Open(filepath.Join(e.prefix, name))
}

// FileTag 文件信息
type FileTag struct {
	Name    string
	Data    []byte // 待弃用
	ModTime time.Time
	Path    string
}

// FindFile 查找文件
func (e *Engine) FindFile(p string) (FileTag, error) {
	var f FileTag
	var filename string
	_ = filepath.Walk(e.prefix, func(path string, info os.FileInfo, err error) error {
		if info.IsDir() {
			return nil
		}
		if strings.HasSuffix(path, p) {
			filename = path
			f.ModTime = info.ModTime()
			return filepath.SkipDir
		}
		return nil
	})

	if filename == "" {
		return f, io.EOF // 文件不存在
	}
	out, err := os.ReadFile(filename) // nolint
	f.Name = filepath.Base(filename)
	f.Data = out
	return f, err
}

// FindFileInfo 查找文件
func (e *Engine) FindFileInfo(p string) (FileTag, error) {
	var f FileTag
	var filename string
	_ = filepath.Walk(e.prefix, func(path string, info os.FileInfo, err error) error {
		if info.IsDir() {
			return nil
		}
		if strings.HasSuffix(path, p) {
			filename = path
			f.ModTime = info.ModTime()
			return filepath.SkipDir
		}
		return nil
	})

	if filename == "" {
		return f, io.EOF // 文件不存在
	}
	// out, err := os.ReadFile(filename) // nolint
	f.Name = filepath.Base(filename)
	f.Path = filename
	// f.Data = out
	return f, nil
}
