package srt

import (
	"fmt"
	"net/url"
	"strconv"
	"time"
)

const (
	UDP_HEADER_SIZE     = 28
	SRT_HEADER_SIZE     = 16
	MIN_MSS_SIZE        = 76
	MAX_MSS_SIZE        = 1500
	MIN_PAYLOAD_SIZE    = MIN_MSS_SIZE - UDP_HEADER_SIZE - SRT_HEADER_SIZE
	MAX_PAYLOAD_SIZE    = MAX_MSS_SIZE - UDP_HEADER_SIZE - SRT_HEADER_SIZE
	MIN_PASSPHRASE_SIZE = 10
	MAX_PASSPHRASE_SIZE = 79
	MAX_STREAMID_SIZE   = 512
	SRT_VERSION         = 0x010401
)

// Config is the configuration for a SRT connection
type Config struct {
	// Type of congestion control. 'live' or 'file'
	// SRTO_CONGESTION
	Congestion string

	// Connection timeout.
	// SRTO_CONNTIMEO
	ConnectionTimeout time.Duration

	// Enable drift tracer.
	// SRTO_DRIFTTRACER
	DriftTracer bool

	// Reject connection if parties set different passphrase.
	// SRTO_ENFORCEDENCRYPTION
	EnforcedEncryption bool

	// Flow control window size. Packets.
	// SRTO_FC
	FC uint32

	// Accept group connections.
	// SRTO_GROUPCONNECT
	GroupConnect bool

	// Group stability timeout.
	// SRTO_GROUPSTABTIMEO
	GroupStabilityTimeout time.Duration

	// Input bandwidth. Bytes.
	// SRTO_INPUTBW
	InputBW int64

	// IP socket type of service
	// SRTO_IPTOS
	IPTOS int

	// Defines IP socket "time to live" option.
	// SRTO_IPTTL
	IPTTL int

	// Allow only IPv6.
	// SRTO_IPV6ONLY
	IPv6Only int

	// Duration of Stream Encryption key switchover. Packets.
	// SRTO_KMPREANNOUNCE
	KMPreAnnounce uint64

	// Stream encryption key refresh rate. Packets.
	// SRTO_KMREFRESHRATE
	KMRefreshRate uint64

	// Defines the maximum accepted transmission latency.
	// SRTO_LATENCY
	Latency time.Duration

	// Packet reorder tolerance.
	// SRTO_LOSSMAXTTL
	LossMaxTTL uint32

	// Bandwidth limit in bytes/s.
	// SRTO_MAXBW
	MaxBW int64

	// Enable SRT message mode.
	// SRTO_MESSAGEAPI
	MessageAPI bool

	// Minimum input bandwidth
	// This option is effective only if both SRTO_MAXBW and SRTO_INPUTBW are set to 0. It controls the minimum allowed value of the input bitrate estimate.
	// SRTO_MININPUTBW
	MinInputBW int64

	// Minimum SRT library version of a peer.
	// SRTO_MINVERSION
	MinVersion uint32

	// MTU size
	// SRTO_MSS
	MSS uint32

	// Enable periodic NAK reports
	// SRTO_NAKREPORT
	NAKReport bool

	// Limit bandwidth overhead, percents
	// SRTO_OHEADBW
	OverheadBW int64

	// Set up the packet filter.
	// SRTO_PACKETFILTER
	PacketFilter string

	// Password for the encrypted transmission.
	// SRTO_PASSPHRASE
	Passphrase string

	// Maximum payload size. Bytes.
	// SRTO_PAYLOADSIZE
	PayloadSize uint32

	// Crypto key length in bytes.
	// SRTO_PBKEYLEN
	PBKeylen int

	// Peer idle timeout.
	// SRTO_PEERIDLETIMEO
	PeerIdleTimeout time.Duration

	// Minimum receiver latency to be requested by sender.
	// SRTO_PEERLATENCY
	PeerLatency time.Duration

	// Receiver buffer size. Bytes.
	// SRTO_RCVBUF
	ReceiverBufferSize uint32

	// Receiver-side latency.
	// SRTO_RCVLATENCY
	ReceiverLatency time.Duration

	// Sender buffer size. Bytes.
	// SRTO_SNDBUF
	SendBufferSize uint32

	// Sender's delay before dropping packets.
	// SRTO_SNDDROPDELAY
	SendDropDelay time.Duration

	// Stream ID (settable in caller mode only, visible on the listener peer)
	// SRTO_STREAMID
	StreamId string

	// Drop too late packets.
	// SRTO_TLPKTDROP
	TooLatePacketDrop bool

	// Transmission type. 'live' or 'file'.
	// SRTO_TRANSTYPE
	TransmissionType string

	// Timestamp-based packet delivery mode.
	// SRTO_TSBPDMODE
	TSBPDMode bool

	// An implementation of the Logger interface
	Logger Logger
}

// DefaultConfig is the default configuration for a SRT connection
// if no individual configuration has been provided.
var defaultConfig Config = Config{
	Congestion:            "live",
	ConnectionTimeout:     3 * time.Second,
	DriftTracer:           true,
	EnforcedEncryption:    true,
	FC:                    25600,
	GroupConnect:          false,
	GroupStabilityTimeout: 0,
	InputBW:               0,
	IPTOS:                 0,
	IPTTL:                 0,
	IPv6Only:              -1,
	KMPreAnnounce:         1 << 12,
	KMRefreshRate:         1 << 24,
	Latency:               -1,
	LossMaxTTL:            0,
	MaxBW:                 -1,
	MessageAPI:            false,
	MinVersion:            SRT_VERSION,
	MSS:                   MAX_MSS_SIZE,
	NAKReport:             true,
	OverheadBW:            25,
	PacketFilter:          "",
	Passphrase:            "",
	PayloadSize:           MAX_PAYLOAD_SIZE,
	PBKeylen:              16,
	PeerIdleTimeout:       2 * time.Second,
	PeerLatency:           120 * time.Millisecond,
	ReceiverBufferSize:    0,
	ReceiverLatency:       120 * time.Millisecond,
	SendBufferSize:        0,
	SendDropDelay:         1 * time.Second,
	StreamId:              "",
	TooLatePacketDrop:     true,
	TransmissionType:      "live",
	TSBPDMode:             true,
}

// DefaultConfig returns the default configuration for Dial and Listen.
func DefaultConfig() Config {
	return defaultConfig
}

// UnmarshalURL takes a SRT URL and parses out the configuration. A SRT URL is
// srt://[host]:[port]?[key1]=[value1]&[key2]=[value2]... It returns the host:port
// of the URL.
func (c *Config) UnmarshalURL(srturl string) (string, error) {
	u, err := url.Parse(srturl)
	if err != nil {
		return "", err
	}

	if u.Scheme != "srt" {
		return "", fmt.Errorf("the URL doesn't seem to be an srt:// URL")
	}

	return u.Host, c.UnmarshalQuery(u.RawQuery)
}

// UnmarshalQuery parses a query string and interprets it as a configuration
// for a SRT connection. The key in each key/value pair corresponds to the
// respective field in the Config type, but with only lower case letters. Bool
// values can be represented as "true"/"false", "on"/"off", "yes"/"no", or "0"/"1".
func (c *Config) UnmarshalQuery(query string) error {
	v, err := url.ParseQuery(query)
	if err != nil {
		return err
	}

	// https://github.com/Haivision/srt/blob/master/docs/apps/srt-live-transmit.md

	if s := v.Get("congestion"); len(s) != 0 {
		c.Congestion = s
	}

	if s := v.Get("conntimeo"); len(s) != 0 {
		if d, err := strconv.Atoi(s); err == nil {
			c.ConnectionTimeout = time.Duration(d) * time.Millisecond
		}
	}

	if s := v.Get("drifttracer"); len(s) != 0 {
		switch s {
		case "yes", "on", "true", "1":
			c.DriftTracer = true
		case "no", "off", "false", "0":
			c.DriftTracer = false
		}
	}

	if s := v.Get("enforcedencryption"); len(s) != 0 {
		switch s {
		case "yes", "on", "true", "1":
			c.EnforcedEncryption = true
		case "no", "off", "false", "0":
			c.EnforcedEncryption = false
		}
	}

	if s := v.Get("fc"); len(s) != 0 {
		if d, err := strconv.ParseUint(s, 10, 32); err == nil {
			c.FC = uint32(d)
		}
	}

	if s := v.Get("groupconnect"); len(s) != 0 {
		switch s {
		case "yes", "on", "true", "1":
			c.GroupConnect = true
		case "no", "off", "false", "0":
			c.GroupConnect = false
		}
	}

	if s := v.Get("groupstabtimeo"); len(s) != 0 {
		if d, err := strconv.Atoi(s); err == nil {
			c.GroupStabilityTimeout = time.Duration(d) * time.Millisecond
		}
	}

	if s := v.Get("inputbw"); len(s) != 0 {
		if d, err := strconv.ParseInt(s, 10, 64); err == nil {
			c.InputBW = d
		}
	}

	if s := v.Get("iptos"); len(s) != 0 {
		if d, err := strconv.Atoi(s); err == nil {
			c.IPTOS = d
		}
	}

	if s := v.Get("ipttl"); len(s) != 0 {
		if d, err := strconv.Atoi(s); err == nil {
			c.IPTTL = d
		}
	}

	if s := v.Get("ipv6only"); len(s) != 0 {
		if d, err := strconv.Atoi(s); err == nil {
			c.IPv6Only = d
		}
	}

	if s := v.Get("kmpreannounce"); len(s) != 0 {
		if d, err := strconv.ParseUint(s, 10, 64); err == nil {
			c.KMPreAnnounce = d
		}
	}

	if s := v.Get("kmrefreshrate"); len(s) != 0 {
		if d, err := strconv.ParseUint(s, 10, 64); err == nil {
			c.KMRefreshRate = d
		}
	}

	if s := v.Get("latency"); len(s) != 0 {
		if d, err := strconv.Atoi(s); err == nil {
			c.Latency = time.Duration(d) * time.Millisecond
		}
	}

	if s := v.Get("lossmaxttl"); len(s) != 0 {
		if d, err := strconv.ParseUint(s, 10, 32); err == nil {
			c.LossMaxTTL = uint32(d)
		}
	}

	if s := v.Get("maxbw"); len(s) != 0 {
		if d, err := strconv.ParseInt(s, 10, 64); err == nil {
			c.MaxBW = d
		}
	}

	if s := v.Get("mininputbw"); len(s) != 0 {
		if d, err := strconv.ParseInt(s, 10, 64); err == nil {
			c.MinInputBW = d
		}
	}

	if s := v.Get("messageapi"); len(s) != 0 {
		switch s {
		case "yes", "on", "true", "1":
			c.MessageAPI = true
		case "no", "off", "false", "0":
			c.MessageAPI = false
		}
	}

	// minversion is ignored

	if s := v.Get("mss"); len(s) != 0 {
		if d, err := strconv.ParseUint(s, 10, 32); err == nil {
			c.MSS = uint32(d)
		}
	}

	if s := v.Get("nakreport"); len(s) != 0 {
		switch s {
		case "yes", "on", "true", "1":
			c.NAKReport = true
		case "no", "off", "false", "0":
			c.NAKReport = false
		}
	}

	if s := v.Get("oheadbw"); len(s) != 0 {
		if d, err := strconv.ParseInt(s, 10, 64); err == nil {
			c.OverheadBW = d
		}
	}

	if s := v.Get("packetfilter"); len(s) != 0 {
		c.PacketFilter = s
	}

	if s := v.Get("passphrase"); len(s) != 0 {
		c.Passphrase = s
	}

	if s := v.Get("payloadsize"); len(s) != 0 {
		if d, err := strconv.ParseUint(s, 10, 32); err == nil {
			c.PayloadSize = uint32(d)
		}
	}

	if s := v.Get("pbkeylen"); len(s) != 0 {
		if d, err := strconv.Atoi(s); err == nil {
			c.PBKeylen = d
		}
	}

	if s := v.Get("peeridletimeo"); len(s) != 0 {
		if d, err := strconv.Atoi(s); err == nil {
			c.PeerIdleTimeout = time.Duration(d) * time.Millisecond
		}
	}

	if s := v.Get("peerlatency"); len(s) != 0 {
		if d, err := strconv.Atoi(s); err == nil {
			c.PeerLatency = time.Duration(d) * time.Millisecond
		}
	}

	if s := v.Get("rcvbuf"); len(s) != 0 {
		if d, err := strconv.ParseUint(s, 10, 32); err == nil {
			c.ReceiverBufferSize = uint32(d)
		}
	}

	if s := v.Get("rcvlatency"); len(s) != 0 {
		if d, err := strconv.Atoi(s); err == nil {
			c.ReceiverLatency = time.Duration(d) * time.Millisecond
		}
	}

	// retransmitalgo not implemented (there's only one)

	if s := v.Get("sndbuf"); len(s) != 0 {
		if d, err := strconv.ParseUint(s, 10, 32); err == nil {
			c.SendBufferSize = uint32(d)
		}
	}

	if s := v.Get("snddropdelay"); len(s) != 0 {
		if d, err := strconv.Atoi(s); err == nil {
			c.SendDropDelay = time.Duration(d) * time.Millisecond
		}
	}

	if s := v.Get("streamid"); len(s) != 0 {
		c.StreamId = s
	}

	if s := v.Get("tlpktdrop"); len(s) != 0 {
		switch s {
		case "yes", "on", "true", "1":
			c.TooLatePacketDrop = true
		case "no", "off", "false", "0":
			c.TooLatePacketDrop = false
		}
	}

	if s := v.Get("transtype"); len(s) != 0 {
		c.TransmissionType = s
	}

	if s := v.Get("tsbpdmode"); len(s) != 0 {
		switch s {
		case "yes", "on", "true", "1":
			c.TSBPDMode = true
		case "no", "off", "false", "0":
			c.TSBPDMode = false
		}
	}

	return nil
}

// MarshalURL returns the SRT URL for this config and the given address (host:port).
func (c *Config) MarshalURL(address string) string {
	return "srt://" + address + "?" + c.MarshalQuery()
}

// MarshalQuery returns the corresponding query string for a configuration.
func (c *Config) MarshalQuery() string {
	q := url.Values{}

	if c.Congestion != defaultConfig.Congestion {
		q.Set("congestion", c.Congestion)
	}

	if c.ConnectionTimeout != defaultConfig.ConnectionTimeout {
		q.Set("conntimeo", strconv.FormatInt(c.ConnectionTimeout.Milliseconds(), 10))
	}

	if c.DriftTracer != defaultConfig.DriftTracer {
		q.Set("drifttracer", strconv.FormatBool(c.DriftTracer))
	}

	if c.EnforcedEncryption != defaultConfig.EnforcedEncryption {
		q.Set("enforcedencryption", strconv.FormatBool(c.EnforcedEncryption))
	}

	if c.FC != defaultConfig.FC {
		q.Set("fc", strconv.FormatUint(uint64(c.FC), 10))
	}

	if c.GroupConnect != defaultConfig.GroupConnect {
		q.Set("groupconnect", strconv.FormatBool(c.GroupConnect))
	}

	if c.GroupStabilityTimeout != defaultConfig.GroupStabilityTimeout {
		q.Set("groupstabtimeo", strconv.FormatInt(c.GroupStabilityTimeout.Milliseconds(), 10))
	}

	if c.InputBW != defaultConfig.InputBW {
		q.Set("inputbw", strconv.FormatInt(c.InputBW, 10))
	}

	if c.IPTOS != defaultConfig.IPTOS {
		q.Set("iptos", strconv.FormatInt(int64(c.IPTOS), 10))
	}

	if c.IPTTL != defaultConfig.IPTTL {
		q.Set("ipttl", strconv.FormatInt(int64(c.IPTTL), 10))
	}

	if c.IPv6Only != defaultConfig.IPv6Only {
		q.Set("ipv6only", strconv.FormatInt(int64(c.IPv6Only), 10))
	}

	if len(c.Passphrase) != 0 {
		if c.KMPreAnnounce != defaultConfig.KMPreAnnounce {
			q.Set("kmpreannounce", strconv.FormatUint(c.KMPreAnnounce, 10))
		}

		if c.KMRefreshRate != defaultConfig.KMRefreshRate {
			q.Set("kmrefreshrate", strconv.FormatUint(c.KMRefreshRate, 10))
		}
	}

	if c.Latency != defaultConfig.Latency {
		q.Set("latency", strconv.FormatInt(c.Latency.Milliseconds(), 10))
	}

	if c.LossMaxTTL != defaultConfig.LossMaxTTL {
		q.Set("lossmaxttl", strconv.FormatInt(int64(c.LossMaxTTL), 10))
	}

	if c.MaxBW != defaultConfig.MaxBW {
		q.Set("maxbw", strconv.FormatInt(c.MaxBW, 10))
	}

	if c.MinInputBW != defaultConfig.InputBW {
		q.Set("mininputbw", strconv.FormatInt(c.MinInputBW, 10))
	}

	if c.MessageAPI != defaultConfig.MessageAPI {
		q.Set("messageapi", strconv.FormatBool(c.MessageAPI))
	}

	if c.MSS != defaultConfig.MSS {
		q.Set("mss", strconv.FormatUint(uint64(c.MSS), 10))
	}

	if c.NAKReport != defaultConfig.NAKReport {
		q.Set("nakreport", strconv.FormatBool(c.NAKReport))
	}

	if c.OverheadBW != defaultConfig.OverheadBW {
		q.Set("oheadbw", strconv.FormatInt(c.OverheadBW, 10))
	}

	if c.PacketFilter != defaultConfig.PacketFilter {
		q.Set("packetfilter", c.PacketFilter)
	}

	if len(c.Passphrase) != 0 {
		q.Set("passphrase", c.Passphrase)
	}

	if c.PayloadSize != defaultConfig.PayloadSize {
		q.Set("payloadsize", strconv.FormatUint(uint64(c.PayloadSize), 10))
	}

	if c.PBKeylen != defaultConfig.PBKeylen {
		q.Set("pbkeylen", strconv.FormatInt(int64(c.PBKeylen), 10))
	}

	if c.PeerIdleTimeout != defaultConfig.PeerIdleTimeout {
		q.Set("peeridletimeo", strconv.FormatInt(c.PeerIdleTimeout.Milliseconds(), 10))
	}

	if c.PeerLatency != defaultConfig.PeerLatency {
		q.Set("peerlatency", strconv.FormatInt(c.PeerLatency.Milliseconds(), 10))
	}

	if c.ReceiverBufferSize != defaultConfig.ReceiverBufferSize {
		q.Set("rcvbuf", strconv.FormatInt(int64(c.ReceiverBufferSize), 10))
	}

	if c.ReceiverLatency != defaultConfig.ReceiverLatency {
		q.Set("rcvlatency", strconv.FormatInt(c.ReceiverLatency.Milliseconds(), 10))
	}

	if c.SendBufferSize != defaultConfig.SendBufferSize {
		q.Set("sndbuf", strconv.FormatInt(int64(c.SendBufferSize), 10))
	}

	if c.SendDropDelay != defaultConfig.SendDropDelay {
		q.Set("snddropdelay", strconv.FormatInt(c.SendDropDelay.Milliseconds(), 10))
	}

	if len(c.StreamId) != 0 {
		q.Set("streamid", c.StreamId)
	}

	if c.TooLatePacketDrop != defaultConfig.TooLatePacketDrop {
		q.Set("tlpktdrop", strconv.FormatBool(c.TooLatePacketDrop))
	}

	if c.TransmissionType != defaultConfig.TransmissionType {
		q.Set("transtype", c.TransmissionType)
	}

	if c.TSBPDMode != defaultConfig.TSBPDMode {
		q.Set("tsbpdmode", strconv.FormatBool(c.TSBPDMode))
	}

	return q.Encode()
}

// Validate validates a configuration, returns an error if a field
// has an invalid value.
func (c *Config) Validate() error {
	if c.TransmissionType != "live" {
		return fmt.Errorf("config: TransmissionType must be 'live'")
	}

	c.Congestion = "live"
	c.NAKReport = true
	c.TooLatePacketDrop = true
	c.TSBPDMode = true

	if c.Congestion != "live" {
		return fmt.Errorf("config: Congestion mode must be 'live'")
	}

	if c.ConnectionTimeout <= 0 {
		return fmt.Errorf("config: ConnectionTimeout must be greater than 0")
	}

	if c.GroupConnect {
		return fmt.Errorf("config: GroupConnect is not supported")
	}

	if c.IPTOS > 0 && c.IPTOS > 255 {
		return fmt.Errorf("config: IPTOS must be lower than 255")
	}

	if c.IPTTL > 0 && c.IPTTL > 255 {
		return fmt.Errorf("config: IPTTL must be between 1 and 255")
	}

	if c.IPv6Only > 0 {
		return fmt.Errorf("config: IPv6Only is not supported")
	}

	if c.KMRefreshRate != 0 {
		if c.KMPreAnnounce < 1 || c.KMPreAnnounce > c.KMRefreshRate/2 {
			return fmt.Errorf("config: KMPreAnnounce must be greater than 1 and smaller than KMRefreshRate/2")
		}
	}

	if c.Latency >= 0 {
		c.PeerLatency = c.Latency
		c.ReceiverLatency = c.Latency
	}

	if c.MinVersion != SRT_VERSION {
		return fmt.Errorf("config: MinVersion must be %#06x", SRT_VERSION)
	}

	if c.MSS < MIN_MSS_SIZE || c.MSS > MAX_MSS_SIZE {
		return fmt.Errorf("config: MSS must be between %d and %d (both inclusive)", MIN_MSS_SIZE, MAX_MSS_SIZE)
	}

	if !c.NAKReport {
		return fmt.Errorf("config: NAKReport must be enabled")
	}

	if c.OverheadBW < 10 || c.OverheadBW > 100 {
		return fmt.Errorf("config: OverheadBW must be between 10 and 100")
	}

	if len(c.PacketFilter) != 0 {
		return fmt.Errorf("config: PacketFilter are not supported")
	}

	if len(c.Passphrase) != 0 {
		if len(c.Passphrase) < MIN_PASSPHRASE_SIZE || len(c.Passphrase) > MAX_PASSPHRASE_SIZE {
			return fmt.Errorf("config: Passphrase must be between %d and %d bytes long", MIN_PASSPHRASE_SIZE, MAX_PASSPHRASE_SIZE)
		}
	}

	if c.PayloadSize < MIN_PAYLOAD_SIZE || c.PayloadSize > MAX_PAYLOAD_SIZE {
		return fmt.Errorf("config: PayloadSize must be between %d and %d (both inclusive)", MIN_PAYLOAD_SIZE, MAX_PAYLOAD_SIZE)
	}

	if c.PayloadSize > c.MSS-uint32(SRT_HEADER_SIZE+UDP_HEADER_SIZE) {
		return fmt.Errorf("config: PayloadSize must not be larger than %d (MSS - %d)", c.MSS-uint32(SRT_HEADER_SIZE+UDP_HEADER_SIZE), SRT_HEADER_SIZE-UDP_HEADER_SIZE)
	}

	if c.PBKeylen != 16 && c.PBKeylen != 24 && c.PBKeylen != 32 {
		return fmt.Errorf("config: PBKeylen must be 16, 24, or 32 bytes")
	}

	if c.PeerLatency < 0 {
		return fmt.Errorf("config: PeerLatency must be greater than 0")
	}

	if c.ReceiverLatency < 0 {
		return fmt.Errorf("config: ReceiverLatency must be greater than 0")
	}

	if c.SendDropDelay < 0 {
		return fmt.Errorf("config: SendDropDelay must be greater than 0")
	}

	if len(c.StreamId) > MAX_STREAMID_SIZE {
		return fmt.Errorf("config: StreamId must be shorter than or equal to %d bytes", MAX_STREAMID_SIZE)
	}

	if !c.TooLatePacketDrop {
		return fmt.Errorf("config: TooLatePacketDrop must be enabled")
	}

	if c.TransmissionType != "live" {
		return fmt.Errorf("config: TransmissionType must be 'live'")
	}

	if !c.TSBPDMode {
		return fmt.Errorf("config: TSBPDMode must be enabled")
	}

	return nil
}
