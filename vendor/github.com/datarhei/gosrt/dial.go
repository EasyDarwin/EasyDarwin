package srt

import (
	"bytes"
	"context"
	"encoding/binary"
	"errors"
	"fmt"
	"net"
	"os"
	"sync"
	"time"

	"github.com/datarhei/gosrt/circular"
	"github.com/datarhei/gosrt/crypto"
	"github.com/datarhei/gosrt/packet"
	"github.com/datarhei/gosrt/rand"
)

// ErrClientClosed is returned when the client connection has
// been voluntarily closed.
var ErrClientClosed = errors.New("srt: client closed")

// dialer implements the Conn interface
type dialer struct {
	version uint32

	pc *net.UDPConn

	localAddr  net.Addr
	remoteAddr net.Addr

	config Config

	socketId                    uint32
	initialPacketSequenceNumber circular.Number

	crypto crypto.Crypto

	conn     *srtConn
	connLock sync.RWMutex
	connChan chan connResponse

	start time.Time

	rcvQueue chan packet.Packet // for packets that come from the wire

	sndMutex sync.Mutex
	sndData  bytes.Buffer // for packets that go to the wire

	shutdown     bool
	shutdownLock sync.RWMutex
	shutdownOnce sync.Once

	stopReader context.CancelFunc

	doneChan chan error
}

type connResponse struct {
	conn *srtConn
	err  error
}

// Dial connects to the address using the SRT protocol with the given config
// and returns a Conn interface.
//
// The address is of the form "host:port".
//
// Example:
//
//	Dial("srt", "127.0.0.1:3000", DefaultConfig())
//
// In case of an error the returned Conn is nil and the error is non-nil.
func Dial(network, address string, config Config) (Conn, error) {
	if network != "srt" {
		return nil, fmt.Errorf("the network must be 'srt'")
	}

	if err := config.Validate(); err != nil {
		return nil, fmt.Errorf("invalid config: %w", err)
	}

	if config.Logger == nil {
		config.Logger = NewLogger(nil)
	}

	dl := &dialer{
		config: config,
	}

	netdialer := net.Dialer{
		Control: DialControl(config),
	}

	conn, err := netdialer.Dial("udp", address)
	if err != nil {
		return nil, fmt.Errorf("failed dialing: %w", err)
	}

	pc, ok := conn.(*net.UDPConn)
	if !ok {
		conn.Close()
		return nil, fmt.Errorf("failed dialing: connection is not a UDP connection")
	}

	dl.pc = pc

	dl.localAddr = pc.LocalAddr()
	dl.remoteAddr = pc.RemoteAddr()

	dl.conn = nil
	dl.connChan = make(chan connResponse)

	dl.rcvQueue = make(chan packet.Packet, 2048)

	dl.doneChan = make(chan error)

	dl.start = time.Now()

	// create a new socket ID
	dl.socketId, err = rand.Uint32()
	if err != nil {
		dl.Close()
		return nil, err
	}

	seqNum, err := rand.Uint32()
	if err != nil {
		dl.Close()
		return nil, err
	}
	dl.initialPacketSequenceNumber = circular.New(seqNum&packet.MAX_SEQUENCENUMBER, packet.MAX_SEQUENCENUMBER)

	go func() {
		buffer := make([]byte, MAX_MSS_SIZE) // MTU size

		for {
			if dl.isShutdown() {
				dl.doneChan <- ErrClientClosed
				return
			}

			pc.SetReadDeadline(time.Now().Add(3 * time.Second))
			n, _, err := pc.ReadFrom(buffer)
			if err != nil {
				if errors.Is(err, os.ErrDeadlineExceeded) {
					continue
				}

				if dl.isShutdown() {
					dl.doneChan <- ErrClientClosed
					return
				}

				dl.doneChan <- err
				return
			}

			p, err := packet.NewPacketFromData(dl.remoteAddr, buffer[:n])
			if err != nil {
				continue
			}

			// non-blocking
			select {
			case dl.rcvQueue <- p:
			default:
				dl.log("dial", func() string { return "receive queue is full" })
			}
		}
	}()

	var readerCtx context.Context
	readerCtx, dl.stopReader = context.WithCancel(context.Background())
	go dl.reader(readerCtx)

	// Send the initial handshake request
	dl.sendInduction()

	dl.log("dial", func() string { return "waiting for response" })

	timer := time.AfterFunc(dl.config.ConnectionTimeout, func() {
		dl.connChan <- connResponse{
			conn: nil,
			err:  fmt.Errorf("connection timeout. server didn't respond"),
		}
	})

	// Wait for handshake to conclude
	response := <-dl.connChan
	if response.err != nil {
		dl.Close()
		return nil, response.err
	}

	timer.Stop()

	dl.connLock.Lock()
	dl.conn = response.conn
	dl.connLock.Unlock()

	return dl, nil
}

func (dl *dialer) checkConnection() error {
	select {
	case err := <-dl.doneChan:
		dl.Close()
		return err
	default:
	}

	return nil
}

// reader reads packets from the receive queue and pushes them into the connection
func (dl *dialer) reader(ctx context.Context) {
	defer func() {
		dl.log("dial", func() string { return "left reader loop" })
	}()

	dl.log("dial", func() string { return "reader loop started" })

	for {
		select {
		case <-ctx.Done():
			return
		case p := <-dl.rcvQueue:
			if dl.isShutdown() {
				break
			}

			dl.log("packet:recv:dump", func() string { return p.Dump() })

			if p.Header().DestinationSocketId != dl.socketId {
				break
			}

			if p.Header().IsControlPacket && p.Header().ControlType == packet.CTRLTYPE_HANDSHAKE {
				dl.handleHandshake(p)
				break
			}

			dl.connLock.RLock()
			if dl.conn == nil {
				dl.connLock.RUnlock()
				break
			}

			dl.conn.push(p)
			dl.connLock.RUnlock()
		}
	}
}

// Send a packet to the wire. This function must be synchronous in order to allow to safely call Packet.Decommission() afterward.
func (dl *dialer) send(p packet.Packet) {
	dl.sndMutex.Lock()
	defer dl.sndMutex.Unlock()

	dl.sndData.Reset()

	if err := p.Marshal(&dl.sndData); err != nil {
		p.Decommission()
		dl.log("packet:send:error", func() string { return "marshalling packet failed" })
		return
	}

	buffer := dl.sndData.Bytes()

	dl.log("packet:send:dump", func() string { return p.Dump() })

	// Write the packet's contents to the wire
	dl.pc.Write(buffer)

	if p.Header().IsControlPacket {
		// Control packets can be decommissioned because they will not be sent again (data packets might be retransferred)
		p.Decommission()
	}
}

func (dl *dialer) handleHandshake(p packet.Packet) {
	cif := &packet.CIFHandshake{}

	err := p.UnmarshalCIF(cif)

	dl.log("handshake:recv:dump", func() string { return p.Dump() })
	dl.log("handshake:recv:cif", func() string { return cif.String() })

	if err != nil {
		dl.log("handshake:recv:error", func() string { return err.Error() })
		return
	}

	// assemble the response (4.3.1.  Caller-Listener Handshake)

	p.Header().ControlType = packet.CTRLTYPE_HANDSHAKE
	p.Header().SubType = 0
	p.Header().TypeSpecific = 0
	p.Header().Timestamp = uint32(time.Since(dl.start).Microseconds())
	p.Header().DestinationSocketId = 0 // must be 0 for handshake

	if cif.HandshakeType == packet.HSTYPE_INDUCTION {
		if cif.Version < 4 || cif.Version > 5 {
			dl.connChan <- connResponse{
				conn: nil,
				err:  fmt.Errorf("peer responded with unsupported handshake version (%d)", cif.Version),
			}

			return
		}

		cif.IsRequest = true
		cif.HandshakeType = packet.HSTYPE_CONCLUSION
		cif.InitialPacketSequenceNumber = dl.initialPacketSequenceNumber
		cif.MaxTransmissionUnitSize = dl.config.MSS // MTU size
		cif.MaxFlowWindowSize = dl.config.FC
		cif.SRTSocketId = dl.socketId
		cif.PeerIP.FromNetAddr(dl.localAddr)

		// Setup crypto context
		if len(dl.config.Passphrase) != 0 {
			keylen := dl.config.PBKeylen

			// If the server advertises a specific block cipher family and key size,
			// use this one, otherwise, use the configured one
			if cif.EncryptionField != 0 {
				switch cif.EncryptionField {
				case 2:
					keylen = 16
				case 3:
					keylen = 24
				case 4:
					keylen = 32
				}
			}

			cr, err := crypto.New(keylen)
			if err != nil {
				dl.connChan <- connResponse{
					conn: nil,
					err:  fmt.Errorf("failed creating crypto context: %w", err),
				}
			}

			dl.crypto = cr
		}

		// Verify version
		if cif.Version == 5 {
			dl.version = 5

			// Verify magic number
			if cif.ExtensionField != 0x4A17 {
				dl.connChan <- connResponse{
					conn: nil,
					err:  fmt.Errorf("peer sent the wrong magic number"),
				}

				return
			}

			cif.HasHS = true
			cif.SRTHS = &packet.CIFHandshakeExtension{
				SRTVersion: SRT_VERSION,
				SRTFlags: packet.CIFHandshakeExtensionFlags{
					TSBPDSND:      true,
					TSBPDRCV:      true,
					CRYPT:         true, // must always set to true
					TLPKTDROP:     true,
					PERIODICNAK:   true,
					REXMITFLG:     true,
					STREAM:        false,
					PACKET_FILTER: false,
				},
				RecvTSBPDDelay: uint16(dl.config.ReceiverLatency.Milliseconds()),
				SendTSBPDDelay: uint16(dl.config.PeerLatency.Milliseconds()),
			}

			cif.HasSID = true
			cif.StreamId = dl.config.StreamId

			if dl.crypto != nil {
				cif.HasKM = true
				cif.SRTKM = &packet.CIFKeyMaterialExtension{}

				if err := dl.crypto.MarshalKM(cif.SRTKM, dl.config.Passphrase, packet.EvenKeyEncrypted); err != nil {
					dl.connChan <- connResponse{
						conn: nil,
						err:  err,
					}

					return
				}
			}
		} else {
			dl.version = 4

			cif.EncryptionField = 0
			cif.ExtensionField = 2

			cif.HasHS = false
			cif.HasKM = false
			cif.HasSID = false
		}

		p.MarshalCIF(cif)

		dl.log("handshake:send:dump", func() string { return p.Dump() })
		dl.log("handshake:send:cif", func() string { return cif.String() })

		dl.send(p)
	} else if cif.HandshakeType == packet.HSTYPE_CONCLUSION {
		if cif.Version < 4 || cif.Version > 5 {
			dl.connChan <- connResponse{
				conn: nil,
				err:  fmt.Errorf("peer responded with unsupported handshake version (%d)", cif.Version),
			}

			return
		}

		recvTsbpdDelay := uint16(dl.config.ReceiverLatency.Milliseconds())
		sendTsbpdDelay := uint16(dl.config.PeerLatency.Milliseconds())

		if cif.Version == 5 {
			if cif.SRTHS == nil {
				dl.connChan <- connResponse{
					conn: nil,
					err:  fmt.Errorf("missing handshake extension"),
				}
				return
			}

			// Check if the peer version is sufficient
			if cif.SRTHS.SRTVersion < dl.config.MinVersion {
				dl.sendShutdown(cif.SRTSocketId)

				dl.connChan <- connResponse{
					conn: nil,
					err:  fmt.Errorf("peer SRT version is not sufficient"),
				}

				return
			}

			// Check the required SRT flags
			if !cif.SRTHS.SRTFlags.TSBPDSND || !cif.SRTHS.SRTFlags.TSBPDRCV || !cif.SRTHS.SRTFlags.TLPKTDROP || !cif.SRTHS.SRTFlags.PERIODICNAK || !cif.SRTHS.SRTFlags.REXMITFLG {
				dl.sendShutdown(cif.SRTSocketId)

				dl.connChan <- connResponse{
					conn: nil,
					err:  fmt.Errorf("peer doesn't agree on SRT flags"),
				}

				return
			}

			// We only support live streaming
			if cif.SRTHS.SRTFlags.STREAM {
				dl.sendShutdown(cif.SRTSocketId)

				dl.connChan <- connResponse{
					conn: nil,
					err:  fmt.Errorf("peer doesn't support live streaming"),
				}

				return
			}

			// Select the largest TSBPD delay advertised by the listener, but at least 120ms
			if cif.SRTHS.SendTSBPDDelay > recvTsbpdDelay {
				recvTsbpdDelay = cif.SRTHS.SendTSBPDDelay
			}

			if cif.SRTHS.RecvTSBPDDelay > sendTsbpdDelay {
				sendTsbpdDelay = cif.SRTHS.RecvTSBPDDelay
			}
		}

		// If the peer has a smaller MTU size, adjust to it
		if cif.MaxTransmissionUnitSize < dl.config.MSS {
			dl.config.MSS = cif.MaxTransmissionUnitSize
			dl.config.PayloadSize = dl.config.MSS - SRT_HEADER_SIZE - UDP_HEADER_SIZE

			if dl.config.PayloadSize < MIN_PAYLOAD_SIZE {
				dl.sendShutdown(cif.SRTSocketId)

				dl.connChan <- connResponse{
					conn: nil,
					err:  fmt.Errorf("effective MSS too small (%d bytes) to fit the minimal payload size (%d bytes)", dl.config.MSS, MIN_PAYLOAD_SIZE),
				}

				return
			}
		}

		// Create a new connection
		conn := newSRTConn(srtConnConfig{
			version:                     cif.Version,
			isCaller:                    true,
			localAddr:                   dl.localAddr,
			remoteAddr:                  dl.remoteAddr,
			config:                      dl.config,
			start:                       dl.start,
			socketId:                    dl.socketId,
			peerSocketId:                cif.SRTSocketId,
			tsbpdTimeBase:               uint64(time.Since(dl.start).Microseconds()),
			tsbpdDelay:                  uint64(recvTsbpdDelay) * 1000,
			peerTsbpdDelay:              uint64(sendTsbpdDelay) * 1000,
			initialPacketSequenceNumber: cif.InitialPacketSequenceNumber,
			crypto:                      dl.crypto,
			keyBaseEncryption:           packet.EvenKeyEncrypted,
			onSend:                      dl.send,
			onShutdown:                  func(socketId uint32) { dl.Close() },
			logger:                      dl.config.Logger,
		})

		dl.log("connection:new", func() string { return fmt.Sprintf("%#08x (%s)", conn.SocketId(), conn.StreamId()) })

		dl.connChan <- connResponse{
			conn: conn,
			err:  nil,
		}
	} else {
		var err error

		if cif.HandshakeType.IsRejection() {
			err = fmt.Errorf("connection rejected: %s", cif.HandshakeType.String())
		} else {
			err = fmt.Errorf("unsupported handshake: %s", cif.HandshakeType.String())
		}

		dl.connChan <- connResponse{
			conn: nil,
			err:  err,
		}
	}
}

func (dl *dialer) sendInduction() {
	p := packet.NewPacket(dl.remoteAddr)

	p.Header().IsControlPacket = true

	p.Header().ControlType = packet.CTRLTYPE_HANDSHAKE
	p.Header().SubType = 0
	p.Header().TypeSpecific = 0

	p.Header().Timestamp = uint32(time.Since(dl.start).Microseconds())
	p.Header().DestinationSocketId = 0

	cif := &packet.CIFHandshake{
		IsRequest:                   true,
		Version:                     4,
		EncryptionField:             0,
		ExtensionField:              2,
		InitialPacketSequenceNumber: circular.New(0, packet.MAX_SEQUENCENUMBER),
		MaxTransmissionUnitSize:     dl.config.MSS, // MTU size
		MaxFlowWindowSize:           dl.config.FC,
		HandshakeType:               packet.HSTYPE_INDUCTION,
		SRTSocketId:                 dl.socketId,
		SynCookie:                   0,
	}

	cif.PeerIP.FromNetAddr(dl.localAddr)

	p.MarshalCIF(cif)

	dl.log("handshake:send:dump", func() string { return p.Dump() })
	dl.log("handshake:send:cif", func() string { return cif.String() })

	dl.send(p)
}

func (dl *dialer) sendShutdown(peerSocketId uint32) {
	p := packet.NewPacket(dl.remoteAddr)

	data := [4]byte{}
	binary.BigEndian.PutUint32(data[0:], 0)

	p.SetData(data[0:4])

	p.Header().IsControlPacket = true

	p.Header().ControlType = packet.CTRLTYPE_SHUTDOWN
	p.Header().TypeSpecific = 0

	p.Header().Timestamp = uint32(time.Since(dl.start).Microseconds())
	p.Header().DestinationSocketId = peerSocketId

	dl.log("control:send:shutdown:dump", func() string { return p.Dump() })

	dl.send(p)
}

func (dl *dialer) LocalAddr() net.Addr {
	dl.connLock.RLock()
	defer dl.connLock.RUnlock()

	if dl.conn == nil {
		return nil
	}

	return dl.conn.LocalAddr()
}

func (dl *dialer) RemoteAddr() net.Addr {
	dl.connLock.RLock()
	defer dl.connLock.RUnlock()

	if dl.conn == nil {
		return nil
	}

	return dl.conn.RemoteAddr()
}

func (dl *dialer) SocketId() uint32 {
	dl.connLock.RLock()
	defer dl.connLock.RUnlock()

	if dl.conn == nil {
		return 0
	}

	return dl.conn.SocketId()
}

func (dl *dialer) PeerSocketId() uint32 {
	dl.connLock.RLock()
	defer dl.connLock.RUnlock()

	if dl.conn == nil {
		return 0
	}

	return dl.conn.PeerSocketId()
}

func (dl *dialer) StreamId() string {
	dl.connLock.RLock()
	defer dl.connLock.RUnlock()

	if dl.conn == nil {
		return ""
	}

	return dl.conn.StreamId()
}

func (dl *dialer) Version() uint32 {
	dl.connLock.RLock()
	defer dl.connLock.RUnlock()

	if dl.conn == nil {
		return 0
	}

	return dl.conn.Version()
}

func (dl *dialer) isShutdown() bool {
	dl.shutdownLock.RLock()
	defer dl.shutdownLock.RUnlock()

	return dl.shutdown
}

func (dl *dialer) Close() error {
	dl.shutdownOnce.Do(func() {
		dl.shutdownLock.Lock()
		dl.shutdown = true
		dl.shutdownLock.Unlock()

		dl.connLock.RLock()
		if dl.conn != nil {
			dl.conn.Close()
		}
		dl.connLock.RUnlock()

		dl.stopReader()

		dl.log("dial", func() string { return "closing socket" })
		dl.pc.Close()

		select {
		case <-dl.doneChan:
		default:
		}
	})

	return nil
}

func (dl *dialer) Read(p []byte) (n int, err error) {
	if err := dl.checkConnection(); err != nil {
		return 0, err
	}

	dl.connLock.RLock()
	defer dl.connLock.RUnlock()

	if dl.conn == nil {
		return 0, fmt.Errorf("no connection")
	}

	return dl.conn.Read(p)
}

func (dl *dialer) ReadPacket() (packet.Packet, error) {
	if err := dl.checkConnection(); err != nil {
		return nil, err
	}

	dl.connLock.RLock()
	defer dl.connLock.RUnlock()

	if dl.conn == nil {
		return nil, fmt.Errorf("no connection")
	}

	return dl.conn.ReadPacket()
}

func (dl *dialer) Write(p []byte) (n int, err error) {
	if err := dl.checkConnection(); err != nil {
		return 0, err
	}

	dl.connLock.RLock()
	defer dl.connLock.RUnlock()

	if dl.conn == nil {
		return 0, fmt.Errorf("no connection")
	}

	return dl.conn.Write(p)
}

func (dl *dialer) WritePacket(p packet.Packet) error {
	if err := dl.checkConnection(); err != nil {
		return err
	}

	dl.connLock.RLock()
	defer dl.connLock.RUnlock()

	if dl.conn == nil {
		return fmt.Errorf("no connection")
	}

	return dl.conn.WritePacket(p)
}

func (dl *dialer) SetDeadline(t time.Time) error      { return dl.conn.SetDeadline(t) }
func (dl *dialer) SetReadDeadline(t time.Time) error  { return dl.conn.SetReadDeadline(t) }
func (dl *dialer) SetWriteDeadline(t time.Time) error { return dl.conn.SetWriteDeadline(t) }

func (dl *dialer) Stats(s *Statistics) {
	dl.connLock.RLock()
	defer dl.connLock.RUnlock()

	if dl.conn == nil {
		return
	}

	dl.conn.Stats(s)
}

func (dl *dialer) log(topic string, message func() string) {
	if dl.config.Logger == nil {
		return
	}

	dl.config.Logger.Print(topic, dl.socketId, 2, message)
}
