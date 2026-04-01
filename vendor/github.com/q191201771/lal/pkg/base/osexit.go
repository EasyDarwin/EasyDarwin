// Copyright 2021, Chef.  All rights reserved.
// https://github.com/q191201771/lal
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

package base

import (
	"bufio"
	"fmt"
	"os"
	"runtime"
)

func OsExitAndWaitPressIfWindows(code int) {
	if runtime.GOOS == "windows" {
		_, _ = fmt.Fprintf(os.Stderr, "Press Enter to exit...")
		r := bufio.NewReader(os.Stdin)
		_, _ = r.ReadByte()
	}
	os.Exit(code)
}
