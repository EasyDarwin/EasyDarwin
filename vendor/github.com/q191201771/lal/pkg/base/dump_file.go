// Copyright 2022, Chef.  All rights reserved.
// https://github.com/q191201771/lal
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

package base

import (
	"encoding/hex"
	"fmt"
	"github.com/q191201771/naza/pkg/bele"
	"github.com/q191201771/naza/pkg/nazabytes"
	"github.com/q191201771/naza/pkg/nazalog"
	"os"
	"path/filepath"
	"time"
)

// TODO(chef): [refactor] move to naza 202208

//
// lal中的支持情况列表：
//
// | 支持情况 | 协议          | 类型 | 应用           | 开关手段     | 方式           | 测试(dump, parse) |
// | 已支持  | ps            | pub | lalserver      | http-api参数 | hook到logic中 | 00               |
// | 已支持  | rtsp          | pull | lalserver     | http-api参数 | 回调到logic中 | 00                |
// | 已支持  | rtsp          | pull | demo/pullrtsp | 运行参数     |  回调到上层逻辑 | 11               |
// | 已支持  | customize pub | pub  | lalserver     | 参数         | 接口提供选项   | 00               |
// | 未支持  | rtmp          |      |               |             |              |                   |

const (
	DumpTypeDefault                             uint32 = 0
	DumpTypePsRtpData                           uint32 = 1  // 1
	DumpTypeRtspRtpData                         uint32 = 17 // 1+16
	DumpTypeRtspSdpData                         uint32 = 18
	DumpTypeCustomizePubData                    uint32 = 33 // 1+16*2
	DumpTypeCustomizePubAudioSpecificConfigData uint32 = 34
	DumpTypeInnerFileHeaderData                 uint32 = 49 // 1+16*3
)

func (d *DumpFile) WriteAvPacket(packet AvPacket, typ uint32) error {
	out := make([]byte, 4+8+8+len(packet.Payload))
	bele.BePutUint32(out, uint32(packet.PayloadType))
	bele.BePutUint64(out[4:], uint64(packet.Timestamp))
	bele.BePutUint64(out[12:], uint64(packet.Pts))
	copy(out[20:], packet.Payload)
	return d.WriteWithType(out, typ)
}

// ---------------------------------------------------------------------------------------------------------------------

const (
	writeVer uint32 = 3
)

type DumpFile struct {
	file *os.File
}

type DumpFileMessage struct {
	Ver       uint32 // 制造数据时的代码版本
	Typ       uint32
	Len       uint32 // Body 的长度
	Timestamp uint64 // 写入时的时间戳
	Reserve   uint32
	Body      []byte
}

func NewDumpFile() *DumpFile {
	return &DumpFile{}
}

func (d *DumpFile) OpenToWrite(filename string) (err error) {
	dir := filepath.Dir(filename)
	if err = os.MkdirAll(dir, 0755); err != nil {
		return err
	}
	d.file, err = os.Create(filename)
	return d.WriteWithType([]byte(LalFullInfo), DumpTypeInnerFileHeaderData)
}

func (d *DumpFile) OpenToRead(filename string) (err error) {
	d.file, err = os.Open(filename)
	return
}

func (d *DumpFile) WriteWithType(b []byte, typ uint32) error {
	_, err := d.file.Write(d.pack(b, typ))
	return err
}

func (d *DumpFile) ReadOneMessage() (m DumpFileMessage, err error) {
	m.Ver, err = bele.ReadBeUint32(d.file)
	if err != nil {
		return
	}

	if m.Ver < writeVer {
		nazalog.Warnf("invalid ver. ver=%d", m.Ver)
	}

	m.Typ, err = bele.ReadBeUint32(d.file)
	if err != nil {
		return
	}
	m.Len, err = bele.ReadBeUint32(d.file)
	if err != nil {
		return
	}
	m.Timestamp, err = bele.ReadBeUint64(d.file)
	if err != nil {
		return
	}
	m.Reserve, err = bele.ReadBeUint32(d.file)
	if err != nil {
		return
	}

	m.Body = make([]byte, m.Len)
	_, err = d.file.Read(m.Body)
	return
}

func (d *DumpFile) Close() error {
	if d.file == nil {
		return nil
	}
	return d.file.Close()
}

// ---------------------------------------------------------------------------------------------------------------------

func (m *DumpFileMessage) DebugString() string {
	return fmt.Sprintf("ver: %d, typ: %d, len: %d, timestamp: %d, len: %d, hex: %s",
		m.Ver, m.Typ, m.Len, m.Timestamp, len(m.Body), hex.Dump(nazabytes.Prefix(m.Body, 16)))
}

// ---------------------------------------------------------------------------------------------------------------------

func (d *DumpFile) pack(b []byte, typ uint32) []byte {
	// TODO(chef): [perf] 优化这块内存 202211
	ret := make([]byte, len(b)+24)
	i := 0
	bele.BePutUint32(ret[i:], writeVer) // Ver
	i += 4
	bele.BePutUint32(ret[i:], typ) // Typ
	i += 4
	bele.BePutUint32(ret[i:], uint32(len(b))) // Len
	i += 4
	bele.BePutUint64(ret[i:], uint64(UnixMilli(time.Now()))) // Timestamp
	i += 8
	copy(ret[i:], "LALD")
	i += 4
	copy(ret[i:], b)
	return ret
}
