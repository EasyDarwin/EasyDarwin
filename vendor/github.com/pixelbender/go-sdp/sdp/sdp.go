package sdp

import (
	"time"
)

// ContentType is the media type for an SDP session description.
const ContentType = "application/sdp"

// Session represents an SDP session description.
type Session struct {
	Version     int         // Protocol Version ("v=")
	Origin      *Origin     // Origin ("o=")
	Name        string      // Session Name ("s=")
	Information string      // Session Information ("i=")
	URI         string      // URI ("u=")
	Email       []string    // Email Address ("e=")
	Phone       []string    // Phone Number ("p=")
	Connection  *Connection // Connection Data ("c=")
	Bandwidth   Bandwidth   // Bandwidth ("b=")
	TimeZone    []*TimeZone // TimeZone ("z=")
	Key         []*Key      // Encryption Keys ("k=")
	Timing      *Timing     // Timing ("t=")
	Repeat      []*Repeat   // Repeat Times ("r=")
	Attributes              // Session Attributes ("a=")
	Media       []*Media    // Media Descriptions ("m=")

	Mode string // Streaming mode ("sendrecv", "recvonly", "sendonly", or "inactive")
}

// String returns the encoded session description as string.
func (s *Session) String() string {
	return string(s.Bytes())
}

// Bytes returns the encoded session description as buffer.
func (s *Session) Bytes() []byte {
	return new(Encoder).session(s).Bytes()
}

// Origin represents an originator of the session.
type Origin struct {
	Username string
	/**
		<sess-id> is a numeric string such that the tuple of <username>,
	      <sess-id>, <nettype>, <addrtype>, and <unicast-address> forms a
	      globally unique identifier for the session.  The method of
	      <sess-id> allocation is up to the creating tool, but it has been
	      suggested that a Network Time Protocol (NTP) format timestamp be
	      used to ensure uniqueness [13].

		some IPC do is a non-numeric string. i.e., o=RTSP Session 0 0 IN IP4 0.0.0.0
	*/
	SessionID      string
	SessionVersion int64
	Network        string
	Type           string
	Address        string
}

// Connection contains connection data.
type Connection struct {
	Network    string
	Type       string
	Address    string
	TTL        int
	AddressNum int
}

// Bandwidth contains session or media bandwidth information.
type Bandwidth map[string]int

// TimeZone represents a time zones change information for a repeated session.
type TimeZone struct {
	Time   time.Time
	Offset time.Duration
}

// Key contains a key exchange information.
// Deprecated: Not recommended, supported for compatibility with older implementations.
type Key struct {
	Method, Value string
}

// Timing specifies start and stop times for a session.
type Timing struct {
	Start time.Time
	Stop  time.Time
}

// Repeat specifies repeat times for a session.
type Repeat struct {
	Interval time.Duration
	Duration time.Duration
	Offsets  []time.Duration
}

// Media contains media description.
type Media struct {
	Type    string
	Port    int
	PortNum int
	Proto   string

	Information string        // Media Information ("i=")
	Connection  []*Connection // Connection Data ("c=")
	Bandwidth   Bandwidth     // Bandwidth ("b=")
	Key         []*Key        // Encryption Keys ("k=")
	Attributes                // Attributes ("a=")

	Mode    string    // Streaming mode ("sendrecv", "recvonly", "sendonly", or "inactive")
	Formats []*Format // Media Formats ("rtpmap")
}

// Streaming modes.
const (
	SendRecv = "sendrecv"
	SendOnly = "sendonly"
	RecvOnly = "recvonly"
	Inactive = "inactive"
)

// NegotiateMode negotiates streaming mode.
func NegotiateMode(local, remote string) string {
	switch local {
	case SendRecv:
		switch remote {
		case RecvOnly:
			return SendOnly
		case SendOnly:
			return RecvOnly
		default:
			return remote
		}
	case SendOnly:
		switch remote {
		case SendRecv, RecvOnly:
			return SendOnly
		}
	case RecvOnly:
		switch remote {
		case SendRecv, SendOnly:
			return RecvOnly
		}
	}
	return Inactive
}

// DeleteAttr removes all elements with name from attrs.
func DeleteAttr(attrs Attributes, name ...string) Attributes {
	n := 0
loop:
	for _, it := range attrs {
		for _, v := range name {
			if it.Name == v {
				continue loop
			}
		}
		attrs[n] = it
		n++
	}
	return attrs[:n]
}

// Format returns format description by payload type.
func (m *Media) Format(pt int) *Format {
	for _, f := range m.Formats {
		if f.Payload == pt {
			return f
		}
	}
	return nil
}

// Format is a media format description represented by "rtpmap" attributes.
type Format struct {
	Payload   int
	Name      string
	ClockRate int
	Channels  int
	Feedback  []string // "rtcp-fb" attributes
	Params    []string // "fmtp" attributes
}

var epoch = time.Date(1900, time.January, 1, 0, 0, 0, 0, time.UTC)

// GetAttribute returns session or first determined media attribute.
func (sess *Session) GetAttribute(name string) string {
	for _, it := range sess.Attributes {
		if it.Name == name {
			return it.Value
		}
	}
	for _, media := range sess.Media {
		for _, it := range media.Attributes {
			if it.Name == name {
				return it.Value
			}
		}
	}
	return ""
}
