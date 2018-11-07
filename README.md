# EasyDarwin

EasyDarwin Open Source Media Server

## 准备工具

- node 系

        npm i -g rimraf

- go 系

        go get -u github.com/kardianos/govendor
        go get -u github.com/caixw/gobuild

## 编译命令

- 获取代码

        cd $GOPATH/src/github.com
        git clone https://github.com/easydarwin/EasyDarwin.git EasyDarwin
        cd EasyDarwin

- 以开发模式运行 server

        npm run dev

- 以开发模式运行 www

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

### push

ffmpeg -re -i C:\Users\Administrator\Videos\test.mkv -rtsp_transport tcp -vcodec h264 -f rtsp rtsp://localhost/test

ffmpeg -re -i C:\Users\Administrator\Videos\test.mkv -rtsp_transport udp -vcodec h264 -f rtsp rtsp://localhost/test

### play

ffplay -rtsp_transport tcp rtsp://localhost/test

ffplay rtsp://localhost/test