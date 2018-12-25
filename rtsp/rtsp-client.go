package rtsp

import (
	"bufio"
	"bytes"
	"crypto/md5"
	"encoding/binary"
	"fmt"
	"io"
	"log"
	"net"
	"net/url"
	"os"
	"regexp"
	"strconv"
	"strings"
	"time"

	"github.com/teris-io/shortid"

	"github.com/penggy/EasyGoLib/utils"

	"github.com/pixelbender/go-sdp/sdp"
)

type RTSPClient struct {
	Server *Server
	SessionLogger
	Stoped               bool
	Status               string
	URL                  string
	Path                 string
	CustomPath           string //custom path for pusher
	ID                   string
	Conn                 net.Conn
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

	authLine string

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
	client.logger = log.New(os.Stdout, fmt.Sprintf("[%s]", client.ID), log.LstdFlags|log.Lshortfile)
	if !utils.Debug {
		client.logger.SetOutput(utils.GetLogWriter())
	}
	return
}

func digestAuth(authLine string, method string, URL string) (string, error) {
	l, err := url.Parse(URL)
	if err != nil {
		return "", fmt.Errorf("Url parse error:%v,%v", URL, err)
	}
	realm := ""
	nonce := ""
	realmRex := regexp.MustCompile(`realm="(\S+)"`)
	result1 := realmRex.FindStringSubmatch(authLine)

	nonceRex := regexp.MustCompile(`nonce="(\S+)"`)
	result2 := nonceRex.FindStringSubmatch(authLine)

	if len(result1) == 2 {
		realm = result1[1]
	} else {
		return "", fmt.Errorf("auth error : no realm found")
	}
	if len(result2) == 2 {
		nonce = result2[1]
	} else {
		return "", fmt.Errorf("auth error : no nonce found")
	}
	// response= md5(md5(username:realm:password):nonce:md5(public_method:url));
	username := l.User.Username()
	password, _ := l.User.Password()
	l.User = nil
	if l.Port() == "" {
		l.Host = fmt.Sprintf("%s:%s", l.Host, "554")
	}
	md5UserRealmPwd := fmt.Sprintf("%x", md5.Sum([]byte(fmt.Sprintf("%s:%s:%s", username, realm, password))))
	md5MethodURL := fmt.Sprintf("%x", md5.Sum([]byte(fmt.Sprintf("%s:%s", method, l.String()))))

	response := fmt.Sprintf("%x", md5.Sum([]byte(fmt.Sprintf("%s:%s:%s", md5UserRealmPwd, nonce, md5MethodURL))))
	Authorization := fmt.Sprintf("Authorization: Digest username=\"%s\", realm=\"%s\", nonce=\"%s\", uri=\"%s\", response=\"%s\"", username, realm, nonce, l.String(), response)
	return Authorization, nil
}

func (client *RTSPClient) checkAuth(method string, resp *Response) (string, error) {
	if resp.StatusCode == 401 {

		// need auth.
		AuthHeaders := resp.Header["WWW-Authenticate"]
		auths, ok := AuthHeaders.([]string)

		if ok {
			for _, authLine := range auths {

				if strings.IndexAny(authLine, "Digest") == 0 {
					// 					realm="HipcamRealServer",
					// nonce="3b27a446bfa49b0c48c3edb83139543d"
					client.authLine = authLine
					return digestAuth(authLine, method, client.URL)
				} else if strings.IndexAny(authLine, "Basic") == 0 {
					// not support yet
					// TODO..
				}

			}
			return "", fmt.Errorf("auth error")
		} else {
			authLine, _ := AuthHeaders.(string)
			if strings.IndexAny(authLine, "Digest") == 0 {
				client.authLine = authLine
				return digestAuth(authLine, method, client.URL)
			} else if strings.IndexAny(authLine, "Basic") == 0 {
				// not support yet
				// TODO..
				return "", fmt.Errorf("not support Basic auth yet")
			}
		}
	}
	return "", nil
}

func (client *RTSPClient) Start(timeout time.Duration) error {
	//source := make(chan interface{})
	logger := client.logger
	if !utils.Debug {
		logger.SetOutput(utils.GetLogWriter())
	}
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
			Authorization, _ := client.checkAuth("OPTIONS", resp)
			if len(Authorization) > 0 {
				headers := make(map[string]string)
				headers["Require"] = "implicit-play"
				headers["Authorization"] = Authorization
				// An OPTIONS request returns the request types the server will accept.
				resp, err = client.Request("OPTIONS", headers)
			}
			if err != nil {
				return err
			}
		}

		// A DESCRIBE request includes an RTSP URL (rtsp://...), and the type of reply data that can be handled. This reply includes the presentation description,
		// typically in Session Description Protocol (SDP) format. Among other things, the presentation description lists the media streams controlled with the aggregate URL.
		// In the typical case, there is one media stream each for audio and video.
		headers = make(map[string]string)
		headers["Accept"] = "application/sdp"
		resp, err = client.Request("DESCRIBE", headers)
		if err != nil {
			Authorization, _ := client.checkAuth("DESCRIBE", resp)
			if len(Authorization) > 0 {
				headers := make(map[string]string)
				headers["Authorization"] = Authorization
				headers["Accept"] = "application/sdp"
				resp, err = client.Request("DESCRIBE", headers)
			}
			if err != nil {
				return err
			}
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
				if Session != "" {
					headers["Session"] = Session
				}
				resp, err = client.RequestWithPath("SETUP", _url, headers, true)
				if err != nil {
					return err
				}
				Session, _ = resp.Header["Session"].(string)
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
				if Session != "" {
					headers["Session"] = Session
				}
				resp, err = client.RequestWithPath("SETUP", _url, headers, true)
				if err != nil {
					return err
				}
				Session, _ = resp.Header["Session"].(string)
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
					logger.Printf("client.connRW.ReadByte err:%v", err)
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
						logger.Printf("io.ReadFull err:%v", err)
					}
					return
				}
				channel := int(header[1])
				length := binary.BigEndian.Uint16(header[2:])
				content := make([]byte, length)
				_, err = io.ReadFull(client.connRW, content)
				if err != nil {
					if !client.Stoped {
						logger.Printf("io.ReadFull err:%v", err)
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
					logger.Printf("unknow rtp pack type, channel:%v", channel)
					continue
				}
				if pack == nil {
					logger.Printf("session tcp got nil rtp pack")
					continue
				}
				elapsed := time.Now().Sub(loggerTime)
				if elapsed >= 10*time.Second {
					logger.Printf("%v read rtp frame.", client)
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
							logger.Printf("client.connRW.ReadLine err:%v", err)
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
						logger.Printf("<<<\n%s", builder.String())
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
								logger.Printf("strconv.Atoi err:%v, str:%v", err, splits[1])
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
	logger := client.logger
	headers["User-Agent"] = "EasyDarwinGo"
	if len(headers["Authorization"]) == 0 {
		if len(client.authLine) != 0 {
			Authorization, _ := digestAuth(client.authLine, method, client.URL)
			if len(Authorization) > 0 {
				headers["Authorization"] = Authorization
			}
		}
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
	logger.Printf(">>>\n%s", s)
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
	respHeader := make(map[string]interface{})
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
				resp.Header = respHeader
				logger.Printf("<<\n%s", builder.String())

				if !(statusCode >= 200 && statusCode <= 300) {
					err = fmt.Errorf("Response StatusCode is :%d", statusCode)
					return
				}
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
				status = splits[2]
			}
			lineCount++
			splits := strings.Split(s, ":")
			if len(splits) == 2 {
				if val, ok := respHeader[splits[0]]; ok {
					if slice, ok2 := val.([]string); ok2 {
						slice = append(slice, strings.TrimSpace(splits[1]))
						respHeader[splits[0]] = slice
					} else {
						str, _ := val.(string)
						slice := []string{str, strings.TrimSpace(splits[1])}
						respHeader[splits[0]] = slice
					}
				} else {
					respHeader[splits[0]] = strings.TrimSpace(splits[1])
				}
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

func (client *RTSPClient) Request(method string, headers map[string]string) (*Response, error) {
	l, err := url.Parse(client.URL)
	if err != nil {
		return nil, fmt.Errorf("Url parse error:%v", err)
	}
	l.User = nil
	return client.RequestWithPath(method, l.String(), headers, true)
}

func (client *RTSPClient) RequestNoResp(method string, headers map[string]string) (err error) {
	l, err := url.Parse(client.URL)
	if err != nil {
		return fmt.Errorf("Url parse error:%v", err)
	}
	l.User = nil
	if _, err = client.RequestWithPath(method, l.String(), headers, false); err != nil {
		return err
	}
	return nil
}
