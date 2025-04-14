package hik

import (
	"encoding/xml"
	"fmt"
	"log/slog"
	"net"
	"time"
)

// hk 发现海康设备
type HK struct {
	conn net.PacketConn
	to   net.Addr
}

func NewHK() *HK {
	conn, err := net.ListenPacket("udp4", ":0")
	if err != nil {
		panic(err)
	}

	// conn, err := net.ListenPacket("udp4", ":0")
	// if err != nil {
	// panic(err)
	// }
	hk := HK{
		conn: conn,
		to:   &net.UDPAddr{IP: net.ParseIP("239.255.255.250"), Port: 37020},
	}
	return &hk
}

func (h *HK) Discover() {
	_, err := h.conn.WriteTo([]byte(searchInput), h.to)
	if err != nil {
		fmt.Println(err)
		return
	}
	buf := make([]byte, 2000)
	for {
		_ = h.conn.SetReadDeadline(time.Now().Add(3 * time.Second))
		n, _, err := h.conn.ReadFrom(buf)
		if err != nil {
			return
		}

		var out ProbeMatch
		if err := xml.Unmarshal(buf[:n], &out); err != nil {
			slog.Error("xml unmarshal failed", "err", err, "n", n, "body", string(buf[:n]))
			continue
		}
		fmt.Println(out.IPv4Address)
	}
}

const searchInput = `<?xml version="1.0" encoding="utf-8"?>
<Probe><Uuid>F93CF8DC-DF53-424B-98A7-9FC0536E1083</Uuid>
<Types>inquiry</Types></Probe>`
