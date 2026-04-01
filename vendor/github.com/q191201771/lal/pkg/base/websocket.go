// Copyright 2021, Chef.  All rights reserved.
// https://github.com/q191201771/lal
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: joestarzxh

package base

import (
	"bufio"
	"crypto/sha1"
	"encoding/base64"
	"encoding/binary"
	"fmt"
	"io"
	"math"

	"github.com/q191201771/naza/pkg/bele"
)

// WsOpcode The WebSocket Protocol
// https://tools.ietf.org/html/rfc6455
//
// 0                   1                   2                   3
// 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
// +-+-+-+-+-------+-+-------------+-------------------------------+
// |F|R|R|R| opcode|M| Payload len |    Extended payload length    |
// |I|S|S|S|  (4)  |A|     (7)     |             (16/64)           |
// |N|V|V|V|       |S|             |   (if payload len==126/127)   |
// | |1|2|3|       |K|             |                               |
// +-+-+-+-+-------+-+-------------+ - - - - - - - - - - - - - - - +
// |     Extended payload length continued, if payload len == 127  |
// + - - - - - - - - - - - - - - - +-------------------------------+
// |                               |Masking-key, if MASK set to 1  |
// +-------------------------------+-------------------------------+
// | Masking-key (continued)       |          Payload Data         |
// +-------------------------------- - - - - - - - - - - - - - - - +
// :                     Payload Data continued ...                :
// + - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
// |                     Payload Data continued ...                |
// +---------------------------------------------------------------+
// opcode:
// *  %x0 denotes a continuation frame
// *  %x1 denotes a text frame
// *  %x2 denotes a binary frame
// *  %x3-7 are reserved for further non-control frames
// *  %x8 denotes a connection close
// *  %x9 denotes a ping
// *  %xA denotes a pong
// *  %xB-F are reserved for further control frames
// Payload length:  7 bits, 7+16 bits, or 7+64 bits
// Masking-key:  0 or 4 bytes
// mark 加密
//
//	for i := 0; i < datalen; i {
//	    m := markingkeys[i%4]
//	    data[i] = msg[i] ^ m
//	}
type WsOpcode = uint8

const (
	Wso_Continuous WsOpcode = iota //连续消息片断
	Wso_Text                       //文本消息片断,
	Wso_Binary                     //二进制消息片断,

	// Wso_Rsv3 非控制消息片断保留的操作码,
	Wso_Rsv3
	Wso_Rsv4
	Wso_Rsv5
	Wso_Rsv6
	Wso_Rsv7
	Wso_Close //连接关闭,
	Wso_Ping  //心跳检查的ping,
	Wso_Pong  //心跳检查的pong,

	// Wso_RsvB 为将来的控制消息片断的保留操作码
	Wso_RsvB
	Wso_RsvC
	Wso_RsvD
	Wso_RsvE
	Wso_RsvF
)

type WsHeader struct {
	Fin    bool
	Rsv1   bool
	Rsv2   bool
	Rsv3   bool
	Opcode WsOpcode

	PayloadLength uint64

	Masked  bool
	MaskKey uint32
}

const WsMagicStr = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"

func MakeWsFrameHeader(wsHeader WsHeader) (buf []byte) {
	headerSize := 2
	payload := uint64(0)
	switch {
	case wsHeader.PayloadLength < 126:
		payload = wsHeader.PayloadLength
	case wsHeader.PayloadLength <= math.MaxUint16:
		payload = 126
		headerSize += 2
	case wsHeader.PayloadLength > math.MaxUint16:
		payload = 127
		headerSize += 8
	}
	if wsHeader.Masked {
		headerSize += 4
	}
	buf = make([]byte, headerSize, headerSize)
	if wsHeader.Fin {
		buf[0] |= 1 << 7
	}
	if wsHeader.Rsv1 {
		buf[0] |= 1 << 6
	}
	if wsHeader.Rsv2 {
		buf[0] |= 1 << 5
	}
	if wsHeader.Rsv3 {
		buf[0] |= 1 << 4
	}
	buf[0] |= wsHeader.Opcode

	if wsHeader.Masked {
		buf[1] |= 1 << 7
	}
	buf[1] |= (uint8(payload) & 0x7F)
	if payload == 126 {
		bele.BePutUint16(buf[2:], uint16(wsHeader.PayloadLength))
	} else if payload == 127 {
		bele.BePutUint64(buf[2:], wsHeader.PayloadLength)
	}

	if wsHeader.Masked {
		bele.LePutUint32(buf[headerSize-4:], wsHeader.MaskKey)
	}
	return buf
}

func UpdateWebSocketHeader(secWebSocketKey, protocol string) []byte {
	firstLine := "HTTP/1.1 101 Switching Protocol\r\n"
	sha1Sum := sha1.Sum([]byte(secWebSocketKey + WsMagicStr))
	secWebSocketAccept := base64.StdEncoding.EncodeToString(sha1Sum[:])

	var webSocketResponseHeaderStr string
	if protocol == "" {
		webSocketResponseHeaderStr = firstLine +
			"Server: " + LalHttpflvSubSessionServer + "\r\n" +
			"Sec-WebSocket-Accept:" + secWebSocketAccept + "\r\n" +
			"Keep-Alive: timeout=15, max=100\r\n" +
			"Connection: Upgrade\r\n" +
			"Upgrade: websocket\r\n" +
			CorsHeaders +
			"\r\n"
	} else {
		webSocketResponseHeaderStr = firstLine +
			"Server: " + LalHttpflvSubSessionServer + "\r\n" +
			"Sec-WebSocket-Accept:" + secWebSocketAccept + "\r\n" +
			"Keep-Alive: timeout=15, max=100\r\n" +
			"Connection: Upgrade\r\n" +
			"Upgrade: websocket\r\n" +
			CorsHeaders +
			"Sec-WebSocket-Protocol:" + protocol + "\r\n" +
			"\r\n"
	}
	return []byte(webSocketResponseHeaderStr)
}

func ReadWsPayload(r *bufio.Reader) ([]byte, error) {
	var h WsHeader

	buf := make([]byte, 2)
	_, err := io.ReadFull(r, buf)
	if err != nil {
		return nil, err
	}

	h.Fin = (buf[0] & 0x80) != 0
	h.Rsv1 = (buf[0] & 0x40) != 0
	h.Rsv2 = (buf[0] & 0x20) != 0
	h.Rsv3 = (buf[0] & 0x10) != 0
	h.Opcode = buf[0] & 0x0f

	if buf[1]&0x80 != 0 {
		h.Masked = true
	}

	length := buf[1] & 0x7f
	switch {
	case length < 126:
		h.PayloadLength = uint64(length)
	case length == 126:
		buf = make([]byte, 2)
		_, err := io.ReadFull(r, buf)
		if err != nil {
			return nil, err
		}

		h.PayloadLength = uint64(binary.BigEndian.Uint16(buf))
	case length == 127:
		buf = make([]byte, 8)
		_, err := io.ReadFull(r, buf)
		if err != nil {
			return nil, err
		}

		h.PayloadLength = binary.BigEndian.Uint64(buf)

	default:
		err = fmt.Errorf("header error: the most significant bit must be 0")
		return nil, err
	}

	if h.Masked {
		buf = make([]byte, 4)
		_, err := io.ReadFull(r, buf)
		if err != nil {
			return nil, err
		}

		h.MaskKey = bele.BeUint32(buf)
	}

	payload := make([]byte, h.PayloadLength)
	_, err = io.ReadFull(r, payload)
	if err != nil {
		return nil, err
	}

	if h.Masked {
		mask := make([]byte, 4)
		binary.BigEndian.PutUint32(mask, h.MaskKey)
		cipher(payload, mask, 0)
	}

	return payload, nil
}

func cipher(payload []byte, mask []byte, offset int) {
	n := len(payload)
	if n < 8 {
		for i := 0; i < n; i++ {
			payload[i] ^= mask[(offset+i)%4]
		}
		return
	}

	// Calculate position in mask due to previously processed bytes number.
	mpos := offset % 4
	// Count number of bytes will processed one by one from the beginning of payload.
	ln := remain[mpos]
	// Count number of bytes will processed one by one from the end of payload.
	// This is done to process payload by 8 bytes in each iteration of main loop.
	rn := (n - ln) % 8

	for i := 0; i < ln; i++ {
		payload[i] ^= mask[(mpos+i)%4]
	}
	for i := n - rn; i < n; i++ {
		payload[i] ^= mask[(mpos+i)%4]
	}

	// NOTE: we use here binary.LittleEndian regardless of what is real
	// endianness on machine is. To do so, we have to use binary.LittleEndian in
	// the masking loop below as well.
	var (
		m  = binary.LittleEndian.Uint32((mask[:]))
		m2 = uint64(m)<<32 | uint64(m)
	)
	// Skip already processed right part.
	// Get number of uint64 parts remaining to process.
	n = (n - ln - rn) >> 3
	for i := 0; i < n; i++ {
		var (
			j     = ln + (i << 3)
			chunk = payload[j : j+8]
		)
		p := binary.LittleEndian.Uint64(chunk)
		p = p ^ m2
		binary.LittleEndian.PutUint64(chunk, p)
	}
}

// remain maps position in masking key [0,4) to number
// of bytes that need to be processed manually inside Cipher().
var remain = [4]int{0, 3, 2, 1}
