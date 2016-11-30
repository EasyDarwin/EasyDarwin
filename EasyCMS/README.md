# EasyCMS #

EasyCMS做为EasyDarwin开源流媒体平台解决方案的一部分，主要进行的是EasyCamera/EasyNVR设备的接入和管理，同时用户也可以复用做为其他类型项目设备接入与管理的框架，EasyCMS也源于EasyDarwin服务架构，具备一套完整的网络I/O框架以及Utility，开发者很容易在EasyDarwin的基础上开发跨平台服务程序，例如Windows、Linux、Mac、Solaris等系统平台，只要一次熟悉，将会受用终身；

## 编译EasyCMS ##
Windows版本编译：可以直接用Visual Studio 2015打开源码文件中的：**/EasyDarwin-master/EasyCMS/WinNTSupport/EasyCMS.sln** 解决方案文件，编译出exe可执行文件EasyCMS.exe；

Linux版本编译：

	cd ./EasyDarwin-master/EasyCMS/
	chmod +x ./Buildit
	./Buildit
	（./Buildit i386 or ./Buildit x64编译出相应版本的可执行文件）
	cd ./x64  (or cd ./Release)

## 配置easycms.xml ##
EasyCMS主要的几个配置项：

***monitor\_lan\_port***：EasyCMS主服务监听的端口；

***snap\_local\_path***：快照文件存储的本地文件系统路径；

***snap\_web\_path***：快照文件存储的目录对应的http网络路径；

***redis\_ip***：连接的redis的ip地址；

***redis\_port***：连接的redis的端口；

***redis\_password***：redis的连接密码；

以Linux系统nginx做WEB服务器为例，比如我们将快照文件存储在*/EasyDarwin/snap/*目录，也就是

    <PREF NAME="snap_local_path" >/EasyDarwin/snap/</PREF>
Nginx的WEB地址为：http://8.8.8.8/ ，且WEB根目录配置为*/EasyDarwin/*那么我们配置：

    <PREF NAME="snap_web_path" >http://8.8.8.8/snap/</PREF>
这样就能够将EasyCMS存储的快照文件发布到公网了。

## 运行EasyCMS ##
Windows版本运行(控制台调试运行)：

	EasyCMS.exe -c ./easycms.xml -d

Windows服务方式运行：
我们提供一段脚本

	cd ./
	set curPath="%cd%"
	echo service path：%curPath%
	sc create EasyCMS binPath= "%curPath%EasyCMS.exe -c %curPath%easycms.xml" start= auto
	net start EasyCMS
	pause

将这段脚本做成bat，运行，我们就创建了一个叫做EasyCMS的Windows服务了，通过系统服务（services.msc）可以查看到。

**注：Windows不同版本可能稍有差异，建议在命令行运行bat脚本，而不是直接双击运行，这样能看到具体出错原因！**

Linux版本运行（具体配置文件路径根据实际情况设置）：
调试模式运行,

    ./easycms -c ../WinNTSupport/easycms.xml  -d
后台服务运行,

    ./easycms -c /etc/streaming/EasyCMS/easycms.xml  &
注：如果xml配置文件路径不能确定，建议最保险的方式就是用全路径，例如 “/etc/streaming/EasyCMS/easycms.xml”，这样在下一次更新服务的时候，配置文件可以保留！

## 检查EasyCMS是否部署成功 ##

	<PREF NAME="MSG_debug_printfs" TYPE="Bool16" >true</PREF>

将easycms.xml中输出打印配置为true，这样就能在调试模式中看是否有报文发来，也可以在easycms.xml中配置log输出的目录和文件名称，再根据log确定问题（具体个性化log，需要自己添加代码，编译部署）；


## 外部调用接口 ##

注：{}中的参数为可选

- 获取在线设备列表
<pre>
http://[IP]:[cms_port]/api/getdevicelist?{AppType=[AppType]&TerminalType=[TerminalType]}
</pre>
- 请求具体设备信息 
<pre>
http://[ip]:[port]/api/getdeviceinfo?device=[Serial]
</pre>

- 通过设备取流
<pre>
http://[ip]:[port]/api/getdevicestream?device=001002000001{&channel=0}&protocol=RTSP{&reserve=1}
</pre>

## 获取更多信息 ##

邮件：[support@easydarwin.org](mailto:support@easydarwin.org) 

WEB：[www.EasyDarwin.org](http://www.easydarwin.org)

QQ交流群：288214068

Copyright &copy; EasyDarwin.org 2012-2015

![EasyDarwin](http://www.easydarwin.org/skin/easydarwin/images/wx_qrcode.jpg)