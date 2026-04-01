package live

import (
	"sync"
	"time"

	"github.com/datarhei/gosrt/circular"
	"github.com/datarhei/gosrt/congestion"
	"github.com/datarhei/gosrt/packet"
)

type fakeLiveReceive struct {
	maxSeenSequenceNumber       circular.Number
	lastACKSequenceNumber       circular.Number
	lastDeliveredSequenceNumber circular.Number

	nPackets uint

	periodicACKInterval uint64 // config
	periodicNAKInterval uint64 // config

	lastPeriodicACK uint64

	avgPayloadSize float64 // bytes

	rate struct {
		last   time.Time
		period time.Duration

		packets uint64
		bytes   uint64

		pps float64
		bps float64
	}

	sendACK func(seq circular.Number, light bool)
	sendNAK func(from, to circular.Number)
	deliver func(p packet.Packet)

	lock sync.RWMutex
}

func NewFakeLiveReceive(config ReceiveConfig) congestion.Receiver {
	r := &fakeLiveReceive{
		maxSeenSequenceNumber:       config.InitialSequenceNumber.Dec(),
		lastACKSequenceNumber:       config.InitialSequenceNumber.Dec(),
		lastDeliveredSequenceNumber: config.InitialSequenceNumber.Dec(),

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

	r.rate.last = time.Now()
	r.rate.period = time.Second

	return r
}

func (r *fakeLiveReceive) Stats() congestion.ReceiveStats { return congestion.ReceiveStats{} }
func (r *fakeLiveReceive) PacketRate() (pps, bps, capacity float64) {
	r.lock.Lock()
	defer r.lock.Unlock()

	tdiff := time.Since(r.rate.last)

	if tdiff < r.rate.period {
		pps = r.rate.pps
		bps = r.rate.bps

		return
	}

	r.rate.pps = float64(r.rate.packets) / tdiff.Seconds()
	r.rate.bps = float64(r.rate.bytes) / tdiff.Seconds()

	r.rate.packets, r.rate.bytes = 0, 0
	r.rate.last = time.Now()

	pps = r.rate.pps
	bps = r.rate.bps

	return
}

func (r *fakeLiveReceive) Flush() {}

func (r *fakeLiveReceive) Push(pkt packet.Packet) {
	r.lock.Lock()
	defer r.lock.Unlock()

	if pkt == nil {
		return
	}

	r.nPackets++

	pktLen := pkt.Len()

	r.rate.packets++
	r.rate.bytes += pktLen

	//  5.1.2. SRT's Default LiveCC Algorithm
	r.avgPayloadSize = 0.875*r.avgPayloadSize + 0.125*float64(pktLen)

	if pkt.Header().PacketSequenceNumber.Lte(r.lastDeliveredSequenceNumber) {
		// Too old, because up until r.lastDeliveredSequenceNumber, we already delivered
		return
	}

	if pkt.Header().PacketSequenceNumber.Lt(r.lastACKSequenceNumber) {
		// Already acknowledged, ignoring
		return
	}

	if pkt.Header().PacketSequenceNumber.Lte(r.maxSeenSequenceNumber) {
		return
	}

	r.maxSeenSequenceNumber = pkt.Header().PacketSequenceNumber
}

func (r *fakeLiveReceive) periodicACK(now uint64) (ok bool, sequenceNumber circular.Number, lite bool) {
	r.lock.RLock()
	defer r.lock.RUnlock()

	// 4.8.1. Packet Acknowledgement (ACKs, ACKACKs)
	if now-r.lastPeriodicACK < r.periodicACKInterval {
		if r.nPackets >= 64 {
			lite = true // Send light ACK
		} else {
			return
		}
	}

	ok = true
	sequenceNumber = r.maxSeenSequenceNumber.Inc()

	r.lastACKSequenceNumber = r.maxSeenSequenceNumber

	r.lastPeriodicACK = now
	r.nPackets = 0

	return
}

func (r *fakeLiveReceive) Tick(now uint64) {
	if ok, sequenceNumber, lite := r.periodicACK(now); ok {
		r.sendACK(sequenceNumber, lite)
	}

	// Deliver packets whose PktTsbpdTime is ripe
	r.lock.Lock()
	defer r.lock.Unlock()

	r.lastDeliveredSequenceNumber = r.lastACKSequenceNumber
}

func (r *fakeLiveReceive) SetNAKInterval(nakInterval uint64) {
	r.lock.Lock()
	defer r.lock.Unlock()

	r.periodicNAKInterval = nakInterval
}
