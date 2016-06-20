---本文由EasyDarwin开源团队成员Babosa贡献

## 前言 ##

前一段时间，我们为EasyDarwin实现了客户端快速显示画面/听到同步声音的缓存关键帧检索方案，具体的实现方法分别在https://github.com/EasyDarwin/EasyDarwin/blob/master/Doc/keyframe%20relay%20optimization.md中可以了解到具体的实现原理，实现的方法代码也可以在EasyDarwin的ReflectorStream类中找到：
![EasyDarwin关键帧索引](http://www.easydarwin.org/github/images/easydarwin/doc/keyframerelay/20160426000016728.jpg)

## 问题需求 ##

经过了一段时间的测试和项目检验，按照上述描述的流程，能够很好地实现快速显示功能，但转发过程中，还会出现随着转发时间的不断累积，转发的延时会越来越大，而通过我们的测试发现，这个延时主要是存在于长时间观看的客户端，而且新加入的客户端延时会很小，但过一段时间，延时就开始累积，那么我们就来进行一下问题的分析：
![EasyDarwin低延时转发](http://www.easydarwin.org/github/images/easydarwin/doc/lowlatency/201606210030.png)

由上图的分析我们可以看出，由于EasyDarwin转发到不同的客户端，客户端所在的网络环境不一样，发送的速度也就各不相同，一开始发送的开始位置P1的时候延时最小，最接近实时场景，但当客户端网络较差的时候，数据包发送就缓慢，那么这个过程中累积的数据就会越来越多，造成P2缓慢推进，而最新关键帧数据会不断更新P3，那么P2-P3的这个累积，就是主要延时所在；

## 问题解决 ##

于是，我们定义了一个**P2**与**P3**之间的最大距离阀值，当**P2** - **P3**超过了这个时间阀值，我们就发送的索引位置由**P2**跳动到**P3**，或者称这个过程为缓存跟进，由于是从当前发送帧（无论是I帧或者P帧）跳到最新的I关键帧，所以客户端不会出现花屏，只会出现画面跳动，**P2**与**P3**之间的最大距离阀值我们定义的越大，画面跳跃性可能就会越大，当然，阀值越大，出现跳跃的次数可能会越少，所以，我们需要定一个自己需要的阀值大小；

**此算法在EasyDarwin源码中的实现位置：ReflectorStream.cpp中的ReflectorSender::NeedRelocateBookMark方法**；

## 版本及源码下载 ##
1. 流媒体服务器EasyDarwin：[https://github.com/EasyDarwin/EasyDarwin](https://github.com/EasyDarwin/EasyDarwin)
2. EasyCamera手机直播/移动单兵监控端v1.0.16.0620：[https://github.com/EasyDarwin/EasyCamera/releases/tag/v1.0.16.0620](https://github.com/EasyDarwin/EasyCamera/releases/tag/v1.0.16.0620) 
3. 手机直播客户端EasyClient：[https://github.com/EasyDarwin/EasyClient](https://github.com/EasyDarwin/EasyClient)


## 获取更多信息 ##

邮件：[support@easydarwin.org](mailto:support@easydarwin.org) 

WEB：[www.EasyDarwin.org](http://www.easydarwin.org)

Copyright &copy; EasyDarwin.org 2012-2016

![EasyDarwin](http://www.easydarwin.org/skin/easydarwin/images/wx_qrcode.jpg)