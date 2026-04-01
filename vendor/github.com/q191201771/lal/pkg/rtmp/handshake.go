// Copyright 2019, Chef.  All rights reserved.
// https://github.com/q191201771/lal
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

package rtmp

import (
	"bytes"
	"crypto/hmac"
	"crypto/sha256"
	"fmt"
	"io"
	"time"

	"github.com/q191201771/lal/pkg/base"

	"github.com/q191201771/naza/pkg/bele"
)

// https://pengrl.com/p/20027

const version = uint8(3)

const (
	c0c1Len   = 1537
	c2Len     = 1536
	s0s1Len   = 1537
	s1Len     = 1536
	s2Len     = 1536
	s0s1s2Len = 3073
)

const (
	clientPartKeyLen = 30
	clientFullKeyLen = 62
	serverPartKeyLen = 36
	serverFullKeyLen = 68
	keyLen           = 32
)

var (
	clientVersionMockFromFfmpeg = []byte{9, 0, 124, 2} // emulated Flash client version - 9.0.124.2 on Linux
	clientVersion               = []byte{0x0C, 0x00, 0x0D, 0x0E}
	serverVersion               = []byte{0x0D, 0x0E, 0x0A, 0x0D}
)

// 30+32
var clientKey = []byte{
	'G', 'e', 'n', 'u', 'i', 'n', 'e', ' ', 'A', 'd', 'o', 'b', 'e', ' ',
	'F', 'l', 'a', 's', 'h', ' ', 'P', 'l', 'a', 'y', 'e', 'r', ' ',
	'0', '0', '1',

	0xF0, 0xEE, 0xC2, 0x4A, 0x80, 0x68, 0xBE, 0xE8, 0x2E, 0x00, 0xD0, 0xD1,
	0x02, 0x9E, 0x7E, 0x57, 0x6E, 0xEC, 0x5D, 0x2D, 0x29, 0x80, 0x6F, 0xAB,
	0x93, 0xB8, 0xE6, 0x36, 0xCF, 0xEB, 0x31, 0xAE,
}

// 36+32
var serverKey = []byte{
	'G', 'e', 'n', 'u', 'i', 'n', 'e', ' ', 'A', 'd', 'o', 'b', 'e', ' ',
	'F', 'l', 'a', 's', 'h', ' ', 'M', 'e', 'd', 'i', 'a', ' ',
	'S', 'e', 'r', 'v', 'e', 'r', ' ',
	'0', '0', '1',

	0xF0, 0xEE, 0xC2, 0x4A, 0x80, 0x68, 0xBE, 0xE8, 0x2E, 0x00, 0xD0, 0xD1,
	0x02, 0x9E, 0x7E, 0x57, 0x6E, 0xEC, 0x5D, 0x2D, 0x29, 0x80, 0x6F, 0xAB,
	0x93, 0xB8, 0xE6, 0x36, 0xCF, 0xEB, 0x31, 0xAE,
}

var random1528Buf []byte

type IHandshakeClient interface {
	WriteC0C1(writer io.Writer) error
	ReadS0S1(reader io.Reader) error
	WriteC2(writer io.Writer) error
	ReadS2(reader io.Reader) error
}

type HandshakeClientSimple struct {
	buf []byte
}

type HandshakeClientComplex struct {
	buf []byte
}

type HandshakeServer struct {
	isSimpleMode bool
	s0s1s2       []byte
}

func (c *HandshakeClientSimple) WriteC0C1(writer io.Writer) error {
	c.buf = make([]byte, c0c1Len)
	c.buf[0] = version
	bele.BePutUint32(c.buf[1:5], uint32(time.Now().UnixNano()))
	bele.BePutUint32(c.buf[5:9], 0) // 4字节模式串保持为0，标识是简单模式
	random1528(c.buf[9:])

	_, err := writer.Write(c.buf)
	return err
}

func (c *HandshakeClientSimple) ReadS0S1(reader io.Reader) error {
	_, err := io.ReadAtLeast(reader, c.buf, s0s1Len)
	return err
}

func (c *HandshakeClientSimple) WriteC2(writer io.Writer) error {
	// use s1 as c2
	_, err := writer.Write(c.buf[1:])
	return err
}

func (c *HandshakeClientSimple) ReadS2(reader io.Reader) error {
	_, err := io.ReadAtLeast(reader, c.buf, s2Len)
	return err
}

func (c *HandshakeClientComplex) WriteC0C1(writer io.Writer) error {
	c.buf = make([]byte, c0c1Len)

	c.buf[0] = version
	// mock ffmpeg
	bele.BePutUint32(c.buf[1:5], 0)
	copy(c.buf[5:9], clientVersionMockFromFfmpeg)
	random1528(c.buf[9:])

	offs := int(c.buf[9]) + int(c.buf[10]) + int(c.buf[11]) + int(c.buf[12])
	offs = (offs % 728) + 12
	makeDigestWithoutCenterPart(c.buf[1:c0c1Len], offs, clientKey[:clientPartKeyLen], c.buf[1+offs:])

	_, err := writer.Write(c.buf)
	return err
}

func (c *HandshakeClientComplex) ReadS0S1(reader io.Reader) error {
	s0s1 := make([]byte, s0s1Len)
	if _, err := io.ReadAtLeast(reader, s0s1, s0s1Len); err != nil {
		return err
	}

	c2key := parseChallenge(s0s1, serverKey[:serverPartKeyLen], clientKey[:clientFullKeyLen])

	// simple mode
	if c2key == nil {
		// use s1 as c2
		copy(c.buf, s0s1[1:])
		return nil
	}

	// complex mode
	random1528(c.buf)
	replayOffs := c2Len - keyLen
	makeDigestWithoutCenterPart(c.buf[:c2Len], replayOffs, c2key, c.buf[replayOffs:replayOffs+keyLen])
	return nil
}

func (c *HandshakeClientComplex) WriteC2(writer io.Writer) error {
	_, err := writer.Write(c.buf[:c2Len])
	return err
}

func (c *HandshakeClientComplex) ReadS2(reader io.Reader) error {
	_, err := io.ReadAtLeast(reader, c.buf, s2Len)
	return err
}

func (s *HandshakeServer) ReadC0C1(reader io.Reader) (err error) {
	c0c1 := make([]byte, c0c1Len)
	if _, err = io.ReadAtLeast(reader, c0c1, c0c1Len); err != nil {
		return err
	}

	s.s0s1s2 = make([]byte, s0s1s2Len)

	s2key := parseChallenge(c0c1, clientKey[:clientPartKeyLen], serverKey[:serverFullKeyLen])
	s.isSimpleMode = len(s2key) == 0

	s.s0s1s2[0] = version

	s1 := s.s0s1s2[1:]
	s2 := s.s0s1s2[s0s1Len:]

	bele.BePutUint32(s1, uint32(time.Now().UnixNano()))
	random1528(s1[8:])

	if s.isSimpleMode {
		// s1
		bele.BePutUint32(s1[4:], 0)

		copy(s2, c0c1[1:])
	} else {
		// s1
		copy(s1[4:], serverVersion)

		offs := int(s1[8]) + int(s1[9]) + int(s1[10]) + int(s1[11])
		offs = (offs % 728) + 12
		makeDigestWithoutCenterPart(s.s0s1s2[1:s0s1Len], offs, serverKey[:serverPartKeyLen], s.s0s1s2[1+offs:])

		// s2
		// make digest to s2 suffix position
		random1528(s2)

		replyOffs := s2Len - keyLen
		makeDigestWithoutCenterPart(s2, replyOffs, s2key, s2[replyOffs:])
	}

	return nil
}

func (s *HandshakeServer) WriteS0S1S2(writer io.Writer) error {
	_, err := writer.Write(s.s0s1s2)
	return err
}

func (s *HandshakeServer) ReadC2(reader io.Reader) error {
	c2 := make([]byte, c2Len)
	if _, err := io.ReadAtLeast(reader, c2, c2Len); err != nil {
		return err
	}
	return nil
}

// c0c1 clientPartKey serverFullKey
// s0s1 serverPartKey clientFullKey
func parseChallenge(b []byte, peerKey []byte, key []byte) []byte {
	//if b[0] != version {
	//	return nil, ErrRtmp
	//}
	ver := bele.BeUint32(b[5:])
	if ver == 0 {
		Log.Debug("handshake simple mode.")
		return nil
	}

	offs := findDigest(b[1:], 764+8, peerKey)
	if offs == -1 {
		offs = findDigest(b[1:], 8, peerKey)
	}
	if offs == -1 {
		Log.Warn("get digest offs failed. roll back to try simple handshake.")
		return nil
	}
	Log.Debug("handshake complex mode.")

	// use c0c1 digest to make a new digest
	digest := makeDigest(b[1+offs:1+offs+keyLen], key)

	return digest
}

// @param b c1或s1
func findDigest(b []byte, base int, key []byte) int {
	// calc offs
	offs := int(b[base]) + int(b[base+1]) + int(b[base+2]) + int(b[base+3])
	offs = (offs % 728) + base + 4
	// calc digest
	digest := make([]byte, keyLen)
	makeDigestWithoutCenterPart(b, offs, key, digest)
	// compare origin digest in buffer with calced digest
	if bytes.Equal(digest, b[offs:offs+keyLen]) {
		return offs
	}
	return -1
}

// <b> could be `c1` or `s1` or `s2`
func makeDigestWithoutCenterPart(b []byte, offs int, key []byte, out []byte) {
	mac := hmac.New(sha256.New, key)
	// left
	if offs != 0 {
		mac.Write(b[:offs])
	}
	// right
	if len(b)-offs-keyLen > 0 {
		mac.Write(b[offs+keyLen:])
	}
	// calc
	copy(out, mac.Sum(nil))
}

func makeDigest(b []byte, key []byte) []byte {
	mac := hmac.New(sha256.New, key)
	mac.Write(b)
	return mac.Sum(nil)
}

func random1528(out []byte) {
	copy(out, random1528Buf)
}

func init() {
	random1528Buf = make([]byte, 1528)
	hack := []byte(fmt.Sprintf("random buf of rtmp handshake gen by %s", base.LalRtmpHandshakeWaterMark))
	for i := 0; i < 1528; i += len(hack) {
		copy(random1528Buf[i:], hack)
	}
}
