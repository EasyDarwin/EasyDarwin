---本文由EasyDarwin开源团队成员Babosa贡献

## 前言 ##
经常有人提到最近比较火的映客直播、花椒直播这种，是如何做到在打开手机直播中的某一个主播房间后，立即就能显示出主播视频，非常非常快，而且延时也比较小，是怎么做到的？

其实，这并不是什么高深的技术，就是最基本的关键帧索引/推送，在用户打开直播，请求直播流的时候，服务器将缓存中最新的关键帧开始推送给客户端，那么客户端收到关键帧就能够立即解码显示，一个720p的关键帧，一般的十几k，即使在网速不好的情况下，也是秒秒钟内就能下载完成，下载完成就能立即显示，这样就是快速第一时间出画面的效果了；


## 服务器端处理 ##
我们在EasyDarwin服务端将接收到的推送端推送的数据都以队列的形式进行缓冲，我们在缓冲的同时，对帧进行过滤，区分音/视频RTP包和I/P视频帧，我们每次都将最新的I帧位置在Queue中进行标记，这样在每一个直播推送分发的时候，我们都从最新的I关键帧开始推送，这样既保证了能够第一时间显示画面，又能够非常低延时低进行直播分发，具体算法如图所示：

![EasyDarwin关键帧索引](http://www.easydarwin.org/github/images/easydarwin/doc/keyframerelay/20160426000016728.jpg)

## 音频处理 ##
同样，当我们在缓冲区过滤检测到最新的视频关键帧推送的时候，立即进行标志位置位，置位后，第一个到达的音频RTP包就将作为音频的关键帧被记录，当有新的客户端加入到分发列表中的时候，我们就以当前音频帧为索引，第一个发送，这样就能够最大程度低保证音视频到达客户端是基本同步了；这里我们也考虑过记录视频关键帧的时间戳，然后再在音频缓冲队列中查找最接近的音频帧，但这种方式复杂度高，对效率的影响太大，而且基于标志位的形式进行音频关键帧定位，已经能够非常接近时间戳同步了；

![EasyDarwin音视频关键帧索引推送](http://www.easydarwin.org/github/images/easydarwin/doc/keyframerelay/20160430162516365.jpg)


## 显示效果 ##
![视频秒开](http://www.easydarwin.org/github/images/easydarwin/doc/keyframerelay/20160426001707750.jpg)


## 版本及源码下载 ##
1. 流媒体服务器EasyDarwin：[https://github.com/EasyDarwin/EasyDarwin](https://github.com/EasyDarwin/EasyDarwin)
2. 手机直播推送端EasyPusher：[https://github.com/EasyDarwin/EasyPusher](https://github.com/EasyDarwin/EasyPusher) 
3. 手机直播客户端EasyClient：[https://github.com/EasyDarwin/EasyClient](https://github.com/EasyDarwin/EasyClient)


## 获取更多信息 ##

邮件：[support@easydarwin.org](mailto:support@easydarwin.org) 

WEB：[www.EasyDarwin.org](http://www.easydarwin.org)

Copyright &copy; EasyDarwin.org 2012-2016

![EasyDarwin](http://www.easydarwin.org/skin/easydarwin/images/wx_qrcode.jpg)