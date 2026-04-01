// Copyright 2019, Chef.  All rights reserved.
// https://github.com/q191201771/lal
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

package hls

import (
	"github.com/q191201771/lal/pkg/base"
)

// TODO chef:
// - 补充单元测试
// - 配置项
// - Server
//     - 超时时间

// https://developer.apple.com/documentation/http_live_streaming/example_playlists_for_http_live_streaming/incorporating_ads_into_a_playlist
// https://developer.apple.com/documentation/http_live_streaming/example_playlists_for_http_live_streaming/event_playlist_construction
// #EXTM3U                     // 固定串
// #EXT-X-VERSION:3            // 固定串
// #EXT-X-MEDIA-SEQUENCE       // 列表中首个TS文件的序号（该序号指的是+1自增的序号，从首个TS文件开始，包含已经从列表中移除的TS文件）
// #EXT-X-TARGETDURATION       // 所有TS文件，最长的时长
// #EXT-X-PLAYLIST-TYPE: EVENT
// #EXT-X-DISCONTINUITY        //
// #EXTINF:                    // 时长以及TS文件名

// 进来的数据称为Frame帧，188字节的封装称为TSPacket包，TS文件称为Fragment

const (
	// TODO chef 这些在配置项中提供
	negMaxfraglen             uint64 = 1000 * 90 // 当前包时间戳回滚了，比当前fragment的首个时间戳还小，强制切割新的fragment，单位（毫秒*90）
	maxAudioCacheDelayByAudio uint64 = 150 * 90  // 单位（毫秒*90）
	maxAudioCacheDelayByVideo uint64 = 300 * 90  // 单位（毫秒*90）
)

func SplitFragment2TsPackets(content []byte) (ret [][]byte, err error) {
	if len(content)%188 != 0 {
		err = base.ErrHls
		return
	}
	for {
		if len(content) == 0 {
			break
		}

		ret = append(ret, content[0:188])
		content = content[188:]
	}
	return
}
