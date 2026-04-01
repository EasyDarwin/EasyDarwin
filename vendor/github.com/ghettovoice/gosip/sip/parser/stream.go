// Forked from github.com/StefanKopieczek/gossip by @StefanKopieczek
package parser

import (
	"bytes"
	"errors"
	"fmt"
	"strconv"
	"strings"
	"sync"

	"github.com/tevino/abool"

	"github.com/ghettovoice/gosip/log"
	"github.com/ghettovoice/gosip/sip"
)

// NewParser create a new Parser.
//
// Parsed SIP messages will be sent down the 'output' chan provided.
// Any errors which force the parser to terminate will be sent down the 'errs' chan provided.
//
// If streamed=false, each Write call to the parser should contain data for one complete SIP message.
// If streamed=true, Write calls can contain a portion of a full SIP message.
// The end of one message and the start of the next may be provided in a single call to Write.
// When streamed=true, all SIP messages provided must have a Content-Length header.
// SIP messages without a Content-Length will cause the parser to permanently stop, and will result in an error on the errs chan.
//
// 'streamed' should be set to true whenever the caller cannot reliably identify the starts and ends of messages from the transport frames,
// e.g. when using streamed protocols such as TCP.
func NewParser(
	output chan<- sip.Message,
	errs chan<- error,
	streamed bool,
	logger log.Logger,
) Parser {
	p := &parser{
		streamed: streamed,
		done:     make(chan struct{}),
	}
	p.PacketParser = NewPacketParser(logger)
	p.output = output
	p.errs = errs
	// Create a managed buffer to allow message data to be asynchronously provided to the parser, and
	// to allow the parser to block until enough data is available to parse.
	p.input = newParserBuffer(p.Log())
	// Done for input a line at a time, and produce SipMessages to send down p.output.
	go p.parse(p.done)
	return p
}

type parser struct {
	*PacketParser
	streamed bool
	input    *parserBuffer

	output chan<- sip.Message
	errs   chan<- error

	stopped abool.AtomicBool

	mu   sync.Mutex
	done chan struct{}
}

func (p *parser) Log() log.Logger {
	return p.log
}

func (p *parser) Write(data []byte) (int, error) {
	if p.stopped.IsSet() {
		return 0, WriteError(fmt.Sprintf("cannot write data to stopped %s", p))
	}

	var (
		num int
		err error
	)
	if !p.streamed {
		bl := getBodyLength(data)

		if bl == -1 {
			err = InvalidMessageFormat(fmt.Sprintf("%s cannot write data: double CRLF sequence not found in the input data", p))
			return num, err
		}

		data = append([]byte(fmt.Sprintf("%d|%d\r\n", bl, len(data))), data...)
	}

	num, err = p.input.Write(data)
	if err != nil {
		err = WriteError(fmt.Sprintf("%s write data failed: %s", p, err))
		return num, err
	}

	p.Log().Tracef("write %d bytes to input buffer", num)

	return num, nil
}

// Stop parser processing, and allow all resources to be garbage collected.
// The parser will not release its resources until Stop() is called,
// even if the parser object itself is garbage collected.
func (p *parser) Stop() {
	if p.stopped.IsSet() {
		return
	}

	p.Log().Debug("stopping parser...")

	p.stopped.Set()

	p.input.Stop()
	p.mu.Lock()
	done := p.done
	p.mu.Unlock()
	<-done

	p.Log().Debug("parser stopped")
}

func (p *parser) Reset() {
	p.Stop()
	// reset state
	p.mu.Lock()
	done := make(chan struct{})
	p.done = done
	p.mu.Unlock()
	p.input.Reset()
	// and re-run
	go p.parse(done)

	p.stopped.UnSet()
}

// Consume input lines one at a time, producing core.Message objects and sending them down p.output.
func (p *parser) parse(done chan<- struct{}) {
	defer close(done)
	p.Log().Debug("start parsing")
	defer p.Log().Debug("stop parsing")

	var skipStreamedErr bool

	for {
		var bodyLen, msgLen int
		if !p.streamed {
			// extract body/msg len
			line, err := p.input.NextLine()
			if err != nil {
				break
			}
			strs := strings.Split(line, "|")
			if len(strs) != 2 {
				continue
			}
			bodyLen, err = strconv.Atoi(strs[0])
			if err != nil {
				continue
			}
			msgLen, err = strconv.Atoi(strs[1])
			if err != nil {
				continue
			}
		}
		// Parse the StartLine.
		startLine, err := p.input.NextLine()
		if err != nil {
			break
		}

		p.Log().Tracef("start reading start line: %s", startLine)
		msg, termErr := p.parseStartLine(startLine)
		if termErr != nil {
			p.Log().Tracef("%s failed to read start line '%s'", p, startLine)
			termErr = InvalidStartLineError(fmt.Sprintf("%s failed to parse first line of message: %s", p, termErr))

			if p.streamed {
				if !skipStreamedErr {
					skipStreamedErr = true
					p.errs <- termErr
				}
			} else {
				skip := msgLen - len(startLine) - 2
				p.Log().Tracef("skip %d - %d - 2 = %d bytes", p, msgLen, len(startLine), skip)
				if _, err := p.input.NextChunk(skip); err != nil {
					p.Log().Errorf("skip failed: %s", err)
				}

				p.errs <- termErr
			}
			continue
		} else {
			skipStreamedErr = false
		}

		p.Log().Tracef("%s starts reading headers", p)
		lines := make([]string, 0)
		for {
			line, err := p.input.NextLine()
			if err != nil || len(line) == 0 {
				break
			}
			lines = append(lines, line)
		}
		p.fillHeaders(msg, lines)

		var contentLength int
		// Determine the length of the body, so we know when to stop parsing this message.
		if p.streamed {
			// Use the content-length header to identify the end of the message.
			contentLengthHeaders := msg.GetHeaders("Content-Length")
			if len(contentLengthHeaders) == 0 {
				skipStreamedErr = true

				termErr := &sip.MalformedMessageError{
					Err: fmt.Errorf("missing required 'Content-Length' header"),
					Msg: msg.String(),
				}
				p.errs <- termErr

				continue
			} else if len(contentLengthHeaders) > 1 {
				skipStreamedErr = true

				var errbuf bytes.Buffer
				errbuf.WriteString("multiple 'Content-Length' headers on message '")
				errbuf.WriteString(msg.Short())
				errbuf.WriteString(fmt.Sprintf("'; parser: %s:\n", p))
				for _, header := range contentLengthHeaders {
					errbuf.WriteString("\t")
					errbuf.WriteString(header.String())
				}
				termErr := &sip.MalformedMessageError{
					Err: errors.New(errbuf.String()),
					Msg: msg.String(),
				}
				p.errs <- termErr

				continue
			}
			contentLength = int(*(contentLengthHeaders[0].(*sip.ContentLength)))
		} else {
			contentLength = bodyLen
		}

		// Extract the message body.
		p.Log().Tracef("%s reads body with length = %d bytes", p, contentLength)
		body, err := p.input.NextChunk(contentLength)
		if err != nil {
			termErr := &sip.BrokenMessageError{
				Err: fmt.Errorf("read message body failed: %w", err),
				Msg: msg.String(),
			}
			p.errs <- termErr
			continue
		}
		if err = p.fillBody(msg, body, contentLength); err != nil {
			p.errs <- err
			continue
		}
		p.output <- msg
	}
	return
}

// SetHeaderParser implements ParserFactory.SetHeaderParser.
func (p *parser) SetHeaderParser(headerName string, headerParser HeaderParser) {
	headerName = strings.ToLower(headerName)
	p.headerParsers[headerName] = headerParser
}
