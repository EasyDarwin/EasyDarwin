**本文来自EasyDarwin团队Fantasy**

## EasyDarwin网络模型介绍 ##
 
EventContext负责监听所有网络读写事件，EventContext::RequestEvent每次插入一个监听事件到监听列表（select 文件描述符集合），EventThread::Entry()死循环监听添加到该FD_SET的所有文件描述符的事件。**Entry()->select_waitevent()**每次返回下一个要处理的事件节点，包括事件处理对象的哈希key,以及事件类型。

然后投递到线程池中的某一个线程的任务队列，注意这里是每一个线程维护自己的任务队列。相关代码，**ProcessEvent->Signal()**，其中实现了一个简单的均衡算法，决定投递到哪一个线程的任务队列。EventContext每处理完一个事件，会删掉监听的文件描述符，然后执行完后会再次调用RequestEvent()插入到监听列表，并且激活监听(往管道写数据)。**TaskThread::Entry()**负责处理上面投递过来的任务，执行虚方法Run()，相关代码，**theTimeout = theTask->Run()**；所有网络模块均会继承并实现Task类的Run()方法。

## select和epoll的差别 ##

select内核实现，

	sys_select()->do_select()
	{
		for(;;)
		{
			循环遍历FD_SET看是否有事件发生，
		}
	}
	
select最大只支持1024个文件描述符，原因#define __FD_SETSIZE 1024，定义超过1024会造成处理性能上的下降；epoll内核实现，

> 网络事件中断中调用ep_poll_callback()，将网络事件添加到epoll队列。这样，epoll_wait()等待的时候，就不会像select循环，因为队列中的每一个事件都是有效的。
	
由此看来epoll比select效率高，而且队列不受限制，可以任意大小。但是如果select命中到有效的时间的概率也很高的时候，它俩效率是差不多的。

## 把EasyDarwin的网络模型换成epoll ##

实现过程比较简单，按照前面select提供的接口，挨个实现一遍，然后把调用的地方加以操作系统类型的宏开关，兼容windows的编译。

实现之后，调试却花了很长时间，刚开始发现线程池在插入监听事件的时候会出现资源竞争，程序运行了一段时间后就出现异常了，不读取推送的数据了。经过review代码发现应该是没有加锁导致插入的时候资源竞争，有的没有成功插入，所以监听不到事件。后面加锁后经过调试，避免了一些死锁问题后，稳定运行了几天没有出现问题。

经过春节前后将近2个月的开发和稳定调试、测试，EasyDarwin开源流媒体服务器终于成功将底层select网络模型修改优化成epoll网络模型，将EasyDarwin流媒体服务器在网络处理的效率上提升到了另一个档次（这里得感谢EasyDarwin开源团队成员Fantasy的无私贡献，是他牺牲宝贵的业余休息时间，连夜奋战开发和调试，才能取得EasyDarwin底层改造的成果）。

众所周知，select模型在处理大并发量的网络请求上具有一些瓶颈，默认在Linux上同时能够处理的网络连接数FD_SETSIZE为1024，虽然可以通过修改FD_SETSIZE的定义大小，但在FD_SETSIZE大于1024时，由于select模型先天的原因，对网络事件无法做到及时准确定位，其处理性能上并没有得到同步的提升，所以，带来的就是整个流媒体服务器在并发量上的瓶颈；而修改成epoll网络模型之后，其网络事件的处理性能得到提升，再加上EasyDarwin架构上的优势，采用线程池，任务队列及Reactor技术，使得网络事件能够非常高效地被感知、处理（这里的任务队列还涉及到一个无锁队列的优化，这个在后续的博客中会具体分析），这样，使得整个EasyDarwin流媒体服务器具有非常高效的事件处理能力，而且经过长时间的测试，稳定性非常好！

需要说明的是，EasyDarwin在Windows端还是沿用的select网络模型，从目前的情况上来看，大并发量需求的项目多数部署在Linux系统上，Windows上EasyDarwin作为小规模或者研究型项目，完全能够满足需求，所以，将EasyDarwin+IOCP列入以后的开发计划中；

Epoll模型主要的代码在EasyDarwin Github上，目录位置在：https://github.com/EasyDarwin/EasyDarwin/tree/master/CommonUtilitiesLib，主要文件是：epollEvent.h和epollEvent.cpp

EasyDarwin开源流媒体服务器项目还在一直进行更加高性能的优化，项目地址：[https://github.com/EasyDarwin](https://github.com/EasyDarwin)

## 获取更多信息 ##

邮件：[support@easydarwin.org](mailto:support@easydarwin.org) 

WEB：[www.EasyDarwin.org](http://www.easydarwin.org)

Copyright &copy; EasyDarwin.org 2012-2016

![EasyDarwin](http://www.easydarwin.org/skin/easydarwin/images/wx_qrcode.jpg)


	