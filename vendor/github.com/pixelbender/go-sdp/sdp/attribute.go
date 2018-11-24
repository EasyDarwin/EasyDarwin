package sdp

// Attributes represent a list of SDP attributes.
type Attributes []*Attr

// Has returns presence of attribute by name.
func (a Attributes) Has(name string) bool {
	for _, it := range a {
		if it.Name == name {
			return true
		}
	}
	return false
}

// Get returns first attribute value by name.
func (a Attributes) Get(name string) string {
	for _, it := range a {
		if it.Name == name {
			return it.Value
		}
	}
	return ""
}

// Attr represents session or media attribute.
type Attr struct {
	Name, Value string
}

// NewAttr returns a=<attribute>:<value> attribute.
func NewAttr(attr, value string) *Attr {
	return &Attr{attr, value}
}

// NewAttrFlag returns a=<flag> attribute.
func NewAttrFlag(flag string) *Attr {
	return &Attr{flag, ""}
}

func (a *Attr) String() string {
	if a.Value == "" {
		return a.Name
	}
	return a.Name + ":" + a.Value
}

// Session or media attribute values for indication of a streaming mode.
const (
	ModeSendRecv = "sendrecv"
	ModeRecvOnly = "recvonly"
	ModeSendOnly = "sendonly"
	ModeInactive = "inactive"
)
