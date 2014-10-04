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
    File:       shared_udp.c
    Contains:   UDP Sockets implementation with shared ports

    

*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>

#if defined(unix)
#include <unistd.h>

#include <sys/types.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/resource.h>
#include <signal.h>
#include <sys/signal.h>
#include <sys/socket.h>
#elif defined(win32)
#include "WINSOCK.H"
#include "regex.h"
#elif defined(mac)
#include <MacTypes.h>
#include "OpenTransport.h"
#endif

#include "shared_udp.h"
#include "proxy_plat.h"

#if DEBUG
    #define DEBUGPRINT(x)   printf x
#else
    #define DEBUGPRINT(x)
#endif

/**********************************************/
shok    *gShokList = NULL;

/**********************************************/
ipList *find_ip_in_list(ipList *list, int ip)
{
    ipList *cur = list;

    DEBUGPRINT(( "-- -- looking for IP %x in IP list\n", ip));
    while (cur) {
        DEBUGPRINT(("-- -- vs. %x\n", cur->ip));
        if (cur->ip == ip) {
            DEBUGPRINT(("-- -- FOUND\n"));
            return cur;
        }
        cur = cur->next;
    }
    DEBUGPRINT(("-- -- NOT FOUND\n"));
    return NULL;
}

/**********************************************/
int add_ip_to_list(ipList **list, int ip)
{
    ipList  *newEl;

    newEl = (ipList*)malloc(sizeof(ipList));
    if (!newEl)
        return false;
    newEl->ip = ip;
    newEl->what_to_do = NULL;
    newEl->what_to_do_it_with = NULL;
    newEl->next = *list;
    *list = newEl;

    return true;
}

/**********************************************/
int remove_ip_from_list(ipList **list, int ip)
{
    ipList  *last, *theEl = *list;

    if (theEl->ip == ip) {
        *list = theEl->next;
        free(theEl);
        return true;
    }

    last = theEl;
    theEl = theEl->next;
    while (theEl) {
        if (theEl->ip == ip) {
            last->next = theEl->next;
            free(theEl);
            return true;
        }
        last = theEl;
        theEl = theEl->next;
    }
    return false;
}

/**********************************************/
shok *find_available_shok(int fromIP, int toIP, int withSib)
{   
    shok    *cur = gShokList;

    while (cur) {
        DEBUGPRINT(("-- looking for IP %x in shok %p\n", toIP, cur));
        if (find_ip_in_list(cur->ips, toIP) == NULL) {
            if (withSib) {
                DEBUGPRINT(("-- looking for IP %x in SIB shok %p\n", toIP, cur->sib));
                if (find_ip_in_list(cur->sib->ips, toIP) == NULL)
                    return cur;
            }
            else
                return cur;
        }
        cur = cur->next;
    }

    return NULL;
}

/**********************************************/
int add_ips_to_shok(shok *theShok, int fromIP, int toIP, int withSib)
{
    add_ip_to_list(&(theShok->ips), toIP);
    if (withSib)
        add_ip_to_list(&(theShok->sib->ips), toIP);
    return 1;
}

/**********************************************/
static int gUDPPortMin = 4000;
static int gUDPPortMax = 65536;
#define  sInvalidPort -1
static int gNextPort = sInvalidPort;

void set_udp_port_min_and_max(int min, int max)
{
    gUDPPortMin = min;
    gUDPPortMax = max;
    if (gUDPPortMin & 0x1)
        gUDPPortMin++;
}

/**********************************************/
int remove_shok(shok *theShok, int withSib)
{
    shok    *cur = NULL, *last;

    cur = gShokList;
    if (cur == theShok) {
        gShokList = cur->next;
        goto doSib;
    }

    last = cur;
    cur = cur->next;
    while (cur) {
        if (cur == theShok) {
            last->next = cur->next;
            goto doSib;
        }
        last = cur;
        cur = cur->next;
    }
    return false;

doSib:
    if (cur->sib)
        cur->sib->sib = NULL;
    if (withSib && cur->sib)
        remove_shok(cur->sib, false);

    {
        ipList *ipn, *ipl = cur->ips;
        while (ipl) {
            ipn = ipl->next;
            free(ipn);
            ipl = ipn;
        }
    }
    close_socket(cur->socket);
    free(cur);
    return true;
}

/**********************************************/
void remove_shok_ref(shok *theShok, int fromIP, int toIP, int withSib)
{
    remove_ip_from_list(&(theShok->ips), toIP);
    if (withSib)
        remove_ip_from_list(&(theShok->sib->ips), toIP);
    if (theShok->sib->ips == NULL)
        remove_shok(theShok->sib, false);
    if (theShok->ips == NULL)
        remove_shok(theShok, false);
}

/**********************************************/
shok *make_new_shok(int fromIP, int toIP, int withSib)
{
    shok *theShok1 = NULL, *theShok2 = NULL;
    int skt1 = INVALID_SOCKET, skt2 = INVALID_SOCKET;
    int port1 = sInvalidPort;
    int port2 = sInvalidPort;

    theShok1 = (shok*)malloc(sizeof(shok));
    if (!theShok1)
        goto bail_error;
    if (withSib) {
        theShok2 = (shok*)malloc(sizeof(shok));
        if (!theShok2)
            goto bail_error;
    }

    if (gNextPort == -1)
        gNextPort = gUDPPortMin;
retry:
    if ((skt1 = new_socket_udp()) == SOCKET_ERROR)
        goto bail_error;
    if (skt1 == 0) {
        if (GetLastSocketError(skt1) == EINPROGRESS || GetLastSocketError(skt1) == EAGAIN)
            goto retry;
        else
            goto bail_error;
    }
    do {
        if ((gNextPort & 0x1) && withSib)
            gNextPort++;
        if (gNextPort > gUDPPortMax)
            gNextPort = gUDPPortMin;
    } while (bind_socket_to_address(skt1, fromIP, port1 = gNextPort++, false) != 0);

    if (withSib) {
retry_rtcp:
        if ((skt2 = new_socket_udp()) == SOCKET_ERROR)
            goto bail_error;
        if (skt2 == 0) {
            if (GetLastSocketError(skt2) == EINPROGRESS || GetLastSocketError(skt2) == EAGAIN)
                goto retry_rtcp;
            else
                goto bail_error;
        }
        if (bind_socket_to_address(skt2, fromIP, port2 = gNextPort++, false) != 0) {
            close_socket(skt1);
            close_socket(skt2);
            skt1 = INVALID_SOCKET;
            skt2 = INVALID_SOCKET;
            goto retry;
        }
    }

    make_socket_nonblocking(skt1);
    theShok1->socket = skt1;
    theShok1->port = port1;
    theShok1->ips = NULL;

    if (withSib) {
        make_socket_nonblocking(skt2);
        theShok2->socket = skt2;
        theShok2->port = port2;
        theShok2->ips = NULL;
        theShok2->sib = theShok1;

        theShok1->sib = theShok2;
        theShok1->next = theShok2;
        theShok2->next = gShokList;
    }
    else {
        theShok1->sib = NULL;
        theShok1->next = gShokList;
    }

    add_ips_to_shok(theShok1, fromIP, toIP, withSib);
    gShokList = theShok1;

    return theShok1;    

bail_error:
    printf("make_new_shok bail_error\n");
    close_socket(skt1);
    close_socket(skt2);
    if (theShok1 != NULL)
        free(theShok1);
    if (theShok2 != NULL)
        free(theShok2);
    return NULL;
}

/**********************************************/
int make_udp_port_pair(int fromIP, int toIP, shok **rtpSocket, shok **rtcpSocket)
{
    shok    *theShok;

    DEBUGPRINT(("MAKE_UDP_PORT_PAIR from %x to %x\n", fromIP, toIP));
    DEBUGPRINT(("looking for available shok\n"));

    if ((theShok = find_available_shok(fromIP, toIP, true)) != NULL) {
        DEBUGPRINT(("found available shok : SOCKET [%d] PORT [%d]\n", theShok->socket, theShok->port));
        add_ips_to_shok(theShok, fromIP, toIP, true);
    }
    else {
        theShok = make_new_shok(fromIP, toIP, true);
        DEBUGPRINT(("couldn't find shok - made new one : SOCKET [%d] PORT [%d]\n", theShok->socket, theShok->port));
    }

    if (theShok && theShok->sib) {
        *rtpSocket = theShok;
        *rtcpSocket = theShok->sib;
        return 1;
    }
    else
        return -1;
}

/**********************************************/
int upon_receipt_from(shok *theShok, int fromIP, do_routine doThis, void *withThis)
{
    ipList  *listEl;
    DEBUGPRINT(( "UPON_RECEIPT_FROM %x do routine %p\n", fromIP, doThis));
    listEl = find_ip_in_list(theShok->ips, fromIP);
    if (!listEl)
        return -1;
    listEl->what_to_do = doThis;
    listEl->what_to_do_it_with = withThis;
    return 0;
}

/**********************************************/
extern int gNeedsService;
extern unsigned long gPacketsReceived;
extern unsigned long gPacketsSent;
extern unsigned long gBytesReceived;
extern unsigned long gBytesSent;
extern float gDropPercent;
extern unsigned long gDropDelta;


/**********************************************/
int service_shoks()
{
    shok    *cur;
    char    buf[2048];

    cur = gShokList;
    while (cur) {
        do_routine  doit;
        int         ret, fromPort, fromIP;

again:
        doit = NULL;
        ret = recv_udp(cur->socket, buf, 2048, &fromIP, &fromPort);
        if (ret > 0) {
            ipList *ipl = NULL;

            DEBUGPRINT(("Got %d bytes for %x on port %d on socket %d\n", ret, fromIP, fromPort, cur->socket));
            gBytesReceived += ret;
            gPacketsReceived++;
            ipl = find_ip_in_list(cur->ips, fromIP);
            if (ipl)
                doit = ipl->what_to_do;
            if (doit && ipl) {
                gNeedsService++;
                ret = (*doit)(ipl->what_to_do_it_with, buf, ret);
                if (ret == -1) {
                    // what to do about termination, etc.
                    DEBUGPRINT(("put returns error %d\n", errno));
                }
                else if (ret == 0) {
                    // client/server whatever died
                    if (((trans_pb*)ipl->what_to_do_it_with)->status)
                        *(((trans_pb*)ipl->what_to_do_it_with)->status) = 1;
                DEBUGPRINT(("client/server died\n"));
                }
                // what to do about incomplete packet transmission
            }
            goto again;
        }
        else if (ret < 0) {
            if (errno != 11)
                DEBUGPRINT(("recv_udp returns errno %d\n", errno));
            // what to do about termination, etc.
        }
        cur = cur->next;
    }
    return 0;
}

/**********************************************/
int transfer_data(void *refCon, char *buf, int bufSize)
{
    trans_pb    *tpb = (trans_pb*)refCon;
    int     ret;
    int             isRTCP = 0;
    if (!tpb)
        return -1;
        
    if (strstr(tpb->socketName,"RTCP"))
    {   //printf("shared_udp.c transfer_data %s\n",tpb->socketName);
        isRTCP = 1;
    }
    tpb->packetCount++;
    if (gDropPercent > 0.0 )
    {   int packetDropped = 0;
        //printf("transfer_data tpb->nextDropPacket=%qd tpb->packetCount=%qd\n",tpb->nextDropPacket, tpb->packetCount);
        if (tpb->packetCount == tpb->nextDropPacket)
        {   tpb->droppedPacketCount ++;
            tpb->nextDropPacket = 0;
            //printf("transfer_data tpb->droppedPacketCount=%qd tpb->packetCount=%qd\n",tpb->droppedPacketCount, tpb->packetCount);
            packetDropped = 1;
        }
        
        if (tpb->nextDropPacket == 0)
        {   
            int offset = 0;
            
            if (gDropPercent <= 50.0)
                offset =  100.0 / gDropPercent; // drop the percent packet
            else if (gDropPercent < 100.0 )
                offset = 1 + ( ( (rand() % 49) <= ( gDropPercent - 47.0) ) ? 0 : 1);// drop random 1 to 2 packets   1 == next packet or 100% 2== every other 50%    
            else
                offset = 1; // 100% = drop next packet
            
            tpb->nextDropPacket = tpb->packetCount + offset;                
        }
        
        if (packetDropped)
            return bufSize;

    }
    
    ret = send_udp(tpb->send_from->socket, buf, bufSize, tpb->send_to_ip, tpb->send_to_port);
    DEBUGPRINT(("Sent %d bytes to %x on port %d on socket %d\n", 
                        ret, tpb->send_to_ip, tpb->send_to_port, tpb->send_from->socket));
    if (ret > 0)
    {   gBytesSent += ret;
        gPacketsSent++;
    }
    return ret;
}


