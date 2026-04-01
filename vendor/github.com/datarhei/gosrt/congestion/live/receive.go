package live

import (
	"container/list"
	"fmt"
	"strings"
	"sync"
	"time"

	"github.com/datarhei/gosrt/circular"
	"github.com/datarhei/gosrt/congestion"
	"github.com/datarhei/gosrt/packet"
)

// ReceiveConfig is the configuration for the liveRecv congestion control
type ReceiveConfig struct {
	InitialSequenceNumber circular.Number
	PeriodicACKInterval   uint64 // microseconds
	PeriodicNAKInterval   uint64 // microseconds
	OnSendACK             func(seq circular.Number, light bool)
	OnSendNAK             func(from, to circular.Number)
	OnDeliver             func(p packet.Packet)
}

// receiver implements the Receiver interface
type receiver struct {
	maxSeenSequenceNumber       circular.Number
	lastACKSequenceNumber       circular.Number
	lastDeliveredSequenceNumber circular.Number
	packetList                  *list.List
	lock                        sync.RWMutex

	nPackets uint

	periodicACKInterval uint64 // config
	periodicNAKInterval uint64 // config

	lastPeriodicACK uint64
	lastPeriodicNAK uint64

	avgPayloadSize  float64 // bytes
	avgLinkCapacity float64 // packets per second

	probeTime    time.Time
	probeNextSeq circular.Number

	statistics congestion.ReceiveStats

	rate struct {
		last   uint64 // microseconds
		period uint64

		packets      uint64
		bytes        uint64
		bytesRetrans uint64

		packetsPerSecond float64
		bytesPerSecond   float64

		pktLossRate float64
	}

	sendACK func(seq circular.Number, light bool)
	sendNAK func(from, to circular.Number)
	deliver func(p packet.Packet)
}

// NewReceiver takes a ReceiveConfig and returns a new Receiver
func NewReceiver(config ReceiveConfig) congestion.Receiver {
	r := &receiver{
		maxSeenSequenceNumber:       config.InitialSequenceNumber.Dec(),
		lastACKSequenceNumber:       config.InitialSequenceNumber.Dec(),
		lastDeliveredSequenceNumber: config.InitialSequenceNumber.Dec(),
		packetList:                  list.New(),

		periodicACKInterval: config.PeriodicACKInterval,
		periodicNAKInterval: config.PeriodicNAKInterval,

		avgPayloadSize: 1456, //  5.1.2. SRT's Default LiveCC Algorithm

		sendACK: config.OnSendACK,
		sendNAK: config.OnSendNAK,
		deliver: config.OnDeliver,
	}

	if r.sendACK == nil {
		r.sendACK = func(seq circular.Number, light bool) {}
	}

	if r.sendNAK == nil {
		r.sendNAK = func(from, to circular.Number) {}
	}

	if r.deliver == nil {
		r.deliver = func(p packet.Packet) {}
	}

	r.rate.last = 0
	r.rate.period = uint64(time.Second.Microseconds())

	return r
}

func (r *receiver) Stats() congestion.ReceiveStats {
	r.lock.Lock()
	defer r.lock.Unlock()

	r.statistics.BytePayload = uint64(r.avgPayloadSize)
	r.statistics.MbpsEstimatedRecvBandwidth = r.rate.bytesPerSecond * 8 / 1024 / 1024
	r.statistics.MbpsEstimatedLinkCapacity = r.avgLinkCapacity * packet.MAX_PAYLOAD_SIZE * 8 / 1024 / 1024
	r.statistics.PktLossRate = r.rate.pktLossRate

	return r.statistics
}

func (r *receiver) PacketRate() (pps, bps, capacity float64) {
	r.lock.Lock()
	defer r.lock.Unlock()

	pps = r.rate.packetsPerSecond
	bps = r.rate.bytesPerSecond
	capacity = r.avgLinkCapacity

	return
}

func (r *receiver) Flush() {
	r.lock.Lock()
	defer r.lock.Unlock()

	r.packetList = r.packetList.Init()
}

func (r *receiver) Push(pkt packet.Packet) {
	r.lock.Lock()
	defer r.lock.Unlock()

	if pkt == nil {
		return
	}

	// This is not really well (not at all) described in the specs. See core.cpp and window.h
	// and search for PUMASK_SEQNO_PROBE (0xF). Every 16th and 17th packet are
	// sent in pairs. This is used as a probe for the theoretical capacity of the link.
	if !pkt.Header().RetransmittedPacketFlag {
		probe := pkt.Header().PacketSequenceNumber.Val() & 0xF
		if probe == 0 {
			r.probeTime = time.Now()
			r.probeNextSeq = pkt.Header().PacketSequenceNumber.Inc()
		} else if probe == 1 && pkt.Header().PacketSequenceNumber.Equals(r.probeNextSeq) && !r.probeTime.IsZero() && pkt.Len() != 0 {
			// The time between packets scaled to a fully loaded packet
			diff := float64(time.Since(r.probeTime).Microseconds()) * (packet.MAX_PAYLOAD_SIZE / float64(pkt.Len()))
			if diff != 0 {
				// Here we're doing an average of the measurements.
				r.avgLinkCapacity = 0.875*r.avgLinkCapacity + 0.125*1_000_000/diff
			}
		} else {
			r.probeTime = time.Time{}
		}
	} else {
		r.probeTime = time.Time{}
	}

	r.nPackets++

	pktLen := pkt.Len()

	r.rate.packets++
	r.rate.bytes += pktLen

	r.statistics.Pkt++
	r.statistics.Byte += pktLen

	//pkt.PktTsbpdTime = pkt.Timestamp + r.delay
	if pkt.Header().RetransmittedPacketFlag {
		r.statistics.PktRetrans++
		r.statistics.ByteRetrans += pktLen

		r.rate.bytesRetrans += pktLen
	}

	//  5.1.2. SRT's Default LiveCC Algorithm
	r.avgPayloadSize = 0.875*r.avgPayloadSize + 0.125*float64(pktLen)

	if pkt.Header().PacketSequenceNumber.Lte(r.lastDeliveredSequenceNumber) {
		// Too old, because up until r.lastDeliveredSequenceNumber, we already delivered
		r.statistics.PktBelated++
		r.statistics.ByteBelated += pktLen

		r.statistics.PktDrop++
		r.statistics.ByteDrop += pktLen

		return
	}

	if pkt.Header().PacketSequenceNumber.Lt(r.lastACKSequenceNumber) {
		// Already acknowledged, ignoring
		r.statistics.PktDrop++
		r.statistics.ByteDrop += pktLen

		return
	}

	if pkt.Header().PacketSequenceNumber.Equals(r.maxSeenSequenceNumber.Inc()) {
		// In order, the packet we expected
		r.maxSeenSequenceNumber = pkt.Header().PacketSequenceNumber
	} else if pkt.Header().PacketSequenceNumber.Lte(r.maxSeenSequenceNumber) {
		// Out of order, is it a missing piece? put it in the correct position
		for e := r.packetList.Front(); e != nil; e = e.Next() {
			p := e.Value.(packet.Packet)

			if p.Header().PacketSequenceNumber == pkt.Header().PacketSequenceNumber {
				// Already received (has been sent more than once), ignoring
				r.statistics.PktDrop++
				r.statistics.ByteDrop += pktLen

				break
			} else if p.Header().PacketSequenceNumber.Gt(pkt.Header().PacketSequenceNumber) {
				// Late arrival, this fills a gap
				r.statistics.PktBuf++
				r.statistics.PktUnique++

				r.statistics.ByteBuf += pktLen
				r.statistics.ByteUnique += pktLen

				r.packetList.InsertBefore(pkt, e)

				break
			}
		}

		return
	} else {
		// Too far ahead, there are some missing sequence numbers, immediate NAK report
		// here we can prevent a possibly unnecessary NAK with SRTO_LOXXMAXTTL
		r.sendNAK(r.maxSeenSequenceNumber.Inc(), pkt.Header().PacketSequenceNumber.Dec())

		len := uint64(pkt.Header().PacketSequenceNumber.Distance(r.maxSeenSequenceNumber))
		r.statistics.PktLoss += len
		r.statistics.ByteLoss += len * uint64(r.avgPayloadSize)

		r.maxSeenSequenceNumber = pkt.Header().PacketSequenceNumber
	}

	r.statistics.PktBuf++
	r.statistics.PktUnique++

	r.statistics.ByteBuf += pktLen
	r.statistics.ByteUnique += pktLen

	r.packetList.PushBack(pkt)
}

func (r *receiver) periodicACK(now uint64) (ok bool, sequenceNumber circular.Number, lite bool) {
	r.lock.Lock()
	defer r.lock.Unlock()

	// 4.8.1. Packet Acknowledgement (ACKs, ACKACKs)
	if now-r.lastPeriodicACK < r.periodicACKInterval {
		if r.nPackets >= 64 {
			lite = true // Send light ACK
		} else {
			return
		}
	}

	minPktTsbpdTime, maxPktTsbpdTime := uint64(0), uint64(0)

	ackSequenceNumber := r.lastDeliveredSequenceNumber

	// Find the sequence number up until we have all in a row.
	// Where the first gap is (or at the end of the list) is where we can ACK to.

	e := r.packetList.Front()
	if e != nil {
		p := e.Value.(packet.Packet)

		minPktTsbpdTime = p.Header().PktTsbpdTime
		maxPktTsbpdTime = p.Header().PktTsbpdTime

		// If there are packets that should be delivered by now, move foward.
		if p.Header().PktTsbpdTime <= now {
			for e = e.Next(); e != nil; e = e.Next() {
				p = e.Value.(packet.Packet)

				if p.Header().PktTsbpdTime > now {
					break
				}
			}

			ackSequenceNumber = p.Header().PacketSequenceNumber
			maxPktTsbpdTime = p.Header().PktTsbpdTime

			if e != nil {
				if e = e.Next(); e != nil {
					p = e.Value.(packet.Packet)
				}
			}
		}

		if p.Header().PacketSequenceNumber.Equals(ackSequenceNumber.Inc()) {
			ackSequenceNumber = p.Header().PacketSequenceNumber

			for e = e.Next(); e != nil; e = e.Next() {
				p = e.Value.(packet.Packet)
				if !p.Header().PacketSequenceNumber.Equals(ackSequenceNumber.Inc()) {
					break
				}

				ackSequenceNumber = p.Header().PacketSequenceNumber
				maxPktTsbpdTime = p.Header().PktTsbpdTime
			}
		}

		ok = true
		sequenceNumber = ackSequenceNumber.Inc()

		// Keep track of the last ACK's sequence. with this we can faster ignore
		// packets that come in that have a lower sequence number.
		r.lastACKSequenceNumber = ackSequenceNumber
	}

	r.lastPeriodicACK = now
	r.nPackets = 0

	r.statistics.MsBuf = (maxPktTsbpdTime - minPktTsbpdTime) / 1_000

	return
}

func (r *receiver) periodicNAK(now uint64) (ok bool, from, to circular.Number) {
	r.lock.RLock()
	defer r.lock.RUnlock()

	if now-r.lastPeriodicNAK < r.periodicNAKInterval {
		return
	}

	// Send a periodic NAK

	ackSequenceNumber := r.lastDeliveredSequenceNumber

	// Send a NAK only for the first gap.
	// Alternatively send a NAK for max. X gaps because the size of the NAK packet is limited.
	for e := r.packetList.Front(); e != nil; e = e.Next() {
		p := e.Value.(packet.Packet)

		if !p.Header().PacketSequenceNumber.Equals(ackSequenceNumber.Inc()) {
			nackSequenceNumber := ackSequenceNumber.Inc()

			ok = true
			from = nackSequenceNumber
			to = p.Header().PacketSequenceNumber.Dec()

			break
		}

		ackSequenceNumber = p.Header().PacketSequenceNumber
	}

	r.lastPeriodicNAK = now

	return
}

func (r *receiver) Tick(now uint64) {
	if ok, sequenceNumber, lite := r.periodicACK(now); ok {
		r.sendACK(sequenceNumber, lite)
	}

	if ok, from, to := r.periodicNAK(now); ok {
		r.sendNAK(from, to)
	}

	// Deliver packets whose PktTsbpdTime is ripe
	r.lock.Lock()
	removeList := make([]*list.Element, 0, r.packetList.Len())
	for e := r.packetList.Front(); e != nil; e = e.Next() {
		p := e.Value.(packet.Packet)

		if p.Header().PacketSequenceNumber.Lte(r.lastACKSequenceNumber) && p.Header().PktTsbpdTime <= now {
			r.statistics.PktBuf--
			r.statistics.ByteBuf -= p.Len()

			r.lastDeliveredSequenceNumber = p.Header().PacketSequenceNumber

			r.deliver(p)
			removeList = append(removeList, e)
		} else {
			break
		}
	}

	for _, e := range removeList {
		r.packetList.Remove(e)
	}
	r.lock.Unlock()

	r.lock.Lock()
	tdiff := now - r.rate.last // microseconds

	if tdiff > r.rate.period {
		r.rate.packetsPerSecond = float64(r.rate.packets) / (float64(tdiff) / 1000 / 1000)
		r.rate.bytesPerSecond = float64(r.rate.bytes) / (float64(tdiff) / 1000 / 1000)
		if r.rate.bytes != 0 {
			r.rate.pktLossRate = float64(r.rate.bytesRetrans) / float64(r.rate.bytes) * 100
		} else {
			r.rate.bytes = 0
		}

		r.rate.packets = 0
		r.rate.bytes = 0
		r.rate.bytesRetrans = 0

		r.rate.last = now
	}
	r.lock.Unlock()
}

func (r *receiver) SetNAKInterval(nakInterval uint64) {
	r.lock.Lock()
	defer r.lock.Unlock()

	r.periodicNAKInterval = nakInterval
}

func (r *receiver) String(t uint64) string {
	var b strings.Builder

	b.WriteString(fmt.Sprintf("maxSeen=%d lastACK=%d lastDelivered=%d\n", r.maxSeenSequenceNumber.Val(), r.lastACKSequenceNumber.Val(), r.lastDeliveredSequenceNumber.Val()))

	r.lock.RLock()
	for e := r.packetList.Front(); e != nil; e = e.Next() {
		p := e.Value.(packet.Packet)

		b.WriteString(fmt.Sprintf("   %d @ %d (in %d)\n", p.Header().PacketSequenceNumber.Val(), p.Header().PktTsbpdTime, int64(p.Header().PktTsbpdTime)-int64(t)))
	}
	r.lock.RUnlock()

	return b.String()
}
