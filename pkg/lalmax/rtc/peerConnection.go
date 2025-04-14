package rtc

import (
	"github.com/pion/ice/v2"
	"github.com/pion/interceptor"
	"github.com/pion/webrtc/v3"
	"github.com/q191201771/naza/pkg/nazalog"
)

type peerConnection struct {
	*webrtc.PeerConnection
}

func newPeerConnection(ips []string, iceUDPMux ice.UDPMux, iceTCPMux ice.TCPMux) (conn *peerConnection, err error) {
	configuration := webrtc.Configuration{}
	settingsEngine := webrtc.SettingEngine{}

	if len(ips) != 0 {
		settingsEngine.SetNAT1To1IPs(ips, webrtc.ICECandidateTypeHost)
	} else {
		configuration.ICEServers = []webrtc.ICEServer{
			{
				URLs: []string{"stun:stun.l.google.com:19302"},
			},
		}
	}

	if iceUDPMux != nil {
		settingsEngine.SetICEUDPMux(iceUDPMux)
	}

	if iceTCPMux != nil {
		settingsEngine.SetICETCPMux(iceTCPMux)
		settingsEngine.SetNetworkTypes([]webrtc.NetworkType{webrtc.NetworkTypeTCP4})
	}

	mediaEngine := &webrtc.MediaEngine{}
	err = mediaEngine.RegisterCodec(
		webrtc.RTPCodecParameters{
			RTPCodecCapability: webrtc.RTPCodecCapability{
				MimeType:  webrtc.MimeTypeH264,
				ClockRate: 90000,
			},
			PayloadType: 96,
		},
		webrtc.RTPCodecTypeVideo)

	if err != nil {
		nazalog.Error(err)
		return
	}

	err = mediaEngine.RegisterCodec(
		webrtc.RTPCodecParameters{
			RTPCodecCapability: webrtc.RTPCodecCapability{
				MimeType:  webrtc.MimeTypeH265,
				ClockRate: 90000,
			},
			PayloadType: 102,
		},
		webrtc.RTPCodecTypeVideo)

	if err != nil {
		nazalog.Error(err)
		return
	}

	// opus
	err = mediaEngine.RegisterCodec(
		webrtc.RTPCodecParameters{
			RTPCodecCapability: webrtc.RTPCodecCapability{
				MimeType:  webrtc.MimeTypeOpus,
				ClockRate: 48000,
			},
			PayloadType: 111,
		},
		webrtc.RTPCodecTypeAudio)

	if err != nil {
		nazalog.Error(err)
		return
	}

	// PCMU
	err = mediaEngine.RegisterCodec(
		webrtc.RTPCodecParameters{
			RTPCodecCapability: webrtc.RTPCodecCapability{
				MimeType:  webrtc.MimeTypePCMU,
				ClockRate: 8000,
			},
			PayloadType: 0,
		},
		webrtc.RTPCodecTypeAudio)

	if err != nil {
		nazalog.Error(err)
		return
	}

	// PCMA
	err = mediaEngine.RegisterCodec(
		webrtc.RTPCodecParameters{
			RTPCodecCapability: webrtc.RTPCodecCapability{
				MimeType:  webrtc.MimeTypePCMA,
				ClockRate: 8000,
			},
			PayloadType: 8,
		},
		webrtc.RTPCodecTypeAudio)

	if err != nil {
		nazalog.Error(err)
		return
	}

	interceptorRegistry := &interceptor.Registry{}
	if err := webrtc.RegisterDefaultInterceptors(mediaEngine, interceptorRegistry); err != nil {
		return nil, err
	}

	api := webrtc.NewAPI(
		webrtc.WithSettingEngine(settingsEngine),
		webrtc.WithMediaEngine(mediaEngine),
		webrtc.WithInterceptorRegistry(interceptorRegistry))

	pc, err := api.NewPeerConnection(configuration)
	if err != nil {
		nazalog.Error(err)
		return nil, err
	}

	conn = &peerConnection{
		PeerConnection: pc,
	}

	return
}
