package room

//import (
//	lkc "github.com/livekit/livekit-server/pkg/config"
//	lkrouting "github.com/livekit/livekit-server/pkg/routing"
//	lkservice "github.com/livekit/livekit-server/pkg/service"
//	"github.com/livekit/livekit-server/pkg/telemetry/prometheus"
//	"github.com/q191201771/naza/pkg/nazalog"
//)
//
//type RoomServer struct {
//	APIKey    string
//	APISecret string
//	lkcconf   *lkc.Config
//	lkserver  *lkservice.LivekitServer
//}
//
//func NewRoomServer(apiKey, apiSecret string) *RoomServer {
//	s := &RoomServer{
//		APIKey:    apiKey,
//		APISecret: apiSecret,
//	}
//
//	var err error
//	s.lkcconf, err = s.getLivekitConfig()
//	if err != nil {
//		return nil
//	}
//
//	return s
//}
//
//func (s *RoomServer) Start() error {
//	var err error
//	currentNode, err := lkrouting.NewLocalNode(s.lkcconf)
//	if err != nil {
//		nazalog.Error("failed to create local node:", err)
//		return err
//	}
//
//	if err := prometheus.Init(currentNode.Id, currentNode.Type); err != nil {
//		return err
//	}
//
//	s.lkserver, err = lkservice.InitializeServer(s.lkcconf, currentNode)
//	if err != nil {
//		nazalog.Error("failed to initialize server:", err)
//		return err
//	}
//
//	nazalog.Info("starting livekit server")
//
//	return s.lkserver.Start()
//}
//
//func (s *RoomServer) Stop() error {
//	if s.lkserver != nil {
//		nazalog.Error("stopping livekit server")
//		s.lkserver.Stop(true)
//	}
//	return nil
//}
//
//func (s *RoomServer) getLivekitConfig() (*lkc.Config, error) {
//	strictMode := true
//	conf, err := lkc.NewConfig("", strictMode, nil, nil)
//	if err != nil {
//		return nil, err
//	}
//
//	conf.Keys = map[string]string{
//		s.APIKey: s.APISecret,
//	}
//
//	if conf.BindAddresses == nil {
//		conf.BindAddresses = []string{
//			"127.0.0.1",
//			"::1",
//		}
//	}
//
//	lkc.InitLoggerFromConfig(&conf.Logging)
//
//	return conf, nil
//}
