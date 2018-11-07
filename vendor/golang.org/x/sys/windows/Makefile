# Copyright 2014 The Go Authors. All rights reserved.
# Use of this source code is governed by a BSD-style
# license that can be found in the LICENSE file.

all: zsyscall_windows.go

TMP=/tmp/mksyscall_windows

zsyscall_windows.go: syscall_windows.go security_windows.go
	go build -o $(TMP) $(GOROOT)/src/syscall/mksyscall_windows.go
	GOOS=windows $(TMP) $^ | gofmt > $@
	rm $(TMP)
