# EasyDarwin部署视频广场 #

----------

## 一、部署EasyCMS接入服务器 ##
### 1.1、获取EasyCMS最新版本 ###
在Github：https://github.com/EasyDarwin/EasyDarwin中获取最新的EasyCMS版本源码，自行编译成需要的可执行文件，也可以直接在https://github.com/EasyDarwin/EasyDarwin/releases 中下载已经归档的相应版本进行部署。

### 1.2、编译EasyCMS最新版本 ###
Windows版本编译：可以直接用Visual Studio 2008打开源码文件中的：/EasyDarwin-master/EasyCMS/WinNTSupport/EasyCMS.sln解决方案文件，编译出exe可执行文件EasyCMS.exe；
Linux版本编译：

cd ./EasyDarwin-master/EasyCMS/
chmod +x ./Buildit
./Buildit
（./Buildit i386 or ./Buildit x64编译出相应版本的可执行文件）
cd ./x64  (or cd ./Release)

### 1.3、配置easycms.xml ###
EasyCMS主要的几个配置项：

***cms_port***：EasyCMS主服务监听的端口；

***snap_local_path***：快照文件存储的本地文件系统路径；

***snap_web_path***：快照文件存储的目录对应的http网络路径；

以Linux系统nginx做WEB服务器为例，比如我们将快照文件存储在/EasyDarwin/snap/目录，也就是

    <PREF NAME="snap_local_path" >/EasyDarwin/snap/</PREF>
Nginx的WEB地址为：http://8.8.8.8/ ，且WEB根目录配置为/EasyDarwin/那么我们配置：

    <PREF NAME="snap_web_path" >http://8.8.8.8/snap/</PREF>
这样就能够将EasyCMS存储的快照文件发布到公网了。

### 1.4、运行EasyCMS ###
Windows版本运行(控制台调试运行)：
<pre>
EasyCMS.exe -c ./easycms.xml -d
</pre>
Windows服务方式运行：
我们提供一段脚本
<pre>
cd ./
set curPath="%cd%"
echo service path：%curPath%
sc create EasyCMS binPath= "%curPath%EasyCMS.exe -c %curPath%easycms.xml" start= auto
net start EasyCMS
pause
</pre>
将这段脚本做成bat，运行，我们就创建了一个叫做EasyCMS的Windows服务了，通过系统服务（services.msc）可以查看到。
注：Windows不同版本可能稍有差异，建议在命令行运行bat脚本，而不是直接双击运行，这样能看到具体出错原因！

Linux版本运行（具体配置文件路径根据实际情况设置）：
调试模式运行,

    ./easycms -c ../WinNTSupport/easycms.xml  -d
后台服务运行,

    ./easycms -c ../WinNTSupport/easycms.xml  &
注：如果xml配置文件路径不能确定，建议最保险的方式就是用全路径，例如 “/etc/streaming/EasyCMS/easycms.xml”，这样在下一次更新服务的时候，配置文件可以保留！

### 1.5、EasyCMS是否部署成功检查 ###
<PREF NAME="MSG_debug_printfs" TYPE="Bool16" >true</PREF>
将easycms.xml中输出打印配置为true，这样就能在调试模式中看是否有报文发来，也可以在easycms.xml中配置log输出的目录和文件名称，再根据log确定问题（具体个性化log，需要自己添加代码，编译部署）；

## 二、部署EasyDarwin流媒体服务器 ##

### 2.1、获取EasyDarwin最新版本 ###
在Github：https://github.com/EasyDarwin/EasyDarwin中获取最新的EasyDarwin版本源码，自行编译成需要的可执行文件，也可以直接在https://github.com/EasyDarwin/EasyDarwin/releases 中下载已经归档的相应版本进行部署。

### 2.2、编译EasyDarwin最新版本 ###
Windows版本编译：可以直接用Visual Studio 2008打开源码文件中的：/EasyDarwin-master/EasyDarwin/WinNTSupport/EasyDarwin.sln解决方案文件，编译出exe可执行文件EasyDarwin.exe；
Linux版本编译：

<pre>
cd ./EasyDarwin-master/EasyDarwin/
chmod +x ./Buildit
./Buildit
（./Buildit i386 or ./Buildit x64编译出相应版本的可执行文件）
cd ./x64  (or cd ./Release)
</pre>

### 2.3、配置easydarwin.xml ###
EasyDarwin主要的几个配置项：

***rtsp_port***：EasyDarwin RTSP服务监听的端口；

***movie_folder***：媒体文件存储的路径，包括点播文件、直播切片生成的hls文件；

***http_service_port***：Webservice服务端口；

***hls_output_enabled***：配置QTSSReflectorModule在接收推送的同时，是否同步输出hls；

***HTTP_ROOT_DIR***：配置EasyHLSModule的对外WEB路径；

***local_ip_address***：配置EasyRelayModule对外服务的ip地址，因为可能会有多网卡或者内网映射，所以需要手动配置；

以Linux系统nginx做WEB服务器为例，比如我们将点播文件存储在/EasyDarwin/movies/目录，也就是

    <PREF NAME="movie_folder" >/EasyDarwin/movies/</PREF>
Nginx的WEB地址为：http://8.8.8.8/，且WEB根目录配置为/EasyDarwin/那么我们配置：

    <PREF NAME="HTTP_ROOT_DIR" >http://8.8.8.8/movies/</PREF>
这样就能够将EasyDarwin存储的HLS文件WEB发布到公网了。

### 2.4、运行EasyDarwin ###
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

### 2.5、EasyDarwin是否部署成功检查 ###
<PREF NAME="RTSP_debug_printfs" TYPE="Bool16" >true</PREF>
将easydarwin.xml中输出打印配置为true，这样就能在调试模式中看是否有报文发来，也可以在easydarwin.xml中配置log输出的目录和文件名称，再根据log确定问题（具体个性化log，需要自己添加代码，编译部署）；

## 三、部署EasyCamera设备端 ##

### 3.1、获取EasyCamera最新版本(目前仅维护Windows/arm摄像机版本) ###

在Github：https://github.com/EasyDarwin/EasyCamera 中获取最新的EasyCamera版本源码，自行编译成需要的可执行文件，也可以直接在https://github.com/EasyDarwin/EasyCamera/releases 中下载已经归档的相应版本进行部署。

### 3.2、编译EasyCamera最新版本 ###
Windows版本编译：可以直接用Visual Studio 2008打开源码文件中的：/EasyCamera-master/src/WinNTSupport/EasyCamera.sln解决方案文件，编译出exe可执行文件EasyCamera.exe；
arm版本编译：
这里只说明EasyDarwin开源摄像机的编译方法,其他类型摄像机编译方法类似, 前提是配置EasyCamera交叉编译工具链，具体安装交叉编译工具链参考：

> “
> 交叉编译工具链为所提供的EasyCamera-master\SDK\8126交叉编译工具\crosstool.tgz文件，解压crosstool.tgz至Linux开发宿主机的/opt目录，在/etc/profile里设置将交叉编译工具链目录设置到PATH变量，重启完成安装。
> 解压命令：tar zxvf crosstool.tgz -C /opt
> 在/etc/profile添加:
> export PATH="$PATH:/opt/crosstool/arm-none-linux-gnueabi-4.4.0_ARMv5TE/bin"
> “

编译方法,

    cd ./EasyCamera-master/src/
    chmod +x ./Buildit
    ./Buildit
    cd ./Bin


### 3.3、配置easycamera.xml ###
EasyCamera主要的几个配置项：

***cms_addr***：EasyCMS服务的IP地址或者域名；

***cms_port***：EasyCMS服务的监听端口；

***local_camera_addr***：本地摄像机地址，例如EasyCamera Windows版本是与硬件分离的，那么具体配置摄像机的ip地址，arm版本EasyCamera内置于摄像机内部，可以直接配置成127.0.0.1；

***local_camera_port***：摄像机监听端口，默认80，也可以在摄像机中重新配置，具体方法参考http://www.easydarwin.org/article/doc/EasyCamera/32.html ；

***serial_number***：自定义配置的摄像机序列号，12位字母与数字组合；

***run_user_name***：摄像机用户名，默认admin;

***run_password***：摄像机密码，默认admin；

***camera_stream_type***：默认摄像机传输的码流类型，0表示子码流，1表示主码流；

### 3.4、运行EasyCamera ###
Windows版本运行(控制台调试运行)：

    EasyCamera.exe -c ./easycamera.xml -d


摄像机内运行：
首先是将arm程序如何放入摄像机内部，方法见：http://www.easydarwin.org/article/doc/EasyCamera/32.html#5 
调试模式运行（具体配置文件路径根据实际情况设置）,

    ./easycamera -c ./easycamera.xml  -d
后台服务运行,

    ./easycamera -c ./easycamera.xml  &
注：如果xml配置文件路径不能确定，建议最保险的方式就是用全路径，例如 “/mnt/mtd/easydarwin/easycamera.xml”，这样在下一次更新服务的时候，配置文件可以保留！

### 3.5、检查EasyCamera是否运行成功 ###
可以通过EasyCamera -d调试模式，查看是否配置成功，也可以到EasyCMS查看设备是否上线；

## 四、细节说明 ##
1.	1111111
未完待续。
2.	2222222
未完待续。



