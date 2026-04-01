package parser

import (
	"bytes"
	"fmt"
	"strings"

	"github.com/ghettovoice/gosip/log"
	"github.com/ghettovoice/gosip/sip"
)

type PacketParser struct {
	headerParsers map[string]HeaderParser
	log           log.Logger
}

func NewPacketParser(logger log.Logger) *PacketParser {
	p := &PacketParser{}
	p.log = logger.WithPrefix("parser.Parser").WithFields(
		log.Fields{"parser_ptr": fmt.Sprintf("%p", p)})
	p.headerParsers = make(map[string]HeaderParser)
	for headerName, headerParser := range defaultHeaderParsers() {
		p.SetHeaderParser(headerName, headerParser)
	}
	return p
}

func (pp *PacketParser) ParseMessage(data []byte) (sip.Message, error) {
	bodyLen := getBodyLength(data)
	if bodyLen == -1 {
		return nil, InvalidMessageFormat("format error")
	}
	bodyStart := len(data) - bodyLen
	lines := strings.Split(string(data[:bodyStart]), "\r\n")
	filtered := make([]string, 0, len(lines))
	for _, line := range lines {
		if line != "" {
			filtered = append(filtered, line)
		}
	}
	if len(filtered) < 1 {
		return nil, InvalidMessageFormat(fmt.Sprintf("format error:%s", string(data)))
	}
	//parse startLine
	msg, err := pp.parseStartLine(filtered[0])
	if err != nil {
		return nil, InvalidStartLineError(fmt.Sprintf("%s failed to parse first line of message: %s", pp, err))
	}
	pp.fillHeaders(msg, filtered[1:])
	if err = pp.fillBody(msg, string(data[bodyStart:]), bodyLen); err != nil {
		return nil, err
	}
	return msg, nil
}

func (pp *PacketParser) parseStartLine(startLine string) (msg sip.Message, err error) {
	if isRequest(startLine) {
		method, recipient, sipVersion, err := ParseRequestLine(startLine)
		if err == nil {
			msg = sip.NewRequest("", method, recipient, sipVersion, []sip.Header{}, "", nil)
		} else {
			return nil, err
		}
	} else if isResponse(startLine) {
		sipVersion, statusCode, reason, err := ParseStatusLine(startLine)
		if err == nil {
			msg = sip.NewResponse("", sipVersion, statusCode, reason, []sip.Header{}, "", nil)
		} else {
			return nil, err
		}
	} else {
		return nil, fmt.Errorf("transmission beginning '%s' is not a SIP message", startLine)
	}
	return
}

func (pp *PacketParser) Log() log.Logger {
	return pp.log
}

// Parse the header section.
// Headers can be split across lines (marked by whitespace at the start of subsequent lines),
// so store lines into a buffer, and then flush and parse it when we hit the end of the header.
func (pp *PacketParser) fillHeaders(msg sip.Message, lines []string) {
	var buffer bytes.Buffer
	headers := make([]sip.Header, 0)

	flushBuffer := func() {
		if buffer.Len() > 0 {
			newHeaders, err := pp.ParseHeader(buffer.String())
			if err == nil {
				headers = append(headers, newHeaders...)
			} else {
				pp.Log().Warnf("skip header '%s' due to error: %s", buffer, err)
			}
			buffer.Reset()
		}
	}

	for _, line := range lines {
		if !strings.Contains(abnfWs, string(line[0])) {
			// This line starts a new header.
			// Parse anything currently in the buffer, then store the new header line in the buffer.
			flushBuffer()
			buffer.WriteString(line)
		} else if buffer.Len() > 0 {
			// This is a continuation line, so just add it to the buffer.
			buffer.WriteString(" ")
			buffer.WriteString(line)
		} else {
			// This is a continuation line, but also the first line of the whole header section.
			// Discard it and log.
			pp.Log().Tracef(
				"discard unexpected continuation line '%s' at start of header block in message '%s'",
				line,
				msg.Short(),
			)
		}
	}
	flushBuffer()
	// Store the headers in the message object.
	for _, header := range headers {
		msg.AppendHeader(header)
	}
}

func (pp *PacketParser) fillBody(msg sip.Message, body string, bodyLen int) error {
	// RFC 3261 - 18.3.
	if len(body) != bodyLen {
		return &sip.BrokenMessageError{
			Err: fmt.Errorf("incomplete message body: read %d bytes, expected %d bytes", len(body), bodyLen),
			Msg: msg.String(),
		}
	}

	if strings.TrimSpace(body) != "" {
		msg.SetBody(body, false)
	}
	return nil
}

// ParseHeader parse a header string, producing one or more Header objects.
// (SIP messages containing multiple headers of the same type can express them as a
// single header containing a comma-separated argument list).
func (pp *PacketParser) ParseHeader(headerText string) (headers []sip.Header, err error) {
	pp.Log().Tracef("parsing header \"%s\"", headerText)

	headers = make([]sip.Header, 0)

	colonIdx := strings.Index(headerText, ":")
	if colonIdx == -1 {
		err = fmt.Errorf("field name with no value in header: %s", headerText)
		return
	}

	fieldName := strings.TrimSpace(headerText[:colonIdx])
	lowerFieldName := strings.ToLower(fieldName)
	fieldText := strings.TrimSpace(headerText[colonIdx+1:])
	if headerParser, ok := pp.headerParsers[lowerFieldName]; ok {
		// We have a registered parser for this header type - use it.
		headers, err = headerParser(lowerFieldName, fieldText)
	} else {
		// We have no registered parser for this header type,
		// so we encapsulate the header data in a GenericHeader struct.
		pp.Log().Tracef("no parser for header type %s", fieldName)

		header := sip.GenericHeader{
			HeaderName: fieldName,
			Contents:   fieldText,
		}
		headers = []sip.Header{&header}
	}

	return
}

// SetHeaderParser implements ParserFactory.SetHeaderParser.
func (pp *PacketParser) SetHeaderParser(headerName string, headerParser HeaderParser) {
	headerName = strings.ToLower(headerName)
	pp.headerParsers[headerName] = headerParser
}

func (pp *PacketParser) String() string {
	if pp == nil {
		return "Parser <nil>"
	}
	return fmt.Sprintf("Parser %p", pp)
}
