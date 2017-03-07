# EasyCMS #

**EasyCMS**开源中心管理服务器做为EasyDarwin开源流媒体云平台解决方案的云端接入服务，主要进行的是EasyCamera/EasyNVR设备的接入和管理，同时用户也可以复用做为其他类型项目设备接入与管理的框架，EasyCMS也源于EasyDarwin服务架构，具备一套完整的网络I/O框架以及Utility，开发者很容易在EasyDarwin的基础上开发跨平台服务程序，例如Windows、Linux、Mac、Solaris等系统平台，只要一次熟悉，将会受用终身；

## 编译EasyCMS ##

Windows版本编译：可以直接用Visual Studio 2015打开源码文件中的：**/EasyDarwin-master/EasyCMS/WinNTSupport/EasyCMS.sln** 解决方案文件，编译出exe可执行文件EasyCMS.exe；

Linux版本编译：

	cd ./EasyDarwin-master/EasyCMS/
	chmod +x ./Buildit
	./Buildit x64
	cd ./x64

## 配置easycms.xml ##
EasyCMS主要的几个配置项：

	<CONFIGURATION>
		<SERVER>
	
			//快照存储的本地路径
			<PREF NAME="snap_local_path" >./snap/</PREF>
	
			//snap_local_path路径对应的web路径
			<PREF NAME="snap_web_path" >http://121.40.50.44:10080/snap/</PREF>

			//EasyCMS服务监听端口
			<PREF NAME="service_lan_port" TYPE="UInt16" >10000</PREF>
	
			//EasyCMS对外端口，做两个配置主要是为了在端口映射时能灵活使用
			<PREF NAME="service_wan_port" TYPE="UInt16" >10000</PREF>
			
			//EasyCMS对外服务IP地址
			<PREF NAME="service_wan_ip" >121.40.50.44</PREF>
	
		</SERVER>
	
		<MODULE NAME="EasyRedisModule" >
	
			//redis地址
			<PREF NAME="redis_ip" >127.0.0.1</PREF>
			
			//redis端口
			<PREF NAME="redis_port" TYPE="UInt16" >6379</PREF>
	
			//redis密码
			<PREF NAME="redis_password" >RedisPassword</PREF>
	
		</MODULE>
	</CONFIGURATION>

## 运行EasyCMS ##
Windows版本运行(控制台调试运行)：

	EasyCMS.exe -c ./easycms.xml -d

Windows服务方式运行：
我们提供一段脚本在**/EasyDarwin-master/EasyCMS/WinNTSupport/install service.bat**，管理员权限运行，我们就创建了一个叫做EasyCMS的Windows服务了，通过系统服务（services.msc）可以查看到。

**注：Windows不同版本可能稍有差异，建议在命令行运行bat脚本，而不是直接双击运行，这样能看到具体出错原因！**

Linux版本运行（具体配置文件路径根据实际情况设置）：
调试模式运行,

    ./easycms -c ../WinNTSupport/easycms.xml  -d
后台服务运行,

    ./easycms -c /etc/streaming/EasyCMS/easycms.xml  &
注：如果xml配置文件路径不能确定，建议最保险的方式就是用全路径，例如 “/etc/streaming/EasyCMS/easycms.xml”，这样在下一次更新服务的时候，配置文件可以保留！

## 检查EasyCMS是否部署成功 ##

通过访问EasyCMS RESTful接口可以初步判断EasyDarwin流媒体服务器是否已经运行起来了，例如我们可以访问：http://[ip]:[http\_service\_port]/api/v1/getdevicelist 接口查看EasyCMS是否运行响应，后面的版本我们会增加一个获取EasyCMS整体运行配置信息RESTful接口，这样在外部就能查看EasyCMS是否读取到了正确的用户配置：

	{
	   "EasyDarwin" : {
	      "Body" : {
	         "DeviceCount" : "1",
	         "Devices" : [
	            {
	               "AppType" : "EasyNVR",
	               "Name" : "000000",
	               "Serial" : "bbyyj1000001",
	               "Tag" : "none",
	               "TerminalType" : "ARM_Linux"
	            }
	         ]
	      },
	      "Header" : {
	         "CSeq" : "1",
	         "ErrorNum" : "200",
	         "ErrorString" : "Success OK",
	         "MessageType" : "MSG_SC_DEVICE_LIST_ACK",
	         "Version" : "v1"
	      }
	   }
	}


## 外部调用接口 ##

注：{}中的参数为可选

- 获取在线设备列表
<pre>
http://[IP]:[cms_port]/api/v1/getdevicelist?{AppType=[AppType]&TerminalType=[TerminalType]}
</pre>
- 请求具体设备信息 
<pre>
http://[ip]:[port]/api/v1/getdeviceinfo?device=[Serial]
</pre>

- 通过设备取流
<pre>
http://[ip]:[port]/api/v1/startdevicestream?device=001002000001{&channel=0}&protocol=RTSP{&reserve=1}
</pre>

## 获取更多信息 ##

邮件：[support@easydarwin.org](mailto:support@easydarwin.org) 

WEB：[www.EasyDarwin.org](http://www.easydarwin.org)

QQ交流群：[288214068](http://jq.qq.com/?_wv=1027&k=2Dlyhr7 "EasyDarwin交流群1") / [496258327](http://jq.qq.com/?_wv=1027&k=2Hyz2ea "EasyDarwin交流群2")

Copyright &copy; EasyDarwin.org 2012-2017

![EasyDarwin](http://www.easydarwin.org/skin/easydarwin/images/wx_qrcode.jpg)