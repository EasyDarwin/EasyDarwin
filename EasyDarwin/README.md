# 高性能流媒体服务器EasyDarwin #

EasyDarwin开源流媒体服务器是EasyDarwin开源流媒体平台的流媒体服务部分，基于Apple的开源项目Darwin Streaming Server开发和扩展的, EasyDarwin支持标准RTSP/RTP/RTCP协议，具备RTSP/RTP点播、直播（推模式和拉模式）等功能，支持HLS直播，适应安卓、IOS、微信直播等终端平台，最大程度贴近安防监控、移动互联网流媒体需求；EasyDarwin本身提供了一个高性能的平台框架，Linux/Windows跨平台支持，是开发流媒体服务以及其他类型服务的极佳框架工具； 

EasyDarwin具备一套完整的网络I/O框架以及Utility，开发者很容易在EasyDarwin的基础上开发跨平台服务程序，例如Windows、Linux、Mac、Solaris等系统平台，只要一次熟悉，将会受用终身，并且部署和开发过程简单，文档和支持完备，是互联网<sup>+</sup>时代，对企业和开发者来说，最接地气的开源流媒体服务器；

## EasyDarwin目前支持 ##
1. MP4点播回放；
1. 标准RTSP推模式直播(QTSSReflectorModule)；
1. 标准RTSP拉模式直播(EasyRelayModule)；
1. HLS直播(EasyHLSModule)；
1. 接入EasyDarwin流媒体平台，分布式部署；

## EasyDarwin正在进行开发的 ##
1. HLS回放；
1. Onvif支持；

## 编译、配置、部署的方法 ##

### 1、获取EasyDarwin最新版本 ###
在Github：[https://github.com/EasyDarwin/EasyDarwin](https://github.com/EasyDarwin/EasyDarwin "EasyDarwin") 中获取最新的EasyDarwin版本源码，自行编译成需要的可执行文件，也可以直接在 [https://github.com/EasyDarwin/EasyDarwin/releases](https://github.com/EasyDarwin/EasyDarwin/releases "EasyDarwin Release") 中下载已经归档的相应版本进行部署；

### 2、编译EasyDarwin最新版本(可选) ###

*【如果直接下载已编译好的Release归档版本，可跳过此步骤】*  
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
Nginx的WEB地址为：http://8.8.8.8/，那么我们配置：

    <PREF NAME="HTTP_ROOT_DIR" >http://8.8.8.8/</PREF>
这样就能够将EasyDarwin存储的HLS文件WEB发布到公网了，具体配置可以参考后面HLS直播配置章节。

### 4、运行EasyDarwin ###
Windows版本运行(控制台调试运行)：

    EasyDarwin.exe -c ./easydarwin.xml -d  
> 注：需要把libEasyHLS.dll，libEasyPusher.dll，libEasyRTSPClient.dll，html文件夹和Movies文件夹拷贝到可执行程序的同目录下！

Windows服务方式运行：

我们提供一段脚本

    cd ./
    set curPath="%cd%"
    echo service path：%curPath%
    sc create EasyDarwin binPath= "%curPath%\EasyDarwin.exe -c %curPath%\easydarwin.xml" start= auto
	sc failure EasyDarwin reset= 0 actions= restart/0
    net start EasyDarwin
    pause

将这段脚本做成bat，运行，我们就创建了一个叫做EasyDarwin的Windows服务了，通过系统服务（services.msc）可以查看到。
注：Windows不同版本可能稍有差异，建议在命令行运行bat脚本，而不是直接双击运行，这样能看到具体出错原因！

Linux版本运行（具体配置文件路径根据实际情况设置）：

调试模式运行,

    ./easydarwin -c ../WinNTSupport/easydarwin.xml  -d
后台服务运行,

    ./easydarwin -c /etc/streaming/easydarwin.xml  &

> **注：无论是Windows还是Linux运行EasyDarwin，以Debug模式运行时，-c后面带的配置文件路径可以是相对路径也可以是绝对路径，但是以服务/后台方式运行，就必须是用绝对路径！**

### 5、检查EasyDarwin是否部署成功 ###

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

例如EasyDarwin服务器IP地址是：8.8.8.8，RTSP端口(rtsp_port)：554，IPCamera的RTSP地址是：rtsp://admin:admin@192.168.66.189/22，那么我们可以：

1、配置easydarwin.xml EasyRelayModule

	<PREF NAME="local_ip_address" >8.8.8.8</PREF>

2、请求转发：RTSP://8.8.8.8:554/EasyRelayModule?name=live&url="rtsp://admin:admin@192.168.66.189/22"   （**name是定义一个拉模式转发流的唯一标识，不允许重复**）

3、直播URL：RTSP://8.8.8.8:554/EasyRelayModule?name=live

4、请求停止转发：RTSP://8.8.8.8:554/EasyRelayModule?name=live&cmd=stop  （**cmd=stop表示停止拉模式转发**）

# EasyDarwin HLS直播配置 #

### 1、安装Http服务器 ###
  
Windows环境：  
Windows下可以使用IIS或者Nginx，或者其他的Http服务器。（Windows下以IIS为例）  

- 新建一个站点，指向easydarwin的Movies目录（<font color="red">注意这里Movies文件夹中应有crossdomain.xml配置文件</font>） 

![](http://www.easydss.com:8084/1.png)  


- 在MIME中添加.m3u8和.ts文件类型    

![](http://www.easydss.com:8084/3.png)
![](http://www.easydss.com:8084/2.png)  

> 注：ts与.m3u8的添加类似，这里不再截图。  

Linux环境：  
Linux下可以使用Apache或者Nginx等等，建议使用Nginx比较简单。（Linux下以Nginx为例）  

- 安装Nginx  

<pre>
apt-get install nginx 
</pre>

安装完成后，在浏览器输入127.0.0.1，验证是否安装成功，如下图，则安装成功  
![](http://www.easydss.com:8084/4.png) 
> 
  
- 配置Nginx默认路径  
找到Nginx默认配置文件，获取并记录web根目录位置，例如这里位置是：**/var/www/html/**  
![](http://www.easydss.com:8084/5.png)  

- 配置允许跨域访问

<font color="red">将EasyDarwin开源项目Movies目录中的crossdomain.xml文件放到nginx的web目录</font>，例如这里就放到**/var/www/html/**
  
- 重启服务(如果修改了nginx配置)

<pre>
/usr/sbin/nginx  -s reload 
</pre>
注：Nginx配置方式在不同版本不同的系统中可能略有差异！

### 2、拉模式HLS直播配置 ###

EasyDariwn项目中集成了Mongoose服务器，可以通过Web配置EasyDarwin相关参数。

1、登录Web配置

在浏览器中输入如下地址，打开配置页面（用户名和密码都是admin）：   

    http://[ip]:8088

![](http://www.easydss.com:8084/6.png) 

2、HLS直播配置 
  
设置配置完成的Http服务器的地址，以及ts相关参数  
  
![](http://www.easydss.com:8084/7.png) 
  

3、流媒体文件目录配置
  
点击系统中的基本设置，设置流媒体文件的目录，以Linux系统nginx做WEB服务器为例，比如我们将点播文件存储在/var/www/html/目录
  
![](http://www.easydss.com:8084/9.png) 

> 注： <font color="red">2，3步配置完成后，需要重启EasyDarwin服务器，Web配置中有重启的功能，但是功能还没有完善，目前需要用户手动重启服务器,后续会完善重启功能。</font>
  
4、增加HLS直播  

设置拉取的网络设备的RTSP地址，如：rtsp://admin:admin@10.0.0.3/，点击HLS直播列表增加一个直播
  
![](http://www.easydss.com:8084/8.png)   

设置完成后，在HLS直播列表中会出现我们刚刚设置好的直播项，点击Play可以直接进行HLS直播
  
![](http://www.easydss.com:8084/10.png) 
  
### 3、推模式HLS直播配置 ###
  
HLS直播配置，流媒体文件目录配置和拉模式HLS直播配置中的一样
  
1、是否同步输出HLS设置  
  
点击RTSP直播配置，勾选上“是否同步输出HLS”的选项，点击保存，然后重启服务器
![](http://www.easydss.com:8084/11.png) 
  
2、推送数据
  
可以使用EasyPusher推送RTSP数据到EasyDarwin，具体使用方法，请参考[http://doc.easydarwin.org/EasyPusher/README/](http://doc.easydarwin.org/EasyPusher/README/)  

3、自动生成HLS直播项，点击播放
  
![](http://www.easydss.com:8084/12.png)

## 获取更多信息 ##

邮件：[support@easydarwin.org](mailto:support@easydarwin.org) 

WEB：[www.EasyDarwin.org](http://www.easydarwin.org)

QQ交流群：288214068

Copyright &copy; EasyDarwin.org 2012-2015

![EasyDarwin](http://www.easydarwin.org/skin/easydarwin/images/wx_qrcode.jpg)

