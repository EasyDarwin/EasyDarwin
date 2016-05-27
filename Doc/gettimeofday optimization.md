---本文由EasyDarwin开源团队成员Fantasy贡献

## 一、问题描述 ##
Easydarwin中大量使用gettimeofday来获取系统时间，对系统性能造成了一定的影响。我们来做个测试：

![EasyDarwin](http://www.easydarwin.org/github/images/easydarwin/doc/gettimeofday/20160519222500202)


	While(1)
	{
		Gettimeofday(&tv,NULL);
	}


![EasyDarwin](http://www.easydarwin.org/github/images/easydarwin/doc/gettimeofday/20160519222553786)

每秒执行次数为约3000w次；

## 二、我们来看看gettimeofday函数内核实现 ##
参见：[http://blog.csdn.net/russell_tao/article/details/7185588](http://blog.csdn.net/russell_tao/article/details/7185588)

## 三、my_gettimeofday()实现 ##

	static inline int getcpuspeed_mhz(unsigned int wait_us)
	{
	       u_int64_t tsc1, tsc2;
	       struct timespec t;
	
	        t.tv_sec = 0;
	        t.tv_nsec = wait_us * 1000;
	
	        rdtscll(tsc1);
	
	        // If sleep failed, result is unexpected, the caller should retry
	        if(nanosleep(&t, NULL))
	                return -1;
	         rdtscll(tsc2);
	         return (tsc2 - tsc1)/(wait_us);
	} 
	
	int getcpuspeed()
	{
	        static int speed = -1;
	         while(speed<100)
	                speed = getcpuspeed_mhz(50*1000);
	         return speed;
	}
	
	int my_gettimeofday(struct timeval *tv)
	{
	        u_int64_t tick = 0;
	
	        // TSC偏移大于这个值，就需要重新获得系统时间
	        static unsigned int max_ticks = 80000000;
			rdtscll(tick);
	        if(walltime.tv_sec==0 || cpuspeed_mhz==0 ||
	                (tick-walltick) > max_ticks)
	        {
	                if(tick==0 || cpuspeed_mhz==0)
	                {
	                        cpuspeed_mhz = getcpuspeed();
	                        max_ticks = cpuspeed_mhz*RELOAD_TIME_US;
	                }
					//printf("gettimeofday again\n");
	                gettimeofday(tv, NULL);
	                memcpy(&walltime, tv, sizeof(walltime));
	                rdtscll(walltick);
	                return 0;
	        }
	
	        memcpy(tv, &walltime, sizeof(walltime));
	        // if RELOAD_TIME_US is 1, we are in the same us, no need to adjust tv
	#if RELOAD_TIME_US > 1
	        {
	                uint32_t t;
	                t = ((uint32_t)tick) / cpuspeed_mhz;
	                TIME_ADD_US(tv, t);//add 1 us
	        }
	#endif
	        return 0;
	}


通过休眠一段时间，然后检查cpu TSC变化，来大概估算cpu时钟频率，然后每次调用my_gettimeofday时，通过宏，rdtscll获取寄存器rdtsc寄存器的值，系统启动后cpu的tick数，当然第一次要先获取一下当前系统时间，gettimeofday。然后根据上面计算出来的cpu频率，以及获取到的cpu已经走过的tick数，计算出相对前面的gettimeofday时间的偏移，相加后得到当前系统时间。

> 备注：上面讲过，由于通过休眠一段时间，统计cpu tick变化的方式统计的cpu频率有一定误差，因此。Cpu tick每走过80000000重新校验一次，如果想要更高的精度，可以把这个值缩小。

## 四、优化后的测试效果 ##

![EasyDarwin](http://www.easydarwin.org/github/images/easydarwin/doc/gettimeofday/20160519223006766)

8000w+次每秒，性能提高了2-3倍。
在EasyDarwin上测试，通过easypusher_file推送100路，经过my_gettimeofday优化后，cpu消耗降低8%左右。

## 获取更多信息 ##

邮件：[support@easydarwin.org](mailto:support@easydarwin.org) 

WEB：[www.EasyDarwin.org](http://www.easydarwin.org)

Copyright &copy; EasyDarwin.org 2012-2016

![EasyDarwin](http://www.easydarwin.org/skin/easydarwin/images/wx_qrcode.jpg)