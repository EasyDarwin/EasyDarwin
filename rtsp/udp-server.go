package rtsp

import (
	"bytes"
	"log"
	"net"
	"strconv"
	"strings"

	"github.com/penggy/EasyGoLib/utils"
)

type UDPServer struct {
	Session *Session

	APort        int
	AConn        *net.UDPConn
	AControlPort int
	AControlConn *net.UDPConn
	VPort        int
	VConn        *net.UDPConn
	VControlPort int
	VControlConn *net.UDPConn

	Stoped bool
}

func (s *UDPServer) Stop() {
	if s.Stoped {
		return
	}
	s.Stoped = true
	if s.AConn != nil {
		s.AConn.Close()
		s.AConn = nil
	}
	if s.AControlConn != nil {
		s.AControlConn.Close()
		s.AControlConn = nil
	}
	if s.VConn != nil {
		s.VConn.Close()
		s.VConn = nil
	}
	if s.VControlConn != nil {
		s.VControlConn.Close()
		s.VControlConn = nil
	}
}

func (s *UDPServer) SetupAudio() (err error) {
	addr, err := net.ResolveUDPAddr("udp", ":0")
	if err != nil {
		return
	}
	s.AConn, err = net.ListenUDP("udp", addr)
	if err != nil {
		return
	}
	networkBuffer := utils.Conf().Section("rtsp").Key("network_buffer").MustInt(1048576)
	if err := s.AConn.SetReadBuffer(networkBuffer); err != nil {
		log.Printf("udp server audio conn set read buffer error, %v", err)
	}
	if err := s.AConn.SetWriteBuffer(networkBuffer); err != nil {
		log.Printf("udp server audio conn set write buffer error, %v", err)
	}
	la := s.AConn.LocalAddr().String()
	strPort := la[strings.LastIndex(la, ":")+1:]
	s.APort, err = strconv.Atoi(strPort)
	if err != nil {
		return
	}
	go func() {
		bufUDP := make([]byte, UDP_BUF_SIZE)
		log.Printf("udp server start listen audio port[%d]", s.APort)
		defer log.Printf("udp server stop listen audio port[%d]", s.APort)
		for !s.Stoped {
			if n, _, err := s.AConn.ReadFromUDP(bufUDP); err == nil {
				rtpBytes := make([]byte, n)
				s.Session.InBytes += n
				copy(rtpBytes, bufUDP)
				pack := &RTPPack{
					Type:   RTP_TYPE_AUDIO,
					Buffer: bytes.NewBuffer(rtpBytes),
				}
				for _, h := range s.Session.RTPHandles {
					h(pack)
				}
			} else {
				log.Println("udp server read audio pack error", err)
				continue
			}
		}
	}()
	addr, err = net.ResolveUDPAddr("udp", ":0")
	if err != nil {
		return
	}
	s.AControlConn, err = net.ListenUDP("udp", addr)
	if err != nil {
		return
	}
	if err := s.AControlConn.SetReadBuffer(networkBuffer); err != nil {
		log.Printf("udp server audio control conn set read buffer error, %v", err)
	}
	if err := s.AControlConn.SetWriteBuffer(networkBuffer); err != nil {
		log.Printf("udp server audio control conn set write buffer error, %v", err)
	}
	la = s.AControlConn.LocalAddr().String()
	strPort = la[strings.LastIndex(la, ":")+1:]
	s.AControlPort, err = strconv.Atoi(strPort)
	if err != nil {
		return
	}
	go func() {
		bufUDP := make([]byte, UDP_BUF_SIZE)
		log.Printf("udp server start listen audio control port[%d]", s.AControlPort)
		defer log.Printf("udp server stop listen audio control port[%d]", s.AControlPort)
		for !s.Stoped {
			if n, _, err := s.AControlConn.ReadFromUDP(bufUDP); err == nil {
				rtpBytes := make([]byte, n)
				s.Session.InBytes += n
				copy(rtpBytes, bufUDP)
				pack := &RTPPack{
					Type:   RTP_TYPE_AUDIOCONTROL,
					Buffer: bytes.NewBuffer(rtpBytes),
				}
				for _, h := range s.Session.RTPHandles {
					h(pack)
				}
			} else {
				log.Println("udp server read audio control pack error", err)
				continue
			}
		}
	}()
	return
}

func (s *UDPServer) SetupVideo() (err error) {
	addr, err := net.ResolveUDPAddr("udp", ":0")
	if err != nil {
		return
	}
	s.VConn, err = net.ListenUDP("udp", addr)
	if err != nil {
		return
	}
	networkBuffer := utils.Conf().Section("rtsp").Key("network_buffer").MustInt(1048576)
	if err := s.VConn.SetReadBuffer(networkBuffer); err != nil {
		log.Printf("udp server video conn set read buffer error, %v", err)
	}
	if err := s.VConn.SetWriteBuffer(networkBuffer); err != nil {
		log.Printf("udp server video conn set write buffer error, %v", err)
	}
	la := s.VConn.LocalAddr().String()
	strPort := la[strings.LastIndex(la, ":")+1:]
	s.VPort, err = strconv.Atoi(strPort)
	if err != nil {
		return
	}
	go func() {
		bufUDP := make([]byte, UDP_BUF_SIZE)
		log.Printf("udp server start listen video port[%d]", s.VPort)
		defer log.Printf("udp server stop listen video port[%d]", s.VPort)
		for !s.Stoped {
			if n, _, err := s.VConn.ReadFromUDP(bufUDP); err == nil {
				rtpBytes := make([]byte, n)
				s.Session.InBytes += n
				copy(rtpBytes, bufUDP)
				pack := &RTPPack{
					Type:   RTP_TYPE_VIDEO,
					Buffer: bytes.NewBuffer(rtpBytes),
				}
				for _, h := range s.Session.RTPHandles {
					h(pack)
				}
			} else {
				log.Println("udp server read video pack error", err)
				continue
			}
		}
	}()

	addr, err = net.ResolveUDPAddr("udp", ":0")
	if err != nil {
		return
	}
	s.VControlConn, err = net.ListenUDP("udp", addr)
	if err != nil {
		return
	}
	if err := s.VControlConn.SetReadBuffer(networkBuffer); err != nil {
		log.Printf("udp server video control conn set read buffer error, %v", err)
	}
	if err := s.VControlConn.SetWriteBuffer(networkBuffer); err != nil {
		log.Printf("udp server video control conn set write buffer error, %v", err)
	}
	la = s.VControlConn.LocalAddr().String()
	strPort = la[strings.LastIndex(la, ":")+1:]
	s.VControlPort, err = strconv.Atoi(strPort)
	if err != nil {
		return
	}
	go func() {
		bufUDP := make([]byte, UDP_BUF_SIZE)
		log.Printf("udp server start listen video control port[%d]", s.VControlPort)
		defer log.Printf("udp server stop listen video control port[%d]", s.VControlPort)
		for !s.Stoped {
			if n, _, err := s.VControlConn.ReadFromUDP(bufUDP); err == nil {
				rtpBytes := make([]byte, n)
				s.Session.InBytes += n
				copy(rtpBytes, bufUDP)
				pack := &RTPPack{
					Type:   RTP_TYPE_VIDEOCONTROL,
					Buffer: bytes.NewBuffer(rtpBytes),
				}
				for _, h := range s.Session.RTPHandles {
					h(pack)
				}
			} else {
				log.Println("udp server read video control pack error", err)
				continue
			}
		}
	}()
	return
}
