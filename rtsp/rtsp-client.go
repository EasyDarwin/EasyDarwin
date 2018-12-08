package rtsp

import (
	"bufio"
	"bytes"
	"encoding/binary"
	"fmt"
	"io"
	"log"
	"net"
	"net/url"
	"strconv"
	"strings"
	"time"

	"github.com/teris-io/shortid"

	"github.com/penggy/EasyGoLib/utils"

	"github.com/pixelbender/go-sdp/sdp"
)

type RTSPClient struct {
	Server               *Server
	Stoped               bool
	Status               string
	URL                  string
	Path                 string
	CustomPath           string //custom path for pusher
	ID                   string
	Conn                 net.Conn
	AuthHeaders          bool
	Session              *string
	Seq                  int
	connRW               *bufio.ReadWriter
	InBytes              int
	OutBytes             int
	TransType            TransType
	StartAt              time.Time
	Sdp                  *sdp.Session
	AControl             string
	VControl             string
	ACodec               string
	VCodec               string
	OptionIntervalMillis int64
	SDPRaw               string

	//tcp channels
	aRTPChannel        int
	aRTPControlChannel int
	vRTPChannel        int
	vRTPControlChannel int

	RTPHandles  []func(*RTPPack)
	StopHandles []func()
}

func (client *RTSPClient) String() string {
	return fmt.Sprintf("client[%s]", client.URL)
}

func NewRTSPClient(server *Server, rawUrl string, sendOptionMillis int64) (client *RTSPClient, err error) {
	url, err := url.Parse(rawUrl)
	if err != nil {
		return
	}
	client = &RTSPClient{
		Server:               server,
		Stoped:               false,
		URL:                  rawUrl,
		ID:                   shortid.MustGenerate(),
		Path:                 url.Path,
		vRTPChannel:          0,
		vRTPControlChannel:   1,
		aRTPChannel:          2,
		aRTPControlChannel:   3,
		OptionIntervalMillis: sendOptionMillis,
		StartAt:              time.Now(),
	}
	return
}

func (client *RTSPClient) Start(timeout time.Duration) error {
	//source := make(chan interface{})

	if timeout == 0 {
		timeoutMillis := utils.Conf().Section("rtsp").Key("timeout").MustInt(0)
		timeout = time.Duration(timeoutMillis) * time.Millisecond
	}
	requestStream := func() error {
		l, err := url.Parse(client.URL)
		setStatus := func() {
			if err != nil {
				client.Status = "Error"
			} else {
				client.Status = "OK"
			}
		}
		defer setStatus()
		if err != nil {
			return err
		}
		if strings.ToLower(l.Scheme) != "rtsp" {
			err = fmt.Errorf("RTSP url is invalid")
			return err
		}
		if strings.ToLower(l.Hostname()) == "" {
			err = fmt.Errorf("RTSP url is invalid")
			return err
		}
		port := l.Port()
		if len(port) == 0 {
			port = "554"
		}
		conn, err := net.DialTimeout("tcp", l.Hostname()+":"+port, timeout)
		if err != nil {
			// handle error
			return err
		}
		client.Conn = conn

		networkBuffer := utils.Conf().Section("rtsp").Key("network_buffer").MustInt(204800)

		timeoutConn := RichConn{
			conn,
			timeout,
		}
		client.connRW = bufio.NewReadWriter(bufio.NewReaderSize(&timeoutConn, networkBuffer), bufio.NewWriterSize(&timeoutConn, networkBuffer))

		headers := make(map[string]string)
		headers["Require"] = "implicit-play"
		// An OPTIONS request returns the request types the server will accept.
		resp, err := client.Request("OPTIONS", headers)
		if err != nil {
			return err
		}

		// A DESCRIBE request includes an RTSP URL (rtsp://...), and the type of reply data that can be handled. This reply includes the presentation description,
		// typically in Session Description Protocol (SDP) format. Among other things, the presentation description lists the media streams controlled with the aggregate URL.
		// In the typical case, there is one media stream each for audio and video.
		headers = make(map[string]string)
		headers["Accept"] = "application/sdp"
		resp, err = client.Request("DESCRIBE", headers)
		if err != nil {
			return err
		}

		sess, err := sdp.ParseString(resp.Body)
		if err != nil {
			return err
		}
		client.Sdp = sess
		client.SDPRaw = resp.Body
		Session := ""
		for _, media := range sess.Media {
			switch media.Type {
			case "video":
				client.VControl = media.Attributes.Get("control")
				client.VCodec = media.Formats[0].Name
				var _url = ""
				if strings.Index(strings.ToLower(client.VControl), "rtsp://") == 0 {
					_url = client.VControl
				} else {
					_url = strings.TrimRight(client.URL, "/") + "/" + strings.TrimLeft(client.VControl, "/")
				}
				headers = make(map[string]string)
				headers["Transport"] = fmt.Sprintf("RTP/AVP/TCP;unicast;interleaved=%d-%d", client.vRTPChannel, client.vRTPControlChannel)
				resp, err = client.RequestWithPath("SETUP", _url, headers, true)
				if err != nil {
					return err
				}
				Session = resp.Header["Session"]
			case "audio":
				client.AControl = media.Attributes.Get("control")
				client.VCodec = media.Formats[0].Name
				var _url = ""
				if strings.Index(strings.ToLower(client.AControl), "rtsp://") == 0 {
					_url = client.AControl
				} else {
					_url = strings.TrimRight(client.URL, "/") + "/" + strings.TrimLeft(client.AControl, "/")
				}
				headers = make(map[string]string)
				headers["Transport"] = fmt.Sprintf("RTP/AVP/TCP;unicast;interleaved=%d-%d", client.aRTPChannel, client.aRTPControlChannel)
				resp, err = client.RequestWithPath("SETUP", _url, headers, true)
				if err != nil {
					return err
				}
				Session = resp.Header["Session"]
			}
		}
		headers = make(map[string]string)
		if Session != "" {
			headers["Session"] = Session
		}
		resp, err = client.Request("PLAY", headers)
		if err != nil {
			return err
		}
		return nil
	}

	stream := func() {
		OptionIntervalMillis := client.OptionIntervalMillis
		startTime := time.Now()
		loggerTime := time.Now().Add(-10 * time.Second)
		defer client.Stop()
		for !client.Stoped {
			if OptionIntervalMillis > 0 {
				elapse := time.Now().Sub(startTime)
				if elapse > time.Duration(OptionIntervalMillis)*time.Millisecond {
					startTime = time.Now()
					headers := make(map[string]string)
					headers["Require"] = "implicit-play"
					// An OPTIONS request returns the request types the server will accept.
					if err := client.RequestNoResp("OPTIONS", headers); err != nil {
						// ignore...
						//ch <- err
						//return
					}
				}
			}
			b, err := client.connRW.ReadByte()
			if err != nil {
				if !client.Stoped {
					log.Printf("client.connRW.ReadByte err:%v", err)
				}
				return
			}
			switch b {
			case 0x24: // rtp
				header := make([]byte, 4)
				header[0] = b
				_, err := io.ReadFull(client.connRW, header[1:])
				if err != nil {

					if !client.Stoped {
						log.Printf("io.ReadFull err:%v", err)
					}
					return
				}
				channel := int(header[1])
				length := binary.BigEndian.Uint16(header[2:])
				content := make([]byte, length)
				_, err = io.ReadFull(client.connRW, content)
				if err != nil {
					if !client.Stoped {
						log.Printf("io.ReadFull err:%v", err)
					}
					return
				}
				//ch <- append(header, content...)
				rtpBuf := bytes.NewBuffer(content)
				var pack *RTPPack
				switch channel {
				case client.aRTPChannel:
					pack = &RTPPack{
						Type:   RTP_TYPE_AUDIO,
						Buffer: rtpBuf,
					}
				case client.aRTPControlChannel:
					pack = &RTPPack{
						Type:   RTP_TYPE_AUDIOCONTROL,
						Buffer: rtpBuf,
					}
				case client.vRTPChannel:
					pack = &RTPPack{
						Type:   RTP_TYPE_VIDEO,
						Buffer: rtpBuf,
					}
				case client.vRTPControlChannel:
					pack = &RTPPack{
						Type:   RTP_TYPE_VIDEOCONTROL,
						Buffer: rtpBuf,
					}
				default:
					log.Printf("unknow rtp pack type, channel:%v", channel)
					continue
				}
				if pack == nil {
					log.Printf("session tcp got nil rtp pack")
					continue
				}
				elapsed := time.Now().Sub(loggerTime)
				if elapsed >= 10*time.Second {
					log.Printf("%v read rtp frame.", client)
					loggerTime = time.Now()
				}
				client.InBytes += int(length + 4)
				for _, h := range client.RTPHandles {
					h(pack)
				}

			default: // rtsp
				builder := strings.Builder{}
				builder.WriteByte(b)
				contentLen := 0
				for !client.Stoped {
					line, prefix, err := client.connRW.ReadLine()
					if err != nil {
						if !client.Stoped {
							log.Printf("client.connRW.ReadLine err:%v", err)
						}
						return
					}
					if len(line) == 0 {
						if contentLen != 0 {
							content := make([]byte, contentLen)
							_, err = io.ReadFull(client.connRW, content)
							if err != nil {
								if !client.Stoped {
									err = fmt.Errorf("Read content err.ContentLength:%d", contentLen)
								}
								return
							}
							builder.Write(content)
						}
						log.Println("S->C	<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<")
						log.Println(builder.String())
						break
					}
					s := string(line)
					builder.Write(line)
					if !prefix {
						builder.WriteString("\r\n")
					}

					if strings.Index(s, "Content-Length:") == 0 {
						splits := strings.Split(s, ":")
						contentLen, err = strconv.Atoi(strings.TrimSpace(splits[1]))
						if err != nil {
							if !client.Stoped {
								log.Printf("strconv.Atoi err:%v, str:%v", err, splits[1])
							}
							return
						}
					}
				}
			}
		}
	}
	//go func() {
	//	defer client.Stop()
	//	r := requestStream()
	//	source <- r
	//	switch r.(type) {
	//	case error:
	//		return
	//	}
	//	stream(source)
	//}()
	//return observable.Observable(source)

	//return observable.Just(1)
	err := requestStream()
	if err != nil {
		return err
	}
	go stream()
	return nil
}

func (client *RTSPClient) Stop() {
	if client.Stoped {
		return
	}
	client.Stoped = true
	for _, h := range client.StopHandles {
		h()
	}
	if client.Conn != nil {
		client.connRW.Flush()
		client.Conn.Close()
		client.Conn = nil
	}
}

func (client *RTSPClient) RequestWithPath(method string, path string, headers map[string]string, needResp bool) (resp *Response, err error) {
	headers["User-Agent"] = "EasyDarwinGo"
	if client.AuthHeaders {
		//headers["Authorization"] = this.digest(method, _url);
	}
	if client.Session != nil {
		headers["Session"] = *client.Session
	}
	client.Seq++
	cseq := client.Seq
	builder := strings.Builder{}
	builder.WriteString(fmt.Sprintf("%s %s RTSP/1.0\r\n", method, path))
	builder.WriteString(fmt.Sprintf("CSeq: %d\r\n", cseq))
	for k, v := range headers {
		builder.WriteString(fmt.Sprintf("%s: %s\r\n", k, v))
	}
	builder.WriteString(fmt.Sprintf("\r\n"))
	s := builder.String()
	log.Println("C->S	>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>")
	log.Println(s)
	_, err = client.connRW.WriteString(s)
	if err != nil {
		return
	}
	client.connRW.Flush()

	if !needResp {
		return nil, nil
	}
	lineCount := 0
	statusCode := 200
	status := ""
	sid := ""
	contentLen := 0
	respHeader := make(map[string]string)
	var line []byte
	builder.Reset()
	for !client.Stoped {
		isPrefix := false
		if line, isPrefix, err = client.connRW.ReadLine(); err != nil {
			return
		} else {
			if len(line) == 0 {
				body := ""
				if contentLen > 0 {
					content := make([]byte, contentLen)
					_, err = io.ReadFull(client.connRW, content)
					if err != nil {
						err = fmt.Errorf("Read content err.ContentLength:%d", contentLen)
						return
					}
					body = string(content)
					builder.Write(content)
				}
				resp = NewResponse(statusCode, status, strconv.Itoa(cseq), sid, body)

				log.Println("S->C	<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<")
				log.Println(builder.String())
				return
			}
			s := string(line)
			builder.Write(line)
			if !isPrefix {
				builder.WriteString("\r\n")
			}

			if lineCount == 0 {
				splits := strings.Split(s, " ")
				if len(splits) < 3 {
					err = fmt.Errorf("StatusCode Line error:%s", s)
					return
				}
				statusCode, err = strconv.Atoi(splits[1])
				if err != nil {
					return
				}
				if statusCode != 200 {
					err = fmt.Errorf("Response StatusCode is :%d", statusCode)
					return
				}
				status = splits[2]
			}
			lineCount++
			splits := strings.Split(s, ":")
			if len(splits) == 2 {
				respHeader[splits[0]] = strings.TrimSpace(splits[1])
			}
			if strings.Index(s, "Session:") == 0 {
				splits := strings.Split(s, ":")
				sid = strings.TrimSpace(splits[1])
			}
			//if strings.Index(s, "CSeq:") == 0 {
			//	splits := strings.Split(s, ":")
			//	cseq, err = strconv.Atoi(strings.TrimSpace(splits[1]))
			//	if err != nil {
			//		err = fmt.Errorf("Atoi CSeq err. line:%s", s)
			//		return
			//	}
			//}
			if strings.Index(s, "Content-Length:") == 0 {
				splits := strings.Split(s, ":")
				contentLen, err = strconv.Atoi(strings.TrimSpace(splits[1]))
				if err != nil {
					return
				}
			}
		}
	}
	if client.Stoped {
		err = fmt.Errorf("Client Stoped.")
	}
	return
}

func (client *RTSPClient) Request(method string, headers map[string]string) (resp *Response, err error) {
	return client.RequestWithPath(method, client.URL, headers, true)
}

func (client *RTSPClient) RequestNoResp(method string, headers map[string]string) (err error) {
	if _, err := client.RequestWithPath(method, client.URL, headers, false); err != nil {
		return err
	}
	return nil
}
