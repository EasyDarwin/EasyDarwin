/*
	Copyright (c) 2013-2016 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.easydarwin.org
	Author: Fantasy@EasyDarwin.org
*/
#ifndef _COMMON_H__
#define _COMMON_H__
#define EV_RE  1
#define EV_WR  2
#define EV_EX  4//not used
#define EV_RM  8//inner use

struct eventreq {
  int      er_type;
#define EV_FD 1    // file descriptor
  int      er_handle;
  void    *er_data;
  int      er_rcnt;
  int      er_wcnt;
  int      er_ecnt;
  int      er_eventbits;
};

#endif

