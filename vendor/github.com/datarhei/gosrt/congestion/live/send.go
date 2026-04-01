package live

import (
	"container/list"
	"sync"
	"time"

	"github.com/datarhei/gosrt/circular"
	"github.com/datarhei/gosrt/congestion"
	"github.com/datarhei/gosrt/packet"
)

// SendConfig is the configuration for the liveSend congestion control
type SendConfig struct {
	InitialSequenceNumber circular.Number
	DropThreshold         uint64
	MaxBW                 int64
	InputBW               int64
	MinInputBW            int64
	OverheadBW            int64
	OnDeliver             func(p packet.Packet)
}

// sender implements the Sender interface
type sender struct {
	nextSequenceNumber circular.Number
	dropThreshold      uint64

	packetList *list.List
	lossList   *list.List
	lock       sync.RWMutex

	avgPayloadSize float64 // bytes
	pktSndPeriod   float64 // microseconds
	maxBW          float64 // bytes/s
	inputBW        float64 // bytes/s
	overheadBW     float64 // percent

	statistics congestion.SendStats

	probeTime uint64

	rate struct {
		period uint64 // microseconds
		last   uint64

		bytes        uint64
		bytesSent    uint64
		bytesRetrans uint64

		estimatedInputBW float64 // bytes/s
		estimatedSentBW  float64 // bytes/s

		pktLossRate float64
	}

	deliver func(p packet.Packet)
}

// NewSender takes a SendConfig and returns a new Sender
func NewSender(config SendConfig) congestion.Sender {
	s := &sender{
		nextSequenceNumber: config.InitialSequenceNumber,
		dropThreshold:      config.DropThreshold,
		packetList:         list.New(),
		lossList:           list.New(),

		avgPayloadSize: packet.MAX_PAYLOAD_SIZE, //  5.1.2. SRT's Default LiveCC Algorithm
		maxBW:          float64(config.MaxBW),
		inputBW:        float64(config.InputBW),
		overheadBW:     float64(config.OverheadBW),

		deliver: config.OnDeliver,
	}

	if s.deliver == nil {
		s.deliver = func(p packet.Packet) {}
	}

	s.maxBW = 128 * 1024 * 1024 // 1 Gbit/s
	s.pktSndPeriod = (s.avgPayloadSize + 16) * 1_000_000 / s.maxBW

	s.rate.period = uint64(time.Second.Microseconds())
	s.rate.last = 0

	return s
}

func (s *sender) Stats() congestion.SendStats {
	s.lock.Lock()
	defer s.lock.Unlock()

	s.statistics.UsPktSndPeriod = s.pktSndPeriod
	s.statistics.BytePayload = uint64(s.avgPayloadSize)
	s.statistics.MsBuf = 0

	max := s.lossList.Back()
	min := s.lossList.Front()

	if max != nil && min != nil {
		s.statistics.MsBuf = (max.Value.(packet.Packet).Header().PktTsbpdTime - min.Value.(packet.Packet).Header().PktTsbpdTime) / 1_000
	}

	s.statistics.MbpsEstimatedInputBandwidth = s.rate.estimatedInputBW * 8 / 1024 / 1024
	s.statistics.MbpsEstimatedSentBandwidth = s.rate.estimatedSentBW * 8 / 1024 / 1024

	s.statistics.PktLossRate = s.rate.pktLossRate

	return s.statistics
}

func (s *sender) Flush() {
	s.lock.Lock()
	defer s.lock.Unlock()

	s.packetList = s.packetList.Init()
	s.lossList = s.lossList.Init()
}

func (s *sender) Push(p packet.Packet) {
	s.lock.Lock()
	defer s.lock.Unlock()

	if p == nil {
		return
	}

	// Give to the packet a sequence number
	p.Header().PacketSequenceNumber = s.nextSequenceNumber
	p.Header().PacketPositionFlag = packet.SinglePacket
	p.Header().OrderFlag = false
	p.Header().MessageNumber = 1

	s.nextSequenceNumber = s.nextSequenceNumber.Inc()

	pktLen := p.Len()

	s.statistics.PktBuf++
	s.statistics.ByteBuf += pktLen

	// Input bandwidth calculation
	s.rate.bytes += pktLen

	p.Header().Timestamp = uint32(p.Header().PktTsbpdTime & uint64(packet.MAX_TIMESTAMP))

	// Every 16th and 17th packet should be sent at the same time in order
	// for the receiver to determine the link capacity. Not really well
	// documented in the specs.
	// PktTsbpdTime is used for the timing of sending the packets. Here we
	// can modify it because it has already been used to set the packet's
	// timestamp.
	probe := p.Header().PacketSequenceNumber.Val() & 0xF
	if probe == 0 {
		s.probeTime = p.Header().PktTsbpdTime
	} else if probe == 1 {
		p.Header().PktTsbpdTime = s.probeTime
	}

	s.packetList.PushBack(p)

	s.statistics.PktFlightSize = uint64(s.packetList.Len())
}

func (s *sender) Tick(now uint64) {
	// Deliver packets whose PktTsbpdTime is ripe
	s.lock.Lock()
	removeList := make([]*list.Element, 0, s.packetList.Len())
	for e := s.packetList.Front(); e != nil; e = e.Next() {
		p := e.Value.(packet.Packet)
		if p.Header().PktTsbpdTime <= now {
			s.statistics.Pkt++
			s.statistics.PktUnique++

			pktLen := p.Len()

			s.statistics.Byte += pktLen
			s.statistics.ByteUnique += pktLen

			s.statistics.UsSndDuration += uint64(s.pktSndPeriod)

			//  5.1.2. SRT's Default LiveCC Algorithm
			s.avgPayloadSize = 0.875*s.avgPayloadSize + 0.125*float64(pktLen)

			s.rate.bytesSent += pktLen

			s.deliver(p)
			removeList = append(removeList, e)
		} else {
			break
		}
	}

	for _, e := range removeList {
		s.lossList.PushBack(e.Value)
		s.packetList.Remove(e)
	}
	s.lock.Unlock()

	s.lock.Lock()
	removeList = make([]*list.Element, 0, s.lossList.Len())
	for e := s.lossList.Front(); e != nil; e = e.Next() {
		p := e.Value.(packet.Packet)

		if p.Header().PktTsbpdTime+s.dropThreshold <= now {
			// Dropped packet because too old
			s.statistics.PktDrop++
			s.statistics.PktLoss++
			s.statistics.ByteDrop += p.Len()
			s.statistics.ByteLoss += p.Len()

			removeList = append(removeList, e)
		}
	}

	// These packets are not needed anymore (too late)
	for _, e := range removeList {
		p := e.Value.(packet.Packet)

		s.statistics.PktBuf--
		s.statistics.ByteBuf -= p.Len()

		s.lossList.Remove(e)

		// This packet has been ACK'd and we don't need it anymore
		p.Decommission()
	}
	s.lock.Unlock()

	s.lock.Lock()
	tdiff := now - s.rate.last

	if tdiff > s.rate.period {
		s.rate.estimatedInputBW = float64(s.rate.bytes) / (float64(tdiff) / 1000 / 1000)
		s.rate.estimatedSentBW = float64(s.rate.bytesSent) / (float64(tdiff) / 1000 / 1000)
		if s.rate.bytesSent != 0 {
			s.rate.pktLossRate = float64(s.rate.bytesRetrans) / float64(s.rate.bytesSent) * 100
		} else {
			s.rate.pktLossRate = 0
		}

		s.rate.bytes = 0
		s.rate.bytesSent = 0
		s.rate.bytesRetrans = 0

		s.rate.last = now
	}
	s.lock.Unlock()
}

func (s *sender) ACK(sequenceNumber circular.Number) {
	s.lock.Lock()
	defer s.lock.Unlock()

	removeList := make([]*list.Element, 0, s.lossList.Len())
	for e := s.lossList.Front(); e != nil; e = e.Next() {
		p := e.Value.(packet.Packet)
		if p.Header().PacketSequenceNumber.Lt(sequenceNumber) {
			// Remove packet from buffer because it has been successfully transmitted
			removeList = append(removeList, e)
		} else {
			break
		}
	}

	// These packets are not needed anymore (ACK'd)
	for _, e := range removeList {
		p := e.Value.(packet.Packet)

		s.statistics.PktBuf--
		s.statistics.ByteBuf -= p.Len()

		s.lossList.Remove(e)

		// This packet has been ACK'd and we don't need it anymore
		p.Decommission()
	}

	s.pktSndPeriod = (s.avgPayloadSize + 16) * 1000000 / s.maxBW
}

func (s *sender) NAK(sequenceNumbers []circular.Number) {
	if len(sequenceNumbers) == 0 {
		return
	}

	s.lock.Lock()
	defer s.lock.Unlock()

	for e := s.lossList.Back(); e != nil; e = e.Prev() {
		p := e.Value.(packet.Packet)

		for i := 0; i < len(sequenceNumbers); i += 2 {
			if p.Header().PacketSequenceNumber.Gte(sequenceNumbers[i]) && p.Header().PacketSequenceNumber.Lte(sequenceNumbers[i+1]) {
				s.statistics.PktRetrans++
				s.statistics.Pkt++
				s.statistics.PktLoss++

				s.statistics.ByteRetrans += p.Len()
				s.statistics.Byte += p.Len()
				s.statistics.ByteLoss += p.Len()

				//  5.1.2. SRT's Default LiveCC Algorithm
				s.avgPayloadSize = 0.875*s.avgPayloadSize + 0.125*float64(p.Len())

				s.rate.bytesSent += p.Len()
				s.rate.bytesRetrans += p.Len()

				p.Header().RetransmittedPacketFlag = true
				s.deliver(p)
			}
		}
	}
}

func (s *sender) SetDropThreshold(threshold uint64) {
	s.lock.Lock()
	defer s.lock.Unlock()

	s.dropThreshold = threshold
}
