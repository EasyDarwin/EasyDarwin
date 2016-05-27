---本文由EasyDarwin开源团队成员Fantasy贡献

## 前言 ##
最近在linux上跑EasyDarwin发现一个很奇怪的问题，当有RTSPSession连接上来的时候，发现进程的虚拟内存映射一下就多了64M，如下图:

![Memory Map](http://www.easydarwin.org/github/images/easydarwin/doc/memorymanage/20160527233622.png)

> 备注：anon标识堆内存

## 过程 ##
把通过在代码里面加system(“pmap pid”)命令，一步步跟，最终确定到是在NEW RTSPSession的时候多出来的64M内存，反复review代码，发现RTSPSession类并没有申请这么大的堆内存，把整个类大小输出，也远没有这么大。表示很奇怪。

决定写一些简单的类，一个个继承RTSPSession继承过的那些类，然后在NEW RTSPSession前面new一个对象，发现就会在NEW RTSPSession之前就多出来64M内存，果真是继承的类有申请大块内存？再次review，还是没有发现。

不继承任何类，new一个对象试试，结果还是多出来了。

查阅资料，发现是glibc 的malloc捣的鬼，glibc为了分配内存的性能的问题，使用了很多叫做arena的memory pool,缺省配置在64bit下面是每一个arena为64M，一个进程可以最多有 cores * 8个arena。假设你的机器是4核的，那么最多可以有4 * 8 = 32个arena，也就是使用32 * 64 = 2048M内存。 当然你也可以通过设置环境变量来改变arena的数量.例如export MALLOC_ARENA_MAX=1

## 分析 ##
我们先分析下进程内存结构：

![EasyDarwin](http://www.easydarwin.org/github/images/easydarwin/doc/memorymanage/20160527233648.png)

每个进程有一个堆空间，glibc为了防止线程之间存在内存分配竞争问题，采用了预先分配的方式来解决问题，即便你malloc 1个自己也给你先分个64M虚拟内存，注意这里是虚拟内存，不是物理内存。
测试代码如下：

	#include <stdio.h>
	#include "tcmalloc.h"
	#include <pthread.h>
	#include <unistd.h>
	#include <stdlib.h>
	#include <iostream>
	using namespace std;
	
	volatile bool start = 0;
	
	void *threadFunc(void *argv)
	{
		int pid = getpid();
		char cmdstr[64] = {0};
		sprintf(cmdstr,"pmap %d|grep total",pid);
		while(1)
		{
			//if(start)
			{
				char *a = (char*)malloc(1024);
				printf("thread malloc\n");
				system(cmdstr);
				//sleep(1);
				start = 0;
			}
			sleep(1);
		}
	}
	int main()
	{
		//char *a = (char*)tc_malloc(100);
	    pthread_t pornsaveId;
	    int ret = 0;
	    ret = pthread_create(&pornsaveId, NULL, threadFunc,NULL);
	    if (ret)
	    {
	        return 0;
	    }	
		//tc_free(a);
		while(getchar())
		{
			c->score = 1000;
			start = 1;
			
		}
		return 0;
	}


## 优化建议 ##

Google开发了一个内存管理库，perftool，其中实现和tcmalloc和jemalloc,效率要比glibc高得多，具体实现细节可以自行百度。
当通过perf工具发现大量的malloc和free的时候，可以考虑引入google的tcmalloc或者jemalloc来解决性能问题。顺便说一句，尽量少在线程池中频繁进行申请和释放内存的操作，对性能影响比较大，因为线程之间存在竞争关系。

## tcmalloc使用例子 ##
先上网下载perftool库，编译：

	#include <iostream>
	#include <stdio.h>
	#include <stdlib.h>
	#include <unistd.h>
	#include <pthread.h>
	#include "tcmalloc.h"
	
	using namespace std;
	volatile bool start = 0;
	void* thread_run( void* )
	{
		while ( 1 )
		{
			if ( start )
			{
				cout << "Thread malloc" << endl;
				char *buf = (char *)tc_malloc(1024);// char[1024];
				start = 0;
			}
			sleep( 1 );
		}
	}
	int main()
	{
		pthread_t th;
		printf("wait input 111\n");
		getchar();
		printf("wait input 222\n");
		getchar();
		printf("wait input 333\n");
		pthread_create( &th, 0, thread_run, 0 );
		while ( (getchar() ) )
		{
			start = 1;
		}
		return(0);
	}


运行后发现，没有再像glibc那样有大块映射的虚拟内存了，而且性能也提高了很多（可以写个死循环进行测试，对比调用次数）
备注：现在机器基本都是64位的，虚拟内存2^64大小，基本不用考虑虚拟内存不够用的情况。


## 获取更多信息 ##

邮件：[support@easydarwin.org](mailto:support@easydarwin.org) 

WEB：[www.EasyDarwin.org](http://www.easydarwin.org)

Copyright &copy; EasyDarwin.org 2012-2016

![EasyDarwin](http://www.easydarwin.org/skin/easydarwin/images/wx_qrcode.jpg)