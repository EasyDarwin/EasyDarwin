package system

import (
	"fmt"
	"log/slog"
	"os"
)

type FileBackup struct {
	sourcePath string
	backupPath string
	ch         chan []byte
	quit       chan struct{}
}

func NewFileBackup(file string) *FileBackup {
	f := FileBackup{
		sourcePath: file,
		backupPath: file + ".back",
		quit:       make(chan struct{}, 1),
		ch:         make(chan []byte, 1),
	}
	go f.start()
	return &f
}

func (f *FileBackup) start() {
	for {
		select {
		case <-f.quit:
			return
		case data := <-f.ch:
			fmt.Println(">>>>>>>")
			if err := os.WriteFile(f.backupPath, data, 0o600); err != nil {
				slog.Error("WriteFile", "err", err)
				continue
			}
			if err := os.Rename(f.backupPath, f.sourcePath); err != nil {
				slog.Error("Rename", "err", err)
				continue
			}
		}
	}
}

func (f *FileBackup) Close() {
	f.quit <- struct{}{}
}

func (f *FileBackup) Write(data []byte) {
	select {
	case f.ch <- data:
	default:
	}
}
