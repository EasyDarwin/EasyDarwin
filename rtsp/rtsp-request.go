package rtsp

import (
	"fmt"
	"log"
	"regexp"
	"strconv"
	"strings"
)

const (
	RTSP_VERSION = "RTSP/1.0"
)

const (
	// Client to server for presentation and stream objects; recommended
	DESCRIBE = "DESCRIBE"
	// Bidirectional for client and stream objects; optional
	ANNOUNCE = "ANNOUNCE"
	// Bidirectional for client and stream objects; optional
	GET_PARAMETER = "GET_PARAMETER"
	// Bidirectional for client and stream objects; required for Client to server, optional for server to client
	OPTIONS = "OPTIONS"
	// Client to server for presentation and stream objects; recommended
	PAUSE = "PAUSE"
	// Client to server for presentation and stream objects; required
	PLAY = "PLAY"
	// Client to server for presentation and stream objects; optional
	RECORD = "RECORD"
	// Server to client for presentation and stream objects; optional
	REDIRECT = "REDIRECT"
	// Client to server for stream objects; required
	SETUP = "SETUP"
	// Bidirectional for presentation and stream objects; optional
	SET_PARAMETER = "SET_PARAMETER"
	// Client to server for presentation and stream objects; required
	TEARDOWN = "TEARDOWN"
	DATA     = "DATA"
)

type Request struct {
	Method  string
	URL     string
	Version string
	Header  map[string]string
	Content string
	Body    string
}

func NewRequest(content string) *Request {
	lines := strings.Split(strings.TrimSpace(content), "\r\n")
	if len(lines) == 0 {
		return nil
	}
	items := regexp.MustCompile("\\s+").Split(strings.TrimSpace(lines[0]), -1)
	if len(items) < 3 {
		return nil
	}
	if !strings.HasPrefix(items[2], "RTSP") {
		log.Printf("invalid rtsp request, line[0] %s", lines[0])
		return nil
	}
	header := make(map[string]string)
	for i := 1; i < len(lines); i++ {
		line := strings.TrimSpace(lines[i])
		headerItems := regexp.MustCompile(":\\s+").Split(line, 2)
		if len(headerItems) < 2 {
			continue
		}
		header[headerItems[0]] = headerItems[1]
	}
	return &Request{
		Method:  items[0],
		URL:     items[1],
		Version: items[2],
		Header:  header,
		Content: content,
		Body:    "",
	}
}

func (r *Request) String() string {
	str := fmt.Sprintf("%s %s %s\r\n", r.Method, r.URL, r.Version)
	for key, value := range r.Header {
		str += fmt.Sprintf("%s: %s\r\n", key, value)
	}
	str += "\r\n"
	str += r.Body
	return str
}

func (r *Request) GetContentLength() int {
	v, err := strconv.ParseInt(r.Header["Content-Length"], 10, 64)
	if err != nil {
		return 0
	} else {
		return int(v)
	}
}
