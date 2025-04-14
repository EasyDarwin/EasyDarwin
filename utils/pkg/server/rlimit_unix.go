//go:build !windows
// +build !windows

// Package rlimit contains a function to raise rlimit.
package server

import (
	"syscall"
)

// Raise raises the number of file descriptors that can be opened.
func Raise(limit uint64) error {
	var rlim syscall.Rlimit
	err := syscall.Getrlimit(syscall.RLIMIT_NOFILE, &rlim)
	if err != nil {
		return err
	}

	rlim.Cur = limit
	if rlim.Cur < 4096 {
		rlim.Cur = 4096
	}
	err = syscall.Setrlimit(syscall.RLIMIT_NOFILE, &rlim)
	if err != nil {
		return err
	}

	err = syscall.Getrlimit(syscall.RLIMIT_NOFILE, &rlim)
	if err != nil {
		return err
	}

	return nil
}
