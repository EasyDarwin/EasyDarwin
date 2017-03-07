# 概述 #

EasyDarwin开源流媒体云平台具备EasyCamera（摄像机、Android手机App）、EasyNVR设备接入、直播、对讲、云台控制等多种功能，采用EasyClient（PC、Android、iOS）客户端接入到EasyDarwin云平台中；

## 平台架构 ##

![](http://www.easydarwin.org/github/images/cloud_framework.png)

## 平台所涉及的服务及端口 ##

- EasyCMS: 10000
- EasyDarwin: 10554/10008
- Web服务器: 10080
- Redis：6379

## 启动流程 ##

以Windows平台为例：

示例所用的目录结构：
	
	D:\
	D:\EasyDarwin\
	
	D:\EasyDarwin\nginx
	D:\EasyDarwin\nginx\www
	D:\EasyDarwin\nginx\www\snap
	
	D:\EasyDarwin\Redis
	D:\EasyDarwin\EasyCMS
	D:\EasyDarwin\EasyDarwin

示例所用的服务器公网IP：
	
>	121.40.50.44

用户根据实际所用到的云主机的IP和目录进行动态自定义调整！

### 1.redis ###
Windows平台：

>	运行redis目录中的start.bat即可，默认的redis连接密码已经配置为：EasyDSSEasyDarwinEasyCMSEasyCamera

### 2.nginx ###
Windows平台：

>	运行nginx.exe，其中conf/nginx.conf中已经配置WEB路径为当前路径下的www目录，记住此目录的位置，例如为 D:\EasyDarwin\nginx\www\

### 3.EasyCMS ###
Windows平台：

	配置easycms.xml：
	
	snap_local_path为： D:\EasyDarwin\nginx\www\snap\
	
	snap_web_path为： http://121.40.50.44:10080/snap/
		
	service_wan_ip为： 121.40.50.44
		
	service_wan_port为： 10000
	
	运行EasyCMS/start.bat

### 4.EasyDarwin ###
Windows平台：

	配置easydarwin.xml：

	rtsp_port为： 10554
	
	rtsp_wan_port为： 10554
	
	service_lan_port为： 10008
	
	service_wan_port为： 10008
	
	service_wan_ip为： 121.40.50.44
	
	运行EasyDarwin/start.bat


## 获取更多信息 ##

邮件：[support@easydarwin.org](mailto:support@easydarwin.org) 

WEB：[www.easydarwin.org](http://www.easydarwin.org)

Copyright &copy; EasyDarwin.org 2012-2017

![EasyDarwin](http://www.easydarwin.org/skin/easydarwin/images/wx_qrcode.jpg)