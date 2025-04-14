package source

import (
	"easydarwin/internal/core/livestream"
	"fmt"
	"github.com/go-co-op/gocron"
	"log/slog"
	"runtime/debug"
	"sync"
	"time"
)

var (
	gCheckPullDeviceFlag = false
	// 检查拉流设备并发量
	gCheckConcurrent               = 30
	gCheckPullPushChannelScheduler *gocron.Scheduler
)

func StartScheduler() {
	gCheckPullDeviceFlag = false
	gCheckPullPushChannelScheduler = gocron.NewScheduler(time.Local)
	// 检测间隔时间
	rtspCheckOnlineTime := 20
	checkNetPullAndRtmpPushDevice()
	_, err := gCheckPullPushChannelScheduler.Every(rtspCheckOnlineTime).Seconds().Do(checkNetPullAndRtmpPushDevice)
	if err != nil {
		slog.Info("start scheduler error")
		return
	}
	gCheckPullPushChannelScheduler.StartAsync()
}
func StopScheduler() {
	if gCheckPullPushChannelScheduler != nil {
		gCheckPullPushChannelScheduler.Stop()
		gCheckPullPushChannelScheduler = nil
	}
	gCheckPullDeviceFlag = false
}

// 检查拉流设备
func checkNetPullAndRtmpPushDevice() {
	if gCheckPullDeviceFlag {
		slog.Info("正在检测中...")
		return
	}
	openStreamTimeOut := 20

	slog.Info("开始一次检测 pull 的状态")
	defer slog.Info("结束一次检测 pull 的状态")
	gCheckPullDeviceFlag = true
	defer func() {
		if err := recover(); err != nil {
			slog.Error(fmt.Sprintf("%s\n", err))
			slog.Error(fmt.Sprintln(string(debug.Stack())))
		}
		gCheckPullDeviceFlag = false
	}()

	channelsList, _, _ := LiveCore.FindLiveStreamALl()
	checkNumber := 0
	// 并发检测数量检测数量
	checkCurrentNumber := gCheckConcurrent
	rtspChannelNum := len(channelsList)

	// 如果一次性并发数量大于 pull 通道数量，并发数修改为 pull 通道数量
	if checkCurrentNumber > rtspChannelNum {
		checkCurrentNumber = rtspChannelNum
	}

	var wgChannel sync.WaitGroup
	for i := 0; i < rtspChannelNum; i++ {

		wgChannel.Add(1)
		checkNumber++
		go func(v livestream.LiveStream) {
			defer func() {
				if err := recover(); err != nil {
					slog.Error(fmt.Sprintf("%s\n", err))
					slog.Error(fmt.Sprintln(string(debug.Stack())))
				}
				wgChannel.Done()
			}()
			// 如果不启用，则直接返回
			if !v.Enable {
				return
			}

			// 开始检测
			err := UpdateOnlineStream(v)
			if err != nil {
				slog.Error(fmt.Sprintf("scheduler [%d]%s\n", v.ID, err))
				return
			}
			time.Sleep(time.Duration(openStreamTimeOut) * time.Second)

		}((channelsList)[i])

		checkCurrentNumber = checkCurrentNumber - 1
		if checkCurrentNumber <= 0 {
			wgChannel.Wait()
			checkCurrentNumber = gCheckConcurrent
		}

	}
	slog.Info(fmt.Sprintf("检测完毕. 检测数量 %d", checkNumber))
}
