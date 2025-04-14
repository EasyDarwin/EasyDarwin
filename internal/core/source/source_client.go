package source

import (
	"easydarwin/internal/core/svr"
	"fmt"
	"github.com/q191201771/lal/pkg/aac"
	"github.com/q191201771/lal/pkg/avc"
	"github.com/q191201771/lal/pkg/base"
	"github.com/q191201771/lal/pkg/logic"
	"github.com/q191201771/lal/pkg/rtprtcp"
	"github.com/q191201771/lal/pkg/rtsp"
	"github.com/q191201771/lal/pkg/sdp"
	"io/ioutil"
	"log/slog"
	"os"
	"os/exec"
	"path/filepath"
	"runtime"
	"sync"
	"time"
)

var (
	pullingStreamMapLock sync.RWMutex
	pullingStreamMap     = make(map[uintptr]*StreamClient)
)

type StreamClient struct {
	ChannelID           int
	Name                string
	URL                 string
	AudioEnable         bool
	OnDemand            bool
	TransType           TransType
	Status              StreamStatus
	ErrorString         string
	IsSnap              bool //快照
	OnDemandCloseSource bool
	Fps                 int64 //拉录像视频帧率

	TouchTime     time.Time
	loopDuration  time.Duration
	TouchDuration time.Duration

	quit           chan bool
	Session        logic.ICustomizePubSessionContext
	Online         int
	width          uint
	height         uint
	fpsUs          int //时间戳(微秒)
	gFps           int64
	codecHandle    uintptr
	timestampVideo float32
	timestampAudio float32
	hasPps         bool
	pullSession    *rtsp.PullSession
	ascContext     *aac.AscContext
	videoIFrame    []byte
}

func (client *StreamClient) Start() {

	ticker := time.NewTicker(time.Duration(10) * time.Second)
	defer ticker.Stop()
	for {
		select {
		case <-ticker.C:
			client.DoInTicker()
		case <-client.quit:
			return
		}
	}
}
func (client *StreamClient) UpdateTouchTime() {
	client.TouchTime = time.Now()

}
func (client *StreamClient) UpdateOnDemandCloseSource(v bool) {
	client.OnDemandCloseSource = v
}
func (client *StreamClient) UpdateOnDemand(v bool) {
	client.OnDemand = v
}
func (client *StreamClient) DoInTicker() {
	if client == nil {
		return
	}
	thisTime := time.Now()

	// stop local pusher if touch timeout
	if client.OnDemand && thisTime.After(client.TouchTime.Add(client.TouchDuration)) {
		client.Clean()
	}
}

func (client *StreamClient) Clean() {
	if client == nil {
		return
	}
	client.OnDemandCloseSource = false
	client.quit <- true
	if err := client.Stop(); err != nil {
		slog.Error("close error: ", client.ChannelID, err)
	}
	if client.Session != nil {
		client.DelSession()
	}
}
func (client *StreamClient) SetOnline(online int) {
	if client == nil {
		return
	}
	client.Online = online
	err := LiveCore.UpdateLiveStreamInt(client.ChannelID, "online", online)
	if err != nil {
		slog.Error("set online update live stream online", client.ChannelID, online, err.Error())
		return
	}
}
func (client *StreamClient) regist() {
}
func (client *StreamClient) AddSession() error {
	if client.Session != nil {
		return nil
	}
	id := client.ChannelID
	customizePubStreamName := fmt.Sprintf("%s%d", StreamName, id)
	var err error
	client.Session, err = svr.Lals.GetILalServer().AddCustomizePubSession(customizePubStreamName)
	if err != nil {
		return fmt.Errorf("add customize pub session err %d", id)
	}
	slog.Info("add customize pub session ok", id)
	if client.AudioEnable {
		client.Session.WithOption(func(option *base.AvPacketStreamOption) {
			option.VideoFormat = base.AvPacketStreamVideoFormatAnnexb
			option.AudioFormat = base.AvPacketStreamAudioFormatRawAac
		})
	} else {
		client.Session.WithOption(func(option *base.AvPacketStreamOption) {
			option.VideoFormat = base.AvPacketStreamVideoFormatAnnexb
		})
	}
	return nil
}
func (client *StreamClient) DelSession() {
	if client.Session == nil {
		return
	}
	svr.Lals.GetILalServer().DelCustomizePubSession(client.Session)
	client.Session = nil
	slog.Info("del customize pub session ok", client.ChannelID)
	return
}
func (client *StreamClient) IsTouchTime() bool {
	thisTime := time.Now()
	status := false
	// stop local pusher if touch timeout
	if client.OnDemand && thisTime.After(client.TouchTime.Add(client.TouchDuration)) {
		status = true
	}
	return status
}
func (client *StreamClient) IsOpen() bool {

	status := false
	if client.Status == STREAM_OPENING || client.Status == STREAM_OPENED {
		status = true
	}
	return status
}
func (client *StreamClient) unregist() {
}

// readAudioPackets 从aac es流读取所有音频包
func readAudioPackets(audioContent []byte, t float32, pts float32) (audioPackets []base.AvPacket) {
	pos := 0
	//timestamp = t
	for {
		ctx, err := aac.NewAdtsHeaderContext(audioContent[pos : pos+aac.AdtsHeaderLength])
		if err != nil {
			slog.Error("read err ", err)
		}

		packet := base.AvPacket{
			PayloadType: base.AvPacketPtAac,
			Timestamp:   int64(pts),
			Pts:         int64(pts),
			Payload:     audioContent[pos+aac.AdtsHeaderLength : pos+int(ctx.AdtsLength)],
		}

		audioPackets = append(audioPackets, packet)

		//timestamp += float32(48000*4*2) / float32(8192*2) // (frequence * bytePerSample * channel) / (packetSize * channel)

		pos += int(ctx.AdtsLength)
		if pos == len(audioContent) {
			break
		}
	}

	return
}

// readVideoPackets 从h264 es流读取所有视频包
func readVideoPacketsH264(videoContent []byte, t float32, pts float32) (videoPackets []base.AvPacket) {
	//timestamp = t
	err := avc.IterateNaluAnnexb(videoContent, func(nal []byte) {
		//milliseconds := time.Now().UnixNano() / 1e6
		// 将nal数据转换为lalserver要求的格式输入

		packet := base.AvPacket{
			PayloadType: base.AvPacketPtAvc,
			Timestamp:   int64(pts),
			Pts:         int64(pts),
			Payload:     append(avc.NaluStartCode4, nal...),
		}

		videoPackets = append(videoPackets, packet)

		t := avc.ParseNaluType(nal[0])
		if t == avc.NaluTypeSps || t == avc.NaluTypePps || t == avc.NaluTypeSei {
			// noop
		} else {
			//timestamp += float32(1000) / float32(25) // 1秒 / fps
		}
	})
	if err != nil {
		slog.Error("read err ", err)
	}
	return
}

// readVideoPackets 从h265 es流读取所有视频包
func readVideoPacketsH265(videoContent []byte, t float32, pts float32) (videoPackets []base.AvPacket) {
	//timestamp = t
	err := avc.IterateNaluAnnexb(videoContent, func(nal []byte) {
		//milliseconds := time.Now().UnixNano() / 1e6
		// 将nal数据转换为lalserver要求的格式输入

		packet := base.AvPacket{
			PayloadType: base.AvPacketPtHevc,
			Timestamp:   int64(pts),
			Pts:         int64(pts),
			Payload:     append(avc.NaluStartCode4, nal...),
		}

		videoPackets = append(videoPackets, packet)

		t := avc.ParseNaluType(nal[0])
		if t == avc.NaluTypeSps || t == avc.NaluTypePps || t == avc.NaluTypeSei {
			// noop
		} else {
			//timestamp += float32(1000) / float32(25) // 1秒 / fps
		}
	})
	if err != nil {
		slog.Error("read err ", err)
	}
	return
}
func (client *StreamClient) SaveImage(Buf []byte) (err error) {
	dir, _ := os.Getwd()
	pathDir := filepath.Join(dir, "snap", fmt.Sprintf("stream_%d", client.ChannelID))
	snapPathPtr := filepath.Join(pathDir, fmt.Sprintf("stream_%d.raw", client.ChannelID))

	if err = EnsureDir(filepath.Dir(snapPathPtr)); err != nil {
	}
	err = ioutil.WriteFile(snapPathPtr, Buf, 0644)
	if err != nil {
		return err
	}

	if !Exist(snapPathPtr) {
		return
	}
	snapPathJpg := filepath.Join(pathDir, fmt.Sprintf("stream_%d.jpg", client.ChannelID))
	ConvertFrame2Image(snapPathPtr, snapPathJpg)
	if err = EnsureDir(filepath.Dir(snapPathJpg)); err != nil {
		slog.Error("create snap file failed ", client.ChannelID, err.Error())
		return
	}
	return nil
}

func EnsureDir(dir string) (err error) {
	if _, err = os.Stat(dir); os.IsNotExist(err) {
		err = os.MkdirAll(dir, 0755)
		if err != nil {
			return
		}
	}
	return
}
func Exist(path string) bool {
	if _, err := os.Stat(path); os.IsNotExist(err) {
		return false
	}
	return true
}

func getAvCode(dataByte []byte) int {
	codec := 0x1C
	for j := 0; j < len(dataByte)-5; j++ {
		if dataByte[j] == 0x00 && dataByte[j+1] == 0x00 && dataByte[j+2] == 0x00 && dataByte[j+3] == 0x01 && dataByte[j+4] == 0x40 {
			if dataByte[j+4] == 0x02 {
				codec = 0xAE
				break
			} else if dataByte[j+4] == 0x40 {
				codec = 0xAE
				break
			}
		} else if dataByte[j] == 0x00 && dataByte[j+1] == 0x00 && dataByte[j+2] == 0x01 {
			if dataByte[j+3] == 0x02 {
				codec = 0xAE
				break
			} else if dataByte[j+3] == 0x40 {
				codec = 0xAE
				break
			}
		}
	}
	return codec
}

func ConvertFrame2Image(h264Path string, jpgPath string) {
	defer func() {
		if p := recover(); p != nil {
			err := fmt.Errorf("ConvertFrame2Image fail, %v", p)
			slog.Error(fmt.Sprintf("%v", err))
		}
	}()
	ffmpegPath := FFMPEG()
	args := []string{"-i", h264Path, "-frames:v", "1", "-y", jpgPath}
	cmd := exec.Command(ffmpegPath, args...)
	cmd.Run()
}

func FFMPEG() string {
	dir, _ := os.Getwd()
	switch runtime.GOOS {
	case "windows":
		return filepath.Join(dir, "ffmpeg.exe")
	case "linux":
		path := filepath.Join(dir, "ffmpeg")
		os.Chmod(path, 0755)
		return path
	default:
	}

	return ""
}

func (client *StreamClient) toSnap() {
	if client.IsSnap && client.OnDemandCloseSource == false {
		if len(client.videoIFrame) > 0 {
			client.IsSnap = false
			go func(iframeData []byte) {
				err := client.SaveImage(iframeData)
				if err != nil {
					slog.Error(fmt.Sprintf("stream client SaveImage err: %v", err))
				}
			}(client.videoIFrame)
		}
	}
	if client.OnDemand && client.OnDemandCloseSource {
		client.OnDemandCloseSource = false
		if client.Online != OnLineState {
			client.SetOnline(OnLineState)
		}
		return
	} else {
		if client.Online != LivingState {
			client.SetOnline(LivingState)
		}
	}
}

func (client *StreamClient) OnSdp(sdpCtx sdp.LogicContext) {
	client.SetOnline(1)
	if len(sdpCtx.Asc) > 0 {
		client.ascContext, _ = aac.NewAscContext(sdpCtx.Asc)
	}
}

func (client *StreamClient) OnRtpPacket(pkt rtprtcp.RtpPacket) {

}

func (client *StreamClient) OnAvPacket(pkt base.AvPacket) {
	if client.Session == nil {
		return
	}
	if pkt.IsVideo() {
		if !(pkt.PayloadType == base.AvPacketPtAvc || pkt.PayloadType == base.AvPacketPtHevc) {
			fmt.Printf("pkt.PayloadType:%d\n", pkt.PayloadType)
			return
		}
		pkt.Payload[0] = 0
		pkt.Payload[1] = 0
		pkt.Payload[2] = 0
		pkt.Payload[3] = 1
		flag := false
		if pkt.PayloadType == base.AvPacketPtAvc {
			v := pkt.Payload[4] & 0x1f
			if v == 7 {
				flag = true
				client.videoIFrame = make([]byte, 0)
				client.videoIFrame = append(client.videoIFrame, pkt.Payload...)
			}
			if v == 8 {
				flag = true
				if len(client.videoIFrame) > 0 {
					client.videoIFrame = append(client.videoIFrame, pkt.Payload...)
				}
			}
			if v == 5 {
				flag = true
				if len(client.videoIFrame) > 0 {
					client.videoIFrame = append(client.videoIFrame, pkt.Payload...)
					client.toSnap()
				}
			}
			if v == 6 {
				flag = true
			}
			if v == 1 {
				flag = true
			}
			if flag {
				err := client.Session.FeedAvPacket(pkt)
				if err != nil {
					slog.Error("stream client video av packet err ", err)
					return
				}
			}
		} else {
			v := (pkt.Payload[4] & 0x7E) >> 1
			if v == 32 {
				flag = true
				client.videoIFrame = make([]byte, 0)
				client.videoIFrame = append(client.videoIFrame, pkt.Payload...)
			}
			if v == 33 {
				flag = true
				if len(client.videoIFrame) > 0 {
					client.videoIFrame = append(client.videoIFrame, pkt.Payload...)
				}
			}
			if v == 34 {
				flag = true
				if len(client.videoIFrame) > 0 {
					client.videoIFrame = append(client.videoIFrame, pkt.Payload...)
				}
			}
			if v == 19 {
				flag = true
				if len(client.videoIFrame) > 0 {
					client.videoIFrame = append(client.videoIFrame, pkt.Payload...)
					client.toSnap()
				}
			}
			if v == 39 {
				flag = true
			}
			if v == 1 {
				flag = true
			}
			if flag {
				err := client.Session.FeedAvPacket(pkt)
				if err != nil {
					slog.Error("stream client video av packet err ", err)
					return
				}
			}
		}
	} else if pkt.IsAudio() {
		if !client.AudioEnable || pkt.PayloadType != base.AvPacketPtAac {
			return
		}
		if client.ascContext != nil {
			out := client.ascContext.PackAdtsHeader(len(pkt.Payload))
			asc, err := aac.MakeAscWithAdtsHeader(out)
			err = client.Session.FeedAudioSpecificConfig(asc)
			if err != nil {
				slog.Error("stream client audio specific config err ", err)
				return
			}
			err = client.Session.FeedAvPacket(pkt)
			if err != nil {
				slog.Error("stream client audio av packet err ", err)
				return
			}
		}
	}
}
