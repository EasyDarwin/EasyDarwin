// Copyright 2020, Chef.  All rights reserved.
// https://github.com/q191201771/naza
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

package nazabits

import "errors"

var ErrNazaBits = errors.New("nazabits: fxxk")

// BitReader 按位流式读取字节切片
// 从高位向低位读
// 注意，可以在每次读取后，判断是否发生错误。也可以在多次读取后，判断是否发生错误。
type BitReader struct {
	core  []byte
	avail uint // 还没有读取的bit数量
	index uint // 从0开始，待读取的字节下标
	pos   uint // 从左往右，从高位往低位 [0, 7]
	err   error
}

func NewBitReader(b []byte) BitReader {
	return BitReader{
		core:  b,
		avail: uint(len(b)) * 8,
	}
}

func (br *BitReader) ReadBit() (uint8, error) {
	return br.readBit()
}

// @param n: 取值范围 [1, 8]
func (br *BitReader) ReadBits8(n uint) (r uint8, err error) {
	// TODO chef: 8,16,32都去调用ReadBits64会带来额外开销，所以采用实现拷贝的方式，等泛型出来后重构
	if err = br.reserve(n); err != nil {
		return
	}

	for {
		if br.pos+n > 8 {
			r |= br.core[br.index] & m1[8-br.pos] << (n + br.pos - 8)
			n += br.pos - 8
			br.index++
			br.pos = 0
		} else {
			r |= br.core[br.index] & m1[8-br.pos] >> (8 - br.pos - n)
			br.pos += n
			if br.pos >= 8 {
				br.pos -= 8
				br.index++
			}
			return
		}
	}
	// never reach here
}

// @param n: 取值范围 [1, 16]
func (br *BitReader) ReadBits16(n uint) (r uint16, err error) {
	if err = br.reserve(n); err != nil {
		return
	}
	for {
		if br.pos+n > 8 {
			r |= uint16(br.core[br.index]&m1[8-br.pos]) << (n + br.pos - 8)
			n += br.pos - 8
			br.index++
			br.pos = 0
		} else {
			r |= uint16(br.core[br.index] & m1[8-br.pos] >> (8 - br.pos - n))
			br.pos += n
			if br.pos >= 8 {
				br.pos -= 8
				br.index++
			}
			return
		}
	}
}

// @param n: 取值范围 [1, 32]
func (br *BitReader) ReadBits32(n uint) (r uint32, err error) {
	if err = br.reserve(n); err != nil {
		return
	}

	for {
		if br.pos+n > 8 {
			r |= uint32(br.core[br.index]&m1[8-br.pos]) << (n + br.pos - 8)
			n += br.pos - 8
			br.index++
			br.pos = 0
		} else {
			r |= uint32(br.core[br.index] & m1[8-br.pos] >> (8 - br.pos - n))
			br.pos += n
			if br.pos >= 8 {
				br.pos -= 8
				br.index++
			}
			return
		}
	}
}

// @param n: 取值范围 [1, 64]
func (br *BitReader) ReadBits64(n uint) (r uint64, err error) {
	if err = br.reserve(n); err != nil {
		return
	}

	for {
		if br.pos+n > 8 {
			r |= uint64(br.core[br.index]&m1[8-br.pos]) << (n + br.pos - 8)
			n += br.pos - 8
			br.index++
			br.pos = 0
		} else {
			r |= uint64(br.core[br.index] & m1[8-br.pos] >> (8 - br.pos - n))
			br.pos += n
			if br.pos >= 8 {
				br.pos -= 8
				br.index++
			}
			return
		}
	}
}

// ReadBytes
// @param n: 读取多少个字节
func (br *BitReader) ReadBytes(n uint) (r []byte, err error) {
	// 对常见的pos为0的情况单独做优化
	if br.pos == 0 {
		if err = br.reserve(n * 8); err != nil {
			return
		}
		r = make([]byte, n)
		copy(r, br.core[br.index:br.index+n])
		br.index += n
		return
	}

	var t uint8
	for i := uint(0); i < n; i++ {
		t, err = br.ReadBits8(8)
		if err != nil {
			return
		}
		r = append(r, t)
	}
	return
}

func (br *BitReader) ReadString(n uint) (r string, err error) {
	var bs []byte
	bs, err = br.ReadBytes(n)
	if err != nil {
		return
	}
	r = string(bs)
	return
}

func (br *BitReader) ReadGolomb() (v uint32, err error) {
	return br.ReadUeGolomb()
}

// ReadUeGolomb 0阶指数哥伦布编码，无符号
func (br *BitReader) ReadUeGolomb() (v uint32, err error) {
	var t uint8
	var n uint
	var m uint32
	for {
		t, err = br.readBit()
		if err != nil {
			return
		}
		if t == 0 {
			n++
		} else {
			break
		}
	}
	m, err = br.ReadBits32(n)
	if err != nil {
		return
	}
	v = 1<<n + m - 1
	return
}

// ReadSeGolomb 哥伦布编码，有符号
func (br *BitReader) ReadSeGolomb() (v int32, err error) {
	var vv uint32
	vv, err = br.ReadUeGolomb()
	if err != nil {
		return 0, err
	}
	v = int32(vv) + 1
	sign := -(v & 1)
	v = ((v >> 1) ^ sign) - sign
	return
}

func (br *BitReader) ReadBits32IgnErr(n uint) uint32 {
	r, _ := br.ReadBits32(n)
	return r
}

func (br *BitReader) ReadStringIgnErr(n uint) string {
	r, _ := br.ReadString(n)
	return r
}

func (br *BitReader) SkipBytes(n uint) error {
	if err := br.reserve(n * 8); err != nil {
		return err
	}
	br.index += n
	return nil
}

func (br *BitReader) SkipBits(n uint) error {
	if err := br.reserve(n); err != nil {
		return err
	}
	i := n / 8
	p := n % 8
	br.index += i
	if p != 0 {
		br.pos += p
		if br.pos >= 8 {
			br.pos -= 8
			br.index++
		}
	}
	return nil
}

// 返回可读bit数量
func (br *BitReader) AvailBits() (uint, error) {
	return br.avail, br.err
}

func (br *BitReader) Err() error {
	return br.err
}

func (br *BitReader) readBit() (r uint8, err error) {
	if err = br.reserve(1); err != nil {
		return
	}

	r = br.core[br.index] >> (7 - br.pos) & 1
	br.pos++
	if br.pos == 8 {
		br.pos = 0
		br.index++
	}
	return
}

// 确保可读空间大小
func (br *BitReader) reserve(n uint) error {
	if br.err != nil {
		return br.err
	}
	if br.avail < n {
		br.err = ErrNazaBits
		return ErrNazaBits
	}

	br.avail -= n
	return nil
}

// ----------------------------------------------------------------------------

// TODO chef: BitWriter没有对写越界做检查，由调用方保证这一点，后续可能会加上检查

type BitWriter struct {
	core  []byte
	index int
	pos   uint // 从左往右
}

func NewBitWriter(b []byte) BitWriter {
	return BitWriter{
		core: b,
	}
}

// @param b: 当b不为0和1时，取b的最低位
func (bw *BitWriter) WriteBit(b uint8) {
	if b&0x1 == 1 {
		bw.core[bw.index] |= 1 << (7 - bw.pos)
	} else {
		bw.core[bw.index] &= ^(1 << (7 - bw.pos))
	}
	bw.pos++
	if bw.pos == 8 {
		bw.pos = 0
		bw.index++
	}
}

// 将<v>的低<n>位写入
// @param n: 取值范围 [1, 8]
func (bw *BitWriter) WriteBits8(n uint, v uint8) {
	for i := n - 1; ; i-- {
		bw.WriteBit(v >> i & 0x1)
		if i == 0 {
			break
		}
	}
}

func (bw *BitWriter) WriteBits16(n uint, v uint16) {
	for i := n - 1; ; i-- {
		bw.WriteBit(uint8(v >> i & 0x1))
		if i == 0 {
			break
		}
	}
}

// ----------------------------------------------------------------------------

// TODO chef: func GetBitX和func GetBitsX没有对写越界做检查，由调用方保证这一点，后续可能会加上检查

// @param pos: 取值范围 [0, 7]，0表示最低位
// @return: [0, 1]
func GetBit8(v uint8, pos uint) uint8 {
	return v >> pos & 1
}

// @param pos: 取值范围 [0, 7]，0表示最低位
// @param n:   取多少位， 取值范围 [1, 8]
//
// 举例，GetBits8(105, 2, 4) = 10（即1010）
//
//	v: 0110 1001
//
// pos:       2
//
//	n:   .. ..
func GetBits8(v uint8, pos uint, n uint) uint8 {
	return v >> pos & m1[n]
}

func GetBit16(v []byte, pos uint) uint8 {
	if pos < 8 {
		return GetBit8(v[1], pos)
	}
	return GetBit8(v[0], pos-8)
}

func GetBits16(v []byte, pos uint, n uint) uint16 {
	if pos < 8 {
		if pos+n < 9 {
			return uint16(GetBits8(v[1], pos, n))
		}
		return uint16(GetBits8(v[1], pos, 8-pos)) | uint16(GetBits8(v[0], 0, pos+n-8))<<(8-pos)
	}

	return uint16(GetBits8(v[0], pos-8, n))
}

var (
	m1 []uint8
)

func init() {
	m1 = []uint8{0, 1, 3, 7, 15, 31, 63, 127, 255} // 0 is dummy
}
