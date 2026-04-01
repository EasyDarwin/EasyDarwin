package rtsp

import (
	"net"
	"net/http"
	"strings"

	"github.com/q191201771/lal/pkg/base"
)

type WebsocketServer struct {
	addr     string
	observer IServerObserver

	ln         net.Listener
	auth       ServerAuthConfig
	httpServer http.Server
}

func NewWebsocketServer(addr string, observer IServerObserver, auth ServerAuthConfig) *WebsocketServer {
	return &WebsocketServer{
		addr:     addr,
		observer: observer,
		auth:     auth,
	}
}

func (s *WebsocketServer) Listen() (err error) {
	s.ln, err = net.Listen("tcp", s.addr)
	if err != nil {
		return
	}
	Log.Infof("start ws rtsp server listen. addr=%s", s.addr)

	server := http.Server{
		Handler: http.HandlerFunc(s.HandleWebsocket),
	}
	server.Serve(s.ln)
	return
}

func (s *WebsocketServer) HandleWebsocket(w http.ResponseWriter, r *http.Request) {
	conn, bio, err := w.(http.Hijacker).Hijack()
	if err != nil {
		Log.Errorf("hijack failed. err=%+v", err)
		return
	}
	if bio.Reader.Buffered() != 0 || bio.Writer.Buffered() != 0 {
		Log.Errorf("hijack but buffer not empty. rb=%d, wb=%d", bio.Reader.Buffered(), bio.Writer.Buffered())
	}

	var (
		isWebSocket  bool
		webSocketKey string
	)
	// 火狐浏览器 Connection = [keep-alive, Upgrade]
	if strings.Contains(r.Header.Get("Connection"), "Upgrade") && r.Header.Get("Upgrade") == "websocket" {
		isWebSocket = true
		webSocketKey = r.Header.Get("Sec-WebSocket-Key")
	}

	session := NewServerCommandSession(s, conn, s.auth, isWebSocket, webSocketKey)
	s.observer.OnNewRtspSessionConnect(session)

	session.conn.Write(base.UpdateWebSocketHeader(webSocketKey, "rtsp"))

	err = session.RunLoop()
	Log.Info(err)

	if session.pubSession != nil {
		s.observer.OnDelRtspPubSession(session.pubSession)
		_ = session.pubSession.Dispose()
	} else if session.subSession != nil {
		s.observer.OnDelRtspSubSession(session.subSession)
		_ = session.subSession.Dispose()
	}
	s.observer.OnDelRtspSession(session)

}

func (s *WebsocketServer) Dispose() {
	if s.ln == nil {
		return
	}
	if err := s.ln.Close(); err != nil {
		Log.Error(err)
	}
}

// ----- ServerCommandSessionObserver ----------------------------------------------------------------------------------

func (s *WebsocketServer) OnNewRtspPubSession(session *PubSession) error {
	return s.observer.OnNewRtspPubSession(session)
}

func (s *WebsocketServer) OnNewRtspSubSessionDescribe(session *SubSession) (ok bool, sdp []byte) {
	return s.observer.OnNewRtspSubSessionDescribe(session)
}

func (s *WebsocketServer) OnNewRtspSubSessionPlay(session *SubSession) error {
	return s.observer.OnNewRtspSubSessionPlay(session)
}

func (s *WebsocketServer) OnDelRtspPubSession(session *PubSession) {
	s.observer.OnDelRtspPubSession(session)
}

func (s *WebsocketServer) OnDelRtspSubSession(session *SubSession) {
	s.observer.OnDelRtspSubSession(session)
}
