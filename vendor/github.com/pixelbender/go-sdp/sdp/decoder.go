package sdp

import (
	"bufio"
	"errors"
	"fmt"
	"io"
	"strconv"
	"strings"
	"time"
)

// Parse reads session description from the buffer.
func Parse(b []byte) (*Session, error) {
	return ParseString(string(b))
}

// ParseString reads session description from the string.
func ParseString(s string) (*Session, error) {
	return NewDecoderString(s).Decode()
}

// A Decoder reads a session description from a stream.
type Decoder struct {
	r lineReader
	p []string
}

// NewDecoder returns new decoder that reads from r.
func NewDecoder(r io.Reader) *Decoder {
	return &Decoder{r: &reader{b: bufio.NewReaderSize(r, maxLineSize)}}
}

// NewDecoderString returns new decoder that reads from s.
func NewDecoderString(s string) *Decoder {
	return &Decoder{r: &stringReader{s: s}}
}

// Decode encodes the session description.
func (d *Decoder) Decode() (*Session, error) {
	line := 0
	sess := new(Session)
	var media *Media

	for {
		line++
		s, err := d.r.ReadLine()
		if err != nil {
			if err == io.EOF && sess.Origin != nil {
				break
			}
			return nil, err
		}
		if len(s) == 0 && sess.Origin != nil {
			break
		}
		if len(s) < 2 || s[1] != '=' {
			return nil, &errDecode{errFormat, line, s}
		}
		f, v := s[0], s[2:]
		if f == 'm' {
			media = new(Media)
			err = d.media(media, f, v)
			if err == nil {
				sess.Media = append(sess.Media, media)
			}
		} else if media == nil {
			err = d.session(sess, f, v)
		} else {
			err = d.media(media, f, v)
		}
		if err != nil {
			return nil, &errDecode{err, line, s}
		}
	}
	return sess, nil
}

func (d *Decoder) session(s *Session, f byte, v string) error {
	var err error
	switch f {
	case 'v':
		s.Version, err = strconv.Atoi(v)
	case 'o':
		if s.Origin != nil {
			return errUnexpectedField
		}
		s.Origin, err = d.origin(v)
	case 's':
		s.Name = v
	case 'i':
		s.Information = v
	case 'u':
		s.URI = v
	case 'e':
		s.Email = append(s.Email, v)
	case 'p':
		s.Phone = append(s.Phone, v)
	case 'c':
		if s.Connection != nil {
			return errUnexpectedField
		}
		s.Connection, err = d.connection(v)
	case 'b':
		if s.Bandwidth == nil {
			s.Bandwidth = make(Bandwidth)
		}
		err = d.bandwidth(s.Bandwidth, v)
	case 'z':
		s.TimeZone, err = d.timezone(v)
	case 'k':
		s.Key = append(s.Key, d.key(v))
	case 'a':
		a := d.attr(v)
		switch a.Name {
		case ModeInactive, ModeRecvOnly, ModeSendOnly, ModeSendRecv:
			s.Mode = a.Name
		default:
			s.Attributes = append(s.Attributes, a)
		}
	case 't':
		s.Timing, err = d.timing(v)
	case 'r':
		r, err := d.repeat(v)
		if err != nil {
			return err
		}
		s.Repeat = append(s.Repeat, r)
	default:
		return errUnexpectedField
	}
	return err
}

func (d *Decoder) media(m *Media, f byte, v string) error {
	var err error
	switch f {
	case 'm':
		err = d.proto(m, v)
	case 'i':
		m.Information = v
	case 'c':
		conn, err := d.connection(v)
		if err != nil {
			return err
		}
		m.Connection = append(m.Connection, conn)
	case 'b':
		if m.Bandwidth == nil {
			m.Bandwidth = make(Bandwidth)
		}
		err = d.bandwidth(m.Bandwidth, v)
	case 'k':
		m.Key = append(m.Key, d.key(v))
	case 'a':
		a := d.attr(v)
		switch a.Name {
		case ModeInactive, ModeRecvOnly, ModeSendOnly, ModeSendRecv:
			m.Mode = a.Name
		case "rtpmap", "rtcp-fb", "fmtp":
			err = d.format(m, a)
		default:
			m.Attributes = append(m.Attributes, a)
		}
	default:
		return errUnexpectedField
	}
	return err
}

func (d *Decoder) format(m *Media, a *Attr) error {
	p, ok := d.fields(a.Value, 2)
	if !ok {
		return errFormat
	}
	pt, err := strconv.Atoi(p[0])
	if err != nil {
		return err
	}
	f, v := m.Format(pt), p[1]
	if f == nil {
		return nil
	}
	switch a.Name {
	case "rtpmap":
		err = d.rtpmap(f, v)
	case "rtcp-fb":
		f.Feedback = append(f.Feedback, v)
	case "fmtp":
		f.Params = append(f.Params, v)
	}
	return err
}

func (d *Decoder) rtpmap(f *Format, v string) error {
	p, ok := d.split(v, '/', 3)
	if len(p) < 2 {
		return errFormat
	}
	f.Name = p[0]
	var err error
	if ok {
		if f.Channels, err = strconv.Atoi(strings.TrimSpace(p[2])); err != nil {
			return err
		}
	}
	if f.ClockRate, err = strconv.Atoi(strings.TrimSpace(p[1])); err != nil {
		return err
	}
	return nil
}

func (d *Decoder) proto(m *Media, v string) error {
	p, ok := d.fields(v, 4)
	if !ok {
		return errFormat
	}
	formats := p[3]
	m.Type, m.Proto = p[0], p[2]
	p, ok = d.split(p[1], '/', 2)
	var err error
	if ok {
		if m.PortNum, err = strconv.Atoi(p[1]); err != nil {
			return err
		}
	}
	if m.Port, err = strconv.Atoi(p[0]); err != nil {
		return err
	}
	p, _ = d.fields(formats, maxLineSize)
	for _, it := range p {
		if it == "*" {
			continue
		}
		pt, err := strconv.Atoi(it)
		if err != nil {
			return err
		}
		m.Formats = append(m.Formats, &Format{Payload: pt})
	}
	return nil
}

func (d *Decoder) origin(v string) (*Origin, error) {
	p, ok := d.fields(v, 6)
	if !ok {
		return nil, errFormat
	}
	o := new(Origin)
	o.Username, o.SessionID, o.Network, o.Type, o.Address = p[0], p[1], p[3], p[4], p[5]
	var err error
	if o.SessionVersion, err = d.int(p[2]); err != nil {
		return nil, err
	}
	return o, nil
}

func (d *Decoder) connection(v string) (*Connection, error) {
	p, ok := d.fields(v, 3)
	if !ok {
		return nil, errFormat
	}
	c := new(Connection)
	c.Network, c.Type, c.Address = p[0], p[1], p[2]
	p, ok = d.split(c.Address, '/', 3)
	if ok {
		ttl, err := d.int(p[1])
		if err != nil {
			return nil, err
		}
		c.TTL = int(ttl)
		p = p[1:]
	}
	if len(p) > 1 {
		num, err := d.int(p[1])
		if err != nil {
			return nil, err
		}
		c.Address, c.AddressNum = p[0], int(num)
	}
	return c, nil
}

func (d *Decoder) bandwidth(b Bandwidth, v string) error {
	p, ok := d.split(v, ':', 2)
	if !ok {
		return errFormat
	}
	val, err := d.int(p[1])
	if err != nil {
		return err
	}
	b[p[0]] = int(val)
	return nil
}

func (d *Decoder) timezone(v string) ([]*TimeZone, error) {
	p, _ := d.fields(v, 40)
	zone := make([]*TimeZone, 0, 1)
	var err error
	for len(p) > 1 {
		it := new(TimeZone)
		if it.Time, err = d.time(p[0]); err != nil {
			return nil, err
		}
		if it.Offset, err = d.duration(p[1]); err != nil {
			return nil, err
		}
		zone = append(zone, it)
		p = p[2:]
	}
	return zone, nil
}

func (d *Decoder) key(v string) *Key {
	if p, ok := d.split(v, ':', 2); ok {
		return &Key{p[0], p[1]}
	}
	return &Key{v, ""}
}

func (d *Decoder) attr(v string) *Attr {
	if p, ok := d.split(v, ':', 2); ok {
		return &Attr{p[0], p[1]}
	}
	return &Attr{v, ""}
}

func (d *Decoder) timing(v string) (*Timing, error) {
	p, ok := d.fields(v, 2)
	if !ok {
		return nil, errFormat
	}
	start, err := d.time(p[0])
	if err != nil {
		return nil, err
	}
	stop, err := d.time(p[1])
	if err != nil {
		return nil, err
	}
	return &Timing{start, stop}, nil
}

func (d *Decoder) repeat(v string) (*Repeat, error) {
	p, _ := d.fields(v, maxLineSize)
	if len(p) < 2 {
		return nil, errFormat
	}
	r := new(Repeat)
	var err error
	if r.Interval, err = d.duration(p[0]); err != nil {
		return nil, err
	}
	if r.Duration, err = d.duration(p[1]); err != nil {
		return nil, err
	}
	for _, it := range p[2:] {
		off, err := d.duration(it)
		if err != nil {
			return nil, err
		}
		r.Offsets = append(r.Offsets, off)
	}
	return r, nil
}

func (d *Decoder) time(v string) (time.Time, error) {
	sec, err := d.int(v)
	if err != nil || sec == 0 {
		return time.Time{}, err
	}
	return epoch.Add(time.Second * time.Duration(sec)), nil
}

func (d *Decoder) duration(v string) (time.Duration, error) {
	m := int64(1)
	if n := len(v) - 1; n >= 0 {
		switch v[n] {
		case 'd':
			m, v = 86400, v[:n]
		case 'h':
			m, v = 3600, v[:n]
		case 'm':
			m, v = 60, v[:n]
		case 's':
			v = v[:n]
		}
	}
	sec, err := d.int(v)
	if err != nil {
		return 0, err
	}
	return time.Duration(sec*m) * time.Second, nil
}

func (d *Decoder) int(v string) (int64, error) {
	return strconv.ParseInt(v, 10, 64)
}

func (d *Decoder) fields(s string, n int) ([]string, bool) {
	return d.split(s, ' ', n)
}

func (d *Decoder) split(s string, sep rune, n int) ([]string, bool) {
	p, pos := d.p[:0], 0
	for i, c := range s {
		if c != sep {
			continue
		}
		p = append(p, s[pos:i])
		pos = i + 1
		if len(p) >= n-1 {
			break
		}
	}
	p = append(p, s[pos:])
	d.p = p[:0]
	return p, len(p) == n
}

const maxLineSize = 1024

type lineReader interface {
	ReadLine() (string, error)
}

type stringReader struct {
	s string
}

func (r *stringReader) ReadLine() (string, error) {
	s, n := r.s, len(r.s)
	if n == 0 {
		return "", io.EOF
	}
	for i, ch := range s {
		if ch == '\n' {
			r.s = s[i+1:]
			for i > 0 && s[i-1] == '\r' {
				i--
			}
			return s[:i], nil
		}
	}
	r.s = ""
	return s, nil
}

type reader struct {
	b *bufio.Reader
}

func (r *reader) ReadLine() (string, error) {
	b, prefix, err := r.b.ReadLine()
	if prefix && err == nil {
		err = errLineTooLong
	}
	if err != nil {
		return "", err
	}
	return string(b), nil
}

var errLineTooLong = errors.New("sdp: line is too long")
var errUnexpectedField = errors.New("unexpected field")
var errFormat = errors.New("format error")

type errDecode struct {
	err  error
	line int
	text string
}

func (e *errDecode) Error() string {
	return fmt.Sprintf("sdp: %s on line %d '%s'", e.err.Error(), e.line, e.text)
}
