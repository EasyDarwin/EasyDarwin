package net

import (
	"encoding/binary"
	"fmt"
	"net"
	"strings"
)

type IP struct {
	ip net.IP
}

func (i *IP) setDefault() {
	i.ip = net.IPv4(127, 0, 0, 1)
}

func (i *IP) isValid() bool {
	if i.ip == nil || i.ip.String() == "<nil>" || i.ip.IsUnspecified() {
		return false
	}

	return true
}

func (i IP) String() string {
	if i.ip == nil {
		return ""
	}

	return i.ip.String()
}

func (i *IP) Parse(ip string) {
	i.setDefault()

	iip := net.ParseIP(ip)
	if iip == nil {
		return
	}

	i.ip = iip

	if !i.isValid() {
		i.setDefault()
	}
}

func (i *IP) FromNetIP(ip net.IP) {
	if ip == nil {
		return
	}

	iip := net.ParseIP(ip.String())
	if iip == nil {
		return
	}

	i.ip = iip

	if !i.isValid() {
		i.setDefault()
	}
}

func (i *IP) FromNetAddr(addr net.Addr) {
	if addr == nil {
		i.setDefault()
		return
	}

	if addr.Network() != "udp" {
		i.setDefault()
		return
	}

	if a, err := net.ResolveUDPAddr("udp", addr.String()); err == nil {
		if a == nil || a.IP == nil {
			i.setDefault()
		} else {
			i.ip = a.IP
		}
	} else {
		i.setDefault()
	}
}

// Unmarshal converts 16 bytes in host byte order to IP
func (i *IP) Unmarshal(data []byte) error {
	if len(data) != 4 && len(data) != 16 {
		return fmt.Errorf("invalid number of bytes")
	}

	if len(data) == 4 {
		ip0 := binary.LittleEndian.Uint32(data[0:])

		i.ip = net.IPv4(byte((ip0&0xff000000)>>24), byte((ip0&0x00ff0000)>>16), byte((ip0&0x0000ff00)>>8), byte(ip0&0x0000ff))
	} else {
		ip3 := binary.LittleEndian.Uint32(data[0:])
		ip2 := binary.LittleEndian.Uint32(data[4:])
		ip1 := binary.LittleEndian.Uint32(data[8:])
		ip0 := binary.LittleEndian.Uint32(data[12:])

		if ip0 == 0 && ip1 == 0 && ip2 == 0 {
			i.ip = net.IPv4(byte((ip3&0xff000000)>>24), byte((ip3&0x00ff0000)>>16), byte((ip3&0x0000ff00)>>8), byte(ip3&0x0000ff))
		} else {
			var b strings.Builder

			fmt.Fprintf(&b, "%04x:", (ip0&0xffff0000)>>16)
			fmt.Fprintf(&b, "%04x:", ip0&0x0000ffff)
			fmt.Fprintf(&b, "%04x:", (ip1&0xffff0000)>>16)
			fmt.Fprintf(&b, "%04x:", ip1&0x0000ffff)
			fmt.Fprintf(&b, "%04x:", (ip2&0xffff0000)>>16)
			fmt.Fprintf(&b, "%04x:", ip2&0x0000ffff)
			fmt.Fprintf(&b, "%04x:", (ip3&0xffff0000)>>16)
			fmt.Fprintf(&b, "%04x", ip3&0x0000ffff)

			iip := net.ParseIP(b.String())
			if iip == nil {
				return fmt.Errorf("invalid ip")
			}

			i.ip = iip
		}
	}

	if !i.isValid() {
		i.setDefault()
	}

	return nil
}

// Marshal converts an IP to 16 byte host byte order
func (i *IP) Marshal(data []byte) {
	if i.ip == nil || !i.isValid() {
		i.setDefault()
	}

	if len([]byte(i.ip)) == 4 {
		i.ip = net.IPv4(i.ip[0], i.ip[1], i.ip[2], i.ip[3])
	}

	if len(data) < 16 {
		return
	}

	data[0] = i.ip[15]
	data[1] = i.ip[14]
	data[2] = i.ip[13]
	data[3] = i.ip[12]

	if i.ip.To4() != nil {
		data[4] = 0
		data[5] = 0
		data[6] = 0
		data[7] = 0

		data[8] = 0
		data[9] = 0
		data[10] = 0
		data[11] = 0

		data[12] = 0
		data[13] = 0
		data[14] = 0
		data[15] = 0
	} else {
		data[4] = i.ip[11]
		data[5] = i.ip[10]
		data[6] = i.ip[9]
		data[7] = i.ip[8]

		data[8] = i.ip[7]
		data[9] = i.ip[6]
		data[10] = i.ip[5]
		data[11] = i.ip[4]

		data[12] = i.ip[3]
		data[13] = i.ip[2]
		data[14] = i.ip[1]
		data[15] = i.ip[0]
	}
}
