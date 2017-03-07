# 高性能流媒体服务器EasyDarwin #

EasyDarwin开源流媒体服务器是EasyDarwin开源流媒体云平台的流媒体服务部分，是整个流媒体云平台的核心服务，EasyDarwin基于Apple的开源项目Darwin Streaming Server v6.0.3开发和扩展的，EasyDarwin支持标准RTSP/RTP/RTCP协议，具备RTSP直播功能，适应PC、安卓、IOS直播等各终端平台，最大程度贴近安防监控、移动互联网流媒体需求；

EasyDarwin本身提供了一个高性能的服务端框架，Linux/Windows跨平台支持，是开发流媒体服务以及其他类型服务的极佳框架工具，EasyDarwin具备一套完整的网络I/O框架以及Utility，EasyDarwin开源团队也在不断进行更优的性能优化（epoll、线程池、内存池、堆栈调用、寄存器调用等）、应用优化（RESTful接口、WEB管理后台、配套APP等），开发者很容易在EasyDarwin的基础上开发跨平台服务程序，例如Windows、Linux、Mac、Solaris等系统平台，只要一次熟悉，将会受用终身，并且部署和开发过程简单，文档和支持完备，是互联网<sup>+</sup>时代，对企业和开发者来说，最接地气的开源流媒体服务器；

## 视频教程 ##

EasyDarwin开源流媒体服务器：编译、配置、部署：[http://edu.csdn.net/course/detail/2431](http://edu.csdn.net/course/detail/2431 "EasyDarwin开源流媒体服务器视频教程")

## EasyDarwin 功能 ##

### 支持 ###
1. 标准RTSP/RTP推送直播；
2. 接入EasyDarwin流媒体云平台，分布式、多点部署；

### 开发中… ###
1. EasyDarwin Web管理后台；
1. EasyDarwin商用流媒体点播与直播服务器；


## 编译、配置、部署的方法 ##

### 1、获取EasyDarwin最新版本 ###
在Github：[https://github.com/EasyDarwin/EasyDarwin](https://github.com/EasyDarwin/EasyDarwin "EasyDarwin") 中获取最新的EasyDarwin版本源码，自行编译成需要的可执行文件，也可以直接在 [https://github.com/EasyDarwin/EasyDarwin/releases](https://github.com/EasyDarwin/EasyDarwin/releases "EasyDarwin Release") 中下载已经归档的相应版本进行部署；
> 最好的选择就是：从归档版本中获取可执行文件或者获取源码编译成可执行文件，未归档的版本可能正在开发迭代中，各个模块都可能不是很完善；

### 2、编译EasyDarwin可执行文件 ###

*【如果直接下载已经编译好的Release归档版本，可跳过此步骤】*  

- Windows版本编译，可以直接用**Visual Studio 2015**打开源码文件中的：**/EasyDarwin-master/EasyDarwin/WinNTSupport/EasyDarwin.sln**解决方案文件，编译出exe可执行文件EasyDarwin.exe，也可以用更高版本的vs进行编译，vs向下兼容，所以编译应该不是什么大问题，可能会有部分编译选项需要调整，这个根据实际情况调整即可，可以肯定的是，源码编译都是没有问题的；
> 经常会有开发者在编译完成后直接vs运行出现无法运行xxx.lib或者缺少xxx.dll的问题，建议好好补补基础知识：
> 
> 1. vs调试运行需要设置EasyDarwin项目为启动项；
> 2. 运行缺少dll时，可以将dll复制到vs的EasyDarwin.vcproj同级目录，或者设置EasyDarwin.vcproj项目熟悉，将dll路径以环境变量的形式添加到vs：
> ![EasyDarwin](http://www.easydarwin.org/github/images/easydarwin/00.png)


- Linux版本编译(**gcc4.8+，support c++ 11**)，将从Github获取的EasyDarwin源码zip文件进行unzip解压，再进行具体编译：

>
	cd ./EasyDarwin-master/EasyDarwin/
	chmod +x ./Buildit
	./Buildit	（./Buildit i386 or ./Buildit x64编译出相应版本的可执行文件）
	cd ./x64  (or cd ./Release)


### 3、配置easydarwin.xml ###
EasyDarwin主要的几个配置项：

***rtsp_port***：EasyDarwin RTSP服务监听的端口；

***service\_lan\_port***：HTTP RESTful服务端口；

### 4、运行EasyDarwin ###
 

- Windows调试运行：

    EasyDarwin.exe -c ./easydarwin.xml -d  

- Windows服务方式运行：

我们在EasyDarwin\WinNTSupport目录提供一段脚本install service.bat，运行，我们就创建了一个叫做EasyDarwin的Windows服务了，通过系统服务（services.msc）可以查看到。
注：Windows不同版本可能稍有差异，建议在命令行运行bat脚本，而不是直接双击运行，这样能看到具体出错原因！

- Linux调试运行

    ./easydarwin -c ../WinNTSupport/easydarwin.xml  -d

- Linux后台服务方式运行

    ./easydarwin -c /etc/streaming/easydarwin.xml  &

> **注：无论是Windows还是Linux运行EasyDarwin，以Debug模式运行时，-c后面带的配置文件路径可以是相对路径也可以是绝对路径，但是以服务/后台方式运行，就必须是用绝对路径！**


### 5、检查EasyDarwin是否部署成功 ###

通过访问EasyDarwin RESTful接口可以初步判断EasyDarwin流媒体服务器是否已经运行起来了，例如我们可以访问：http://[ip]:[http\_service\_port]/api/v1/getserverinfo 接口查看EasyDarwin是否运行响应，后面的版本我们会增加一个获取EasyDarwin整体运行配置信息RESTful接口，这样在外部就能查看EasyDarwin是否读取到了正确的用户配置：

    {
	   "EasyDarwin" : {
	      "Body" : {
	         "Hardware" : "x86",
	         "InterfaceVersion" : "v1",
	         "RunningTime" : "2 Days 4 Hours 51 Mins 41 Secs",
	         "Server" : "Server: EasyDarwin/7.2 (Build/16.1231; Platform/Win32; Release/EasyDarwin; State/Development; )"
	      },
	      "Header" : {
	         "CSeq" : "1",
	         "ErrorNum" : "200",
	         "ErrorString" : "Success OK",
	         "MessageType" : "MSG_SC_SERVER_INFO_ACK",
	         "Version" : "v1"
	      }
	   }
	}


## 调用方法 ##

### 点播 ###
重要的事情说3遍：

	EasyDarwin RTSP版本的流媒体服务器不支持点播，只有商用版本EasyDarwin才支持点播！！！
	EasyDarwin RTSP版本的流媒体服务器不支持点播，只有商用版本EasyDarwin才支持点播！！！
	EasyDarwin RTSP版本的流媒体服务器不支持点播，只有商用版本EasyDarwin才支持点播！！！


### 直播转发 ###

直接通过标准RTSP/RTP推送流程（ANNOUNCE/SETUP/PLAY/RTP）向EasyDarwin推送音视频数据进行转发和分发，例如rtsp_port配置为8554，那我们可以直接用[**EasyDarwin EasyPusher**](https://github.com/EasyDarwin/EasyPusher "EasyPusher")或者**live555 DarwinInjector**向8554端口进行直播推送，推送后，我们可以通过  **rtsp://[ip]:[http\_service\_port]/api/v1/getrtsplivesessions**  接口获取当前正在进行RTSP直播的列表；

## FAQ ##

### 1. RTSP推送到EasyDarwin出现404错误 ###
	
方法一：用EasyPusher做直播推送；

方法二：修改RTSP ANNOUNCE里面的sdp信息，把 "o=- %u %u IN IP4 %s/r/n" 和 "c=IN IP4 %s/r/n"里面的ip地址改成127.0.0.1就可以了;

方法三：我们将EasyDarwin部署到公网，当服务器置身内网，用端口映射的方式对外提供服务，在接收RTSP/RTP推送的时候，经常会出现在SETUP步骤Darwin返回404错误，经过查找原因，主要是EasyDarwin对推送的sdp信息中的IP地址不能识别，服务器并不知道自己已经置身于公网的地址：

**ifconfig查看地址信息**

![EasyDarwin](http://www.easydarwin.org/github/images/easydarwin/9d86ccd17b3d38b84c27ed1008a9bf38.jpg)

**ip addr查看eth0地址列表**

![EasyDarwin](http://www.easydarwin.org/github/images/easydarwin/45d612cba5ddc67617e734f532df78c7.jpg)

我们通过命令：ip addr add dev eth0 [公网IP]，向eth0添加一个公网地址就解决问题了：

![](http://www.easydarwin.org/github/images/easydarwin/267035e801fadf06ea5f6acb5486a988.jpg)

Windows添加公网地址的方法：

![EasyDarwin](http://www.easydarwin.org/github/images/easydarwin/8604f16ba3eb39ba2dd4e32a63f1fcf1.jpg)

![EasyDarwin](http://www.easydarwin.org/github/images/easydarwin/8604f16ba3eb39ba2dd4e32a63f1fcf1.jpg)

### 2. EasyDarwin做转发延时10几秒? ###
	
在EasyDarwin QTSSReflectorModule转发模块中，有一个控制转发Buffer时间的配置**reflector_buffer_size_sec**，我们将这个配置改成0，也就是在服务器端不做缓存，直接转发，这样在网络条件充足的情况下对比转发和实时流，转发带来的延时也几乎可以忽略了：


### 3. 看不到直播视频，如何排查? ###

我们可以先配置easydarwin.xml文件中的

	<PREF NAME="RTSP_debug_printfs" TYPE="Bool16" >true</PREF>

字段为true，然后重新启动EasyDarwin，请求EasyDarwin，如果一点报文都没有打印，那就是你访问的地址错了！如果报文有打印，那就可以具体看看返回的错误码是多少了，错误码对照表：
	
<table>
	<tbody>
		<tr><td><em>响应码</em></td><td><em>报文描述</em></td><td><em>定义</em></td></tr>
		<tr><td>200</td><td>Success OK</td><td>成功创建</td></tr>
		<tr><td>201</td><td>Success Created</td><td>成功创建</td></tr>
		<tr><td>202</td><td>Success Accepted	</td><td>已接受用于处理，但处理尚未完成</td></tr>
		<tr><td>204</td><td>Success No Content	</td><td>已接收请求，但不存在要回送的信息</td></tr>
		<tr><td>206</td><td>Success Partial Content	</td><td>已接收请求，但要回送的信息不完整</td></tr>
		<tr><td>301</td><td>Redirect Permanent Moved	</td><td>请求的数据具有新的位置且更改是永久的。</td></tr>
		<tr><td>302</td><td>Redirect Temp Moved	</td><td>请求的数据临时具有不同 URI</td></tr>
		<tr><td>303</td><td>Redirect See Other	</td><td>可在另一 URI 下找到对请求的响应</td></tr>
		<tr><td>305</td><td>Use Proxy	</td><td>必须通过位置字段中提供的代理来访问请求的资源</td></tr>
		<tr><td>400</td><td>Client Bad Request	</td><td>请求中有语法问题，或不能满足请求</td></tr>
		<tr><td>401</td><td>Client Unauthorized	</td><td>未授权客户端访问数据</td></tr>
		<tr><td>402</td><td>Payment Required	</td><td>需要付款,表示计费系统已有效</td></tr>
		<tr><td>403</td><td>Client Forbidden	</td><td>禁止, 即使有授权也不需要访问</td></tr>
		<tr><td>404</td><td>Not Found	</td><td>服务器找不到给定的资源</td></tr>
		<tr><td>405</td><td>Method Not Allowed	</td><td>请求的方法不支持</td></tr>
		<tr><td>407</td><td>Proxy Authentication Required	</td><td>代理认证请求，客户机首先必须使用代理认证自身</td></tr>
		<tr><td>408</td><td>Request Timeout	</td><td>请求超时</td></tr>
		<tr><td>409</td><td>Conflict	</td><td>请求冲突</td></tr>
		<tr><td>412</td><td>Precondition Failed	</td><td>前提条件失败</td></tr>
		<tr><td>415</td><td>Unsupported Media Type	</td><td>服务器拒绝服务请求，因为不支持请求实体的格式</td></tr>
		<tr><td>500</td><td>Server Internal Error	</td><td>内部错误,因为意外情况，服务器不能完成请求</td></tr>
		<tr><td>501</td><td>Server Not Implemented	</td><td>未执行,服务器不支持请求</td></tr>
		<tr><td>502</td><td>Server Bad Gateway	</td><td>错误网关,服务器接收到来自上游服务器的无效响应</td></tr>
		<tr><td>503</td><td>Server Unavailable	</td><td>由于临时过载或无法获得服务护,服务器无法处理请求</td></tr>
		<tr><td>505</td><td>RTSP Version Not Supported	</td><td>不支持的RTSP版本</td></tr>

	</tbody>
</table>


### 4. /bin/sh^M: bad interpreter: No such file or directory ###

**感谢QQ:591003999提供的完整解决方案**

从git上导下来的Buildit的格式是：fileformat=dos，但是在Linux下的话 是需要fileformat=unix 所以 ./Buildit 就出现错误 无法执行，需要修改文件格式，

- 在Windows下转换： 

> 利用一些编辑器如UltraEdit或EditPlus等工具先将脚本编码转换，再放到Linux中执行。转换方式如下
> 
> **（UltraEdit）：File-->Conversions-->DOS->UNIX**
> 
> 即可。 

- 也可在Linux中转换： 

> 首先要确保文件有可执行权限 
> 
> 	#sh>chmod a+x filename 
> 
> 然后修改文件格式 
> 
> 	#sh>vi filename 
> 
> 利用如下命令查看文件格式 
> 
> 	:set ff 或 :set fileformat 
> 
> 可以看到如下信息 
> 
> 	fileformat=dos 或 fileformat=unix 
> 
> 利用如下命令修改文件格式
>  
> 	:set ff=unix 或 :set fileformat=unix 
> 
> :wq (存盘退出) 
>
>
>**(又或者用：dos2unix ./Buildit直接修改编码更方便，BTW：感谢Denny.bai提供的方法)**
>
>
> 最后再执行文件 
> 	
> 	#sh>./filename
> 


## 获取更多信息 ##

邮件：[support@easydarwin.org](mailto:support@easydarwin.org) 

WEB：[www.easydarwin.org](http://www.easydarwin.org)

QQ交流群：288214068/496258327

Copyright &copy; EasyDarwin.org 2012-2017

![EasyDarwin](http://www.easydarwin.org/skin/easydarwin/images/wx_qrcode.jpg) 