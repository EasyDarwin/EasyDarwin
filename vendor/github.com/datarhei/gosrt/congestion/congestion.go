// Package congestions provides interfaces and types congestion control implementations for SRT
package congestion

import (
	"github.com/datarhei/gosrt/circular"
	"github.com/datarhei/gosrt/packet"
)

// Sender is the sending part of the congestion control
type Sender interface {
	// Stats returns sender statistics.
	Stats() SendStats

	// Flush flushes all queued packages.
	Flush()

	// Push pushes a packet to be send on the sender queue.
	Push(p packet.Packet)

	// Tick gets called from a connection in order to proceed with the queued packets. The provided value for
	// now is corresponds to the timestamps in the queued packets. Those timestamps are the microseconds
	// since the start of the connection.
	Tick(now uint64)

	// ACK gets called when a sequence number has been confirmed from a receiver.
	ACK(sequenceNumber circular.Number)

	// NAK get called when packets with the listed sequence number should be resend.
	NAK(sequenceNumbers []circular.Number)

	// SetDropThreshold sets the threshold in microseconds for when to drop too late packages from the queue.
	SetDropThreshold(threshold uint64)
}

// Receiver is the receiving part of the congestion control
type Receiver interface {
	// Stats returns receiver statistics.
	Stats() ReceiveStats

	// PacketRate returns the current packets and bytes per second, and the capacity of the link.
	PacketRate() (pps, bps, capacity float64)

	// Flush flushes all queued packages.
	Flush()

	// Push pushed a recieved packet to the receiver queue.
	Push(pkt packet.Packet)

	// Tick gets called from a connection in order to proceed with queued packets. The provided value for
	// now is corresponds to the timestamps in the queued packets. Those timestamps are the microseconds
	// since the start of the connection.
	Tick(now uint64)

	// SetNAKInterval sets the interval between two periodic NAK messages to the sender in microseconds.
	SetNAKInterval(nakInterval uint64)
}

// SendStats are collected statistics from a sender
type SendStats struct {
	Pkt  uint64 // Sent packets in total
	Byte uint64 // Sent bytes in total

	PktUnique  uint64
	ByteUnique uint64

	PktLoss  uint64
	ByteLoss uint64

	PktRetrans  uint64
	ByteRetrans uint64

	UsSndDuration uint64 // microseconds

	PktDrop  uint64
	ByteDrop uint64

	// instantaneous
	PktBuf  uint64
	ByteBuf uint64
	MsBuf   uint64

	PktFlightSize uint64

	UsPktSndPeriod float64 // microseconds
	BytePayload    uint64

	MbpsEstimatedInputBandwidth float64
	MbpsEstimatedSentBandwidth  float64

	PktLossRate float64
}

// ReceiveStats are collected statistics from a reciever
type ReceiveStats struct {
	Pkt  uint64
	Byte uint64

	PktUnique  uint64
	ByteUnique uint64

	PktLoss  uint64
	ByteLoss uint64

	PktRetrans  uint64
	ByteRetrans uint64

	PktBelated  uint64
	ByteBelated uint64

	PktDrop  uint64
	ByteDrop uint64

	// instantaneous
	PktBuf  uint64
	ByteBuf uint64
	MsBuf   uint64

	BytePayload uint64

	MbpsEstimatedRecvBandwidth float64
	MbpsEstimatedLinkCapacity  float64

	PktLossRate float64
}
