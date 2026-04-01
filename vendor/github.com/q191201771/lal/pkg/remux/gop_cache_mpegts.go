// Copyright 2022, Chef.  All rights reserved.
// https://github.com/q191201771/lal
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

package remux

// TODO(chef) 将gop_cache.go和gop_cache_mpegts.go的待完成项统一记录在这里
// - GopCache 和 GopCacheMpegts 尽量统一
// - 是否有必要单独存储帧，也即一个Gop的多个帧是一块内存，还是多块内存，从性能，功能，可读考虑
// - GopCache中非gop功能（包括meta和header的缓存）考虑移动到其他地方

type GopCacheMpegts struct {
	uniqueKey string
	gopNum    int

	gopRing              []GopMpegts
	gopRingFirst         int
	gopRingLast          int
	gopSize              int
	singleGopMaxFrameNum int
}

func NewGopCacheMpegts(uniqueKey string, gopNum int, singleGopMaxFrameNum int) *GopCacheMpegts {
	return &GopCacheMpegts{
		uniqueKey:            uniqueKey,
		gopNum:               gopNum,
		gopSize:              gopNum + 1,
		gopRing:              make([]GopMpegts, gopNum+1, gopNum+1),
		gopRingFirst:         0,
		gopRingLast:          0,
		singleGopMaxFrameNum: singleGopMaxFrameNum,
	}
}

// Feed
//
// @param b: 内部持有该内存块
func (gc *GopCacheMpegts) Feed(b []byte, boundary bool) {
	if gc.gopSize > 1 {
		if boundary {
			gc.feedNewGop(b)
		} else {
			gc.feedLastGop(b)
		}
	}
}

// GetGopCount 获取GOP数量，注意，最后一个可能是不完整的
func (gc *GopCacheMpegts) GetGopCount() int {
	return (gc.gopRingLast + gc.gopSize - gc.gopRingFirst) % gc.gopSize
}

func (gc *GopCacheMpegts) GetGopDataAt(pos int) [][]byte {
	if pos >= gc.GetGopCount() || pos < 0 {
		return nil
	}
	return gc.gopRing[(pos+gc.gopRingFirst)%gc.gopSize].data
}

func (gc *GopCacheMpegts) Clear() {
	gc.gopRingLast = 0
	gc.gopRingFirst = 0
}

// ---------------------------------------------------------------------------------------------------------------------

// feedLastGop
//
// 往最后一个GOP元素追加一个msg
// 注意，如果GopCache为空，则不缓存msg
func (gc *GopCacheMpegts) feedLastGop(b []byte) {
	if !gc.isGopRingEmpty() {
		gopPos := (gc.gopRingLast - 1 + gc.gopSize) % gc.gopSize
		if gc.gopRing[gopPos].len() <= gc.singleGopMaxFrameNum || gc.singleGopMaxFrameNum == 0 {
			gc.gopRing[gopPos].Feed(b)
		}

	}
}

// feedNewGop
//
// 生成一个最新的GOP元素，并往里追加一个msg
func (gc *GopCacheMpegts) feedNewGop(b []byte) {
	if gc.isGopRingFull() {
		gc.gopRingFirst = (gc.gopRingFirst + 1) % gc.gopSize
	}
	gc.gopRing[gc.gopRingLast].Clear()
	gc.gopRing[gc.gopRingLast].Feed(b)
	gc.gopRingLast = (gc.gopRingLast + 1) % gc.gopSize
}

func (gc *GopCacheMpegts) isGopRingFull() bool {
	return (gc.gopRingLast+1)%gc.gopSize == gc.gopRingFirst
}

func (gc *GopCacheMpegts) isGopRingEmpty() bool {
	return gc.gopRingFirst == gc.gopRingLast
}

// ---------------------------------------------------------------------------------------------------------------------

// GopMpegts
//
// 单个Gop，包含多帧数据
type GopMpegts struct {
	data [][]byte
}

// Feed
//
// @param b: 内部持有`b`内存块
func (g *GopMpegts) Feed(b []byte) {
	g.data = append(g.data, b)
}

func (g *GopMpegts) Clear() {
	g.data = g.data[:0]
}
func (g *GopMpegts) len() int {
	return len(g.data)
}
