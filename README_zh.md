[English](./README.md) | [简体中文](./README_zh.md)

# EasyDarwin

[EasyDarwin](http://www.easydarwin.com/) 是一款开源、简单、高效的流媒体服务器，支持 RTMP/RTSP 的推流和拉流，支持分发 RTMP/RTSP/HLS/HTTP-FLV/WebSocket-FLV/WebRTC 等协议, 可编译支持 Linux/Windows/macOS 操作系统, 支持 X86_64/ARMv7/AARCH64/M1/RISCV/LOONGARCH/MIPS 各种架构。

EasyDarwin8.x 基于原 [EasyDarwin](http://www.easydarwin.com/) 软件基础上结合 [lalmax](https://github.com/q191201771/lalmax) 项目二次开发。

EasyDarwin 采用 MIT 协议授权.

![示例](images/main.png)

## 功能特性

+ 集成web界面
+ 视频预览
+ 支持按需播放，无人观看自动断流，节省流量
+ 支持输出多种协议(RTMP/RTSP/HLS/HTTP-FLV/WebSocket-FLV/WebRTC)
+ 支持通过一个流地址直接观看摄像头，无需登录以及调用任何接口
+ 协议支持播放H264和H265
+ 支持拉RTSP流，分发各种流协议
+ 支持推流鉴权
+ 离线在线监控
+ RESTful API，apidoc文档工具(在web目录下)

### 待增加功能
+ 点播功能

## 使用
目前仅支持源码编译生成，后续支持一键安装包。请参看部署章节进行源码构建后使用。

## 目录

```text
├── cmd	                    可执行程序
│   └── server
├── configs                 配置文件
├── internal                私有业务
│   ├── conf                配置模型
│   ├── core                业务领域
│   ├── data                数据库及主要配置文件
│   └── web
│       └── api             RESTful API
├── pkg                     依赖库
├── utils                   工具
└── web                     前端
```

## 部署
### 源码构建
前提条件
+ Go 1.23.0安装
+ Go中bin的安装目录需要添加到系统环境变量

然后下载
```shell
git clone https://github.com/EasyDarwin/EasyDarwin.git
cd EasyDarwin
go mod tidy
```
### Windows 版本构建

Windows 系统使用 Makefile 时，请使用 `git bash` 终端，并提前安装好 mingw。
```shell
mingw32-make.exe build/windows
cd build
cd EasyDarwin-win-"版本号"-"编译时间"
EasyDarwin.exe
```
### Linux
```shell
make build/linux
cd build
cd EasyDarwin-lin-"版本号"-"编译时间"
easydarwin
```

## 入门指南

打开 [http://localhost:10086](http://localhost:10086) ,并添加流协议。

1. **rtmp推流**

   _在添加推流协议时，需要查看推流实际地址，如下地址只是实例。_

   然后使用以下 [ffmpeg](https://ffmpeg.org/download.html) 命令流式传输
    ```shell
    ffmpeg -re -i ./video.flv -c copy -f flv -y rtmp://localhost:21935/live/stream_1?sign=5F9ZgWP6fN
    ```

   或者，使用以下配置通过 [OBS Studio](https://obsproject.com/download) 流式传输
    + 服务：`自定义`
    + 服务器：`rtmp://localhost:21935/live/`
    + 推流码：`stream_1?sign=5F9ZgWP6fN`

2. **rtsp拉流**

   _在添加拉流协议，需要更具摄像机的rtsp地址输入。_

   如使用海康rtsp地址格式如下：
    ```text
    rtsp://username:password@host:port/Streaming/Channels/101
   ```

   或者，使用大华rtsp地址格式如下：
    ```text
    rtsp://username:password@ip:port/cam/realmonitor?channel=1&subtype=0
   ```

## 自定义配置

默认配置目录为可执行文件同目录下的 configs下`config.toml`

### 端口
// TODO

## 项目主要依赖

+ lalmax
+ gin
+ gorm
+ slog / zap
+ lal
+ sqlite
+ pion