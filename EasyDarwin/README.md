# 高性能流媒体服务器EasyDarwin #

EasyDarwin开源流媒体服务器是EasyDarwin开源流媒体平台的流媒体服务部分，基于Apple的开源项目Darwin Streaming Server开发和扩展的, EasyDarwin支持标准RTSP/RTP/RTCP协议，具备RTSP/RTP点播、直播（推模式和拉模式）等功能，支持HLS直播，适应安卓、IOS、微信直播等终端平台，最大程度贴近安防监控、移动互联网流媒体需求；EasyDarwin本身提供了一个高性能的平台框架，Linux/Windows跨平台支持，是开发流媒体服务以及其他类型服务的极佳框架工具； 

EasyDarwin具备一套完整的网络I/O框架以及Utility，开发者很容易在EasyDarwin的基础上开发跨平台服务程序，例如Windows、Linux、Mac、Solaris等系统平台，只要一次熟悉，将会受用终身，并且部署和开发过程简单，文档和支持完备，是互联网<sup>+</sup>时代，对企业和开发者来说，最接地气的开源流媒体服务器；

## EasyDarwin目前支持: ##
1. MP4点播回放；
1. 标准RTSP推模式直播(QTSSReflectorModule)；
1. 标准RTSP拉模式直播(EasyRelayModule)；
1. HLS直播(EasyHLSModule)；
1. 接入EasyDarwin流媒体平台，分布式部署；

## EasyDarwin正在进行开发的: ##
1. HLS回放；
1. Onvif支持；
1. WEB配置与管理模块(集成Mongoose)；

## 编译、配置、部署的方法 ##

### 1、获取EasyDarwin最新版本 ###
在Github：[https://github.com/EasyDarwin/EasyDarwin](https://github.com/EasyDarwin/EasyDarwin "EasyDarwin") 中获取最新的EasyDarwin版本源码，自行编译成需要的可执行文件，也可以直接在 [https://github.com/EasyDarwin/EasyDarwin/releases](https://github.com/EasyDarwin/EasyDarwin/releases "EasyDarwin Release") 中下载已经归档的相应版本进行部署；

### 2、编译EasyDarwin最新版本 ###

Windows版本编译，可以直接用**Visual Studio 2008**打开源码文件中的：**/EasyDarwin-master/EasyDarwin/WinNTSupport/EasyDarwin.sln**解决方案文件，编译出exe可执行文件EasyDarwin.exe；

Linux版本编译：

	cd ./EasyDarwin-master/EasyDarwin/
	chmod +x ./Buildit
	./Buildit	（./Buildit i386 or ./Buildit x64编译出相应版本的可执行文件）
	cd ./x64  (or cd ./Release)


### 3、配置easydarwin.xml ###
EasyDarwin主要的几个配置项：

***rtsp_port***：EasyDarwin RTSP服务监听的端口；

***movie_folder***：媒体文件存储的路径，包括点播文件、直播切片生成的hls文件；

***http\_service\_port***：Webservice服务端口；

***hls\_output\_enabled***：配置QTSSReflectorModule在接收推送的同时，是否同步输出hls；

***HTTP\_ROOT\_DIR***：配置EasyHLSModule的对外WEB路径；

***local\_ip\_address***：配置EasyRelayModule对外服务的ip地址，因为可能会有多网卡或者内网映射，所以需要手动配置；

以Linux系统nginx做WEB服务器为例，比如我们将点播文件存储在/EasyDarwin/movies/目录，也就是

    <PREF NAME="movie_folder" >/EasyDarwin/movies/</PREF>
Nginx的WEB地址为：http://8.8.8.8/，且WEB根目录配置为/EasyDarwin/那么我们配置：

    <PREF NAME="HTTP_ROOT_DIR" >http://8.8.8.8/movies/</PREF>
这样就能够将EasyDarwin存储的HLS文件WEB发布到公网了。

### 4、运行EasyDarwin ###
Windows版本运行(控制台调试运行)：

    EasyDarwin.exe -c ./easydarwin.xml -d

Windows服务方式运行：

我们提供一段脚本

    cd ./
    set curPath="%cd%"
    echo service path：%curPath%
    sc create EasyDarwin binPath= "%curPath%EasyDarwin.exe -c %curPath%easydarwin.xml" start= auto
    net start EasyDarwin
    pause
将这段脚本做成bat，运行，我们就创建了一个叫做EasyDarwin的Windows服务了，通过系统服务（services.msc）可以查看到。
注：Windows不同版本可能稍有差异，建议在命令行运行bat脚本，而不是直接双击运行，这样能看到具体出错原因！

Linux版本运行（具体配置文件路径根据实际情况设置）：

调试模式运行,

    ./easydarwin -c ../WinNTSupport/easydarwin.xml  -d
后台服务运行,

    ./easydarwin -c ../WinNTSupport/easydarwin.xml  &
注：如果xml配置文件路径不能确定，建议最保险的方式就是用全路径，例如 “/etc/streaming/EasyDarwin/easydarwin.xml”，这样在下一次更新服务的时候，配置文件可以保留！

### 5、EasyDarwin是否部署成功检查 ###
<PREF NAME="RTSP_debug_printfs" TYPE="Bool16" >true</PREF>
将easydarwin.xml中输出打印配置为true，这样就能在调试模式中看是否有报文发来，也可以在easydarwin.xml中配置log输出的目录和文件名称，再根据log确定问题（具体个性化log，需要自己添加代码，编译部署）；

## 调用方法 ##

### MP4点播 ###
我们将经过Hint处理过的mp4文件存在movie_folder目录中，访问RTSP地址：

    rtsp:://[ip]:[rtsp_port]/[filename]
例如EasyDarwin服务器地址：8.8.8.8，rtsp_port配置为：8554，MP4文件名：demo.mp4，用vlc或者ffplay等播放器访问：rtsp://8.8.8.8:8554/demo.mp4；

### 推模式转发 ###
直接通过标准RTSP/RTP推送流程（ANNOUNCE/SETUP/PLAY/RTP）向EasyDarwin推送音视频数据进行转发和分发，例如rtsp_port配置为8554，那我们可以直接用[**EasyDarwin EasyPusher**](https://github.com/EasyDarwin/EasyPusher "EasyPusher")或者**live555 DarwinInjector**向8554端口进行直播推送；

### 拉模式转发 ###

调用接口（用vlc、ffplay、live555等RTSP Client调用）

    RTSP://[ip]:[rtsp_port]/EasyRelayModule?name=[relayName]&url="[RTSP_URL]"

例如EasyDarwin服务器IP地址是：8.8.8.8，RTSP端口(rtsp_port)：8554，IPCamera的RTSP地址是：rtsp://admin:admin@192.168.66.189/22，那么我们可以：

1、配置easydarwin.xml EasyRelayModule

	<PREF NAME="local_ip_address" >8.8.8.8</PREF>

2、请求转发：RTSP://8.8.8.8:8554/EasyRelayModule?name=live&url="rtsp://admin:admin@192.168.66.189/22"   （**name是定义一个拉模式转发流的唯一标识，不允许重复**）

3、直播URL：RTSP://8.8.8.8:8554/EasyRelayModule?name=live

4、请求停止转发：RTSP://8.8.8.8:8554/EasyRelayModule?name=live&cmd=stop  （**cmd=stop表示停止拉模式转发**）

### HLS直播 ###

调用接口（用浏览器或者其他http client调用）

	RTSP://[ip]:[http_service_port]/api/easyhlsmodule?name=[hlsName]&url=[RTSP_URL]
例如EasyDarwin服务器IP地址是：8.8.8.8，EasyDarwin WebService端口(**http_service_port**)：8080，IPCamera的RTSP地址是：rtsp://admin:admin@192.168.66.189/22，同时，我们在EasyDarwin服务器上部署了nginx，端口为8088，WEB目录为easydarwin.xml中 **movie_folder** 同一个目录，那么我们可以：

1、配置easydarwin.xml EasyHLSModule

	<PREF NAME="HTTP_ROOT_DIR" >http://8.8.8.8:8088/</PREF>

2、请求接口：http://8.8.8.8:8080/api/easyhlsmodule?name=live&url=rtsp://admin:admin@192.168.66.189/22   （**接口会返回http+json格式的hls流地址**）

3、请求停止转发：http://8.8.8.8:8080/api/easyhlsmodule?name=live&cmd=stop  （**cmd=stop表示停止HLS切片**）

## 获取更多信息 ##

邮件：[support@easydarwin.org](mailto:support@easydarwin.org) 

WEB：[www.EasyDarwin.org](http://www.easydarwin.org)

QQ交流群：288214068

Copyright &copy; EasyDarwin.org 2012-2015

![EasyDarwin](http://www.easydarwin.org/skin/easydarwin/images/wx_qrcode.jpg)
