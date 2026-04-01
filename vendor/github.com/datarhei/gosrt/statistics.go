// https://github.com/Haivision/srt/blob/master/docs/API/statistics.md

package srt

// Statistics represents the statistics for a connection
type Statistics struct {
	MsTimeStamp uint64 // The time elapsed, in milliseconds, since the SRT socket has been created

	// Accumulated
	Accumulated StatisticsAccumulated

	// Interval
	Interval StatisticsInterval

	// Instantaneous
	Instantaneous StatisticsInstantaneous
}

type StatisticsAccumulated struct {
	PktSent          uint64 // The total number of sent DATA packets, including retransmitted packets
	PktRecv          uint64 // The total number of received DATA packets, including retransmitted packets
	PktSentUnique    uint64 // The total number of unique DATA packets sent by the SRT sender
	PktRecvUnique    uint64 // The total number of unique original, retransmitted or recovered by the packet filter DATA packets received in time, decrypted without errors and, as a result, scheduled for delivery to the upstream application by the SRT receiver.
	PktSendLoss      uint64 // The total number of data packets considered or reported as lost at the sender side. Does not correspond to the packets detected as lost at the receiver side.
	PktRecvLoss      uint64 // The total number of SRT DATA packets detected as presently missing (either reordered or lost) at the receiver side
	PktRetrans       uint64 // The total number of retransmitted packets sent by the SRT sender
	PktRecvRetrans   uint64 // The total number of retransmitted packets registered at the receiver side
	PktSentACK       uint64 // The total number of sent ACK (Acknowledgement) control packets
	PktRecvACK       uint64 // The total number of received ACK (Acknowledgement) control packets
	PktSentNAK       uint64 // The total number of sent NAK (Negative Acknowledgement) control packets
	PktRecvNAK       uint64 // The total number of received NAK (Negative Acknowledgement) control packets
	PktSentKM        uint64 // The total number of sent KM (Key Material) control packets
	PktRecvKM        uint64 // The total number of received KM (Key Material) control packets
	UsSndDuration    uint64 // The total accumulated time in microseconds, during which the SRT sender has some data to transmit, including packets that have been sent, but not yet acknowledged
	PktRecvBelated   uint64
	PktSendDrop      uint64 // The total number of dropped by the SRT sender DATA packets that have no chance to be delivered in time
	PktRecvDrop      uint64 // The total number of dropped by the SRT receiver and, as a result, not delivered to the upstream application DATA packets
	PktRecvUndecrypt uint64 // The total number of packets that failed to be decrypted at the receiver side

	ByteSent          uint64 // Same as pktSent, but expressed in bytes, including payload and all the headers (IP, TCP, SRT)
	ByteRecv          uint64 // Same as pktRecv, but expressed in bytes, including payload and all the headers (IP, TCP, SRT)
	ByteSentUnique    uint64 // Same as pktSentUnique, but expressed in bytes, including payload and all the headers (IP, TCP, SRT)
	ByteRecvUnique    uint64 // Same as pktRecvUnique, but expressed in bytes, including payload and all the headers (IP, TCP, SRT)
	ByteRecvLoss      uint64 // Same as pktRecvLoss, but expressed in bytes, including payload and all the headers (IP, TCP, SRT), bytes for the presently missing (either reordered or lost) packets' payloads are estimated based on the average packet size
	ByteRetrans       uint64 // Same as pktRetrans, but expressed in bytes, including payload and all the headers (IP, TCP, SRT)
	ByteRecvRetrans   uint64 // Same as pktRecvRetrans, but expressed in bytes, including payload and all the headers (IP, TCP, SRT)
	ByteRecvBelated   uint64
	ByteSendDrop      uint64 // Same as pktSendDrop, but expressed in bytes, including payload and all the headers (IP, TCP, SRT)
	ByteRecvDrop      uint64 // Same as pktRecvDrop, but expressed in bytes, including payload and all the headers (IP, TCP, SRT)
	ByteRecvUndecrypt uint64 // Same as pktRecvUndecrypt, but expressed in bytes, including payload and all the headers (IP, TCP, SRT)
}

type StatisticsInterval struct {
	MsInterval uint64 // Length of the interval, in milliseconds

	PktSent        uint64 // Number of sent DATA packets, including retransmitted packets
	PktRecv        uint64 // Number of received DATA packets, including retransmitted packets
	PktSentUnique  uint64 // Number of unique DATA packets sent by the SRT sender
	PktRecvUnique  uint64 // Number of unique original, retransmitted or recovered by the packet filter DATA packets received in time, decrypted without errors and, as a result, scheduled for delivery to the upstream application by the SRT receiver.
	PktSendLoss    uint64 // Number of data packets considered or reported as lost at the sender side. Does not correspond to the packets detected as lost at the receiver side.
	PktRecvLoss    uint64 // Number of SRT DATA packets detected as presently missing (either reordered or lost) at the receiver side
	PktRetrans     uint64 // Number of retransmitted packets sent by the SRT sender
	PktRecvRetrans uint64 // Number of retransmitted packets registered at the receiver side
	PktSentACK     uint64 // Number of sent ACK (Acknowledgement) control packets
	PktRecvACK     uint64 // Number of received ACK (Acknowledgement) control packets
	PktSentNAK     uint64 // Number of sent NAK (Negative Acknowledgement) control packets
	PktRecvNAK     uint64 // Number of received NAK (Negative Acknowledgement) control packets

	MbpsSendRate float64 // Sending rate, in Mbps
	MbpsRecvRate float64 // Receiving rate, in Mbps

	UsSndDuration uint64 // Accumulated time in microseconds, during which the SRT sender has some data to transmit, including packets that have been sent, but not yet acknowledged

	PktReorderDistance uint64
	PktRecvBelated     uint64 // Number of packets that arrive too late
	PktSndDrop         uint64 // Number of dropped by the SRT sender DATA packets that have no chance to be delivered in time
	PktRecvDrop        uint64 // Number of dropped by the SRT receiver and, as a result, not delivered to the upstream application DATA packets
	PktRecvUndecrypt   uint64 // Number of packets that failed to be decrypted at the receiver side

	ByteSent          uint64 // Same as pktSent, but expressed in bytes, including payload and all the headers (IP, TCP, SRT)
	ByteRecv          uint64 // Same as pktRecv, but expressed in bytes, including payload and all the headers (IP, TCP, SRT)
	ByteSentUnique    uint64 // Same as pktSentUnique, but expressed in bytes, including payload and all the headers (IP, TCP, SRT)
	ByteRecvUnique    uint64 // Same as pktRecvUnique, but expressed in bytes, including payload and all the headers (IP, TCP, SRT)
	ByteRecvLoss      uint64 // Same as pktRecvLoss, but expressed in bytes, including payload and all the headers (IP, TCP, SRT), bytes for the presently missing (either reordered or lost) packets' payloads are estimated based on the average packet size
	ByteRetrans       uint64 // Same as pktRetrans, but expressed in bytes, including payload and all the headers (IP, TCP, SRT)
	ByteRecvRetrans   uint64 // Same as pktRecvRetrans, but expressed in bytes, including payload and all the headers (IP, TCP, SRT)
	ByteRecvBelated   uint64 // Same as pktRecvBelated, but expressed in bytes, including payload and all the headers (IP, TCP, SRT)
	ByteSendDrop      uint64 // Same as pktSendDrop, but expressed in bytes, including payload and all the headers (IP, TCP, SRT)
	ByteRecvDrop      uint64 // Same as pktRecvDrop, but expressed in bytes, including payload and all the headers (IP, TCP, SRT)
	ByteRecvUndecrypt uint64 // Same as pktRecvUndecrypt, but expressed in bytes, including payload and all the headers (IP, TCP, SRT)
}

type StatisticsInstantaneous struct {
	UsPktSendPeriod       float64 // Current minimum time interval between which consecutive packets are sent, in microseconds
	PktFlowWindow         uint64  // The maximum number of packets that can be "in flight"
	PktFlightSize         uint64  // The number of packets in flight
	MsRTT                 float64 // Smoothed round-trip time (SRTT), an exponentially-weighted moving average (EWMA) of an endpoint's RTT samples, in milliseconds
	MbpsSentRate          float64 // Current transmission bandwidth, in Mbps
	MbpsRecvRate          float64 // Current receiving bandwidth, in Mbps
	MbpsLinkCapacity      float64 // Estimated capacity of the network link, in Mbps
	ByteAvailSendBuf      uint64  // The available space in the sender's buffer, in bytes
	ByteAvailRecvBuf      uint64  // The available space in the receiver's buffer, in bytes
	MbpsMaxBW             float64 // Transmission bandwidth limit, in Mbps
	ByteMSS               uint64  // Maximum Segment Size (MSS), in bytes
	PktSendBuf            uint64  // The number of packets in the sender's buffer that are already scheduled for sending or even possibly sent, but not yet acknowledged
	ByteSendBuf           uint64  // Instantaneous (current) value of pktSndBuf, but expressed in bytes, including payload and all headers (IP, TCP, SRT)
	MsSendBuf             uint64  // The timespan (msec) of packets in the sender's buffer (unacknowledged packets)
	MsSendTsbPdDelay      uint64  // Timestamp-based Packet Delivery Delay value of the peer
	PktRecvBuf            uint64  // The number of acknowledged packets in receiver's buffer
	ByteRecvBuf           uint64  // Instantaneous (current) value of pktRcvBuf, expressed in bytes, including payload and all headers (IP, TCP, SRT)
	MsRecvBuf             uint64  // The timespan (msec) of acknowledged packets in the receiver's buffer
	MsRecvTsbPdDelay      uint64  // Timestamp-based Packet Delivery Delay value set on the socket via SRTO_RCVLATENCY or SRTO_LATENCY
	PktReorderTolerance   uint64  // Instant value of the packet reorder tolerance
	PktRecvAvgBelatedTime uint64  // Accumulated difference between the current time and the time-to-play of a packet that is received late
	PktSendLossRate       float64 // Percentage of resent data vs. sent data
	PktRecvLossRate       float64 // Percentage of retransmitted data vs. received data
}
