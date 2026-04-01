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

	"github.com/q191201771/lal/pkg/httpflv"
)

// startRecordFlvIfNeeded 必要时开启flv录制
func (group *Group) startRecordFlvIfNeeded(nowUnix int64) {
	if !group.config.RecordConfig.EnableFlv {
		return
	}

	// 构造文件名
	filename := fmt.Sprintf("%s-%d.flv", group.streamName, nowUnix)
	filenameWithPath := filepath.Join(group.config.RecordConfig.FlvOutPath, filename)

	// 初始化录制
	group.recordFlv = &httpflv.FlvFileWriter{}
	if err := group.recordFlv.Open(filenameWithPath); err != nil {
		Log.Errorf("[%s] record flv open file failed. filename=%s, err=%+v",
			group.UniqueKey, filenameWithPath, err)
		group.recordFlv = nil
		return
	}
	if err := group.recordFlv.WriteFlvHeader(); err != nil {
		Log.Errorf("[%s] record flv write flv header failed. filename=%s, err=%+v",
			group.UniqueKey, filenameWithPath, err)
		group.recordFlv = nil
	}
}

func (group *Group) stopRecordFlvIfNeeded() {
	if !group.config.RecordConfig.EnableFlv {
		return
	}

	if group.recordFlv != nil {
		_ = group.recordFlv.Dispose()
		group.recordFlv = nil
	}
}
