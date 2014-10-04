/*
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * Copyright (c) 1999-2008 Apple Inc.  All Rights Reserved.
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 *
 */
 /*
 * proxy_plat.h
 *
 *
 */

#ifndef _PLAT_H_
#define _PLAT_H_

/**********************************************/
#define bool    char
#if !defined(mac)
#define true    1
#define false   0
#endif
#if !defined(WIN32)
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#endif

/**********************************************/
#if defined(WIN32)
#define EACCES      WSAEACCES
#define EINTR       WSAEINTR
#define EAGAIN      WSAEWOULDBLOCK      /* good enough? */
#define EPIPE       WSAESHUTDOWN        /* good enough? */
#define ENOTCONN    WSAENOTCONN
#define ECONNRESET  WSAECONNRESET
#define EISCONN     WSAEISCONN
#define EINPROGRESS WSAEINPROGRESS
#define EALREADY    WSAEALREADY
#include "WINSOCK.H"
#elif defined(mac)
#define EACCES      kEACCESErr
#define EPIPE       kEPIPEErr
#define EINTR       kEINTRErr
#define EAGAIN      kEAGAINErr
#define ENOTCONN    kENOTCONNErr
#define ECONNRESET  kECONNRESETErr
#define EISCONN     kEISCONNErr
#define EINPROGRESS kEINPROGRESSErr
#define EALREADY    kEALREADYErr
#endif

/**********************************************/
#define MSEC_PER_SEC    1000
#define USEC_PER_SEC    1000000
#define USEC_PER_MSEC   1000
#define timer_sub(ntime,subtime,eqtime)         \
    if ((subtime).tv_usec > (ntime).tv_usec) {                          \
        (eqtime).tv_sec = ((ntime).tv_sec - 1) - (subtime).tv_sec;      \
        (eqtime).tv_usec = (ntime).tv_usec + USEC_PER_SEC -  \
        (subtime).tv_usec;            \
    }                                           \
    else {                                      \
        (eqtime).tv_sec = (ntime).tv_sec - (subtime).tv_sec;            \
        (eqtime).tv_usec = (ntime).tv_usec - (subtime).tv_usec;         \
    }


extern char *gConfigFilePath;
extern char *gOptionsString;
extern char gOptionsChar;

typedef struct stats_chunk {
    unsigned long   elapsedSeconds;
    unsigned long   bpsReceived;
    unsigned long   bpsSent;
    unsigned long   ppsReceived;
    unsigned long   ppsSent;
    unsigned long   totalPacketsReceived;
    unsigned long   totalPacketsSent;
    unsigned long   numClients;
    unsigned long   numPorts;
    float           percentLostPackets;
} stats_chunk;


void daemonize(void);
int init_network(void);
int term_network(void);
int init_ui(void);
int service_ui(int sleep_ticks);

void sleep_milliseconds(int ms);
time_t microseconds();
#define kPENDING_ADDRESS -2
int name_to_ip_num(char *name, int *ip_num, int async);
int get_remote_address(int skt, int *port);
int get_local_address(int skt, int *port);
int get_local_ip_address(void);
bool isReadable(int fd);
bool isWritable(int fd);

int new_socket_udp(void);
int new_socket_tcp(int is_listener);
void close_socket(int skt);
void set_socket_reuse_address(int skt);
void set_socket_max_buf(int skt);
void make_socket_nonblocking(int skt);
int bind_socket_to_address(int skt, int address, int port, int is_listener);
int listen_to_socket(int skt);
int call_is_waiting(int skt, int *incoming_skt);
int accept_connection(int from, int *to);
int get_interface_addr(int skt);
#if DO_ASYNC
pascal void conn_finished_proc(void* contextPtr, OTEventCode code, OTResult /*result*/, void* /*cookie*/);
int connect_to_address(void *context, OTNotifyProcPtr proc, int skt, int address, int port);
#else
int connect_to_address(int skt, int address, int port);
#endif

int tcp_data_ready(int skt);

int recv_udp(int skt, char *buf, int amt, int *fromAddress, int *fromPort);
int send_udp(int skt, char *buf, int amt, int address, int port);
int recv_tcp(int skt, char *buf, int amt);
int send_tcp(int skt, char *buf, int amt);

// int make_udp_port_pair(int *socket1, int *socket2);

int GetLastSocketError(int skt);
void DoStats(stats_chunk *stats);

#if defined(mac) || defined(WIN32)
extern char gLastErrorString[256];
#define ErrorString(a)      sprintf(gLastErrorString, a)
#define ErrorString1(a,b)   sprintf(gLastErrorString, a, b)
#define ErrorStringS(a,b)   sprintf(gLastErrorString, a, b)
#define DebugString(a)      printf(a "\n")
#define DebugString1(a,b)   printf(a "\n", b)
#define DebugStringS(a,b)   printf(a "\n", b)
#else
void ErrorString(char *string);
void ErrorString1(char *string, int d);
void ErrorStringS(char *string, char *arg);
#define DebugString(a)      printf(a "\n")
#define DebugString1(a,b)   printf(a "\n", b)
#define DebugStringS(a,b)   printf(a "\n", b)
#endif
#endif // _PLAT_H_

