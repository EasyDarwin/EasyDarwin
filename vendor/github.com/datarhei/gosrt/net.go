//go:build !windows

package srt

import "syscall"

func ListenControl(config Config) func(network, address string, c syscall.RawConn) error {
	return func(network, address string, c syscall.RawConn) error {
		var opErr error
		err := c.Control(func(fd uintptr) {
			// Set REUSEADDR
			opErr = syscall.SetsockoptInt(int(fd), syscall.SOL_SOCKET, syscall.SO_REUSEADDR, 1)
			if opErr != nil {
				return
			}

			// Set TOS
			if config.IPTOS > 0 {
				opErr = syscall.SetsockoptInt(int(fd), syscall.IPPROTO_IP, syscall.IP_TOS, config.IPTOS)
				if opErr != nil {
					return
				}
			}

			// Set TTL
			if config.IPTTL > 0 {
				opErr = syscall.SetsockoptInt(int(fd), syscall.IPPROTO_IP, syscall.IP_TTL, config.IPTTL)
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
				opErr = syscall.SetsockoptInt(int(fd), syscall.IPPROTO_IP, syscall.IP_TOS, config.IPTOS)
				if opErr != nil {
					return
				}
			}

			// Set TTL
			if config.IPTTL > 0 {
				opErr = syscall.SetsockoptInt(int(fd), syscall.IPPROTO_IP, syscall.IP_TTL, config.IPTTL)
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
