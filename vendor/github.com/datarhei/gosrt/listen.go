package srt

import (
	"bytes"
	"context"
	"errors"
	"fmt"
	"net"
	"os"
	"sync"
	"time"

	srtnet "github.com/datarhei/gosrt/net"
	"github.com/datarhei/gosrt/packet"
)

// ConnType represents the kind of connection as returned
// from the AcceptFunc. It is one of REJECT, PUBLISH, or SUBSCRIBE.
type ConnType int

// String returns a string representation of the ConnType.
func (c ConnType) String() string {
	switch c {
	case REJECT:
		return "REJECT"
	case PUBLISH:
		return "PUBLISH"
	case SUBSCRIBE:
		return "SUBSCRIBE"
	default:
		return ""
	}
}

const (
	REJECT    ConnType = ConnType(1 << iota) // Reject a connection
	PUBLISH                                  // This connection is meant to write data to the server
	SUBSCRIBE                                // This connection is meant to read data from a PUBLISHed stream
)

// RejectionReason are the rejection reasons that can be returned from the AcceptFunc in order to send
// another reason than the default one (REJ_PEER) to the client.
type RejectionReason uint32

// Table 7: Handshake Rejection Reason Codes
const (
	REJ_UNKNOWN    RejectionReason = 1000 // unknown reason
	REJ_SYSTEM     RejectionReason = 1001 // system function error
	REJ_PEER       RejectionReason = 1002 // rejected by peer
	REJ_RESOURCE   RejectionReason = 1003 // resource allocation problem
	REJ_ROGUE      RejectionReason = 1004 // incorrect data in handshake
	REJ_BACKLOG    RejectionReason = 1005 // listener's backlog exceeded
	REJ_IPE        RejectionReason = 1006 // internal program error
	REJ_CLOSE      RejectionReason = 1007 // socket is closing
	REJ_VERSION    RejectionReason = 1008 // peer is older version than agent's min
	REJ_RDVCOOKIE  RejectionReason = 1009 // rendezvous cookie collision
	REJ_BADSECRET  RejectionReason = 1010 // wrong password
	REJ_UNSECURE   RejectionReason = 1011 // password required or unexpected
	REJ_MESSAGEAPI RejectionReason = 1012 // stream flag collision
	REJ_CONGESTION RejectionReason = 1013 // incompatible congestion-controller type
	REJ_FILTER     RejectionReason = 1014 // incompatible packet filter
	REJ_GROUP      RejectionReason = 1015 // incompatible group
)

// These are the extended rejection reasons that may be less well supported
// Codes & their meanings taken from https://github.com/Haivision/srt/blob/f477af533562505abf5295f059cf2156b17be740/srtcore/access_control.h
const (
	REJX_BAD_REQUEST   RejectionReason = 1400 // General syntax error in the SocketID specification (also a fallback code for undefined cases)
	REJX_UNAUTHORIZED  RejectionReason = 1401 // Authentication failed, provided that the user was correctly identified and access to the required resource would be granted
	REJX_OVERLOAD      RejectionReason = 1402 // The server is too heavily loaded, or you have exceeded credits for accessing the service and the resource.
	REJX_FORBIDDEN     RejectionReason = 1403 // Access denied to the resource by any kind of reason.
	REJX_NOTFOUND      RejectionReason = 1404 // Resource not found at this time.
	REJX_BAD_MODE      RejectionReason = 1405 // The mode specified in `m` key in StreamID is not supported for this request.
	REJX_UNACCEPTABLE  RejectionReason = 1406 // The requested parameters specified in SocketID cannot be satisfied for the requested resource. Also when m=publish and the data format is not acceptable.
	REJX_CONFLICT      RejectionReason = 1407 // The resource being accessed is already locked for modification. This is in case of m=publish and the specified resource is currently read-only.
	REJX_NOTSUP_MEDIA  RejectionReason = 1415 // The media type is not supported by the application. This is the `t` key that specifies the media type as stream, file and auth, possibly extended by the application.
	REJX_LOCKED        RejectionReason = 1423 // The resource being accessed is locked for any access.
	REJX_FAILED_DEPEND RejectionReason = 1424 // The request failed because it specified a dependent session ID that has been disconnected.
	REJX_ISE           RejectionReason = 1500 // Unexpected internal server error
	REJX_UNIMPLEMENTED RejectionReason = 1501 // The request was recognized, but the current version doesn't support it.
	REJX_GW            RejectionReason = 1502 // The server acts as a gateway and the target endpoint rejected the connection.
	REJX_DOWN          RejectionReason = 1503 // The service has been temporarily taken over by a stub reporting this error. The real service can be down for maintenance or crashed.
	REJX_VERSION       RejectionReason = 1505 // SRT version not supported. This might be either unsupported backward compatibility, or an upper value of a version.
	REJX_NOROOM        RejectionReason = 1507 // The data stream cannot be archived due to lacking storage space. This is in case when the request type was to send a file or the live stream to be archived.
)

// ErrListenerClosed is returned when the listener is about to shutdown.
var ErrListenerClosed = errors.New("srt: listener closed")

// AcceptFunc receives a connection request and returns the type of connection
// and is required by the Listener for each Accept of a new connection.
type AcceptFunc func(req ConnRequest) ConnType

// Listener waits for new connections
type Listener interface {
	// Accept2 waits for new connections.
	// On closing the err will be ErrListenerClosed.
	Accept2() (ConnRequest, error)

	// Accept waits for new connections. For each new connection the AcceptFunc
	// gets called. Conn is a new connection if AcceptFunc is PUBLISH or SUBSCRIBE.
	// If AcceptFunc returns REJECT, Conn is nil. In case of failure error is not
	// nil, Conn is nil and ConnType is REJECT. On closing the listener err will
	// be ErrListenerClosed and ConnType is REJECT.
	//
	// Deprecated: replaced by Accept2().
	Accept(AcceptFunc) (Conn, ConnType, error)

	// Close closes the listener. It will stop accepting new connections and
	// close all currently established connections.
	Close()

	// Addr returns the address of the listener.
	Addr() net.Addr
}

// listener implements the Listener interface.
type listener struct {
	pc   *net.UDPConn
	addr net.Addr

	config Config

	backlog  chan packet.Packet
	connReqs map[uint32]*connRequest
	conns    map[uint32]*srtConn
	lock     sync.RWMutex

	start time.Time

	rcvQueue chan packet.Packet

	sndMutex sync.Mutex
	sndData  bytes.Buffer

	syncookie *srtnet.SYNCookie

	shutdown     bool
	shutdownLock sync.RWMutex
	shutdownOnce sync.Once

	stopReader context.CancelFunc

	doneChan chan struct{}
	doneErr  error
	doneOnce sync.Once
}

// Listen returns a new listener on the SRT protocol on the address with
// the provided config. The network parameter needs to be "srt".
//
// The address has the form "host:port".
//
// Examples:
//
//	Listen("srt", "127.0.0.1:3000", DefaultConfig())
//
// In case of an error, the returned Listener is nil and the error is non-nil.
func Listen(network, address string, config Config) (Listener, error) {
	if network != "srt" {
		return nil, fmt.Errorf("listen: the network must be 'srt'")
	}

	if err := config.Validate(); err != nil {
		return nil, fmt.Errorf("listen: invalid config: %w", err)
	}

	if config.Logger == nil {
		config.Logger = NewLogger(nil)
	}

	ln := &listener{
		config: config,
	}

	lc := net.ListenConfig{
		Control: ListenControl(config),
	}

	lp, err := lc.ListenPacket(context.Background(), "udp", address)
	if err != nil {
		return nil, fmt.Errorf("listen: %w", err)
	}

	pc := lp.(*net.UDPConn)

	ln.pc = pc
	ln.addr = pc.LocalAddr()
	if ln.addr == nil {
		return nil, fmt.Errorf("listen: no local address")
	}

	ln.connReqs = make(map[uint32]*connRequest)
	ln.conns = make(map[uint32]*srtConn)

	ln.backlog = make(chan packet.Packet, 128)

	ln.rcvQueue = make(chan packet.Packet, 2048)

	syncookie, err := srtnet.NewSYNCookie(ln.addr.String(), nil)
	if err != nil {
		ln.Close()
		return nil, err
	}
	ln.syncookie = syncookie

	ln.doneChan = make(chan struct{})

	ln.start = time.Now()

	var readerCtx context.Context
	readerCtx, ln.stopReader = context.WithCancel(context.Background())
	go ln.reader(readerCtx)

	go func() {
		buffer := make([]byte, config.MSS) // MTU size

		for {
			if ln.isShutdown() {
				ln.markDone(ErrListenerClosed)
				return
			}

			ln.pc.SetReadDeadline(time.Now().Add(3 * time.Second))
			n, addr, err := ln.pc.ReadFrom(buffer)
			if err != nil {
				if errors.Is(err, os.ErrDeadlineExceeded) {
					continue
				}

				if ln.isShutdown() {
					ln.markDone(ErrListenerClosed)
					return
				}

				ln.markDone(err)
				return
			}

			p, err := packet.NewPacketFromData(addr, buffer[:n])
			if err != nil {
				continue
			}

			// non-blocking
			select {
			case ln.rcvQueue <- p:
			default:
				ln.log("listen", func() string { return "receive queue is full" })
			}
		}
	}()

	return ln, nil
}

func (ln *listener) Accept2() (ConnRequest, error) {
	if ln.isShutdown() {
		return nil, ErrListenerClosed
	}

	for {
		select {
		case <-ln.doneChan:
			return nil, ln.error()

		case p := <-ln.backlog:
			req := newConnRequest(ln, p)
			if req == nil {
				break
			}

			return req, nil
		}
	}
}

func (ln *listener) Accept(acceptFn AcceptFunc) (Conn, ConnType, error) {
	for {
		req, err := ln.Accept2()
		if err != nil {
			return nil, REJECT, err
		}

		if acceptFn == nil {
			req.Reject(REJ_PEER)
			continue
		}

		mode := acceptFn(req)
		if mode != PUBLISH && mode != SUBSCRIBE {
			// Figure out the reason
			reason := REJ_PEER
			if req.(*connRequest).rejectionReason > 0 {
				reason = req.(*connRequest).rejectionReason
			}
			req.Reject(reason)
			continue
		}

		conn, err := req.Accept()
		if err != nil {
			continue
		}

		return conn, mode, nil
	}
}

// markDone marks the listener as done by closing
// the done channel & sets the error
func (ln *listener) markDone(err error) {
	ln.doneOnce.Do(func() {
		ln.lock.Lock()
		defer ln.lock.Unlock()
		ln.doneErr = err
		close(ln.doneChan)
	})
}

// error returns the error that caused the listener to be done
// if it's nil then the listener is not done
func (ln *listener) error() error {
	ln.lock.Lock()
	defer ln.lock.Unlock()
	return ln.doneErr
}

func (ln *listener) handleShutdown(socketId uint32) {
	ln.lock.Lock()
	delete(ln.conns, socketId)
	ln.lock.Unlock()
}

func (ln *listener) isShutdown() bool {
	ln.shutdownLock.RLock()
	defer ln.shutdownLock.RUnlock()

	return ln.shutdown
}

func (ln *listener) Close() {
	ln.shutdownOnce.Do(func() {
		ln.shutdownLock.Lock()
		ln.shutdown = true
		ln.shutdownLock.Unlock()

		ln.lock.RLock()
		for _, conn := range ln.conns {
			conn.close()
		}
		ln.lock.RUnlock()

		ln.stopReader()

		ln.log("listen", func() string { return "closing socket" })

		ln.pc.Close()
	})
}

func (ln *listener) Addr() net.Addr {
	addrString := "0.0.0.0:0"
	if ln.addr != nil {
		addrString = ln.addr.String()
	}

	addr, _ := net.ResolveUDPAddr("udp", addrString)
	return addr
}

func (ln *listener) reader(ctx context.Context) {
	defer func() {
		ln.log("listen", func() string { return "left reader loop" })
	}()

	ln.log("listen", func() string { return "reader loop started" })

	for {
		select {
		case <-ctx.Done():
			return
		case p := <-ln.rcvQueue:
			if ln.isShutdown() {
				break
			}

			ln.log("packet:recv:dump", func() string { return p.Dump() })

			if p.Header().DestinationSocketId == 0 {
				if p.Header().IsControlPacket && p.Header().ControlType == packet.CTRLTYPE_HANDSHAKE {
					select {
					case ln.backlog <- p:
					default:
						ln.log("handshake:recv:error", func() string { return "backlog is full" })
					}
				}
				break
			}

			ln.lock.RLock()
			conn, ok := ln.conns[p.Header().DestinationSocketId]
			ln.lock.RUnlock()

			if !ok {
				// ignore the packet, we don't know the destination
				break
			}

			conn.push(p)
		}
	}
}

// Send a packet to the wire. This function must be synchronous in order to allow to safely call Packet.Decommission() afterward.
func (ln *listener) send(p packet.Packet) {
	ln.sndMutex.Lock()
	defer ln.sndMutex.Unlock()

	ln.sndData.Reset()

	if err := p.Marshal(&ln.sndData); err != nil {
		p.Decommission()
		ln.log("packet:send:error", func() string { return "marshalling packet failed" })
		return
	}

	buffer := ln.sndData.Bytes()

	ln.log("packet:send:dump", func() string { return p.Dump() })

	// Write the packet's contents to the wire
	ln.pc.WriteTo(buffer, p.Header().Addr)

	if p.Header().IsControlPacket {
		// Control packets can be decommissioned because they will not be sent again (data packets might be retransferred)
		p.Decommission()
	}
}

func (ln *listener) log(topic string, message func() string) {
	if ln.config.Logger == nil {
		return
	}

	ln.config.Logger.Print(topic, 0, 2, message)
}
