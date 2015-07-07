## EasyDarwin开源流媒体云平台 ##

**EasyDarwin** 是由国内开源流媒体团队开发和维护的一款开源流媒体云平台，从2013年发展至今，从原有的单流媒体服务器形式，扩展成现在的平台化开源项目，更好地帮助广大流媒体开发者和创业型企业快速构建流媒体平台，更加贴近最新的移动互联网(安卓、IOS、微信)流媒体需求；

EasyDarwin平台目前包括CMS(中心管理服务)，EasyDarwin(流媒体服务)，EasyCamera(开源流媒体摄像机)、EasyPlayer（开源流媒体播放器）以及后续将扩展的录像、回放等多个服务单元：

- **CMS** 开源的设备接入与管理服务，具体见[https://github.com/EasyDarwin/](https://github.com/EasyDarwin/EasyDarwin/)；
- **EasyDarwin** 开源流媒体服务，具体见[https://github.com/EasyDarwin/](https://github.com/EasyDarwin/EasyDarwin/)；


### System Architecture

<pre>

      +---------+         +----------+        +------------+        +------------+
      +  Client +         +    CMS   +        + EasyCamera +        + EasyDarwin +
      +---|-----+         +----|-----+        +----|-------+        +------|-----+
+---------+--------------------+-------------------+-----------------------+
|         |                    |<-Register Online--+                       |
+---------+--------------------+-------------------+-----------------------+
|         +--Get Device List-->|                   |                       |
|         |                    |                   |                       |
|         |<-Device List Json--|                   |                       |
+---------+--------------------+-------------------+-----------------------+
|         |                    |                   |                       |
|         +-Get Device Stream->|                   |                       |
|         |    (Device SN)     |                   |                       |
|         |                    +--request stream-->|                       |
|         |                    | (EasyDarwin Addr) |                       |
|         |                    |                   +---RTSP Stream Push--->|
|         |                    |                   +====RTP Streaming=====>|
|         |                    |                   |                       |
|         |                    |<---Streaming OK---+                       |
|         |<--live stream url--+                   |                       |
|         |                    |                   |                       |
|         +-------------------HTTP or RTSP Streaming---------------------->|
|         |                    |                   |                       |
+---------+--------------------+-------------------+-----------------------+

</pre>

### 获取更多信息 ###

邮件：[support@easydarwin.org](mailto:support@easydarwin.org) 

WEB：[EasyDarwin.org](http://www.easydarwin.org)

QQ交流群：288214068



