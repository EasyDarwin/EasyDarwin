:: Copyright 2013 The Go Authors. All rights reserved.
:: Use of this source code is governed by a BSD-style
:: license that can be found in the LICENSE file.
@echo off

if exist syscall_windows.go goto dirok
echo mkall_windows.bat must be run from go.sys\windows directory
goto :end
:dirok

go build -o mksyscall_windows.exe %GOROOT%\src\pkg\syscall\mksyscall_windows.go
mksyscall_windows.exe syscall_windows.go security_windows.go |gofmt >zsyscall_windows.go
del mksyscall_windows.exe

:end
