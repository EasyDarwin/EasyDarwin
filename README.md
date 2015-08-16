## EasyDarwin开源流媒体平台 ##

**EasyDarwin** 是由国内开源流媒体团队开发和维护的一款开源流媒体平台框架，从2012年12月创建并发展至今，从原有的单服务的流媒体服务器形式，扩展成现在的云平台架构的开源项目，更好地帮助广大流媒体开发者和创业型企业快速构建流媒体服务平台，更快、更简单地实现最新的移动互联网(安卓、IOS、微信)流媒体直播与点播的需求；

EasyDarwin流媒体平台目前包括有：CMS(中心管理服务)，EasyDarwin(流媒体服务)，EasyCamera(开源流媒体摄像机)、EasyPlayer（开源流媒体播放器）、以及Utility Library([EasyHLS](https://github.com/EasyDarwin/EasyHLS "EasyHLS") / [EasyRTMP](https://github.com/EasyDarwin/EasyRTMP "EasyRTMP") / [EasyRTSPClient](https://github.com/EasyDarwin/EasyRTSPClient "EasyRTSPClient") / [EasyPusher](https://github.com/EasyDarwin/EasyPusher "EasyPusher") / [EasyG7112AAC](https://github.com/EasyDarwin/EasyG7112AAC "EasyG7112AAC"))，后续将扩展的录像、回放等多个服务单元，形成一整套的流媒体解决方案：

- **CMS** 开源的设备接入与管理服务，支持设备接入与客户端接入，能非常快速地帮助大家实现稳定的接入服务，可以根据自己的需求进行服务功能拆分（例如用户接入服务与设备接入服务拆分等），具体见[https://github.com/EasyDarwin/](https://github.com/EasyDarwin/EasyDarwin/)；

- **EasyDarwin** 开源流媒体服务，高效、稳定、可靠、功能齐全，支持RTSP/HLS/HTTP流媒体协议，具体见[https://github.com/EasyDarwin/](https://github.com/EasyDarwin/EasyDarwin/)；

- **EasyCamera** 摄像机平台对接方案，是EasyDarwin实现的一个摄像机硬件与EasyDarwin平台进行对接的方案，摄像机硬件用的是我们定制的，购买参考设备可以在：[https://easydarwin.taobao.com/](https://easydarwin.taobao.com/ "EasyDarwin")，用户可以将摄像机定制的部分替换成自己摄像机的硬件SDK，具体接入方法见[https://github.com/EasyDarwin/EasyCamera](https://github.com/EasyDarwin/EasyCamera)；


### 系统架构
![](http://www.easydarwin.org/skin/easydarwin/images/architecture20150805.png)

### 直播流程

<pre>

          +---------+         +----------+        +------------+        +------------+
          +  Client +         +    CMS   +        + EasyCamera +        + EasyDarwin +
          +---|-----+         +----|-----+        +----|-------+        +------|-----+
    +---------+--------------------+-------------------+-----------------------+---------+
    |         |                    |<-Register Online--+                       |         |
    +---------+--------------------+-------------------+-----------------------+---------+
    |         +--Get Device List-->|                   |                       |         |
    |         |                    |                   |                       |         |
    |         |<-Device List Json--|                   |                       |         |
    +---------+--------------------+-------------------+-----------------------+---------+
    |         |                    |                   |                       |         |
    |         +-Get Device Stream->|                   |                       |         |
    |         |    (Device SN)     |                   |                       |         |
    |         |                    +--request stream-->|                       |         |
    |         |                    | (EasyDarwin Addr) |                       |         |
    |         |                    |                   +---RTSP Stream Push--->|         |
    |         |                    |                   +====RTP Streaming=====>|         |
    |         |                    |                   |                       |         |
    |         |                    |<---Streaming OK---+                       |         |
    |         |<--live stream url--+                   |                       |         |
    |         |                    |                   |                       |         |
    |         +-------------------HTTP or RTSP Streaming---------------------->|         |
    |         |                    |                   |                       |         |
    +---------+--------------------+-------------------+-----------------------+---------+

</pre>


### 获取更多信息 ###

邮件：[support@easydarwin.org](mailto:support@easydarwin.org) 

WEB：[EasyDarwin.org](http://www.easydarwin.org)

QQ交流群：288214068

---
![](http://www.easydarwin.org/skin/easydarwin/images/wx_qrcode.jpg)
