# EasyDarwin #

**EasyDarwin**是由国内开源流媒体团队维护和迭代的一整套开源流媒体视频平台框架，从2012年12月创建并发展至今，包含有单点服务的开源流媒体服务器，和扩展后的流媒体云平台架构的开源框架，开辟了诸多的优质开源项目，能更好地帮助广大流媒体开发者和创业型企业快速构建流媒体服务平台，更快、更简单地实现最新的移动互联网(安卓、iOS、H5、微信)流媒体直播与点播的需求，尤其是安防行业与互联网行业的衔接；

## EasyDarwin开源项目（7Project） ##
EasyDarwin开源项目至今包括有：**EasyDarwin**（开源流媒体服务器）、**EasyCMS**（中心管理服务器）、**EasyCamera**（云摄像机服务）、**EasyClient**（云平台客户端）、**EasyAACEncoder**（开源音频编码项目）、**EasyAudioDecoder**（开源音频解码项目）、**EasyProtocol**（开源云平台协议）等多个项目，完整地构架了一套开源流媒体云平台方案：

1. **EasyCMS** 开源的设备接入与管理服务，支持多设备、多客户端接入，能非常快速地帮助大家实现稳定的设备接入服务，可以根据自己的需求进行服务功能拆分（例如用户接入服务与设备接入服务拆分等），具体见[https://github.com/EasyDarwin/EasyDarwin/tree/master/EasyCMS](https://github.com/EasyDarwin/EasyDarwin/tree/master/EasyCMS)；

1. **EasyDarwin** 核心流媒体服务！RTSP开源流媒体直播服务，高效、稳定、可靠、功能齐全，支持RTSP流媒体协议，支持安防行业需要的摄像机流媒体转发功能、支持互联网行业需要的多平台(PC、Android、IOS)**RTSP直播**（H264/MJPEG/MPEG4、AAC/PCMA/PCMU/G726）功能，底层(Select/Epoll网络模型、无锁队列调度)和上层(RESTful接口、WEB管理、多平台编译)、关键帧索引（秒开画面）、远程运维等方面优化，这些都是全代码完全开源的，具体接口调用方法和流程见：[https://github.com/EasyDarwin/EasyDarwin](https://github.com/EasyDarwin/EasyDarwin)；

1. **EasyCamera** 设备端（摄像机、移动设备、桌面程序）对接EasyDarwin平台的方案，跨平台，支持Windows、Linux、ARM，其中EasyDarwin摄像机是我们定制的一款摄像机硬件与EasyDarwin平台进行对接的方案，摄像机采用海思3518E方案，支持RTSP、Onvif、WEB管理、配套SDK工具，作为开发和演示硬件工具，我们提供了全套完备的程序和文档，既可以用于流媒体学习，又可以用于方案移植参考，更可以直接用于项目中，用户可以将摄像机定制的部分替换成自己摄像机的硬件SDK，具体接入方法见[https://github.com/EasyDarwin/EasyCamera](https://github.com/EasyDarwin/EasyCamera)；

1. **EasyClient** 是EasyDarwin开源流媒体云平台的客户端实现，项目地址：[https://github.com/EasyDarwin/EasyClient](https://github.com/EasyDarwin/EasyClient "EasyClient")，功能包含：Windows、Android、iOS、H5(支持微信)四个部分，其主要功能包括：
	> - 云平台设备列表获取；
	> - 设备实时码流请求与播放；
	> - 设备云台控制；
	> - 设备语音对讲；

1. **EasyAACEncoder** 是一套简单、高效、稳定的开源音频编码库，支持将各种音频数据(G.711A/PCMA、G.711U/PCMU、G726、PCM)转码成AAC(with adts)格式，其中aac编码部分采用的是业界公认的faac库，EasyAACEncoder支持Windows、Linux、ARM等多种平台，能够广泛应用于各种移动终端设备、嵌入式设备和流媒体转码服务器！项目地址：[https://github.com/EasyDarwin/EasyAACEncoder](https://github.com/EasyDarwin/EasyAACEncoder "EasyAACEncoder")；

1. **EasyAudioDecoder** 是一套应用于移动端的简单、高效、稳定的开源音频解码库，能够将G.711A/PCMA、G.711U/PCMU、G726、AAC等音频格式转码到Linear PCM，再提供给流媒体播放器进行音频播放，EasyAudioDecoder支持跨平台，支持Android & iOS，目前已稳定应用于EasyPlayer、EasyClient等多个开源及商业项目！项目地址：[https://github.com/EasyDarwin/EasyAudioDecoder](https://github.com/EasyDarwin/EasyAudioDecoder "EasyAudioDecoder")；

1. **EasyProtocol** 是EasyDarwin开源流媒体服务器和开源平台使用的一套开源json协议，具有合理的结构设计、完善的层次逻辑以及简单精炼的调用接口，非常易于使用和扩展，不仅长期应用于EasyDarwin的服务器及平台中，而且能够快速扩展用户的自定义需求，非常好用！项目地址：[https://github.com/EasyDarwin/EasyProtocol](https://github.com/EasyDarwin/EasyProtocol "EasyProtocol")；

EasyDarwin RTSP流媒体服务器完全开源，EasyDarwin RTSP流媒体服务器在Darwin Streaming Server基础上做的优化和迭代，完全开源！后续也将继续扩展的录像、回放等多种服务和工具集，各个功能单元既可以独立使用于项目，又可以整体使用，形成一个完整、简单、易用、高效的流媒体解决方案！


## EasyDarwin商业项目（8Project） ##
同时，EasyDarwin开源团队也开发了很多流媒体方面的商业项目，包括有：

1. **EasyPlayer** 是一款精炼、高效、稳定的流媒体播放器，分为RTSP版和Pro版本，EasyPlayer RTSP版本支持Windows(支持多窗口、包含ActiveX，npAPI Web插件)、Android、iOS多个平台，EasyPlayerPro支持Android、iOS，支持各种各样的流媒体音视频直播/点播播放，项目地址：[https://github.com/EasyDarwin/EasyPlayer](https://github.com/EasyDarwin/EasyPlayer "EasyPlayer")；

1. **EasyPusher** 是一款简单、高效、稳定的一款标准RTSP/RTP协议直播推送库，支持将H.264/G.711/G.726/AAC等音视频数据推送到RTSP流媒体服务器进行低延时直播或者视频通信，支持Windows、Linux、ARM、Android、iOS等平台，EasyPusher配套EasyDarwin流媒体服务器、EasyPlayer RTSP播放器适用于特殊行业的低延时应急指挥需求！项目地址：[https://github.com/EasyDarwin/EasyPusher](https://github.com/EasyDarwin/EasyPusher "EasyPusher")；

1. **EasyNVR** 摄像机（通用RTSP、Onvif摄像机）接入服务，EasyNVR能够通过简单的摄像机通道配置、存储配置、云平台对接配置、CDN配置等，将统监控行业里面的高清网络摄像机IP Camera、NVR、移动拍摄设备接入到EasyNVR，EasyNVR能够将这些视频源 的音视频数据采集到设备端，进行全平台终端直播、录像存储、录像检索和录像回放。并且EasyNVR能够将视频源的直播数据对接到第三方视频平台、CDN网络，实现互联网直播分发，具体接入方法见[https://github.com/EasyDarwin/EasyNVR](https://github.com/EasyDarwin/EasyNVR)；

1. **EasyIPCamera** 是一套精炼、高效、稳定的RTSP服务器组件,调用简单灵活,轻松嵌入部署到IPCamera中，并发性能属于行业领先水平，广泛应用于IPCamera RTSP服务、Android/Windows投屏/同屏直播服务，例如课堂教学同屏、会议同屏、广告投放同屏等！项目地址：[https://github.com/EasyDarwin/EasyIPCamera](https://github.com/EasyDarwin/EasyIPCamera "EasyIPCamera")；

1. **EasyRTMP** 是一套调用简单、功能完善、运行高效稳定的RTMP功能组件，经过多年实战和线上运行打造，支持RTMP推送断线重连、环形缓冲、智能丢帧、网络事件回调，支持Windows、Linux、arm（hisiv100/hisiv200/hisiv300/hisiv400/etc..）、Android、iOS平台，支持市面上绝大部分的RTMP流媒体服务器，包括Red5、Ngnix_rtmp、crtmpserver等主流RTMP服务器，能够完美应用于各种行业的直播需求，手机直播、桌面直播、摄像机直播、课堂直播等等方面！项目地址：[https://github.com/EasyDarwin/EasyRTMP](https://github.com/EasyDarwin/EasyRTMP "EasyRTMP")；

1. **EasyRTSPClient** 是一套简单、稳定、高效、易用的RTSPClient工具库，支持Windows、Linux、ARM、Android、iOS等几乎所有平台，支持RTP Over TCP/UDP，支持断线重连，能够接入市面上99%以上的IPC，调用简单且成熟稳定，能广泛应用于播放器、NVR、流媒体系统级联等产品中！项目地址：[https://github.com/EasyDarwin/EasyRTSPClient](https://github.com/EasyDarwin/EasyRTSPClient "EasyRTSPClient")；

1. **EasyHLS** 是一套简单、可靠、高效、稳定的HLS直播切片SDK，能够将实时的H.264视频和AAC音频流实时切片成可供WEB、Android、iOS、微信等全平台客户端观看的HLS（m3u8+ts）直播流，搭配EasyRTSPClient、EasyAACEncoder等项目，可将大部分的安防摄像机对外进行HLS直播发布，同时也可灵活集成在各种流媒体服务中！项目地址：[https://github.com/EasyDarwin/EasyHLS](https://github.com/EasyDarwin/EasyHLS "EasyHLS")；

1. **EasyRMS** 是一套基于HLS协议的录像与回放服务器，EasyRMS能够将RTSP源获取到本地进行本地存储或者存储到阿里云对象存储OSS云存储等第三方存储平台，同时EasyRMS提供录像的检索与查询接口，检索出录像的HLS地址进行录像回放！项目地址：[https://github.com/EasyDarwin/EasyRMS](https://github.com/EasyDarwin/EasyRMS "EasyRMS")；

## EasyDarwin云平台 ##

EasyDarwin云平台是一套由EasyDarwin、EasyCMS、EasyCamera、EasyClient、nginx、redis构成的完整云平台架构，支持分布式、跨平台、多点部署，流媒体服务器支持负载均衡，按需直播，非常适用于互联网化的安防、智能家居、幼教平台、透明厨房、透明家装等多个行业应用：

### 平台架构 ###

![](http://www.easydarwin.org/github/images/cloud_framework0310.jpg)

### 平台协议 ###

[![EasyDarwin Protocol](http://www.easydarwin.org/github/images/EProtocol.jpg)](https://github.com/EasyDarwin/EasyDarwin/tree/master/Doc "EasyDarwin Protocol")

### 平台端口 ###

- EasyCMS：接口服务端口,10000
- EasyDarwin：接口服务端口,10008
- EasyDarwin：RTSP服务端口,10554
- EasyDarwin：HTTP后台管理端口,10080
- EasyNVR：接口服务端口,10010
- EasyNVR：后台管理端口,10080


### 平台演示 ###

- Web客户端：[http://www.easydarwin.org/easyclient/](http://www.easydarwin.org/easyclient/ "EasyClient H5")

- Android版本：[http://fir.im/EasyClient](http://fir.im/EasyClient "EasyClient Android")

![EasyClient Android](http://www.easydarwin.org/github/images/firimeasyclientandroid.png)

- iOS版本：[https://itunes.apple.com/us/app/easyclient/id1141850816](https://itunes.apple.com/us/app/easyclient/id1141850816 "EasyClient iOS")

![EasyClient iOS](http://www.easydarwin.org/github/images/firimeasyclientios170228.png)

## 商务合作 ##
#### 我们欢迎的合作模式： ####

越来越多的企业选择EasyDarwin开源平台作为他们产品和项目的基础，从而也产生了越来越多各种各样的需求，EasyDarwin开源团队接受与企业的项目合作：

- EasyDarwin平台现有项目的技术咨询与培训合作；
- EasyDarwin平台大方向上新项目/新技术的拓展合作；

#### 技术合作找Babosa@EasyDarwin.org： ####
- 邮件:Babosa@EasyDarwin.org

## 捐赠您自己的项目 ##


**EasyDarwin**从发展至今，从最初单纯的流媒体服务器EasyDarwin，已经发展成为一个非常丰富的流媒体开源社区了，目前EasyDarwin Github所有的开源项目，有EasyDarwin开源团队开发，也有外部开发者贡献给EasyDarwin社区的，并由原始作者和EasyDarwin开源团队一起进行后续的开发和维护；

#### 捐赠的好处 ####

捐赠进入EasyDarwin社区的开源项目，必须命名以Easy前缀打头，项目捐赠进入EasyDarwin开源社区，会有非常庞大的流媒体社区用户使用，帮助您修复项目bug，快速进行功能迭代，并且有开源团队和开源社区的大牛来帮助您解决诸多棘手的问题，对提升项目和个人技术水平，都是非常有益的；

#### 捐赠的方法 ####

捐赠渠道：

1. 发送邮件到：support@easydarwin.org or  babosa@easydarwin.org；
2. 加EasyDarwin项目创始人QQ：289042893 沟通；

记得附上项目具体说明；

## 获取更多信息 ##

邮件：[support@easydarwin.org](mailto:support@easydarwin.org) 

WEB：[www.EasyDarwin.org](http://www.easydarwin.org)

QQ交流群：[288214068](http://jq.qq.com/?_wv=1027&k=2Dlyhr7 "EasyDarwin交流群1") / [496258327](http://jq.qq.com/?_wv=1027&k=2Hyz2ea "EasyDarwin交流群2")

Copyright &copy; www.EasyDarwin.org 2012-2017

![EasyDarwin](http://www.easydarwin.org/skin/easydarwin/images/wx_qrcode.jpg)
