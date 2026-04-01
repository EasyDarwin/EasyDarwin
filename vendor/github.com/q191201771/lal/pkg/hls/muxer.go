// Copyright 2020, Chef.  All rights reserved.
// https://github.com/q191201771/lal
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

package hls

import (
	"bytes"
	"fmt"

	"github.com/q191201771/naza/pkg/nazaerrors"

	"github.com/q191201771/lal/pkg/mpegts"

	"github.com/q191201771/lal/pkg/base"
)

type IMuxerObserver interface {
	OnHlsMakeTs(info base.HlsMakeTsInfo)

	// OnFragmentOpen
	//
	// 内部决定开启新的fragment切片，将该事件通知给上层
	//
	// TODO(chef): [refactor] 考虑用OnHlsMakeTs替代OnFragmentOpen 202206
	//
	OnFragmentOpen()
}

// MuxerConfig
//
// 各字段含义见文档： https://pengrl.com/lal/#/ConfigBrief
type MuxerConfig struct {
	OutPath            string `json:"out_path"`
	FragmentDurationMs int    `json:"fragment_duration_ms"`
	FragmentNum        int    `json:"fragment_num"`
	DeleteThreshold    int    `json:"delete_threshold"`
	CleanupMode        int    `json:"cleanup_mode"` // TODO chef: lalserver的模式1的逻辑是在上层做的，应该重构到hls模块中
}

const (
	CleanupModeNever    = 0
	CleanupModeInTheEnd = 1
	CleanupModeAsap     = 2
)

// Muxer
//
// 输入mpegts流，输出hls(m3u8+ts)至文件中
type Muxer struct {
	UniqueKey string

	streamName                string // const after init
	outPath                   string // const after init
	playlistFilename          string // const after init
	playlistFilenameBak       string // const after init
	recordPlayListFilename    string // const after init
	recordPlayListFilenameBak string // const after init

	config   *MuxerConfig
	observer IMuxerObserver

	fragment Fragment
	videoCc  uint8
	audioCc  uint8

	// 初始值为false，调用openFragment时设置为true，调用closeFragment时设置为false
	// 整个对象关闭时设置为false
	// 中途切换Fragment时，调用close后会立即调用open
	opened bool

	fragTs                uint64 // 新建立fragment时的时间戳，毫秒 * 90
	recordMaxFragDuration float64

	nfrags int            // 该值代表直播m3u8列表中ts文件的数量
	frag   int            // frag 写入m3u8的EXT-X-MEDIA-SEQUENCE字段
	frags  []fragmentInfo // frags TS文件的固定大小环形队列，记录TS的信息

	patpmt []byte
}

// 记录fragment的一些信息，注意，写m3u8文件时可能还需要用到历史fragment的信息
type fragmentInfo struct {
	id       int     // fragment的自增序号
	duration float64 // 当前fragment中数据的时长，单位秒
	discont  bool    // #EXT-X-DISCONTINUITY
	filename string
}

// NewMuxer
//
// @param observer 可以为nil，如果不为nil，TS流将回调给上层
func NewMuxer(streamName string, config *MuxerConfig, observer IMuxerObserver) *Muxer {
	uk := base.GenUkHlsMuxer()
	op := PathStrategy.GetMuxerOutPath(config.OutPath, streamName)
	playlistFilename := PathStrategy.GetLiveM3u8FileName(op, streamName)
	recordPlaylistFilename := PathStrategy.GetRecordM3u8FileName(op, streamName)
	playlistFilenameBak := fmt.Sprintf("%s.bak", playlistFilename)
	recordPlaylistFilenameBak := fmt.Sprintf("%s.bak", recordPlaylistFilename)
	m := &Muxer{
		UniqueKey:                 uk,
		streamName:                streamName,
		outPath:                   op,
		playlistFilename:          playlistFilename,
		playlistFilenameBak:       playlistFilenameBak,
		recordPlayListFilename:    recordPlaylistFilename,
		recordPlayListFilenameBak: recordPlaylistFilenameBak,
		config:                    config,
		observer:                  observer,
	}
	m.makeFrags()
	Log.Infof("[%s] lifecycle new hls muxer. muxer=%p, streamName=%s", uk, m, streamName)
	return m
}

func (m *Muxer) Start() {
	Log.Infof("[%s] start hls muxer.", m.UniqueKey)
	m.ensureDir()
}

func (m *Muxer) Dispose() {
	Log.Infof("[%s] lifecycle dispose hls muxer.", m.UniqueKey)
	if err := m.closeFragment(true); err != nil {
		Log.Errorf("[%s] close fragment error. err=%+v", m.UniqueKey, err)
	}
}

// ---------------------------------------------------------------------------------------------------------------------

// OnPatPmt OnTsPackets
//
// 实现 remux.IRtmp2MpegtsRemuxerObserver，方便直接将 remux.Rtmp2MpegtsRemuxer 的数据喂入 hls.Muxer
func (m *Muxer) OnPatPmt(b []byte) {
	m.FeedPatPmt(b)
}

func (m *Muxer) OnTsPackets(tsPackets []byte, frame *mpegts.Frame, boundary bool) {
	m.FeedMpegts(tsPackets, frame, boundary)
}

// ---------------------------------------------------------------------------------------------------------------------

func (m *Muxer) FeedPatPmt(b []byte) {
	m.patpmt = b
}

func (m *Muxer) FeedMpegts(tsPackets []byte, frame *mpegts.Frame, boundary bool) {
	//Log.Debugf("> FeedMpegts. boundary=%v, frame=%p, sid=%d", boundary, frame, frame.Sid)
	if frame.Sid == mpegts.StreamIdAudio {
		// TODO(chef): 为什么音频用pts，视频用dts
		if err := m.updateFragment(frame.Pts, boundary, frame); err != nil {
			Log.Errorf("[%s] update fragment error. err=%+v", m.UniqueKey, err)
			return
		}
		if !m.opened {
			Log.Warnf("[%s] FeedMpegts A not opened. boundary=%t", m.UniqueKey, boundary)
			return
		}
		//Log.Debugf("[%s] WriteFrame A. dts=%d, len=%d", m.UniqueKey, frame.DTS, len(frame.Raw))
	} else {
		if err := m.updateFragment(frame.Dts, boundary, frame); err != nil {
			Log.Errorf("[%s] update fragment error. err=%+v", m.UniqueKey, err)
			return
		}
		if !m.opened {
			// 走到这，可能是第一个包并且boundary为false
			Log.Warnf("[%s] FeedMpegts V not opened. boundary=%t, key=%t", m.UniqueKey, boundary, frame.Key)
			return
		}
		//Log.Debugf("[%s] WriteFrame V. dts=%d, len=%d", m.UniqueKey, frame.Dts, len(frame.Raw))
	}

	if err := m.fragment.WriteFile(tsPackets); err != nil {
		Log.Errorf("[%s] fragment write error. err=%+v", m.UniqueKey, err)
		return
	}
}

// ---------------------------------------------------------------------------------------------------------------------

func (m *Muxer) OutPath() string {
	return m.outPath
}

// ---------------------------------------------------------------------------------------------------------------------

// updateFragment 决定是否开启新的TS切片文件（注意，可能已经有TS切片，也可能没有，这是第一个切片）
//
// @param boundary: 调用方认为可能是开启新TS切片的时间点
//
// @param frame: 内部只在打日志时使用
//
// @return: 理论上，只有文件操作失败才会返回错误
func (m *Muxer) updateFragment(ts uint64, boundary bool, frame *mpegts.Frame) error {
	discont := true

	// 如果已经有TS切片，检查是否需要强制开启新的切片，以及切片是否发生跳跃
	// 注意，音频和视频是在一起检查的
	if m.opened {
		f := m.getCurrFrag()

		// 以下情况，强制开启新的分片：
		// 1. 当前时间戳 - 当前分片的初始时间戳 > 配置中单个ts分片时长的10倍
		//    原因可能是：
		//        1. 当前包的时间戳发生了大的跳跃
		//        2. 一直没有I帧导致没有合适的时间重新切片，堆积的包达到阈值
		// 2. 往回跳跃超过了阈值
		//
		maxfraglen := uint64(m.config.FragmentDurationMs * 90 * 10)
		if (ts > m.fragTs && ts-m.fragTs > maxfraglen) || (m.fragTs > ts && m.fragTs-ts > negMaxfraglen) {
			Log.Warnf("[%s] force fragment split. fragTs=%d, ts=%d, frame=%s", m.UniqueKey, m.fragTs, ts, frame.DebugString())

			if err := m.closeFragment(false); err != nil {
				return err
			}
			if err := m.openFragment(ts, true); err != nil {
				return err
			}
		}

		// 更新当前分片的时间长度
		//
		// TODO chef:
		// f.duration（也即写入m3u8中记录分片时间长度）的做法我觉得有问题
		// 此处用最新收到的数据更新f.duration
		// 但是假设fragment翻滚，数据可能是写入下一个分片中
		// 是否就导致了f.duration和实际分片时间长度不一致
		if ts > m.fragTs {
			duration := float64(ts-m.fragTs) / 90000
			if duration > f.duration {
				f.duration = duration
			}
		}
		discont = false

		// 已经有TS切片，切片时长没有达到设置的阈值，则不开启新的切片
		if f.duration < float64(m.config.FragmentDurationMs)/1000 {
			return nil
		}
	}

	// 开启新的fragment
	// 此时的情况是，上层认为是合适的开启分片的时机（比如是I帧），并且
	// 1. 当前是第一个分片
	// 2. 当前不是第一个分片，但是上一个分片已经达到配置时长
	if boundary {
		if err := m.closeFragment(false); err != nil {
			return err
		}
		if err := m.openFragment(ts, discont); err != nil {
			return err
		}
	}

	return nil
}

// openFragment
//
// @param discont: 不连续标志，会在m3u8文件的fragment前增加`#EXT-X-DISCONTINUITY`
//
// @return: 理论上，只有文件操作失败才会返回错误
func (m *Muxer) openFragment(ts uint64, discont bool) error {
	if m.opened {
		return nazaerrors.Wrap(base.ErrHls)
	}

	id := m.getFragmentId()

	filename := PathStrategy.GetTsFileName(m.streamName, id, int(Clock.Now().UnixNano()/1e6))
	filenameWithPath := PathStrategy.GetTsFileNameWithPath(m.outPath, filename)

	if err := m.fragment.OpenFile(filenameWithPath); err != nil {
		return err
	}

	if err := m.fragment.WriteFile(m.patpmt); err != nil {
		return err
	}

	m.opened = true

	frag := m.getCurrFrag()
	frag.discont = discont
	frag.id = id
	frag.filename = filename
	frag.duration = 0

	m.fragTs = ts

	if m.observer == nil {
		return nil
	}

	// nrm said: start fragment with audio to make iPhone happy
	m.observer.OnFragmentOpen()

	m.observer.OnHlsMakeTs(base.HlsMakeTsInfo{
		Event:          "open",
		StreamName:     m.streamName,
		Cwd:            base.GetWd(),
		TsFile:         filenameWithPath,
		LiveM3u8File:   m.playlistFilename,
		RecordM3u8File: m.recordPlayListFilename,
		Id:             id,
		Duration:       frag.duration,
	})

	return nil
}

// closeFragment
//
// @return: 理论上，只有文件操作失败才会返回错误
func (m *Muxer) closeFragment(isLast bool) error {
	if !m.opened {
		// 注意，首次调用closeFragment时，有可能opened为false
		return nil
	}

	if err := m.fragment.CloseFile(); err != nil {
		return err
	}

	m.opened = false

	// 更新序号，为下个分片做准备
	// 注意，后面使用序号的逻辑，都依赖该处
	m.incrFrag()

	m.writePlaylist(isLast)

	if m.config.CleanupMode == CleanupModeNever || m.config.CleanupMode == CleanupModeInTheEnd {
		m.writeRecordPlaylist()
	}
	if m.config.CleanupMode == CleanupModeAsap {
		frag := m.getDeleteFrag()
		if frag.filename != "" {
			filenameWithPath := PathStrategy.GetTsFileNameWithPath(m.outPath, frag.filename)
			if err := fslCtx.Remove(filenameWithPath); err != nil {
				Log.Warnf("[%s] remove stale fragment file failed. filename=%s, err=%+v", m.UniqueKey, filenameWithPath, err)
			}
		}
	}

	if m.observer == nil {
		return nil
	}

	currFrag := m.getClosedFrag()
	m.observer.OnHlsMakeTs(base.HlsMakeTsInfo{
		Event:          "close",
		StreamName:     m.streamName,
		Cwd:            base.GetWd(),
		TsFile:         PathStrategy.GetTsFileNameWithPath(m.outPath, currFrag.filename),
		LiveM3u8File:   m.playlistFilename,
		RecordM3u8File: m.recordPlayListFilename,
		Id:             currFrag.id,
		Duration:       currFrag.duration,
	})
	return nil
}

func (m *Muxer) writeRecordPlaylist() {
	// 找出整个直播流从开始到结束最大的分片时长
	currFrag := m.getClosedFrag()
	if currFrag.duration > m.recordMaxFragDuration {
		m.recordMaxFragDuration = currFrag.duration + 0.5
	}

	fragLines := fmt.Sprintf("#EXTINF:%.3f,\n%s\n", currFrag.duration, currFrag.filename)

	content, err := fslCtx.ReadFile(m.recordPlayListFilename)
	if err == nil {
		// m3u8文件已经存在

		content = bytes.TrimSuffix(content, []byte("#EXT-X-ENDLIST\n"))
		content, err = updateTargetDurationInM3u8(content, int(m.recordMaxFragDuration))
		if err != nil {
			Log.Errorf("[%s] update target duration failed. err=%+v", m.UniqueKey, err)
			return
		}

		if currFrag.discont {
			content = append(content, []byte("#EXT-X-DISCONTINUITY\n")...)
		}

		content = append(content, []byte(fragLines)...)
		content = append(content, []byte("#EXT-X-ENDLIST\n")...)
	} else {
		// m3u8文件不存在
		var buf bytes.Buffer
		buf.WriteString("#EXTM3U\n")
		buf.WriteString("#EXT-X-VERSION:3\n")
		buf.WriteString(fmt.Sprintf("#EXT-X-TARGETDURATION:%d\n", int(m.recordMaxFragDuration)))
		buf.WriteString(fmt.Sprintf("#EXT-X-MEDIA-SEQUENCE:%d\n\n", 0))

		if currFrag.discont {
			buf.WriteString("#EXT-X-DISCONTINUITY\n")
		}

		buf.WriteString(fragLines)
		buf.WriteString("#EXT-X-ENDLIST\n")

		content = buf.Bytes()
	}

	if err := writeM3u8File(content, m.recordPlayListFilename, m.recordPlayListFilenameBak); err != nil {
		Log.Errorf("[%s] write record m3u8 file error. err=%+v", m.UniqueKey, err)
	}
}

func (m *Muxer) writePlaylist(isLast bool) {
	// 找出时长最长的fragment
	maxFrag := float64(m.config.FragmentDurationMs) / 1000
	m.iterateFragsInPlaylist(func(frag *fragmentInfo) {
		if frag.duration > maxFrag {
			maxFrag = frag.duration + 0.5
		}
	})

	// TODO chef 优化这块buffer的构造
	var buf bytes.Buffer
	buf.WriteString("#EXTM3U\n")
	buf.WriteString("#EXT-X-VERSION:3\n")
	buf.WriteString("#EXT-X-ALLOW-CACHE:NO\n")
	buf.WriteString(fmt.Sprintf("#EXT-X-TARGETDURATION:%d\n", int(maxFrag)))
	buf.WriteString(fmt.Sprintf("#EXT-X-MEDIA-SEQUENCE:%d\n\n", m.extXMediaSeq()))

	m.iterateFragsInPlaylist(func(frag *fragmentInfo) {
		if frag.discont {
			buf.WriteString("#EXT-X-DISCONTINUITY\n")
		}

		buf.WriteString(fmt.Sprintf("#EXTINF:%.3f,\n%s\n", frag.duration, frag.filename))
	})

	if isLast {
		buf.WriteString("#EXT-X-ENDLIST\n")
	}

	if err := writeM3u8File(buf.Bytes(), m.playlistFilename, m.playlistFilenameBak); err != nil {
		Log.Errorf("[%s] write live m3u8 file error. err=%+v", m.UniqueKey, err)
	}
}

func (m *Muxer) ensureDir() {
	// 注意，如果路径已经存在，则啥也不干
	err := fslCtx.MkdirAll(m.outPath, 0777)
	Log.Assert(nil, err)
}

// ---------------------------------------------------------------------------------------------------------------------

func (m *Muxer) fragsCapacity() int {
	return m.config.FragmentNum + m.config.DeleteThreshold + 1
}

func (m *Muxer) makeFrags() {
	m.frags = make([]fragmentInfo, m.fragsCapacity())
}

// getFragmentId 获取+1递增的序号
func (m *Muxer) getFragmentId() int {
	return m.frag + m.nfrags
}

func (m *Muxer) getFrag(n int) *fragmentInfo {
	return &m.frags[(m.frag+n)%m.fragsCapacity()]
}

func (m *Muxer) incrFrag() {
	// nfrags增长到config.FragmentNum后，就增长frag
	if m.nfrags == m.config.FragmentNum {
		m.frag++
	} else {
		m.nfrags++
	}
}

func (m *Muxer) getCurrFrag() *fragmentInfo {
	return m.getFrag(m.nfrags)
}

// getClosedFrag 获取当前正关闭的frag信息
func (m *Muxer) getClosedFrag() *fragmentInfo {
	// 注意，由于是在incrFrag()后调用，所以-1
	return m.getFrag(m.nfrags - 1)
}

// iterateFragsInPlaylist 遍历当前的列表，incrFrag()后调用
func (m *Muxer) iterateFragsInPlaylist(fn func(*fragmentInfo)) {
	for i := 0; i < m.nfrags; i++ {
		frag := m.getFrag(i)
		fn(frag)
	}
}

// extXMediaSeq 获取当前列表第一个TS的序号，incrFrag()后调用
func (m *Muxer) extXMediaSeq() int {
	return m.frag
}

// getDeleteFrag 获取需要删除的frag信息，incrFrag()后调用
func (m *Muxer) getDeleteFrag() *fragmentInfo {
	// 删除过期文件
	// 注意，由于前面incrFrag()了，所以此处获取的是环形队列即将被使用的位置的上一轮残留下的数据
	// 举例：
	// config.FragmentNum=6, config.DeleteThreshold=1
	// 环形队列大小为 6+1+1 = 8
	// frags范围为[0, 7]
	//
	// nfrags=1, frag=0 时，本轮被关闭的文件实际为0号文件，此时不会删除过期文件（因为1号文件还没有生成），存在文件为0
	// nfrags=6, frag=0 时，本轮被关闭的文件实际为5号文件，此时不会删除过期文件，存在文件为[0, 5]
	// nfrags=6, frag=1 时，本轮被关闭的文件实际为6号文件，此时不会删除过期文件，存在文件为[0, 6]
	// nfrags=6, frag=2 时，本轮被关闭的文件实际为7号文件，此时删除0号文件，存在文件为[1, 7]
	// nfrags=6, frag=3 时，本轮被关闭的文件实际为8号文件，此时删除1号文件，存在文件为[2, 8]
	// ...
	//
	// 注意，实际磁盘的情况会再多一个TS文件，因为下一个TS文件会在随后立即创建，并逐渐写入数据，比如拿上面其中一个例子
	// nfrags=6, frag=3 时，实际磁盘的情况为 [2, 9]
	//
	return m.getFrag(m.nfrags)
}
