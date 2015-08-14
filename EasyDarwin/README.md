EasyDarwin Streaming Server
===========================

EasyDarwin开源流媒体服务器是EasyDarwin开源流媒体平台的流媒体服务部分，基于Apple的开源项目Darwin Streaming Server开发扩展的, EasyDarwin支持标准RTSP/RTP/RTCP协议，具备RTSP/RTP点播、直播（推模式和拉模式）等功能，支持HLS直播，适应安卓、IOS、微信直播等终端平台，最大程度贴近移动互联网适配需求；EasyDarwin提供了一个高性能的平台框架，Linux/Windows跨平台支持，是开发流媒体服务以及其他类型服务的极佳框架工具。 

EasyDarwin具备一套完整的网络I/O框架以及Utility，开发者很容易在EasyDarwin的基础上开发跨平台服务程序，例如Windows、Linux、Mac、Solaris等系统平台，只要一次熟悉，将会受用终身；

## EasyDarwin目前支持: ##
1. MP4点播回放;
2. 标准RTSP推模式转发(QTSSReflectorModule);
3. 标准RTSP拉模式转发(EasyRelayModule);
4. RTSP转HLS模块(EasyHLSModule);

## 正在进行开发的: ##
1. EasyDarwin云平台架构
2. Onvif支持；

## 计划支持： ##
1. WEB配置与管理模块(集成Mongoose);

## 用法

<strong>步骤一:</strong> 获取EasyDarwin 

<pre>
git clone https://github.com/EasyDarwin/EasyDarwin.git &&
cd EasyDarwin/EasyDarwin
</pre>

<strong>步骤二:</strong> 编译EasyDarwin

<pre>
chmod +x ./Buildit
./Buildit（如果是64位编译：./Buildit x64）
</pre>

<strong>步骤三:</strong> 运行EasyDarwin 

<pre>
./EasyDarwin -c ./WinNTSupport/easydarwin.xml （调试模式加：-d 参数）
</pre>



### 获取更多信息 ###

邮件：[support@easydarwin.org](mailto:support@easydarwin.org) 

WEB：[www.EasyDarwin.org](http://www.easydarwin.org)

QQ交流群：288214068
