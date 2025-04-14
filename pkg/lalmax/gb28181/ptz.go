package gb28181

import (
	"encoding/hex"
	"encoding/xml"
)

type MessagePtz struct {
	XMLName  xml.Name `xml:"Control"`
	CmdType  string   `xml:"CmdType"`
	SN       int      `xml:"SN"`
	DeviceID string   `xml:"DeviceID"`
	PTZCmd   string   `xml:"PTZCmd"`
}

const DeviceControl = "DeviceControl"
const PTZFirstByte = 0xA5
const (
	PresetSet  = 0x81
	PresetCall = 0x82
	PresetDel  = 0x83
)

const (
	CruiseAdd      = 0x84
	CruiseDel      = 0x85
	CruiseSetSpeed = 0x86
	CruiseStopTime = 0x87
	CruiseStart    = 0x88
)
const (
	ScanningStart = 0x89
	ScanningSpeed = 0x8A
)

/*
表 A.3 指令格式
字节 字节1 字节2 字节3 字节4 字节5 字节6 字节7 字节8
含义 A5H 组合码1 地址 指令 数据1 数据2 组合码2 校验码
各字节定义如下:
字节1: 指令的首字节为 A5H。
字节2: 组合码1, 高4 位是版本信息, 低4 位是校验位。 本标准的版本号是1.0, 版本信息为0H。
校验位= (字节1 的高4 位+ 字节1 的低4 位+ 字节2 的高4 位) %16。
字节3: 地址的低8 位。
字节4: 指令码。
字节5、6: 数据1 和数据2。
字节7: 组合码2, 高4 位是数据3, 低4 位是地址的高4 位; 在后续叙述中, 没有特别指明的高4 位,
表示该4 位与所指定的功能无关。
字节8: 校验码, 为前面的第1~7 字节的算术和的低8 位, 即算术和对256 取模后的结果。
字节8= (字节1+ 字节2+ 字节3+ 字节4+ 字节5+ 字节6+ 字节7) %256。
地址范围000H~FFFH(即0~4095) , 其中000H 地址作为广播地址。
注: 前端设备控制中, 不使用字节3 和字节7 的低4 位地址码, 使用前端设备控制消息体中的<DeviceID> 统一编码
标识控制的前端设备
*/
type PtzHead struct {
	FirstByte    uint8
	AssembleByte uint8
	Addr         uint8 //低地址码0-ff
}

// 获取组合码
func getAssembleCode() uint8 {
	return (PTZFirstByte>>4 + PTZFirstByte&0xF + 0) % 16
}
func getVerificationCode(ptz []byte) {
	sum := uint8(0)
	for i := 0; i < len(ptz)-1; i++ {
		sum += ptz[i]
	}
	ptz[len(ptz)-1] = sum
}

/*
注1 : 字节4 中的 Bit5、Bit4 分别控制镜头变倍的缩小和放大, 字节4 中的 Bit3、Bit2、Bit1、Bit0 位分别控制云台
上、 下、 左、 右方向的转动, 相应 Bit 位置1 时, 启动云台向相应方向转动, 相应 Bit 位清0 时, 停止云台相应
方向的转动。 云台的转动方向以监视器显示图像的移动方向为准。
注2: 字节5 控制水平方向速度, 速度范围由慢到快为00H~FFH; 字节6 控制垂直方向速度, 速度范围由慢到快
为00H-FFH。
注3: 字节7 的高4 位为变焦速度, 速度范围由慢到快为0H~FH; 低4 位为地址的高4 位。
*/
type Ptz struct {
	ZoomOut bool
	ZoomIn  bool
	Up      bool
	Down    bool
	Left    bool
	Right   bool
	Speed   byte //0-8
}

func (p *Ptz) Pack() string {
	buf := make([]byte, 8)
	buf[0] = PTZFirstByte
	buf[1] = getAssembleCode()
	buf[2] = 1
	buf[4] = 0
	buf[5] = 0
	buf[6] = 0
	if p.ZoomOut {
		buf[3] |= 1 << 5
		buf[6] = p.Speed << 4
	}

	if p.ZoomIn {
		buf[3] |= 1 << 4
		buf[6] = p.Speed << 4
	}
	if p.Up {
		buf[3] |= 1 << 3
		buf[5] = p.Speed
	}
	if p.Down {
		buf[3] |= 1 << 2
		buf[5] = p.Speed
	}
	if p.Left {
		buf[3] |= 1 << 1
		buf[4] = p.Speed
	}
	if p.Right {
		buf[3] |= 1
		buf[4] = p.Speed
	}
	getVerificationCode(buf)
	return hex.EncodeToString(buf)
}

func (p *Ptz) Stop() string {
	buf := make([]byte, 8)
	buf[0] = PTZFirstByte
	buf[1] = getAssembleCode()
	buf[2] = 1
	buf[3] = 0
	buf[4] = 0
	buf[5] = 0
	buf[6] = 0
	getVerificationCode(buf)
	return hex.EncodeToString(buf)
}

/*
注1 : 字节4 中的 Bit3 为1 时, 光圈缩小;Bit2 为1 时, 光圈放大。 Bit1 为1 时, 聚焦近;Bit0 为1 时, 聚焦远。 Bit3~
Bit0 的相应位清0, 则相应控制操作停止动作。
注2: 字节5 表示聚焦速度, 速度范围由慢到快为00H~FFH。
注3: 字节6 表示光圈速度, 速度范围由慢到快为00H~FFH
*/
type Fi struct {
	IrisIn    bool
	IrisOut   bool
	FocusNear bool
	FocusFar  bool
	Speed     byte //0-8
}

func (f *Fi) Pack() string {
	buf := make([]byte, 8)
	buf[0] = PTZFirstByte
	buf[1] = getAssembleCode()
	buf[2] = 1
	buf[3] |= 1 << 6
	buf[4] = 0
	buf[5] = 0
	buf[6] = 0

	if f.IrisIn {
		buf[3] |= 1 << 3
		buf[5] = f.Speed
	}
	if f.IrisOut {
		buf[3] |= 1 << 2
		buf[5] = f.Speed
	}
	if f.FocusNear {
		buf[3] |= 1 << 1
		buf[4] = f.Speed
	}
	if f.FocusFar {
		buf[3] |= 1
		buf[4] = f.Speed
	}
	getVerificationCode(buf)
	return hex.EncodeToString(buf)
}

type Preset struct {
	CMD   byte
	Point byte
}

func (p *Preset) Pack() string {
	buf := make([]byte, 8)
	buf[0] = PTZFirstByte
	buf[1] = getAssembleCode()
	buf[2] = 1

	buf[3] = p.CMD

	buf[4] = 0
	buf[5] = p.Point
	buf[6] = 0
	getVerificationCode(buf)
	return hex.EncodeToString(buf)
}

/*
注1 : 字节5 表示巡航组号, 字节6 表示预置位号。
注2: 序号2 中, 字节6 为00H 时, 删除对应的整条巡航; 序号3、4 中字节6 表示数据的低8 位, 字节7 的高4 位
表示数据的高4 位。
注3: 巡航停留时间的单位是秒(s) 。
注4: 停止巡航用 PTZ 指令中的字节4 的各 Bit 位均为0 的停止指令。
*/
type Cruise struct {
	CMD      byte
	GroupNum byte
	Value    uint16
}

func (c *Cruise) Pack() string {
	buf := make([]byte, 8)
	buf[0] = PTZFirstByte
	buf[1] = getAssembleCode()
	buf[2] = 1
	buf[3] = c.CMD

	buf[4] = c.GroupNum
	buf[5] = byte(c.Value & 0xFF)
	buf[6] = byte(c.Value>>8) & 0x0F
	getVerificationCode(buf)
	return hex.EncodeToString(buf)
}

/*
注1 : 字节5 表示扫描组号。
注2: 序号4 中, 字节6 表示数据的低8 位, 字节7 的高4 位表示数据的高4 位。
注3: 停止自动扫描用 PTZ 指令中的字节4 的各 Bit 位均为0 的停止指令。
注4: 自动扫描开始时, 整体画面从右向左移动。
*/
type Scanning struct {
	CMD      byte
	No       byte
	Value    byte
	HighAddr byte // 0-f 后4位高地址码 0-f
}
