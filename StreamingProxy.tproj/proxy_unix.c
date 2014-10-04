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
    proxy_unix.c

*/

 
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/time.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <sys/resource.h>
#include <sys/signal.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "util.h"
#include "proxy_plat.h"

#if USE_THREAD
#include "pthread.h"
#endif

#include "defaultPaths.h"

char *gConfigFilePath = DEFAULTPATHS_ETC_DIR "streamingproxy.conf";

char        *gOptionsString = "-c:-p:-d-D-v-h-s-x-i:";
char        gOptionsChar = '-';

extern int  gMaxPorts;
extern float    gDropPercent;

/**********************************************/
int init_network() 
{
    return 0;
}

/**********************************************/
#define kKILL_THREAD -3
int term_network() 
{
    int send = kKILL_THREAD;
    name_to_ip_num("", &send, true);
    return 0;
}

/**********************************************/
typedef struct ghpb_rec {
    char    name[256];
    int *result;
} ghpb_rec, *ghpb;

#if USE_THREAD
void *gethostthread(void *param) 
{
    struct hostent  *hent;
    ghpb        pb = (ghpb)param;
    int     id;
    pthread_t   tid;
    int     tryCount = 0;

    if (*pb->result == kKILL_THREAD)
        exit(0);
    tid = pthread_self();
again:
    hent = gethostbyname(pb->name);
    if (hent == NULL) do 
        {
        tryCount ++;
        if (h_errno == TRY_AGAIN)
        {   if (tryCount < 10)
                goto again;
            else
                return 0;
        }
        *pb->result = -1;
        pthread_exit(NULL);
    } while(0);
    id = ntohl(((struct in_addr *)(hent->h_addr_list[0]))->s_addr);
    *pb->result = id;
    free(pb);
    pthread_exit(NULL);
    return NULL;
}
#endif
/**********************************************/
int name_to_ip_num(char *name, int *ip, int async)
{
    int             ret;
    struct  in_addr addr;
    int             tryAgain = 0;
#if USE_THREAD
    ghpb pb = NULL;
    pthread_t   tid;
#endif
    struct hostent *hent;
    
    if (check_IP_cache(name, &ret) != -1)
    {
        *ip = ret;
        return 0;
    }

#if USE_THREAD
    if (async) 
        {
        *ip = kPENDING_ADDRESS;
        pb = (ghpb)malloc(sizeof(ghpb_rec));
        strcpy(pb->name, name);
        pb->result = ip;
        pthread_create(&tid, NULL, gethostthread, (void*)pb);
        pthread_detach(tid);
        return 1;
    }
#endif

again:

    tryAgain ++;
    if ( inet_aton( name, &addr ) )
    {   *ip = ntohl( addr.s_addr );
        add_to_IP_cache(name, *ip ); 
        return 0;
    }
    
    hent = gethostbyname(name);
    if (hent == NULL) {
        if (h_errno == TRY_AGAIN)
            if (tryAgain < 10)
                goto again;
        add_to_IP_cache(name, -1);
        return -1;
    }
    *ip = ntohl(((struct in_addr *) (hent->h_addr_list[0]))->s_addr);
    add_to_IP_cache(name, *ip);
    return 0;
}


/**********************************************/
int get_remote_address(int skt, int *port) 
{
#if !defined(sparc) && !defined(SGI) && !defined(WIN32)
    unsigned
#endif
        int nAddrSize = sizeof(struct sockaddr_in);
    struct sockaddr_in  remAddr;
    int  status;
    remAddr.sin_addr.s_addr = INADDR_ANY;
    status = getpeername(skt, (struct sockaddr*)&remAddr, &nAddrSize);
    if (status >= 0) 
        {
        if (port)
            *port = ntohs(remAddr.sin_port);
        return ntohl(remAddr.sin_addr.s_addr);
    }
    return -1;
}

/**********************************************/
int get_local_address(int skt, int *port) {
#if !defined(sparc) && !defined(SGI) && !defined(WIN32)
    unsigned
#endif
        int nAddrSize = sizeof(struct sockaddr_in);
    struct sockaddr_in  remAddr;
    int  status;
    remAddr.sin_addr.s_addr = INADDR_ANY;
    status = getsockname(skt, (struct sockaddr*)&remAddr, &nAddrSize);
    if (status >= 0) 
        {
        if (port)
                    *port = ntohs(remAddr.sin_port);
        return ntohl(remAddr.sin_addr.s_addr);
    }
    return -1;
}

/**********************************************/
static int __local_ip_address = -1;

int get_local_ip_address() 
{
    char        buf[256];
    struct hostent  *hent;
    int         tryCount = 0;

    if (__local_ip_address != -1)
        return __local_ip_address;
    
    if (gethostname(buf, 256) < 0)
        return -1;
again:
    tryCount ++;
    hent = gethostbyname(buf);
    if (hent == NULL) 
    {   if (h_errno == TRY_AGAIN)
        {   if (tryCount < 10)
            {   
                            goto again;
            }
            else
            {   
                            return 0;
            }
        }
        return -1;
    }
    __local_ip_address = ntohl(((struct in_addr *)hent->h_addr)->s_addr);
    return __local_ip_address;
}

/**********************************************/
void make_socket_nonblocking(int socket)
{
    int flag;
    flag = fcntl(socket, F_GETFL, 0);
    fcntl(socket, F_SETFL, flag | O_NONBLOCK);
}

/**********************************************/
void sleep_milliseconds(int ms)
{
    struct timeval  tv;

    tv.tv_sec = ms / 1000;
    tv.tv_usec = (ms - (tv.tv_sec * 1000) ) * 1000;
    select(0, NULL, NULL, NULL, &tv);
}

/**********************************************/
time_t microseconds()
{
    static bool us_initted = false;
    static struct timeval us_time_zero;
    struct timeval  tv, t;
    struct timezone tz;

    gettimeofday(&tv, &tz);
    if (us_initted == false) 
        {
        us_initted = true;
        us_time_zero = tv;
        return 0;
    }
    else 
        {
        timer_sub(tv, us_time_zero, t);
        return (t.tv_sec * USEC_PER_SEC + t.tv_usec);
    }
}

/**********************************************/
bool isReadable(int fd)
{

/* causes crash fd_set is wrong size if num users > 255
// not needed anyway

    fd_set  set;
    struct  timeval tv;
    int     err;

    if (fd == INVALID_SOCKET)
        return false;
        
    tv.tv_sec = 0; tv.tv_usec = 0;
    
    FD_ZERO(&set);
    
    FD_SET(fd, &set);
    
    err = select(fd+1, &set, NULL, NULL, &tv);
    if (err > 0)
        if (FD_ISSET(fd, &set))
            return true;
    return false;
*/
    return true;
}

/**********************************************/
bool isWritable(int fd)
{
/* causes crash fd_set is wrong size if num users > 255
// not needed anyway

    fd_set  set;
    struct  timeval tv;
    int     err;

    if (fd == INVALID_SOCKET)
        return false;
    tv.tv_sec = 0; tv.tv_usec = 0;
    FD_ZERO(&set);
    FD_SET(fd, &set);
    err = select(fd+1, NULL, &set, NULL, &tv);
    if (err > 0)
        if (FD_ISSET(fd, &set))
            return true;
    return false;
*/

    return true;
}

/**********************************************/
int new_socket_udp(void)
{
    int ret;
    ret = socket(PF_INET, SOCK_DGRAM, 0);
    gMaxPorts++;
    set_socket_max_buf(ret);
    return ret;
}

/**********************************************/
int new_socket_tcp(int is_listener)
{
    int ret;
    ret = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    gMaxPorts++;
    return ret;
}

/**********************************************/
void set_socket_reuse_address(int skt)
{
    int i = 1;
    setsockopt(skt, SOL_SOCKET, SO_REUSEADDR, (char*)&i, sizeof(i));
}

/**********************************************/
void set_socket_max_buf(int skt)
{
    int i = 1;
    unsigned int len;
    len = sizeof(i);
    getsockopt(skt, SOL_SOCKET, SO_SNDBUF, (char*)&i, &len);
    /*fprintf(stderr, "sndbuf for socket %d was %d\n", skt, i);*/
    i *= 2;
    setsockopt(skt, SOL_SOCKET, SO_SNDBUF, (char*)&i, len);
    getsockopt(skt, SOL_SOCKET, SO_SNDBUF, (char*)&i, &len);
    /*fprintf(stderr, "sndbuf for socket %d is now %d\n", skt, i);*/
    getsockopt(skt, SOL_SOCKET, SO_RCVBUF, (char*)&i, &len);
    /*fprintf(stderr, "rcvbuf for socket %d was %d\n", skt, i);*/
    i *= 2;
    setsockopt(skt, SOL_SOCKET, SO_RCVBUF, (char*)&i, len);
    getsockopt(skt, SOL_SOCKET, SO_RCVBUF, (char*)&i, &len);
    /*fprintf(stderr, "rcvbuf for socket %d is now %d\n", skt, i);*/
}

/**********************************************/
int bind_socket_to_address(int skt, int address, int port, int is_listener)
{
    struct sockaddr_in sin;
    
    if (address == -1)
        address = INADDR_ANY;
        
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port = htons(port);
    sin.sin_addr.s_addr = htonl(address);
    return bind(skt, (struct sockaddr*)&sin, sizeof(sin));
}

/**********************************************/
void close_socket(int skt)
{
    if (skt != INVALID_SOCKET) 
    {
        gMaxPorts--;
        close(skt);
    }
}

/**********************************************/
int listen_to_socket(int skt)
{
    return listen(skt, 5);
}

/**********************************************/
int call_is_waiting(int skt, int *incoming_skt)
{
    int ret;
    
    ret = isReadable(skt);
    
    if (ret) 
    {
            *incoming_skt = accept(skt, 0, 0);
        
            if (*incoming_skt <= 0) 
            {
                ret = 0;
            }
            else
                gMaxPorts++;
    }
    
    return ret;
}

/**********************************************/
int accept_connection(int from, int *to)
{
//  return accept(from, 0, 0);
    return *to;
}

/**********************************************/
int connect_to_address(int skt, int address, int port)
{
    struct sockaddr_in sin;
    
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port = htons(port);
    sin.sin_addr.s_addr = htonl(address);;
    return connect(skt, (struct sockaddr*)&sin, sizeof(sin));
}

int get_interface_addr(int skt)
{
    int err = 0;
    struct sockaddr_in  localAddr;
    unsigned int len = sizeof(localAddr);
    memset(&localAddr, 0, sizeof(localAddr));
    
    err = getsockname(skt, (struct sockaddr*)&localAddr, &len);
    return ntohl(localAddr.sin_addr.s_addr);
}

/**********************************************/
int recv_udp(int socket, char *buf, int amt, int *fromip, int *fromport)
{
    struct sockaddr_in  sin;
    int ret;
	unsigned int len;

    len = sizeof(sin);
    memset(&sin, 0, sizeof(sin));
    ret = recvfrom(socket, buf, (size_t) amt, 0, (struct sockaddr*)&sin, &len);
    if (ret != -1) {
        if (fromip)
            *fromip = ntohl(sin.sin_addr.s_addr);
        if (fromport)
            *fromport = ntohs(sin.sin_port);
    }
    return ret;
}

/**********************************************/
int send_udp(int skt, char *buf, int amt, int address, int port)
{
    struct sockaddr_in sin;
    
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port = htons(port);
    sin.sin_addr.s_addr = htonl(address);
    return sendto(skt, buf, (size_t) amt, 0, (struct sockaddr*)&sin, sizeof(sin));
}

/**********************************************/
int recv_tcp(int socket, char *buf, int amt)
{
    return read(socket, buf, (size_t) amt);
}

/**********************************************/
int send_tcp(int socket, char *buf, int amt)
{
    return write(socket, buf, (size_t) amt);
}

/**********************************************/
int GetLastSocketError(int skt)
{
    return errno;
}

/**********************************************/
int init_ui()
{
    return 0;
}

/**********************************************/
int service_ui(int sleep_time)
{
    return 0;
}

/**********************************************/
void DoStats(stats_chunk *stats)
{
    printf("\033[2J\033[H");
    printf("Elapsed Time (seconds) : %lu\n", stats->elapsedSeconds);
    printf("Number of clients      : %lu\n", stats->numClients);
    printf("bps Received           : %lu\n", stats->bpsReceived);
    printf("bps Sent               : %lu\n", stats->bpsSent);
    printf("Total Packets Received : %lu\n", stats->totalPacketsReceived);
    printf("Total Packets Sent     : %lu\n", stats->totalPacketsSent);
    printf("pps Received           : %lu\n", stats->ppsReceived);
    printf("pps Sent               : %lu\n", stats->ppsSent);
    printf("number of ports used   : %lu\n", stats->numPorts);
    printf("packet loss percent    : %f\n", stats->percentLostPackets);
    printf("force drop percent     : %f\n",gDropPercent);
}

/**********************************************/
void ErrorString(char *string)
{
    fprintf(stderr, string);
}

/**********************************************/
void ErrorString1(char *string, int d)
{
    fprintf(stderr, string, d);
}

/**********************************************/
void ErrorStringS(char *string, char *arg)
{
    fprintf(stderr, string, arg);
}

void daemonize()
{
    switch (fork()) {
    case -1: /* error */
        fprintf(stderr, "can't daemonize!\n");
    case 0: /* child */
        break;
    default: /* parent */
        exit(0);
    }
}
