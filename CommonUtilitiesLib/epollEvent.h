/*
	Copyright (c) 2013-2016 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.easydarwin.org
	Author: Fantasy@EasyDarwin.org
*/

#ifndef _EPOLLEVENT_H__
#define _EPOLLEVENT_H__
#if defined(__linux__)
#include <stdio.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"

int epollInit();

int addEpollEvent(struct eventreq *req,int event);//event {EV_RD,EV_RM}

int deleteEpollEvent(int& fd);

int epollwaitevent();

int epoll_waitevent(struct eventreq *req, void* onlyForMOSX);//stay the same with old event

int epollDestory();
#endif

#endif

