# 高性能流媒体服务器EasyDarwin #

EasyDarwin开源流媒体服务器是EasyDarwin开源流媒体云平台的流媒体服务部分，是整个流媒体云平台的核心服务，EasyDarwin基于Apple的开源项目Darwin Streaming Server开发和扩展的，EasyDarwin支持标准RTSP/RTP/RTCP协议，具备RTSP点播、直播（推模式和拉模式）、HLS直播等功能，适应安卓、IOS、微信直播等各终端平台，最大程度贴近安防监控、移动互联网流媒体需求；

EasyDarwin本身提供了一个高性能的服务端框架，Linux/Windows跨平台支持，是开发流媒体服务以及其他类型服务的极佳框架工具，EasyDarwin具备一套完整的网络I/O框架以及Utility，EasyDarwin开源团队也在不断进行更优的性能优化（epoll、线程池、内存池、堆栈调用、寄存器调用等）、应用优化（RESTful接口、WEB管理后台、配套APP等），开发者很容易在EasyDarwin的基础上开发跨平台服务程序，例如Windows、Linux、Mac、Solaris等系统平台，只要一次熟悉，将会受用终身，并且部署和开发过程简单，文档和支持完备，是互联网<sup>+</sup>时代，对企业和开发者来说，最接地气的开源流媒体服务器；

## 视频教程 ##

EasyDarwin开源流媒体服务器：编译、配置、部署：[http://edu.csdn.net/course/detail/2431](http://edu.csdn.net/course/detail/2431 "EasyDarwin开源流媒体服务器视频教程")

## EasyDarwin目前支持 ##
1. MP4点播(QTSSFileModule)；
2. 标准RTSP推模式直播(QTSSReflectorModule)；
3. 标准RTSP拉模式直播(EasyRelayModule)；
4. HLS直播(EasyHLSModule)；
5. 接入EasyDarwin流媒体平台，分布式部署(EasyCMSModule)；

## EasyDarwin正在进行开发的 ##
1. Onvif支持；
2. 分布式部署负载均衡（结合EasyCMS）；

## 编译、配置、部署的方法 ##

### 1、获取EasyDarwin最新版本 ###
在Github：[https://github.com/EasyDarwin/EasyDarwin](https://github.com/EasyDarwin/EasyDarwin "EasyDarwin") 中获取最新的EasyDarwin版本源码，自行编译成需要的可执行文件，也可以直接在 [https://github.com/EasyDarwin/EasyDarwin/releases](https://github.com/EasyDarwin/EasyDarwin/releases "EasyDarwin Release") 中下载已经归档的相应版本进行部署；
> 最好的选择就是：从归档版本中获取可执行文件或者获取源码编译成可执行文件，未归档的版本可能正在开发迭代中，各个模块都可能不是很完善；

### 2、编译EasyDarwin可执行文件 ###

*【如果直接下载已经编译好的Release归档版本，可跳过此步骤】*  

- Windows版本编译，可以直接用**Visual Studio 2008**打开源码文件中的：**/EasyDarwin-master/EasyDarwin/WinNTSupport/EasyDarwin.sln**解决方案文件，编译出exe可执行文件EasyDarwin.exe，也可以用更高版本的vs进行编译，vs向下兼容，所以编译应该不是什么大问题，可能会有部分编译选项需要调整，这个根据实际情况调整即可，可以肯定的是，源码编译都是没有问题的；
> 经常会有开发者在编译完成后直接vs运行出现无法运行xxx.lib或者缺少xxx.dll的问题，建议好好补补基础知识：
> 
> 1. vs调试运行需要设置EasyDarwin项目为启动项；
> 2. 运行缺少dll时，可以将dll复制到vs的EasyDarwin.vcproj同级目录，或者设置EasyDarwin.vcproj项目熟悉，将dll路径以环境变量的形式添加到vs：
> ![EasyDarwin](http://www.easydarwin.org/github/images/easydarwin/0.png)


- Linux版本编译(**gcc4.8+，support c++ 11**)，将从Github获取的EasyDarwin源码zip文件进行unzip解压，再进行具体编译：

>
	cd ./EasyDarwin-master/EasyDarwin/
	chmod +x ./Buildit
	./Buildit	（./Buildit i386 or ./Buildit x64编译出相应版本的可执行文件）
	cd ./x64  (or cd ./Release)


### 3、配置easydarwin.xml ###
EasyDarwin主要的几个配置项：

***rtsp_port***：EasyDarwin RTSP服务监听的端口；

***movie_folder***：流媒体文件本地存储的路径，包括点播mp4文件、直播切片生成的hls（m3u8+ts）文件；

***http\_service\_port***：RESTful服务端口；

***hls\_output\_enabled***：配置QTSSReflectorModule在接收推送的同时，是否同步输出hls流；

***HTTP\_ROOT\_DIR***：配置EasyHLSModule的对外WEB路径，用于hls分发的web服务器路径；

***local\_ip\_address***：配置EasyRelayModule对外服务的ip地址，因为可能会有多网卡或者内网映射，所以需要手动配置；

以Linux系统nginx做WEB服务器为例，比如我们将点播文件存储在/EasyDarwin/movies/目录，也就是

    <PREF NAME="movie_folder" >/EasyDarwin/movies/</PREF>

Nginx的WEB地址为：http://8.8.8.8/，那么我们配置：

    <PREF NAME="HTTP_ROOT_DIR" >http://8.8.8.8/</PREF>
这样就能够将EasyDarwin存储的HLS文件WEB发布到公网了，具体配置可以参考后面HLS直播配置章节。

### 4、运行EasyDarwin ###


*【前提】EasyDarwin可执行文件必须与/html/目录在同一层目录中* 
 

- Windows调试运行：

    EasyDarwin.exe -c ./easydarwin.xml -d  
> 注：需要把libEasyHLS.dll，libEasyPusher.dll，libEasyRTSPClient.dll，/html/文件夹拷贝到可执行程序的同目录下！

- Windows服务方式运行：

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

- Linux调试运行

    ./easydarwin -c ../WinNTSupport/easydarwin.xml  -d

- Linux后台服务方式运行

    ./easydarwin -c /etc/streaming/easydarwin.xml  &

> **注：无论是Windows还是Linux运行EasyDarwin，以Debug模式运行时，-c后面带的配置文件路径可以是相对路径也可以是绝对路径，但是以服务/后台方式运行，就必须是用绝对路径！**


### 5、检查EasyDarwin是否部署成功 ###

通过访问EasyDarwin RESTful接口可以初步判断EasyDarwin流媒体服务器是否已经运行起来了，例如我们可以访问：rtsp://[ip]:[http\_service\_port]/api/getrtsppushsessions 接口查看EasyDarwin是否运行响应，后面的版本我们会增加一个获取EasyDarwin整体运行配置信息RESTful接口，这样在外部就能查看EasyDarwin是否读取到了正确的用户配置；


## 调用方法 ##

### MP4点播 ###
我们将经过Hint处理过的mp4文件存在movie_folder目录中，访问RTSP地址：

    rtsp://[ip]:[rtsp_port]/[filename]
例如EasyDarwin服务器地址：8.8.8.8，rtsp_port配置为：8554，MP4文件名：demo.mp4，用vlc或者ffplay等播放器访问：rtsp://8.8.8.8:8554/demo.mp4；
>1. 经常会有用户发现自己的mp4文件无法点播，服务器端返回“415 Unsupported Media type”，这是因为MP4文件需要经过RTSP/RTP Hint处理才行，可以用MP4Box等工具进行一下Hint处理；
>2. EasyDarwin后续将移除点播模块（QTSSFileModule），HTTP/HLS点播将会比RTSP点播效果更好；

### 推模式转发 ###

直接通过标准RTSP/RTP推送流程（ANNOUNCE/SETUP/PLAY/RTP）向EasyDarwin推送音视频数据进行转发和分发，例如rtsp_port配置为8554，那我们可以直接用[**EasyDarwin EasyPusher**](https://github.com/EasyDarwin/EasyPusher "EasyPusher")或者**live555 DarwinInjector**向8554端口进行直播推送，推送后，我们可以通过  **rtsp://[ip]:[http\_service\_port]/api/getrtsppushsessions**  接口获取当前正在进行RTSP直播的列表；

### 拉模式转发 ###

调用接口（用vlc、ffplay、live555、EasyPlayer等RTSP Client调用）

    RTSP://[ip]:[rtsp_port]/EasyRelayModule?name=[relayName]&url="[RTSP_URL]"

例如EasyDarwin服务器IP地址是：8.8.8.8，RTSP端口(rtsp_port)：554，IPCamera的RTSP地址是：rtsp://admin:admin@192.168.66.189/22，那么我们可以：

1、配置easydarwin.xml EasyRelayModule

	<PREF NAME="local_ip_address" >8.8.8.8</PREF>

2、请求转发：RTSP://8.8.8.8:554/EasyRelayModule?name=live&url="rtsp://admin:admin@192.168.66.189/22"   （**name是定义一个拉模式转发流的唯一标识，不允许重复**）

3、直播URL：RTSP://8.8.8.8:554/EasyRelayModule?name=live

4、请求停止转发：RTSP://8.8.8.8:554/EasyRelayModule?name=live&cmd=stop  （**cmd=stop表示停止拉模式转发**）

## EasyDarwin HLS直播配置 ##

### 1、安装配套HTTP服务器 ###
  
- Windows环境（Windows下可以使用IIS或者Nginx，或者其他的HTTP服务器，Windows下以IIS为例）

1. 新建一个站点，指向EasyDarwin的Movies目录（<font color="red">注意这里Movies文件夹中应有crossdomain.xml配置文件</font>） 

![EasyDarwin](http://www.easydarwin.org/github/images/easydarwin/1.png)  

2. 在MIME中添加.m3u8和.ts文件类型 

![EasyDarwin](http://www.easydarwin.org/github/images/easydarwin/2.png)
![EasyDarwin](http://www.easydarwin.org/github/images/easydarwin/3.png)


> 注：ts与.m3u8的添加类似，这里不再截图。  

- Linux环境（Linux下可以使用Apache或者Nginx等等，建议使用Nginx比较简单，Linux下以Nginx为例）

1. 安装Nginx  

<pre>
apt-get install nginx 
</pre>

安装完成后，在浏览器输入127.0.0.1，验证是否安装成功，如下图，则安装成功  
![EasyDarwin](http://www.easydarwin.org/github/images/easydarwin/4.png) 
> 
  
2. 配置Nginx默认路径  
找到Nginx默认配置文件，获取并记录web根目录位置，例如这里位置是：**/var/www/html/**  
![EasyDarwin](http://www.easydarwin.org/github/images/easydarwin/5.png)  

3. 配置允许跨域访问

<font color="red">将EasyDarwin开源项目Movies目录中的crossdomain.xml文件放到nginx的web目录</font>，例如这里就放到**/var/www/html/**
  
4. 重启服务(如果修改了nginx配置)

<pre>
/usr/sbin/nginx  -s reload 
</pre>
注：Nginx配置方式在不同版本不同的系统中可能略有差异！

### 2、拉模式HLS直播配置 ###

EasyDariwn项目中集成了Mongoose服务器，可以通过Web配置EasyDarwin相关参数。

1、登录Web配置

在浏览器中输入如下地址，打开配置页面（用户名和密码都是admin）：   

    http://[ip]:8088

![EasyDarwin](http://www.easydarwin.org/github/images/easydarwin/6.png) 

2、HLS直播配置 
  
设置配置完成的HTTP服务器的地址，以及ts相关参数  
  
![EasyDarwin](http://www.easydarwin.org/github/images/easydarwin/7.png) 
  

3、流媒体文件目录配置
  
点击系统中的基本设置，设置流媒体文件的目录，以Linux系统nginx做WEB服务器为例，比如我们将点播文件存储在/var/www/html/目录
  
![EasyDarwin](http://www.easydarwin.org/github/images/easydarwin/9.png) 

> 注： <font color="red">2，3步配置完成后，需要重启EasyDarwin服务器，Debug方式运行需要手动重启服务器，以服务/后台方式运行，可以直接使用Web端中的重启功能。</font>
  
4、增加HLS直播  

设置拉取的网络设备的RTSP地址，如：rtsp://admin:admin@10.0.0.3/，点击HLS直播列表增加一个直播
  
![EasyDarwin](http://www.easydarwin.org/github/images/easydarwin/8.png)   

设置完成后，在HLS直播列表中会出现我们刚刚设置好的直播项，点击Play可以直接进行HLS直播
  
![EasyDarwin](http://www.easydarwin.org/github/images/easydarwin/10.png) 
  
### 3、推模式HLS直播配置 ###
  
HLS直播配置，流媒体文件目录配置和拉模式HLS直播配置中的一样
  
1、是否同步输出HLS设置  
  
点击RTSP直播配置，勾选上“是否同步输出HLS”的选项，点击保存，然后重启服务器
![EasyDarwin](http://www.easydarwin.org/github/images/easydarwin/11.png) 
  
2、推送数据
  
可以使用EasyPusher推送RTSP数据到EasyDarwin，具体使用方法，请参考[http://doc.easydarwin.org/EasyPusher/README/](http://doc.easydarwin.org/EasyPusher/README/)  

3、自动生成HLS直播项，点击播放
  
![EasyDarwin](http://www.easydarwin.org/github/images/easydarwin/12.png)

## FAQ ##

### 1. EasyDarwin返回401 Unauthorized ###

在easydarwin.xml中配置的Movies目录里面放一个名为**qtaccess**的文件，内容为：

	<Limit WRITE>  
	require any-user  
	</Limit>
	<Limit READ>
	require any-user
	</Limit>


### 2. RTSP推送到EasyDarwin出现404错误 ###
	
方法一：用EasyPusher做直播推送；

方法二：修改RTSP ANNOUNCE里面的sdp信息，把 "o=- %u %u IN IP4 %s/r/n" 和 "c=IN IP4 %s/r/n"里面的ip地址改成127.0.0.1就可以了;

方法三：我们将EasyDarwin部署到公网，当服务器置身内网，用端口映射的方式对外提供服务，在接收RTSP/RTP推送的时候，经常会出现在SETUP步骤Darwin返回404错误，经过查找原因，主要是EasyDarwin对推送的sdp信息中的IP地址不能识别，服务器并不知道自己已经置身于公网的地址：

**ifconfig查看地址信息**

![EasyDarwin](http://www.easydarwin.org/d/file/article/doc/EasyDarwin/9d86ccd17b3d38b84c27ed1008a9bf38.jpg)

**ip addr查看eth0地址列表**

![EasyDarwin](http://www.easydarwin.org/d/file/article/doc/EasyDarwin/45d612cba5ddc67617e734f532df78c7.jpg)

我们通过命令：ip addr add dev eth0 [公网IP]，向eth0添加一个公网地址就解决问题了：

![](http://www.easydarwin.org/d/file/article/doc/EasyDarwin/267035e801fadf06ea5f6acb5486a988.jpg)

Windows添加公网地址的方法：

![EasyDarwin](http://www.easydarwin.org/d/file/article/doc/EasyDarwin/8604f16ba3eb39ba2dd4e32a63f1fcf1.jpg)

![EasyDarwin](http://www.easydarwin.org/d/file/article/doc/EasyDarwin/8604f16ba3eb39ba2dd4e32a63f1fcf1.jpg)

### 3. EasyDarwin做转发延时10几秒? ###
	
在EasyDarwin QTSSReflectorModule转发模块中，有一个控制转发Buffer时间的配置**reflector_buffer_size_sec**，我们将这个配置改成0，也就是在服务器端不做缓存，直接转发，这样在网络条件充足的情况下对比转发和实时流，转发带来的延时也几乎可以忽略了：


### 4. 看不到直播或者点播视频，如何排查? ###

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


### 5. mp4点播返回415错误(Requested movie hasn't been hinted) ###

MP4文件需要先经过RTSP/RTP Hint处理，处理工具可以选择MP4Box或者MediaCoder等；

### 6. /bin/sh^M: bad interpreter: No such file or directory ###

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

WEB：[www.EasyDarwin.org](http://www.easydarwin.org)

QQ交流群：288214068

Copyright &copy; EasyDarwin.org 2012-2016

![EasyDarwin](http://www.easydarwin.org/skin/easydarwin/images/wx_qrcode.jpg)

