# EasyDarwin

[EasyDarwin](http://www.easydarwin.com/) is an open-source, simple, and efficient streaming media server that supports RTMP/RTSP push and pull streams. It also supports distributing streams via RTMP/RTSP/HLS/HTTP-FLV/WebSocket-FLV/WebRTC protocols. EasyDarwin can be compiled to support Linux/Windows/macOS operating systems and various architectures including X86_64, ARMv7, AARCH64, M1, RISCV, LOONGARCH, MIPS.

EasyDarwin 8.x is a secondary development based on the original [EasyDarwin](http://www.easydarwin.com/) software combined with the [lalmax](https://github.com/q191201771/lalmax) project.

EasyDarwin is licensed under the MIT License.

![example](images/main.png)

## Features

+ Integrated web interface
+ Video preview
+ Supports on-demand playback; automatically disconnects when no viewers are present to save bandwidth
+ Supports outputting multiple protocols (RTMP/RTSP/HLS/HTTP-FLV/WebSocket-FLV/WebRTC)
+ Allows direct viewing of camera feeds through a single stream URL without requiring login or API calls
+ Protocol supports playing H264 and H265
+ Supports pulling RTSP streams and redistributing them via various protocols
+ Supports push stream authentication
+ Offline and online monitoring
+ RESTful API with apidoc documentation tool (located in the web directory)

### Features to be Added
+ Video on demand functionality

## Usage
Currently, only source code compilation is supported for generation; one-click installation packages will be supported later. Please refer to the deployment section for instructions on building from source code before use.

## Directory Structure

```text
├── cmd	                    Executable programs
│   └── server
├── configs                 Configuration files
├── internal                Private business logic
│   ├── conf                Configuration models
│   ├── core                Business domain
│   ├── data                Database and main configuration files
│   └── web
│       └── api             RESTful API
├── pkg                     Dependency libraries
├── utils                   Utilities
└── web                     Frontend
```

## Deployment
### Building from Source Code
Prerequisites:
+ Go 1.23.0 installed
+ The Go bin directory must be added to the system environment variables

Then download:
```shell
git clone https://github.com/EasyDarwin/EasyDarwin.git
cd EasyDarwin
go mod tidy
```
### Building on Windows

When using Makefile on Windows, please use the `git bash` terminal and ensure Mingw is installed.
```shell
mingw32-make.exe build/windows
cd build
cd EasyDarwin-win-"version"-"build-time"
EasyDarwin.exe
```
### Building on Linux
```shell
make build/linux
cd build
cd EasyDarwin-lin-"version"-"build-time"
easydarwin
```

## Getting Started Guide

Open [http://localhost:10086](http://localhost:10086) and add the streaming protocol.

1. **RTMP Push Stream**

   _When adding a push stream protocol, you need to check the actual push stream address, the following address is just an example._

   Then use the following [ffmpeg](https://ffmpeg.org/download.html) command to stream:
    ```shell
    ffmpeg -re -i ./video.flv -c copy -f flv -y rtmp://localhost:21935/live/stream_1?sign=5F9ZgWP6fN
    ```

   Or, use the following configuration to stream through [OBS Studio](https://obsproject.com/download):
    + Service: `Custom`
    + Server: `rtmp://localhost:21935/live/`
    + Stream Key: `stream_1?sign=5F9ZgWP6fN`

2. **RTSP Pull Stream**

   _When adding a pull stream protocol, you need to input the specific RTSP address of your camera._

   For example, using Hikvision RTSP address format:
    ```text
    rtsp://username:password@host:port/Streaming/Channels/101
   ```

   Or Dahua RTSP address format:
    ```text
    rtsp://username:password@ip:port/cam/realmonitor?channel=1&subtype=0
   ```

## Custom Configuration

The default configuration directory is `config.toml` located in the same directory as the executable file.

### Ports
// TODO

## Project Dependencies

+ lalmax
+ gin
+ gorm
+ slog / zap
+ lal
+ sqlite
+ pion