// Copyright 2022, Chef.  All rights reserved.
// https://github.com/q191201771/lal
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

package logic

import (
	"fmt"
	"path/filepath"

	"github.com/q191201771/lal/pkg/mpegts"
)

// startRecordMpegtsIfNeeded 必要时开启ts录制
func (group *Group) startRecordMpegtsIfNeeded(nowUnix int64) {
	if !group.config.RecordConfig.EnableMpegts {
		return
	}

	// 构造文件名
	filename := fmt.Sprintf("%s-%d.ts", group.streamName, nowUnix)
	filenameWithPath := filepath.Join(group.config.RecordConfig.MpegtsOutPath, filename)

	group.recordMpegts = &mpegts.FileWriter{}
	if err := group.recordMpegts.Create(filenameWithPath); err != nil {
		Log.Errorf("[%s] record mpegts open file failed. filename=%s, err=%+v",
			group.UniqueKey, filenameWithPath, err)
		group.recordMpegts = nil
	}
}

func (group *Group) stopRecordMpegtsIfNeeded() {
	if !group.config.RecordConfig.EnableMpegts {
		return
	}

	if group.recordMpegts != nil {
		_ = group.recordMpegts.Dispose()
		group.recordMpegts = nil
	}
}
