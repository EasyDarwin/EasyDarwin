// SPDX-FileCopyrightText: 2023 The Pion community <https://pion.ly>
// SPDX-License-Identifier: MIT

package mdns

import (
	"context"
	"errors"
	"fmt"
	"net"
	"sync"
	"time"

	"github.com/pion/logging"
	"golang.org/x/net/dns/dnsmessage"
	"golang.org/x/net/ipv4"
)

// Conn represents a mDNS Server
type Conn struct {
	mu  sync.RWMutex
	log logging.LeveledLogger

	socket  *ipv4.PacketConn
	dstAddr *net.UDPAddr

	queryInterval time.Duration
	localNames    []string
	queries       []*query
	ifaces        []net.Interface

	closed chan interface{}
}

type query struct {
	nameWithSuffix  string
	queryResultChan chan queryResult
}

type queryResult struct {
	answer dnsmessage.ResourceHeader
	addr   net.Addr
}

const (
	defaultQueryInterval = time.Second
	destinationAddress   = "224.0.0.251:5353"
	maxMessageRecords    = 3
	responseTTL          = 120
	// maxPacketSize is the maximum size of a mdns packet.
	// From RFC 6762:
	// Even when fragmentation is used, a Multicast DNS packet, including IP
	// and UDP headers, MUST NOT exceed 9000 bytes.
	// https://datatracker.ietf.org/doc/html/rfc6762#section-17
	maxPacketSize = 9000
)

var errNoPositiveMTUFound = errors.New("no positive MTU found")

// Server establishes a mDNS connection over an existing conn.
//
// Currently, the server only supports listening on an IPv4 connection, but internally
// it supports answering with IPv6 AAAA records if this were ever to change.
func Server(conn *ipv4.PacketConn, config *Config) (*Conn, error) {
	if config == nil {
		return nil, errNilConfig
	}

	ifaces := config.Interfaces
	if ifaces == nil {
		var err error
		ifaces, err = net.Interfaces()
		if err != nil {
			return nil, err
		}
	}

	inboundBufferSize := 0
	joinErrCount := 0
	ifacesToUse := make([]net.Interface, 0, len(ifaces))
	for i, ifc := range ifaces {
		if !config.IncludeLoopback && ifc.Flags&net.FlagLoopback == net.FlagLoopback {
			continue
		}
		if err := conn.JoinGroup(&ifaces[i], &net.UDPAddr{IP: net.IPv4(224, 0, 0, 251)}); err != nil {
			joinErrCount++
			continue
		}

		ifcCopy := ifc
		ifacesToUse = append(ifacesToUse, ifcCopy)
		if ifaces[i].MTU > inboundBufferSize {
			inboundBufferSize = ifaces[i].MTU
		}
	}

	if inboundBufferSize == 0 {
		return nil, errNoPositiveMTUFound
	}
	if inboundBufferSize > maxPacketSize {
		inboundBufferSize = maxPacketSize
	}
	if joinErrCount >= len(ifaces) {
		return nil, errJoiningMulticastGroup
	}

	dstAddr, err := net.ResolveUDPAddr("udp", destinationAddress)
	if err != nil {
		return nil, err
	}

	loggerFactory := config.LoggerFactory
	if loggerFactory == nil {
		loggerFactory = logging.NewDefaultLoggerFactory()
	}

	localNames := []string{}
	for _, l := range config.LocalNames {
		localNames = append(localNames, l+".")
	}

	c := &Conn{
		queryInterval: defaultQueryInterval,
		queries:       []*query{},
		socket:        conn,
		dstAddr:       dstAddr,
		localNames:    localNames,
		ifaces:        ifacesToUse,
		log:           loggerFactory.NewLogger("mdns"),
		closed:        make(chan interface{}),
	}
	if config.QueryInterval != 0 {
		c.queryInterval = config.QueryInterval
	}

	if err := conn.SetControlMessage(ipv4.FlagInterface, true); err != nil {
		c.log.Warnf("Failed to SetControlMessage on PacketConn %v", err)
	}

	if config.IncludeLoopback {
		// this is an efficient way for us to send ourselves a message faster instead of it going
		// further out into the network stack.
		if err := conn.SetMulticastLoopback(true); err != nil {
			c.log.Warnf("Failed to SetMulticastLoopback(true) on PacketConn %v; this may cause inefficient network path communications", err)
		}
	}

	// https://www.rfc-editor.org/rfc/rfc6762.html#section-17
	// Multicast DNS messages carried by UDP may be up to the IP MTU of the
	// physical interface, less the space required for the IP header (20
	// bytes for IPv4; 40 bytes for IPv6) and the UDP header (8 bytes).
	go c.start(inboundBufferSize-20-8, config)
	return c, nil
}

// Close closes the mDNS Conn
func (c *Conn) Close() error {
	select {
	case <-c.closed:
		return nil
	default:
	}

	if err := c.socket.Close(); err != nil {
		return err
	}

	<-c.closed
	return nil
}

// Query sends mDNS Queries for the following name until
// either the Context is canceled/expires or we get a result
func (c *Conn) Query(ctx context.Context, name string) (dnsmessage.ResourceHeader, net.Addr, error) {
	select {
	case <-c.closed:
		return dnsmessage.ResourceHeader{}, nil, errConnectionClosed
	default:
	}

	nameWithSuffix := name + "."

	queryChan := make(chan queryResult, 1)
	query := &query{nameWithSuffix, queryChan}
	c.mu.Lock()
	c.queries = append(c.queries, query)
	c.mu.Unlock()

	defer func() {
		c.mu.Lock()
		defer c.mu.Unlock()
		for i := len(c.queries) - 1; i >= 0; i-- {
			if c.queries[i] == query {
				c.queries = append(c.queries[:i], c.queries[i+1:]...)
			}
		}
	}()

	ticker := time.NewTicker(c.queryInterval)
	defer ticker.Stop()

	c.sendQuestion(nameWithSuffix)
	for {
		select {
		case <-ticker.C:
			c.sendQuestion(nameWithSuffix)
		case <-c.closed:
			return dnsmessage.ResourceHeader{}, nil, errConnectionClosed
		case res := <-queryChan:
			// Given https://datatracker.ietf.org/doc/html/draft-ietf-mmusic-mdns-ice-candidates#section-3.2.2-2
			// An ICE agent SHOULD ignore candidates where the hostname resolution returns more than one IP address.
			//
			// We will take the first we receive which could result in a race between two suitable addresses where
			// one is better than the other (e.g. localhost vs LAN).
			return res.answer, res.addr, nil
		case <-ctx.Done():
			return dnsmessage.ResourceHeader{}, nil, errContextElapsed
		}
	}
}

type ipToBytesError struct {
	ip           net.IP
	expectedType string
}

func (err ipToBytesError) Error() string {
	return fmt.Sprintf("ip (%s) is not %s", err.ip, err.expectedType)
}

func ipv4ToBytes(ip net.IP) ([4]byte, error) {
	rawIP := ip.To4()
	if rawIP == nil {
		return [4]byte{}, ipToBytesError{ip, "IPv4"}
	}

	// net.IPs are stored in big endian / network byte order
	var out [4]byte
	copy(out[:], rawIP[:])
	return out, nil
}

func ipv6ToBytes(ip net.IP) ([16]byte, error) {
	rawIP := ip.To16()
	if rawIP == nil {
		return [16]byte{}, ipToBytesError{ip, "IPv6"}
	}

	// net.IPs are stored in big endian / network byte order
	var out [16]byte
	copy(out[:], rawIP[:])
	return out, nil
}

func interfaceForRemote(remote string) (net.IP, error) {
	conn, err := net.Dial("udp", remote)
	if err != nil {
		return nil, err
	}

	localAddr, ok := conn.LocalAddr().(*net.UDPAddr)
	if !ok {
		return nil, errFailedCast
	}

	if err := conn.Close(); err != nil {
		return nil, err
	}

	return localAddr.IP, nil
}

func (c *Conn) sendQuestion(name string) {
	packedName, err := dnsmessage.NewName(name)
	if err != nil {
		c.log.Warnf("Failed to construct mDNS packet %v", err)
		return
	}

	msg := dnsmessage.Message{
		Header: dnsmessage.Header{},
		Questions: []dnsmessage.Question{
			{
				Type:  dnsmessage.TypeA,
				Class: dnsmessage.ClassINET,
				Name:  packedName,
			},
		},
	}

	rawQuery, err := msg.Pack()
	if err != nil {
		c.log.Warnf("Failed to construct mDNS packet %v", err)
		return
	}

	c.writeToSocket(0, rawQuery, false)
}

func (c *Conn) writeToSocket(ifIndex int, b []byte, srcIfcIsLoopback bool) {
	if ifIndex != 0 {
		ifc, err := net.InterfaceByIndex(ifIndex)
		if err != nil {
			c.log.Warnf("Failed to get interface for %d: %v", ifIndex, err)
			return
		}
		if srcIfcIsLoopback && ifc.Flags&net.FlagLoopback == 0 {
			// avoid accidentally tricking the destination that itself is the same as us
			c.log.Warnf("Interface is not loopback %d", ifIndex)
			return
		}
		if err := c.socket.SetMulticastInterface(ifc); err != nil {
			c.log.Warnf("Failed to set multicast interface for %d: %v", ifIndex, err)
		} else {
			if _, err := c.socket.WriteTo(b, nil, c.dstAddr); err != nil {
				c.log.Warnf("Failed to send mDNS packet on interface %d: %v", ifIndex, err)
			}
		}
		return
	}
	for ifcIdx := range c.ifaces {
		if srcIfcIsLoopback && c.ifaces[ifcIdx].Flags&net.FlagLoopback == 0 {
			// avoid accidentally tricking the destination that itself is the same as us
			continue
		}
		if err := c.socket.SetMulticastInterface(&c.ifaces[ifcIdx]); err != nil {
			c.log.Warnf("Failed to set multicast interface for %d: %v", c.ifaces[ifcIdx].Index, err)
		} else {
			if _, err := c.socket.WriteTo(b, nil, c.dstAddr); err != nil {
				c.log.Warnf("Failed to send mDNS packet on interface %d: %v", c.ifaces[ifcIdx].Index, err)
			}
		}
	}
}

func createAnswer(name string, addr net.IP) (dnsmessage.Message, error) {
	packedName, err := dnsmessage.NewName(name)
	if err != nil {
		return dnsmessage.Message{}, err
	}

	msg := dnsmessage.Message{
		Header: dnsmessage.Header{
			Response:      true,
			Authoritative: true,
		},
		Answers: []dnsmessage.Resource{
			{
				Header: dnsmessage.ResourceHeader{
					Type:  dnsmessage.TypeA,
					Class: dnsmessage.ClassINET,
					Name:  packedName,
					TTL:   responseTTL,
				},
			},
		},
	}

	if ip4 := addr.To4(); ip4 != nil {
		ipBuf, err := ipv4ToBytes(addr)
		if err != nil {
			return dnsmessage.Message{}, err
		}
		msg.Answers[0].Body = &dnsmessage.AResource{
			A: ipBuf,
		}
	} else {
		ipBuf, err := ipv6ToBytes(addr)
		if err != nil {
			return dnsmessage.Message{}, err
		}
		msg.Answers[0].Body = &dnsmessage.AAAAResource{
			AAAA: ipBuf,
		}
	}

	return msg, nil
}

func (c *Conn) sendAnswer(name string, ifIndex int, addr net.IP) {
	answer, err := createAnswer(name, addr)
	if err != nil {
		c.log.Warnf("Failed to create mDNS answer %v", err)
		return
	}

	rawAnswer, err := answer.Pack()
	if err != nil {
		c.log.Warnf("Failed to construct mDNS packet %v", err)
		return
	}

	c.writeToSocket(ifIndex, rawAnswer, addr.IsLoopback())
}

func (c *Conn) start(inboundBufferSize int, config *Config) { //nolint gocognit
	defer func() {
		c.mu.Lock()
		defer c.mu.Unlock()
		close(c.closed)
	}()

	b := make([]byte, inboundBufferSize)
	p := dnsmessage.Parser{}

	for {
		n, cm, src, err := c.socket.ReadFrom(b)
		if err != nil {
			if errors.Is(err, net.ErrClosed) {
				return
			}
			c.log.Warnf("Failed to ReadFrom %q %v", src, err)
			continue
		}
		var ifIndex int
		if cm != nil {
			ifIndex = cm.IfIndex
		}
		var srcIP net.IP
		switch addr := src.(type) {
		case *net.UDPAddr:
			srcIP = addr.IP
		case *net.TCPAddr:
			srcIP = addr.IP
		default:
			c.log.Warnf("Failed to determine address type %T for source address %s", src, src)
			continue
		}
		srcIsIPv4 := srcIP.To4() != nil

		func() {
			c.mu.RLock()
			defer c.mu.RUnlock()

			if _, err := p.Start(b[:n]); err != nil {
				c.log.Warnf("Failed to parse mDNS packet %v", err)
				return
			}

			for i := 0; i <= maxMessageRecords; i++ {
				q, err := p.Question()
				if errors.Is(err, dnsmessage.ErrSectionDone) {
					break
				} else if err != nil {
					c.log.Warnf("Failed to parse mDNS packet %v", err)
					return
				}

				for _, localName := range c.localNames {
					if localName == q.Name.String() {
						if config.LocalAddress != nil {
							c.sendAnswer(q.Name.String(), ifIndex, config.LocalAddress)
						} else {
							var localAddress net.IP

							// prefer the address of the interface if we know its index, but otherwise
							// derive it from the address we read from. We do this because even if
							// multicast loopback is in use or we send from a loopback interface,
							// there are still cases where the IP packet will contain the wrong
							// source IP (e.g. a LAN interface).
							// For example, we can have a packet that has:
							// Source: 192.168.65.3
							// Destination: 224.0.0.251
							// Interface Index: 1
							// Interface Addresses @ 1: [127.0.0.1/8 ::1/128]
							if ifIndex != 0 {
								ifc, netErr := net.InterfaceByIndex(ifIndex)
								if netErr != nil {
									c.log.Warnf("Failed to get interface for %d: %v", ifIndex, netErr)
									continue
								}
								addrs, addrsErr := ifc.Addrs()
								if addrsErr != nil {
									c.log.Warnf("Failed to get addresses for interface %d: %v", ifIndex, addrsErr)
									continue
								}
								if len(addrs) == 0 {
									c.log.Warnf("Expected more than one address for interface %d", ifIndex)
									continue
								}
								var selectedIP net.IP
								for _, addr := range addrs {
									var ip net.IP
									switch addr := addr.(type) {
									case *net.IPNet:
										ip = addr.IP
									case *net.IPAddr:
										ip = addr.IP
									default:
										c.log.Warnf("Failed to determine address type %T from interface %d", addr, ifIndex)
										continue
									}

									// match up respective IP types
									if ipv4 := ip.To4(); ipv4 == nil {
										if srcIsIPv4 {
											continue
										} else if !isSupportedIPv6(ip) {
											continue
										}
									} else if !srcIsIPv4 {
										continue
									}
									selectedIP = ip
									break
								}
								if selectedIP == nil {
									c.log.Warnf("Failed to find suitable IP for interface %d; deriving address from source address instead", ifIndex)
								} else {
									localAddress = selectedIP
								}
							} else if ifIndex == 0 || localAddress == nil {
								localAddress, err = interfaceForRemote(src.String())
								if err != nil {
									c.log.Warnf("Failed to get local interface to communicate with %s: %v", src.String(), err)
									continue
								}
							}

							c.sendAnswer(q.Name.String(), ifIndex, localAddress)
						}
					}
				}
			}

			for i := 0; i <= maxMessageRecords; i++ {
				a, err := p.AnswerHeader()
				if errors.Is(err, dnsmessage.ErrSectionDone) {
					return
				}
				if err != nil {
					c.log.Warnf("Failed to parse mDNS packet %v", err)
					return
				}

				if a.Type != dnsmessage.TypeA && a.Type != dnsmessage.TypeAAAA {
					continue
				}

				for i := len(c.queries) - 1; i >= 0; i-- {
					if c.queries[i].nameWithSuffix == a.Name.String() {
						ip, err := ipFromAnswerHeader(a, p)
						if err != nil {
							c.log.Warnf("Failed to parse mDNS answer %v", err)
							return
						}

						c.queries[i].queryResultChan <- queryResult{a, &net.IPAddr{
							IP: ip,
						}}
						c.queries = append(c.queries[:i], c.queries[i+1:]...)
					}
				}
			}
		}()
	}
}

func ipFromAnswerHeader(a dnsmessage.ResourceHeader, p dnsmessage.Parser) (ip []byte, err error) {
	if a.Type == dnsmessage.TypeA {
		resource, err := p.AResource()
		if err != nil {
			return nil, err
		}
		ip = resource.A[:]
	} else {
		resource, err := p.AAAAResource()
		if err != nil {
			return nil, err
		}
		ip = resource.AAAA[:]
	}

	return
}

// The conditions of invalidation written below are defined in
// https://tools.ietf.org/html/rfc8445#section-5.1.1.1
func isSupportedIPv6(ip net.IP) bool {
	if len(ip) != net.IPv6len ||
		isZeros(ip[0:12]) || // !(IPv4-compatible IPv6)
		ip[0] == 0xfe && ip[1]&0xc0 == 0xc0 || // !(IPv6 site-local unicast)
		ip.IsLinkLocalUnicast() ||
		ip.IsLinkLocalMulticast() {
		return false
	}
	return true
}

func isZeros(ip net.IP) bool {
	for i := 0; i < len(ip); i++ {
		if ip[i] != 0 {
			return false
		}
	}
	return true
}
