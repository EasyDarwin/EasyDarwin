package sdp

import (
	"io"
	"strconv"
	"time"
)

// An Encoder writes a session description to a buffer.
type Encoder struct {
	w       io.Writer
	buf     []byte
	pos     int
	newline bool
}

// NewEncoder returns a new encoder that writes to w.
func NewEncoder(w io.Writer) *Encoder {
	return &Encoder{w: w}
}

// Encode encodes the session description.
func (e *Encoder) Encode(s *Session) error {
	e.Reset()
	e.session(s)
	if e.w != nil {
		_, err := e.w.Write(e.Bytes())
		if err != nil {
			return err
		}
	}
	return nil
}

// Reset resets encoder state to be empty.
func (e *Encoder) Reset() {
	e.pos, e.newline = 0, false
}

func (e *Encoder) session(s *Session) *Encoder {
	e.add('v').int(int64(s.Version))
	if s.Origin != nil {
		e.add('o').origin(s.Origin)
	}
	e.add('s').str(s.Name)
	if s.Information != "" {
		e.add('i').str(s.Information)
	}
	if s.URI != "" {
		e.add('u').str(s.URI)
	}
	for _, it := range s.Email {
		e.add('e').str(it)
	}
	for _, it := range s.Phone {
		e.add('p').str(it)
	}
	if s.Connection != nil {
		e.add('c').connection(s.Connection)
	}
	for t, v := range s.Bandwidth {
		e.add('b').bandwidth(t, v)
	}
	if len(s.TimeZone) > 0 {
		e.add('z').timezone(s.TimeZone)
	}
	for _, it := range s.Key {
		e.add('k').key(it)
	}
	e.add('t').timing(s.Timing)
	for _, it := range s.Repeat {
		e.add('r').repeat(it)
	}
	if s.Mode != "" {
		e.add('a').str(s.Mode)
	}
	for _, it := range s.Attributes {
		e.add('a').attr(it)
	}
	for _, it := range s.Media {
		e.media(it)
	}
	return e
}

func (e *Encoder) media(m *Media) *Encoder {
	e.add('m').str(m.Type).sp().int(int64(m.Port))
	if m.PortNum > 0 {
		e.char('/').int(int64(m.PortNum))
	}
	e.sp().str(m.Proto)
	for _, it := range m.Formats {
		e.sp().int(int64(it.Payload))
	}
	if len(m.Formats) == 0 {
		e.sp().char('*')
	}
	if m.Information != "" {
		e.add('i').str(m.Information)
	}
	for _, it := range m.Connection {
		e.add('c').connection(it)
	}
	for t, v := range m.Bandwidth {
		e.add('b').bandwidth(t, v)
	}
	for _, it := range m.Key {
		e.add('k').key(it)
	}
	for _, it := range m.Formats {
		e.format(it)
	}
	if m.Mode != "" {
		e.add('a').str(m.Mode)
	}
	for _, it := range m.Attributes {
		e.add('a').attr(it)
	}
	return e
}

func (e *Encoder) format(f *Format) *Encoder {
	p := int64(f.Payload)
	if f.Name != "" {
		e.add('a').str("rtpmap:").int(p).sp().str(f.Name).char('/').int(int64(f.ClockRate))
		if f.Channels > 0 {
			e.char('/').int(int64(f.Channels))
		}
	}
	for _, it := range f.Feedback {
		e.add('a').str("rtcp-fb:").int(p).sp().str(it)
	}
	for _, it := range f.Params {
		e.add('a').str("fmtp:").int(p).sp().str(it)
	}
	return e
}

func (e *Encoder) attr(a *Attr) *Encoder {
	if a.Value == "" {
		return e.str(a.Name)
	}
	return e.str(a.Name).char(':').str(a.Value)
}

func (e *Encoder) timezone(z []*TimeZone) *Encoder {
	for i, it := range z {
		if i > 0 {
			e.char(' ')
		}
		e.time(it.Time).sp().duration(it.Offset)
	}
	return e
}

func (e *Encoder) timing(t *Timing) *Encoder {
	if t == nil {
		return e.str("0 0")
	}
	return e.time(t.Start).sp().time(t.Stop)
}

func (e *Encoder) repeat(r *Repeat) *Encoder {
	e.duration(r.Interval).sp().duration(r.Duration)
	for _, it := range r.Offsets {
		e.sp().duration(it)
	}
	return e
}

func (e *Encoder) time(t time.Time) *Encoder {
	if t.IsZero() {
		return e.char('0')
	}
	return e.int(int64(t.Sub(epoch).Seconds()))
}

func (e *Encoder) duration(d time.Duration) *Encoder {
	v := int64(d.Seconds())
	switch {
	case v == 0:
		return e.char('0')
	case v%86400 == 0:
		return e.int(v / 86400).char('d')
	case v%3600 == 0:
		return e.int(v / 3600).char('h')
	case v%60 == 0:
		return e.int(v / 60).char('m')
	default:
		return e.int(v)
	}
}

func (e *Encoder) bandwidth(m string, v int) *Encoder {
	return e.str(m).char(':').int(int64(v))
}

func (e *Encoder) key(k *Key) *Encoder {
	if k.Value == "" {
		return e.str(k.Method)
	}
	return e.str(k.Method).char(':').str(k.Value)
}

func (e *Encoder) origin(o *Origin) *Encoder {
	return e.str(strd(o.Username, "-")).sp().int(o.SessionID).sp().int(o.SessionVersion).sp().transport(o.Network, o.Type, o.Address)
}

func (e *Encoder) connection(c *Connection) *Encoder {
	e.transport(c.Network, c.Type, c.Address)
	if c.TTL > 0 {
		e.char('/').int(int64(c.TTL))
	}
	if c.AddressNum > 1 {
		e.char('/').int(int64(c.AddressNum))
	}
	return e
}

func (e *Encoder) transport(network, typ, addr string) *Encoder {
	return e.fields(strd(network, "IN"), strd(typ, "IP4"), strd(addr, "127.0.0.1"))
}

func strd(v, def string) string {
	if v == "" {
		return def
	}
	return v
}

func (e *Encoder) str(v string) *Encoder {
	if v == "" {
		return e.char('-')
	}
	copy(e.next(len(v)), v)
	return e
}

func (e *Encoder) fields(v ...string) *Encoder {
	n := len(v) - 1
	for _, it := range v {
		n += len(it)
	}
	p, b := 0, e.next(n)
	for _, it := range v {
		if p > 0 {
			b[p] = ' '
			p++
		}
		p += copy(b[p:], it)
	}
	return e
}

func (e *Encoder) sp() *Encoder {
	return e.char(' ')
}

func (e *Encoder) char(v byte) *Encoder {
	e.next(1)[0] = v
	return e
}

func (e *Encoder) int(v int64) *Encoder {
	b := e.next(20)
	e.pos += len(strconv.AppendInt(b[:0], v, 10)) - len(b)
	return e
}

func (e *Encoder) add(n byte) *Encoder {
	if e.newline {
		b := e.next(4)
		b[0], b[1], b[2], b[3] = '\r', '\n', n, '='
	} else {
		b := e.next(2)
		b[0], b[1] = n, '='
		e.newline = true
	}
	return e
}

func (e *Encoder) next(n int) (b []byte) {
	p := e.pos + n
	if len(e.buf) < p {
		e.grow(p)
	}
	b, e.pos = e.buf[e.pos:p], p
	return
}

func (e *Encoder) grow(p int) {
	if p < 1024 {
		p = 1024
	} else if s := len(e.buf) << 1; p < s {
		p = s
	}
	b := make([]byte, p)
	if e.pos > 0 {
		copy(b, e.buf[:e.pos])
	}
	e.buf = b
}

// Bytes returns encoded bytes of the last session description.
// The bytes stop being valid at the next encoder call.
func (e *Encoder) Bytes() []byte {
	if e.newline {
		b := e.next(2)
		b[0], b[1] = '\r', '\n'
		e.newline = false
	}
	return e.buf[:e.pos]
}

// Bytes returns the encoded session description as str.
func (e *Encoder) String() string {
	return string(e.Bytes())
}
