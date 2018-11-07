# EasyDarwin开源流媒体服务器

![snapshot](http://ww1.sinaimg.cn/large/79414a05ly1fwzq9dirmij212u0ipwg9.jpg)
 
## 主要功能特点

- 基于Golang开发维护

- 支持windows、linux平台

- 接收RTSP流推送

- RTSP流分发

- 关键帧缓存

- 秒开画面

- Web后台管理

- 分布式负载均衡


## 安装部署

- [下载解压 release 包](https://github.com/EasyDarwin/EasyDarwin/releases)

- 直接运行(Windows)

    EasyDarwin.exe
    
    以 `Ctrl + C` 停止服务

- 以服务启动(Windows)

    ServiceInstall-EasyDarwin.exe
    
    以 ServiceUninstall-EasyDarwin.exe 卸载 EasyDarwin 服务

- 直接运行(Linux)

		cd EasyDarwin
		./easydarwin
		# Ctrl + C

- 以服务启动(Linux)

		cd EasyDarwin
		./start.sh
		# ./stop.sh

- 查看界面
	
	打开浏览器输入 [http://localhost:10008](http://localhost:10008), 进入控制页面,默认用户名密码是admin/admin

- 测试推流

	ffmpeg -re -i C:\Users\Administrator\Videos\test.mkv -rtsp_transport tcp -vcodec h264 -f rtsp rtsp://localhost/test

	ffmpeg -re -i C:\Users\Administrator\Videos\test.mkv -rtsp_transport udp -vcodec h264 -f rtsp rtsp://localhost/test
			

- 测试播放

	ffplay -rtsp_transport tcp rtsp://localhost/test

	ffplay rtsp://localhost/test 


## 手机推流

可以使用EasyPusher测试手机推流,[下载地址](https://github.com/EasyDSS/EasyPusher)

推流URL规则: rtsp://{ip}:{port}/{id} ， 例如 : rtsp://www.easydarwin.org:554/your_stream_id

EasyPusher参数设置如下

![snapshot](http://ww1.sinaimg.cn/large/79414a05ly1fwzqe8oyjxj20u01hcafw.jpg)

可使用vlc播放器、[EasyScreenLive](https://github.com/EasyDSS/EasyScreenLive)、[EasyPlayer-RTSP](https://github.com/EasyDSS/EasyPlayer-RTSP-Win/releases)、[EasyPlayerPro](https://github.com/EasyDSS/EasyPlayerPro-Win)测试播放

## 效果图

![snapshot](http://ww1.sinaimg.cn/large/79414a05ly1fwzqdbi8efj20w00mrn0c.jpg)

## 二次开发

### 准备工具

- node 系

        npm i -g rimraf
        npm i -g @penggy/pack

- go 系

        go get -u github.com/kardianos/govendor
        go get -u github.com/caixw/gobuild

### 编译命令

- 获取代码

        cd $GOPATH/src/github.com
        git clone https://github.com/easydarwin/EasyDarwin.git EasyDarwin
        cd EasyDarwin

- 以开发模式运行 server

        npm run dev

- 以开发模式运行前端

        npm run dev:www       

- 编译前端

        cd web_src && npm i
        cd ..
        npm run build:www

- 编译 Windows 版本

        npm run build:win

- 编译 Linux 版本 (在 bash 环境下执行)

        npm run build:lin       

- 清理编译文件

        npm run clean 

- 打包

        # for windows
        npm run build:win
        pack zip

        # for linux 
        npm run build:lin
        pack tar

        # for clean
        pack clean

## 下一步开发计划

- 系统运行信息统计：CPU、内存、分发流量、累积运行时间等；

- 拉模式转发/分发；

- 服务端录像；


## 技术支持

- 邮件：[support@easydarwin.org](mailto:support@easydarwin.org) 

- QQ交流群：**436297092**

- EasyDarwin开源流媒体服务器是属于EasyDarwin开源团队的免费产品，大家免费使用，同时，EasyDarwin开源团队也能提供相应的收费技术咨询、技术服务和技术定制，谢谢大家支持！


## 获取更多信息

**EasyDarwin**开源项目：[www.EasyDarwin.org](http://www.easydarwin.org)

Copyright &copy; EasyDarwin Team 2012-2018

![EasyDarwin](http://www.easydarwin.org/skin/easydarwin/images/wx_qrcode.jpg)