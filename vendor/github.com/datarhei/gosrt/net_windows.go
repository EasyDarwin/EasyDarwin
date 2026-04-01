//go:build windows

package srt

import (
	"syscall"

	"golang.org/x/sys/windows"
)

func ListenControl(config Config) func(network, address string, c syscall.RawConn) error {
	return func(network, address string, c syscall.RawConn) error {
		var opErr error
		err := c.Control(func(fd uintptr) {
			// Set REUSEADDR
			opErr = windows.SetsockoptInt(windows.Handle(fd), windows.SOL_SOCKET, windows.SO_REUSEADDR, 1)
			if opErr != nil {
				return
			}

			// Set TOS
			if config.IPTOS > 0 {
				opErr = windows.SetsockoptInt(windows.Handle(fd), windows.IPPROTO_IP, windows.IP_TOS, config.IPTOS)
				if opErr != nil {
					return
				}
			}

			// Set TTL
			if config.IPTTL > 0 {
				opErr = windows.SetsockoptInt(windows.Handle(fd), windows.IPPROTO_IP, windows.IP_TTL, config.IPTTL)
				if opErr != nil {
					return
				}
			}
		})
		if err != nil {
			return err
		}
		return opErr
	}
}

func DialControl(config Config) func(network string, address string, c syscall.RawConn) error {
	return func(network, address string, c syscall.RawConn) error {
		var opErr error
		err := c.Control(func(fd uintptr) {
			// Set TOS
			if config.IPTOS > 0 {
				opErr = windows.SetsockoptInt(windows.Handle(fd), windows.IPPROTO_IP, windows.IP_TOS, config.IPTOS)
				if opErr != nil {
					return
				}
			}

			// Set TTL
			if config.IPTTL > 0 {
				opErr = windows.SetsockoptInt(windows.Handle(fd), windows.IPPROTO_IP, windows.IP_TTL, config.IPTTL)
				if opErr != nil {
					return
				}
			}
		})
		if err != nil {
			return err
		}
		return opErr
	}
}
