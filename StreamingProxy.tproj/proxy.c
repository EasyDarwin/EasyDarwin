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
 * proxy.c
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>
#include <assert.h>


#include <sys/types.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/resource.h>
#include <signal.h>
#include <sys/signal.h>
#include <sys/socket.h>

#include <ctype.h>
#include <regex.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "./proxy_plat.h"
#include "util.h"
#include "proxy.h"

#include <unistd.h>

#ifndef __MacOSX__
#include "get_opt.h"
#endif

#include "revision.h"
/**********************************************/
// Globals
int         gQuitting = 0;
int         gVerbose = 0;
int         gDebug = 0;
#if defined(mac) || defined(WIN32)
int         gStats = 1;
#else
int         gStats = 0;
#ifndef unix
	#define unix
#endif

#endif

#define ANY_ADDRESS     -1
#define MAX_CMD_BUFF        512
#define MAX_CONFIG_LINE_LEN 512
#define MAX_LINE_BUFF       2048

int     gNeedsService = 0;

#ifndef __MacOSX__
    extern char     *gConfigFilePath;
#endif

rtsp_session    *gSessions = NULL;

subnet_allow    *gAllowedNets = NULL;
rtsp_listener   *gListeners = NULL;
int     gUserLimit = 0;
int     gNumUsers = 0;

//int       gUDPPortMin = 4000;
//int       gUDPPortMax = 65535;

int     gProxyIP = -1;
int         gRTSPIP   = ANY_ADDRESS;
int     gMaxPorts = 0;

unsigned long   gBytesReceived = 0;
unsigned long   gBytesSent = 0;
unsigned long   gPacketsReceived = 0;
unsigned long   gPacketsSent = 0;
unsigned long   gLastPacketsReceived = 0;
unsigned long   gLastPacketsSent = 0;

/**********************************************/

int         gDropEnabled = 0;
float       gDropPercent = 0.0;
time_t      gStartDropOffset = 0;

/**********************************************/
#if defined(unix)
void sig_catcher(int sig)
{
    if (gDropEnabled)
    {
    
        if (sig == SIGUSR1)
        {   gDropPercent = 0;
#if __solaris__
            signal(SIGUSR1, sig_catcher);
#endif
            return;
        }
            
        if (sig == SIGUSR2)
        {   gDropPercent += 1.0;
            if (gDropPercent > 100.0)
                gDropPercent = 100.0;
                
#if __solaris__
            signal(SIGUSR2, sig_catcher);
#endif
            return;
        }
        
        if (sig == SIGHUP)
        {
            FILE * dropParamFile = fopen("drop","r");
            if (dropParamFile != NULL)
            {   char inBuff[256];
                while (fgets(inBuff, sizeof(inBuff), dropParamFile) != 0)
                {   char *tag;
                    char *value;
                    tag = strtok(inBuff, " ");
                    if (tag != NULL && NULL != strstr(tag,"drop_percent"))
                    {   value = strtok(NULL, " \r\n");
                        printf("drop_percent value=%s\n",value);
                        if (value != NULL)
                            sscanf(value, "%f",&gDropPercent);
                    }
                    
                }
                
                fclose(dropParamFile);
            }
#if __solaris__
            signal(SIGHUP, sig_catcher);
#endif
            return;
        }   
    }
    
    // do nothing cases
    if ( (sig == SIGUSR1) || (sig == SIGUSR2) || (sig == SIGHUP) )
    {   
#if __solaris__
        sigignore(sig);
#endif
        return; 
    }   
    gQuitting = 1;
}
#endif

/**********************************************/
static void print_usage(char *name)
{
    printf("%s/%s Built on %s, %s: [-dDvh] [-p #] [-c <file>]\n", name, kVersionString, __DATE__, __TIME__ );
    printf("  -d        : debug\n");
    printf("  -D        : verbose debug\n");
    printf("  -v        : print usage\n");
    printf("  -h        : help (this message)\n");
    printf("  -p #      : listen on port # (defaults to 554)\n");
    printf("  -c <file> : configuration file (defaults to %s)\n", gConfigFilePath);
    printf("  -i <hostname/ip address> : RTP Hostname/IP Address bind address.\n");
    printf("  -s        : statistics\n");
    printf("  -x        : enable packet drop mode (defaults to 0). SIGUSR1 to reset to 0. SIGUSR2 to add 1 to drop percent\n");
    printf("              use SIGHUP to read tag and value 'drop_percent 5' from the local file 'drop'. Use -s to see drop statistics\n");
    
}

/**********************************************/
int main(int argc, char *argv[])
{
    int i, j;
    int     numOptions = 0; // num command line options spec'd
    signed char ch;
    int listening_port = 554, user_listener = false;
    time_t time_zero, now, last;
    time_t  usnow, uslast;
    char *hostname = NULL;

#ifndef __MacOSX__
    extern char *optarg;
	extern int  optind;
#endif
 
    time_zero = time(0);
    last = time_zero;

#if defined(unix)
    //
    // increase file descriptor limit
    {
        struct rlimit rl;
        rl.rlim_cur = 4096;
        rl.rlim_max = 4096;
        setrlimit(RLIMIT_NOFILE, &rl);
    }
    //
    signal(SIGHUP, sig_catcher);
    signal(SIGTERM, sig_catcher);
    signal(SIGUSR1, sig_catcher);
    signal(SIGUSR2, sig_catcher);
    signal(SIGPIPE, SIG_IGN);
    
    //
    // boost our priority
#if defined( _sparc) || defined(HAVE_SETPRIORITY) || defined(__sgi__)
    i = setpriority(PRIO_PROCESS, 0, -20);
    if (i != 0)
        fprintf(stderr, "schedctl failed %d\n", errno);
#endif
#endif  // defined(unix)
    //
    // initialize the network stacks
    init_network();
    init_ui();

    //
    // read command line options
#if __MacOSX__
    while ((ch = getopt(argc, argv, gOptionsString)) != -1) 
#else
    while ((ch = get_opt(argc, argv, gOptionsString)) != -1) 
#endif
    {
        numOptions++;
        switch (ch) 
        {
            case 'p':
                listening_port = atoi(optarg);
                user_listener = true;
                break;
                
            case 'd':
                gDebug = 1;
                signal(SIGINT, sig_catcher); // try to catch the signal so we show what's still running.
                break;

            case 'D':
                gVerbose = 1;
                gDebug = 1;
                signal(SIGINT, sig_catcher); // try to catch the signal so we show what's still running.
                break;
                
            case 'x':
                gDropEnabled = 1;
                break;
                
            case 'v':
                print_usage(argv[0]);
                exit(0);
                break;
            case 's':
                gStats = 1;
                break;
            case 'h':
                print_usage(argv[0]);
                numOptions -= 2;    // not an option, if -h only, print usage and quit
                break;
               case 'i':
                hostname = optarg;
                break;
            case 'c':
                if ( optarg )
                    gConfigFilePath = str_dup(optarg);
                else
                {
                    printf("-c option requires a file path as an argument.\n");
                    exit(-1);
                }
                break;
        }
    }

    // will be less < 0 if asked for -h only,  if -h with other options we'll
    // continue to run, if no options at all we'll continute to run using 
    // the default config gile path
    
    if ( numOptions < 0 )
        exit(0);
        
    argc -= optind;
    argv += optind;

    //
    // read and deal with config
    read_config();

    //
    // set up rtsp listener (if necessary)
    //  -- if it wasn't specified in the config file, or the user had
    //     one on the command line
    if (!gListeners || user_listener)
        add_rtsp_port_listener(ANY_ADDRESS,listening_port);

    /* exit if we can't listen */
    if (!gListeners) {
        ErrorString("Can't set up any RTSP listeners. Exiting...\n");
        return -1;
    }

    /* if a hostname was specified, it over-writes whatever's in
     * the config file. */
    if (hostname) 
        name_to_ip_num(hostname, &gProxyIP, false);
        
    //
    if (gProxyIP == -1)
    {   //gProxyIP = get_local_ip_address(); this returns the local loopback (127.0.0.1) and is useless
        ErrorString("An rtp-bind-addr configuration line or -i command line option is required.\n");
        return -1;
    }

    if (gVerbose)
        printf("rtp-bind-addr: %s\n", ip_to_string(gProxyIP));
        
    if (!(gVerbose || gDebug || gStats))
        daemonize();
//

    //
    // compile regular expressions for RTSP
    uslast = microseconds();
    usnow = uslast;
    //
    // main event loop
    i = 0, j = 0;
    while (!gQuitting) {
        if ((i++ >= 50) || !gNeedsService) {
            service_listeners();
            i = 0;
        }
        if ((j++ > 25) || !gNeedsService) {
            service_sessions();
            j = 0;
        }
        service_shoks();
        
        if (gStats) {
            now = time(0);
            if ((now - last) >= 2) {
                time_t msElapsed;
                static unsigned long lastBPSReceived = 0, lastBPSSent = 0;
                unsigned long bpsReceived, bpsSent;
                stats_chunk stats;
                stats.numClients = gNumUsers;
                stats.elapsedSeconds = now - time_zero;
                usnow = microseconds();
                msElapsed = (usnow - uslast) / USEC_PER_MSEC;
                bpsReceived = ((gBytesReceived * USEC_PER_MSEC) / msElapsed) * 8;
                bpsSent = ((gBytesSent * USEC_PER_MSEC) / msElapsed) * 8;
                if (lastBPSReceived) {
                    stats.bpsReceived = (bpsReceived + lastBPSReceived) / 2;
                    stats.bpsSent = (bpsSent + lastBPSSent) / 2;
                }
                else {
                    stats.bpsReceived = bpsReceived;
                    stats.bpsSent = bpsSent;
                }
                stats.ppsReceived = ((gPacketsReceived - gLastPacketsReceived) * USEC_PER_MSEC) / msElapsed;
                stats.ppsSent = ((gPacketsSent - gLastPacketsSent) * USEC_PER_MSEC) / msElapsed;
                stats.totalPacketsReceived = gPacketsReceived;
                stats.totalPacketsSent = gPacketsSent;
                if (stats.ppsReceived > 0)
                    stats.percentLostPackets = 100.0 - ((float) stats.ppsSent / (float) stats.ppsReceived  * (float)100.0);
                else
                    stats.percentLostPackets = 0.0;
                stats.numPorts = gMaxPorts;
                DoStats(&stats);
                gBytesReceived = 0;
                gBytesSent = 0;
                gLastPacketsReceived = gPacketsReceived;
                gLastPacketsSent = gPacketsSent;
                lastBPSReceived = bpsReceived;
                lastBPSSent = bpsSent;
                uslast = usnow;
                last = now;
            }
        }
        if (!gNeedsService) {
            sleep_milliseconds(1);  
        }
        else {
            service_ui(0);
            gNeedsService = 0;
        }
    }

    cleanup_sessions();
    cleanup_listeners();
    
    //
    // terminate the network stacks
    term_network();

    return 0;
}

/**********************************************/
int service_listeners()
{
    rtsp_listener *listener;

    listener = gListeners;
    while (listener) 
        {
            answer_new_connection(listener);
            listener = listener->next;
    }
    return 0;
}

/**********************************************/
void cleanup_listeners()
{
    rtsp_listener *listener, *next;

    listener = gListeners;
    while (listener) 
    {
        next = listener->next;
        if (gDebug) 
            DebugString1("Closing listener socket %d", listener->skt);
        close_socket(listener->skt);
        free(listener);
        listener = next;
    }
}

/**********************************************/
void add_rtsp_port_listener(int address,int port)
{
    rtsp_listener *listener;
    int skt;

    if (gVerbose)
        printf("Listening for RTSP messages on port %d\n", port);

    if ((skt = new_socket_tcp(true)) == INVALID_SOCKET) {
        ErrorString1("couldn't set up RTSP listening socket (%d): %m\n",
                 port);
        return;
    }
    set_socket_reuse_address(skt);

    if (bind_socket_to_address(skt, address, port, true) == SOCKET_ERROR) 
        {
        close_socket(skt);
        perror("binding RTSP listening socket");
        return;
    }

    make_socket_nonblocking(skt);

    if (listen_to_socket(skt) == SOCKET_ERROR) 
        {
        close_socket(skt);
        ErrorString1("listening on RTSP socket (%d): %m\n", port);
        return;
    }
    
    listener = (rtsp_listener *)malloc(sizeof(rtsp_listener));
    if (!listener) {
        ErrorString("Couldn't allocate memory for listener.\n");
        exit(1);
    }
    listener->skt = skt;
    listener->port = port;
    listener->next = gListeners;
    gListeners = listener;
}

/**********************************************/
void answer_new_connection(rtsp_listener *listener) 
{
    int             skt;
    rtsp_session    *session;

    if ( !call_is_waiting(listener->skt, &skt))
        return;

    session = new_session();
    if (!session) {
        ErrorString("Couldn't create a new session\n");
        close_socket(skt);
        return;
    }

    session->client_skt = skt;  
    make_socket_nonblocking(session->client_skt);
    session->client_ip = get_remote_address(session->client_skt, NULL);
    session->client_interface_addr = get_interface_addr(session->client_skt);
    session->newSession = true;

    //
    // add him to our session list
    add_session(session);

    if (gVerbose)
        printf("Added connection for client %s.\n", ip_to_string(session->client_ip));

    gNeedsService++;
}

/**********************************************/
void add_session(rtsp_session *session)
{
    if (gSessions == NULL)
    {
        gSessions = session;
    }
    else 
    {
        session->next = gSessions;
        gSessions = session;
    }
    gNumUsers++;
}

/**********************************************/
void remove_session(rtsp_session *session)
{
    if (gSessions == NULL)
    {
            return;
    }
    else 
    {
        if (gSessions == session)
        {
            gSessions = session->next;
        }
        else 
        {
            rtsp_session *cur, *last = gSessions;
            cur = last->next;
            while (cur != NULL) 
            {
                if (cur == session) 
                {
                    last->next = cur->next;
                    break;
                }
                last = cur;
                cur = cur->next;
            }
        }
    }
}

/**********************************************/
rtsp_session *new_session(void)
{
    rtsp_session    *s;
    int     i;

    s = (rtsp_session*)calloc(1, sizeof(rtsp_session));
    if (s != NULL) 
        {
        s->next = NULL;
        s->die = false;
        s->client_skt = INVALID_SOCKET;
        s->client_ip = -1;
        s->server_address = NULL;
        s->server_skt = INVALID_SOCKET;
        s->server_interface_addr = 0;
        s->client_interface_addr = 0;
        s->server_ip = -1;
        s->server_port = 554;
        s->server_skt_pending_connection = false;
        s->state = stRecvClientCommand;
        s->transaction_type = ttNone;
        s->sessionID = NULL;

        s->cur_trk = 0;
        for (i=0; i<MAX_TRACKS; i++) 
                {
            s->trk[i].ID = 0;
            s->trk[i].ClientRTPPort = -1;
            s->trk[i].ServerRTPPort = -1;
            
            s->trk[i].RTP_S2P = NULL;
            s->trk[i].RTCP_S2P = NULL;
            s->trk[i].RTP_P2C = NULL;
            s->trk[i].RTCP_P2C = NULL;
            
            s->trk[i].RTP_S2C_tpb.status = NULL;
            s->trk[i].RTP_S2C_tpb.send_from = NULL;
            s->trk[i].RTP_S2C_tpb.send_to_ip = -1;
            s->trk[i].RTP_S2C_tpb.send_to_port = -1;
            s->trk[i].RTP_S2C_tpb.packetSendCount = 0;
            s->trk[i].RTP_S2C_tpb.nextDropPacket = 0;
            s->trk[i].RTP_S2C_tpb.droppedPacketCount = 0;
            
            s->trk[i].RTCP_S2C_tpb.status = NULL;
            s->trk[i].RTCP_S2C_tpb.send_from = NULL;
            s->trk[i].RTCP_S2C_tpb.send_to_ip = -1;
            s->trk[i].RTCP_S2C_tpb.send_to_port = -1;
            s->trk[i].RTCP_S2C_tpb.packetSendCount = 0;
            s->trk[i].RTCP_S2C_tpb.nextDropPacket = 0;
            s->trk[i].RTCP_S2C_tpb.droppedPacketCount = 0;
            
            s->trk[i].RTCP_C2S_tpb.status = NULL;
            s->trk[i].RTCP_C2S_tpb.send_from = NULL;
            s->trk[i].RTCP_C2S_tpb.send_to_ip = -1;
            s->trk[i].RTCP_C2S_tpb.send_to_port = -1;
            s->trk[i].RTCP_C2S_tpb.packetSendCount = 0;
            s->trk[i].RTCP_C2S_tpb.nextDropPacket = 0;
            s->trk[i].RTCP_C2S_tpb.droppedPacketCount = 0;
        }
        s->numTracks = 0;

        s->amtInClientInBuffer = 0;
        s->amtInClientOutBuffer = 0;
        s->amtInServerInBuffer = 0;
        s->amtInServerOutBuffer = 0;
        s->totalContentLength = 0;  // headers + body
        s->contentLength = 0;       // just body
        s->haveParsedServerReplyHeaders = 0;
    }
    return s;
}

/**********************************************/
void cleanup_session(rtsp_session *s)
{
    int i;

    if (s->client_skt != INVALID_SOCKET) 
    {
            if (gDebug) 
                printf("Closing client rtsp socket %d (ip %s)\n", s->client_skt, ip_to_string(s->client_ip));
            close_socket(s->client_skt);
            s->client_skt = INVALID_SOCKET;
    }
    
    if (s->server_skt != INVALID_SOCKET) 
    {
           if (gDebug) 
                printf("Closing server rtsp socket %d (ip %s)\n", s->server_skt, ip_to_string(s->server_ip));
            close_socket(s->server_skt);
            s->server_skt = INVALID_SOCKET;

    }
    
    if (s->server_address)
    {
            free(s->server_address);
            s->server_address = NULL;
    }
    
    if (s->sessionID)
    {
            free(s->sessionID);
            s->sessionID = NULL;
    }
    
    for (i=0; i<s->numTracks; i++) 
    {
            if (s->trk[i].RTP_S2P)
                    remove_shok_ref(s->trk[i].RTP_S2P, s->server_interface_addr, s->server_ip, true);
            if (s->trk[i].RTP_P2C)
                    remove_shok_ref(s->trk[i].RTP_P2C, s->client_interface_addr, s->client_ip, true);
    }
}

/**********************************************/
void cleanup_sessions(void)
{
    rtsp_session *cur, *next;

    cur = gSessions;
    while (cur) 
        {
            next = cur->next;
            cleanup_session(cur);
            free(cur);
            cur = next;
    }
        gSessions = NULL;
}

/**********************************************/
int service_sessions()
{
    rtsp_session *cur, *next;

    cur = gSessions;
    while (cur) 
        {
            next = cur->next;
            if (cur->newSession) 
            {
                    cur->newSession = false;
                    //
                    // check to see if this user is allowed
                    if (! allow_ip(cur->client_ip)) 
                    {
                        cur->die = true;
                        send_rtsp_error(cur->client_skt, kPermissionDenied);
                        if (gVerbose)
                            printf("Refusing connection for client %s - not allowed\n", 
                                ip_to_string(cur->client_ip));
                    }
                    //
                    // see if we're going beyond our user limit
                    else if (gUserLimit && (gNumUsers > gUserLimit)) 
                    {
                        cur->die = true;
                        send_rtsp_error(cur->client_skt, kTooManyUsers);
                        if (gVerbose)
                            printf("Refusing connection for client %s - too many users\n", 
                                ip_to_string(cur->client_ip));
                    }
            }
            if (cur->die) 
            {
                gNumUsers--;
                remove_session(cur);
                cleanup_session(cur);
                free(cur);
                cur = NULL;
            }
            else 
            {
                if (cur->client_skt != INVALID_SOCKET || cur->server_skt != INVALID_SOCKET)
                    service_session(cur);
            }
            cur = next;
    }
    return 0;
}

/**********************************************/
static const t_cmd_map cmd_map[] = {
    {"DESCRIBE",    ttDescribe},
    {"SETUP",   ttSetup},   
    {"PLAY",    ttPlay},
    {"PAUSE",   ttPause},
    {"STOP",    ttStop},
    {"TEARDOWN",    ttTeardown},
    {"OPTIONS",     ttOptions},
    {"ANNOUNCE",    ttAnnounce},
    {"REDIRECT",    ttRedirect},
    {"GET_PARAMETER", ttGetParameter},
    {"SET_PARAMETER", ttSetParameter},
    {NULL,      ttNone}
};

static int cmd_to_transaction_type(char *cmd)
{
    const t_cmd_map *map;
    map = cmd_map;
    while (map->cmd != NULL) {
        if (str_casecmp(map->cmd, cmd) == 0)
            return map->type;
        map++;
    }
    return ttNone;
}

/**********************************************/
static int track_id_to_idx(rtsp_session *s, int id)
{
    int i;
    for (i=0; i<s->numTracks; i++) {
        if (s->trk[i].ID == id)
            return i;
    }
    return -1;
}

/**********************************************/
static int has_two_crlfs(char *s)
{
    int     l, n;
    char    *p;
    l = strlen(s);
    if (l < 4)
        return 0;
    n = 3;
    p = s + n;
    while (n < l) {
        if (s[n] != '\n')
            n += 1;
        else if (s[n-1] != '\r' || s[n-2] != '\n' || s[n-3] != '\r')
            n += 2;
        else
            return n+1;
    }
    return 0;
}


/**********************************************/
static int is_command(char *inp, char *cmd, int cmdLen, char *server, int serverLen)
{
    int     l;
    char    *p;
    char *firstCmdChar = cmd;
    char *firstServerChar = server;
    
    char *lastCmdChar = NULL;
    if ( cmd && cmdLen > 2)
        lastCmdChar = &cmd[cmdLen -1]; 
        
    char *lastServerChar = NULL;
    if ( server && serverLen > 2)
        lastServerChar = &server[serverLen -1];
        
    if (lastServerChar == NULL || lastCmdChar == NULL)
        return 0;
        
    l = strlen(inp);

    if (l < 17)     /* "RTSP/1.0" (8) + " rtsp:// " (9) */
        return 0;
        
    if (strn_casecmp(inp + l - 8, "RTSP/1.0", 8) != 0)
        return 0;

    if (gVerbose) 
        printf("command hex=");

    p = inp;
    while (*p && (*p != ' ') && (cmd < lastCmdChar) )
    {   *cmd++ = *p++;
        if (gVerbose)
            printf("%x",*cmd);
    }
    *cmd = '\0';
    if (gVerbose)
        printf("%x\ncommand str=%s\ncommand count with term=%d\n",*cmd, firstCmdChar, (cmd - firstCmdChar) + 1);
     
    if (strn_casecmp(p, " rtsp://", 8) != 0)
        return 0;

    if (gVerbose)
        printf("server hex="); 
    p += 8;
    while (*p && (*p != '/') && (server < lastServerChar) )
    {    *server++ = *p++;
        if (gVerbose) 
            printf("%x",*server);
    }
    *server = '\0';
     if (gVerbose)
        printf("%x\nserver str=%s\nserver count with term=%d\n",*server, firstServerChar, (server - firstServerChar) + 1);

    return 1;
}

/**********************************************/
static int has_trackID(char *inp, int *trackID)
{
    int     l;
    char    *p;
    l = strlen(inp);

    if (l < 18)     /* "RTSP/1.0" (8) + "trackID=n " (10) */
        return 0;
    if (strn_casecmp(inp + l - 8, "RTSP/1.0", 8) != 0)
        return 0;

    p = inp;
    while (p) {
        p = strchr(p, '=');
        if (p - 7 < inp) {
            p++;
            continue;
        }
        if (strn_casecmp(p - 7, "trackid=", 8) != 0) {
            p++;
            continue;
        }
        *trackID = atoi(p + 1);
        return 1;
    }
    return 0;
}

/**********************************************/
static int has_content_length(char *inp, int *len)
{
    int     l;
    char    *p;
    l = strlen(inp);

    if (l < 16)     /* "Content-Length:n" (16) */
        return 0;
    if (strn_casecmp(inp, "content-length", 14) != 0)
        return 0;
    p = strchr(inp, ':');
    p++;
    while (*p && (*p == ' '))
        p++;
    if (p) {
        *len = atoi(p);
        return 1;
    }
    else
        return 0;
}

/**********************************************/
static int has_IN_IP(char *inp, char *str)
{
    int     l;
    char    *p;
    l = strlen(inp);

    if (l < 10)     /* "c=IN IP4 n" (10) */
        return 0;
    if (strn_casecmp(inp, "c=IN IP4 ", 9) != 0)
        return 0;
    p = inp + 9;
    while (*p && (*p == ' '))
        p++;

    while (*p && ((*p >= '0' && *p <= '9') || *p == '.'))
        *str++ = *p++;
    *str = '\0';
    return 1;
}

/**********************************************/
static int has_sessionID(char *inp, char *sessionID)
{
    int     l;
    char    *p;
    l = strlen(inp);

    if (l < 9)      /* "Session:x" (9) */
        return 0;
    if (strn_casecmp(inp, "session", 7) != 0)
        return 0;
    p = strchr(inp, ':');
    p++;
    while (*p && (*p == ' '))
        p++;
    if (p) {
        strcpy(sessionID, p);
        return 1;
    }
    else
        return 0;
}

/**********************************************/
static int has_client_port(char *inp, int *port)
{
    int     l;
    char    *p;
    l = strlen(inp);

    if (l < 23)     /* "Transport:<>client_port=n" (23) */
        return 0;
    if (strn_casecmp(inp, "transport", 9) != 0)
        return 0;
    
    p = inp;
    while (p) {
        p = strchr(p, '=');
        if (p - 11 < inp) {
            p++;
            continue;
        }
        if (strn_casecmp(p - 11, "client_port=", 12) != 0) {
            p++;
            continue;
        }
        *port = atoi(p + 1);
        *++p = '\0';
        return 1;
    }
    return 0;
}

/**********************************************/
static char *find_transport_header(char *inp)
{
    inp = strchr(inp, ';');
    if (inp != NULL)
        inp++;
    return inp;
}

/**********************************************/
static int has_ports(char *inp, int *client_port, int *server_port)
{
    int     l, got_server = 0, got_client = 0;
    char    *p;
    l = strlen(inp);

    if (l < 40)     /* "Transport:<>client_port=n-nserver_port=n-n" (40) */
        return 0;
    if (strn_casecmp(inp, "transport", 9) != 0)
        return 0;

    p = inp;
    while (p && !(got_client && got_server)) {
        p = strchr(p, '=');
        if (p - 11 < inp) {
        }
        else if (p == NULL)
        {
        }
        else if (strn_casecmp(p - 11, "client_port=", 12) == 0) {
            got_client = 1;
            *client_port = atoi(p + 1);
        }
        else if (strn_casecmp(p - 11, "server_port=", 12) == 0) {
            got_server = 1;
            *server_port = atoi(p + 1);
        }
        p++;
    }
    if (got_client && got_server)
        return 1;
    else
        return 0;
}

/**********************************************/
void service_session(rtsp_session *s)
{
    int     i, canRead, numDelims = 0;
    int     num = SOCKET_ERROR;
    char        *pBuf, *p;
    char        temp[RTSP_SESSION_BUF_SIZE + 1];
    char        lineBuff[MAX_LINE_BUFF + 1];
    track_info  *t;
    char        cmd[MAX_CMD_BUFF], *w;
    int         responseHeaderLen = 0;
    char        *startBuff;
    
    /* see if we have any commands coming in */
    pBuf = s->cinbuf + s->amtInClientInBuffer;
    canRead = sizeof(s->cinbuf) - s->amtInClientInBuffer - 1;
    if (canRead > 0) {
        if ((num = recv_tcp(s->client_skt, pBuf, canRead)) == SOCKET_ERROR) {
            switch (GetLastSocketError(s->client_skt)) {
                case EAGAIN:
                        /* do nothing, no data to be read. */
                    break;
                case EPIPE:     // connection broke
                case ENOTCONN:      // shut down
                case ECONNRESET:
                    s->state = stClientShutdown;
                    break;
                default:
                    ErrorString1("problems reading from session socket (%d)\n", GetLastSocketError(s->client_skt));
                    break;
            }
        }
        else if (num == 0) {
            // if readable and # of bytes is 0, then the client has shut down
            s->state = stClientShutdown;
        }
        else {
            pBuf[num] = '\0';
            s->amtInClientInBuffer += num;
        }
    }

    /* see what we have to do as a result of this or previous commands */
    switch (s->state) 
        {
        case stIdle:
            break;

        case stRecvClientCommand:
            //
            // have we read a full command yet?
            if (s->amtInClientInBuffer == 0 || ! has_two_crlfs(s->cinbuf))
                break;
            memset(temp, 0, sizeof(temp));

#if __MacOSX__
            strlcpy(temp, s->cinbuf, sizeof(temp));
#else
            strncpy(temp, s->cinbuf, sizeof(temp) -1);   //relies on memset above to 0 term last char.
#endif

            pBuf = temp;
            if ( (p = str_sep(&pBuf, "\r\n")) != NULL ) 
            {
                if (is_command(p, cmd, sizeof(cmd), temp, sizeof(temp))) 
                {
                    if (s->server_address != NULL)
                        free(s->server_address);
                    s->server_address = malloc(strlen(temp) + 1);
                    assert(s->server_address != NULL);
                    strcpy(s->server_address, temp);
                    //
                    // take port off address (if any)
                    if ((w = strchr(s->server_address, ':')) != NULL)
                        *w++ = '\0';
                    //
                    // make an async request for the IP number
#if USE_THREAD
                    name_to_ip_num(s->server_address, &s->tempIP, true);
#else
                    name_to_ip_num(s->server_address, &s->tempIP, false);
#endif
                    s->state = stWaitingForIPAddress;
                }
                else {  
                    ErrorStringS("Couldn't make sense of client command [%s]\n", temp);
                    s->state = stError;
                }
            }
            else {
                ErrorStringS("Couldn't make sense of client command [%s]\n", temp);
                s->state = stError;
            }
            break;

        case stWaitingForIPAddress:
            if (s->tempIP != kPENDING_ADDRESS) 
                        {
                add_to_IP_cache(s->server_address, s->tempIP);
                s->state = stParseClientCommand;
            }
            if (s->tempIP == -1) 
                        {
                send_rtsp_error(s->client_skt, kServerNotFound);
                s->state = stBadServerName;
            }
            break;

        case stParseClientCommand:
            if (gDebug)
                DebugStringS("service_session stParseClientCommand start=%s", s->cinbuf);
            startBuff = s->soutbuf;
            //
            // see what the command and server address is
            //
            // munge the data and snarf what we need
                        pBuf = s->cinbuf;
            while ((p = str_sep(&pBuf, "\r\n")) != NULL) 
                        {
                //
                // Count the empty fields; three in a row is the end of the header
                if (*p == '\0') {
                    if (++numDelims == 3)
                        break;
                    continue;
                }
                else
                                    numDelims = 0;
                //
                // see if we can snarf our data out of the headers
                if (is_command(p, cmd,sizeof(cmd), temp, sizeof(temp) )) 
                                {
                    int ip;
                    //
                    // get server address
                    if (s->server_address != NULL)
                        free(s->server_address);
                    s->server_address = malloc(strlen(temp) + 1);
                                        assert(s->server_address != NULL);
                    strcpy(s->server_address, temp);
                    //
                    // get server port (if any)
                    if ((w = strchr(s->server_address, ':')) != NULL) 
                                        {
                        *w++ = '\0';
                        s->server_port = atoi(w);
                    }
                    //
                    // check to see if command is pointing to the same server that
                    // we're already connected to.
                    name_to_ip_num(s->server_address, &ip, false);
                    if ((ip != s->server_ip) && (s->server_skt != INVALID_SOCKET)) {
                        close_socket(s->server_skt);
                        s->server_skt = INVALID_SOCKET;
                        s->server_ip = ip;
                    }
                    
                    if (ip == -1) {
                        s->state = stBadServerName;
                        return;
                    }

                    if (gProxyIP == ip ) // don't connect to yourself
                    {
                        ErrorStringS("Invalid session: destination IP is the same as this proxy IP (%s).\n",ip_to_string(ip));
                        close_socket(s->server_skt);
                        s->server_skt = INVALID_SOCKET;
                        s->state = stBadServerName;
                        return;
                    }

                    s->server_ip = ip;
                    if (gVerbose)
                        printf("%s command for server %s:%d (ip %s)\n",
                                cmd, s->server_address, s->server_port,
                                ip_to_string(s->server_ip));
                    s->transaction_type = cmd_to_transaction_type(cmd);

                    if (s->transaction_type == ttSetup && has_trackID(p, &i)) 
                    {
                        
                        num = track_id_to_idx(s, i);
                        if (num == -1 )
                        {
                            if  (s->numTracks == MAX_TRACKS) //stop before indexing the track array out of bounds
                            {
                                ErrorString1("Invalid session: The number of tracks are greater than the allowed maximum = (%d).\n",MAX_TRACKS);
                                close_socket(s->server_skt);
                                s->server_skt = INVALID_SOCKET;
                                s->state = stError;
                                return;
                            }

                            num = s->numTracks;                         
                            s->trk[s->numTracks++].ID = i;
                        }
                        s->cur_trk = num;
                    }
                }
                else if (s->transaction_type == ttSetup
                        && has_client_port(p, &(s->trk[s->cur_trk].ClientRTPPort))) 
                                {
                    t = s->trk + s->cur_trk;
                    if (gDebug)
                        printf("Client ports for track %d are %d-%d\n",s->cur_trk, t->ClientRTPPort, t->ClientRTPPort + 1);
                    //
                    // make rtp/rtcp port pair for proxy=>server
                    if (make_udp_port_pair(s->server_interface_addr, s->server_ip, &t->RTP_S2P, &t->RTCP_S2P) == -1) 
                    {   s->server_skt = INVALID_SOCKET;
                        ErrorString1("Couldn't create udp port pair for proxy=>server\n", GetLastSocketError(s->server_skt));
                        s->state = stError;
                        break;
                    }

                    if (gDebug)
                        printf("Created ports for server to proxy on track %d are %d-%d sockets %d-%d\n",
                                s->cur_trk, t->RTP_S2P->port, t->RTCP_S2P->port,
                                t->RTP_S2P->socket, t->RTCP_S2P->socket);
                    //
                    // reconstruct the client port string
                    sprintf(temp, "%s%d-%d", p, t->RTP_S2P->port, t->RTCP_S2P->port);
                    p = temp;
                }

                if (0 == strn_casecmp(p, "x-dynamic-rate", 14))// don't send to server not supported.
                    p = "x-dynamic-rate: 0";

                //
                // put the line in the outgoing buffer
                num = strlen(p);
                memcpy(s->soutbuf + s->amtInServerOutBuffer, p, (size_t)num);
                s->amtInServerOutBuffer += num;
                    
                s->soutbuf[s->amtInServerOutBuffer++] = '\r';
                s->soutbuf[s->amtInServerOutBuffer++] = '\n';
            }

            if (*(s->cinbuf + s->amtInServerOutBuffer) == 0)
            {
                s->soutbuf[s->amtInServerOutBuffer++] = '\r';
                s->soutbuf[s->amtInServerOutBuffer++] = '\n';
            }
            
            s->amtInClientInBuffer -= s->amtInServerOutBuffer;
            if (s->amtInClientInBuffer > 0)
            {   
                memcpy(s->cinbuf, s->cinbuf + s->amtInServerOutBuffer,  (size_t) s->amtInClientInBuffer);
                s->cinbuf[s->amtInClientInBuffer] = 0;
                if (gDebug)
                    DebugStringS("service_session stParseClientCommand memcpy to s->cinbuf=%s", s->cinbuf);
            }
            else if (s->amtInClientInBuffer < 0)
                s->amtInClientInBuffer = 0;
            s->state = stServerTransactionSend;
                
            if (s->soutbuf[s->amtInServerOutBuffer - 4] != '\r')
            {
                s->soutbuf[s->amtInServerOutBuffer++] = '\r';
                s->soutbuf[s->amtInServerOutBuffer++] = '\n';
            }
            
            if (gDebug)
                DebugStringS("service_session stParseClientCommand SEND TO CLIENT=%s", s->soutbuf);
            gNeedsService++;
            break;

        case stServerTransactionSend:
            //
            // check to see if we've got a connection to the server open
            if (s->server_skt == INVALID_SOCKET) 
                        {
                //
                // create a connection if we don't have one
                if ((s->server_skt = new_socket_tcp(false)) == INVALID_SOCKET) {
                    ErrorString("Couldn't open a socket to connect to server.\n");
                    s->state = stError;
                    return;
                }
                set_socket_reuse_address(s->server_skt);
                make_socket_nonblocking(s->server_skt);
                s->server_skt_pending_connection = true;
#if DO_ASYNC
                if ((i = connect_to_address(s, conn_finished_proc, s->server_skt, s->server_ip, s->server_port)) == SOCKET_ERROR) {
#else
                if ((i = connect_to_address(s->server_skt, s->server_ip, s->server_port)) == SOCKET_ERROR) {
#endif
                    num = GetLastSocketError(s->server_skt);
                    switch (GetLastSocketError(s->server_skt)) {
                        case EISCONN:       /* already connected */
                            break;
                        case EINPROGRESS:   /* connection can't be completed immediately */
                        case EAGAIN:        /* connection can't be completed immediately */
                        case EALREADY:      /* previous connection attempt hasn't been completed */
                            return;
                        default:
                            close_socket(s->server_skt);
                            s->server_skt = INVALID_SOCKET;
                            ErrorString1("Couldn't connect to server %d\n", GetLastSocketError(s->server_skt));
                            s->state = stCantConnectToServer;
                            return;
                    }
                }
                s->server_interface_addr = get_interface_addr(s->server_skt);
                s->server_skt_pending_connection = false;
                
            }

            //
            // check to see if we're connected (writable) and send the command
            if (s->amtInServerOutBuffer) {
                if ((num = send_tcp(s->server_skt, s->soutbuf, s->amtInServerOutBuffer)) == SOCKET_ERROR) {
                    switch (GetLastSocketError(s->server_skt)) {
                        case EPIPE:         // connection broke
                        case ENOTCONN:      // shut down
                        case ECONNRESET:
                            s->state = stServerShutdown;
                            break;
                        case EAGAIN:    // was busy - try again
                        case EINTR: // got interrupted - try again
                            break;
                        default:
                            ErrorString1("writing to server error (%d)\n", GetLastSocketError(s->server_skt));
                            s->state = stError;
                            return;
                    }
                }
                else if (num == 0)
                    s->state = stServerShutdown;
                else {
                    s->amtInServerOutBuffer -= num;
                    if (s->amtInServerOutBuffer == 0)
                        s->state = stServerTransactionRecv;
                }
            }

            gNeedsService++;
            break;

        case stServerTransactionRecv:
            //
            // check to see if we've got a response from the server
            if (s->server_skt == INVALID_SOCKET) 
            {   
                if (gDebug) printf("s->server_skt == INVALID_SOCKET\n");
                s->state = stServerShutdown;
                break;
            }
            
            pBuf = s->sinbuf + s->amtInServerInBuffer;
            canRead = sizeof(s->sinbuf) - s->amtInServerInBuffer - 1;

            if (canRead > 0) 
            {
                if ((num = recv_tcp(s->server_skt, pBuf, canRead)) == SOCKET_ERROR) 
                {
                    //if (gDebug) printf("problems reading from server (%d)", 
                                        //  GetLastSocketError(s->server_skt));
                    
                    switch (GetLastSocketError(s->server_skt))
                    {
                        case EAGAIN:
                                                        /* do nothing, no data to be read. */
                            gNeedsService++;
                            break;
                        case EPIPE:     // connection broke
                        case ENOTCONN:      // shut down
                        case ECONNRESET:
                            s->state = stServerShutdown;
                            break;
                        default:
                            ErrorString1("problems reading from server (%d)\n", GetLastSocketError(s->server_skt));
                            break;
                    }
                    
                    
                }
                else 
                {
                    pBuf[num] = '\0';
                    if (gDebug)
                        printf("\nread %d bytes from server:%s\n", num, pBuf);
                    if (0 == num)
                        sleep_milliseconds(1);  
                    s->amtInServerInBuffer += num;
                }
            }
            else
                if (gDebug) printf("can't read now!\n");
                            
            
            // DMS - if there is a content-length, make sure we've gotten all that data too.
            if ((s->totalContentLength > 0) && (s->amtInServerInBuffer < s->totalContentLength))
            {   
                //if (gDebug) printf("1-not enough in buffer content length %li, amt in buffer %li\n",
                                //  (long)s->totalContentLength, (long)s->amtInServerInBuffer);
                break;
            }

            if ( s->totalContentLength == 0 )
            {   //-rt if totalContentLength != 0,then we must have seen "has_two_crlfs"
                // and already parsed out the content-length header.
                // now we won't be able to find it again becuase str_sep
                // in the parsing code below has already replaced the CRLFs with \0's.
            
            
                //
                // did we get complete response headers yet?
                responseHeaderLen = has_two_crlfs(s->sinbuf);
                
                if (responseHeaderLen == 0)
                {   
                    break;
                }
            }
            

            if ( !s->haveParsedServerReplyHeaders )
            {               
                // we can only do this one time!
                
                //
                // munge the data for the client, while snarfing what we need
                
                for (pBuf = s->sinbuf; (p = str_sep(&pBuf, "\r\n")) != NULL; ) 
                {
                    //
                    // Count the empty fields; three in a row is end of the header
                    if (*p == '\0')
                    {
                        if (++numDelims == 3)
                            break;
                        continue;
                    }
                    else
                        numDelims = 0;
    
					if (0 == strn_casecmp(p, "x-Accept-Dynamic-Rate", 21))
					    p = "x-Accept-Dynamic-Rate: 0";

                    // see if we can snarf any data out of the headers
                    if (has_content_length(p, &s->contentLength)) 
                    {
                        // DMS - Set the total content length, if applicable
                        s->totalContentLength = s->contentLength + responseHeaderLen;
                    }
                    else if (has_sessionID(p, temp)) 
                    {
                        if (!s->sessionID) 
                        {
                            s->sessionID = malloc(strlen(temp) + 1);
                                                        if(s->sessionID == NULL)
                                                        {
                                                            ErrorString("Can't allocate session ID");
                                                            exit(1);
                                                        }
                            strcpy(s->sessionID, temp);
                        }
                        else if (str_casecmp(s->sessionID, temp) != 0)
                                ErrorString("Bad session ID in response from server\n");

                    }
                    else if (s->transaction_type == ttSetup && has_ports(p, &i, &num)) 
                    {
                        t = s->trk + s->cur_trk;
                        t->ServerRTPPort = num;
                        
                        if (gDebug)
                            printf("Server ports for track %d are %d-%d \n",
                                                                s->cur_trk, t->ServerRTPPort, t->ServerRTPPort + 1);
                        //
                        // make rtp/rtcp port pair here proxy=>client
                        if (make_udp_port_pair(s->client_interface_addr, s->client_ip, &t->RTP_P2C, &t->RTCP_P2C) == -1) 
                        {                       s->server_skt = INVALID_SOCKET;
                            ErrorString1("Couldn't create udp port pair for proxy=>client\n", GetLastSocketError(s->server_skt));
                            s->state = stError;
                            break;
                        }
                        
                        //
                        // set up transfer param blocks
                        t->RTP_S2C_tpb.status = &s->die;
                        t->RTP_S2C_tpb.send_from = t->RTP_P2C;
                        t->RTP_S2C_tpb.send_to_ip = s->client_ip;
                        t->RTP_S2C_tpb.send_to_port = t->ClientRTPPort;
                                                strcpy(t->RTP_S2C_tpb.socketName, "RTP Server to Client");
                        upon_receipt_from(t->RTP_S2P, s->server_ip, 
                                                    transfer_data, &(t->RTP_S2C_tpb));
    
                        t->RTCP_S2C_tpb.status = &s->die;
                        t->RTCP_S2C_tpb.send_from = t->RTCP_P2C;
                        t->RTCP_S2C_tpb.send_to_ip = s->client_ip;
                        t->RTCP_S2C_tpb.send_to_port = t->ClientRTPPort + 1;
                                                strcpy(t->RTCP_S2C_tpb.socketName,"RTCP Server to Client");
                        upon_receipt_from(t->RTCP_S2P, s->server_ip, 
                                                    transfer_data, &(t->RTCP_S2C_tpb));
    
                        t->RTCP_C2S_tpb.status = &s->die;
                        t->RTCP_C2S_tpb.send_from = t->RTCP_S2P;
                        t->RTCP_C2S_tpb.send_to_ip = s->server_ip;
                        t->RTCP_C2S_tpb.send_to_port = t->ServerRTPPort + 1;
                                                strcpy(t->RTCP_C2S_tpb.socketName,"RTCP Client to Server");
                        upon_receipt_from(t->RTCP_P2C, s->client_ip, 
                                                    transfer_data, &(t->RTCP_C2S_tpb));
    
                        if (gDebug)
                            printf( "Created ports for proxy to client on track %d are %d-%d sockets %d-%d\n",
                                                            s->cur_trk,
                                                            t->RTP_P2C->port, t->RTCP_P2C->port,
                                                            t->RTP_P2C->socket, t->RTCP_P2C->socket
                                                        );
                                    
                        //
                        // reconstruct the client;server string
                        w = find_transport_header(p);
                        if (w != NULL)
                            *w = '\0';
                        sprintf(temp, "%sclient_port=%d-%d;server_port=%d-%d;source=%s", p,
                            t->ClientRTPPort, t->ClientRTPPort+1,
                            t->RTP_P2C->port, t->RTCP_P2C->port,
                            ip_to_string(gProxyIP));
                        p = temp;
                        
                    }
    
                    //
                    // put the line in the outgoing buffer
                    num = strlen(p);
                    
                    assert( num + s->amtInClientOutBuffer + 2 <= RTSP_SESSION_BUF_SIZE );
                        
                    memcpy(s->coutbuf + s->amtInClientOutBuffer, p, (size_t) num);
                    s->amtInClientOutBuffer += num;
                    s->coutbuf[s->amtInClientOutBuffer++] = '\r';
                    s->coutbuf[s->amtInClientOutBuffer++] = '\n';
                    
                    // Drop a pointer to where the content body might begin, 
                                        // if this is in fact the last line
                    s->responseBodyP = pBuf + 3;
                }
    
                assert(  s->amtInClientOutBuffer + 2 <= RTSP_SESSION_BUF_SIZE );
                s->coutbuf[s->amtInClientOutBuffer++] = '\r';
                s->coutbuf[s->amtInClientOutBuffer++] = '\n';
            }
            
            // the headers are done now.
            s->haveParsedServerReplyHeaders = 1;
            
            // DMS - if there is a content-length, make sure we've gotten all that data too.
            if ((s->totalContentLength > 0) && (s->amtInServerInBuffer < s->totalContentLength))
            {   
                //if (gDebug) 
                                //   printf("2- not enough in buffer content length %li, amt in buffer %li\n", 
                                //  (long)s->totalContentLength, (long)s->amtInServerInBuffer);
                break;
            }
            
            //if (gDebug) printf("have complete response\n" );
            
            pBuf = s->responseBodyP;
            
            //
            // munge and add the content if there is any
            if (s->contentLength) 
            {
                if (s->transaction_type == ttDescribe) 
                {
                    char    *nextBuffPos = pBuf;
                    
                    // use "get_line_str" so that we preserve the EOL format
                    // of the describe resonse *exactly*
                    nextBuffPos = get_line_str( lineBuff, nextBuffPos, MAX_LINE_BUFF );

                    while ( *lineBuff )
                    {
                        //  c=IN IP0 ?
                        if (has_IN_IP(lineBuff, temp)) 
                        {
                            //char *nextChar = lineBuff;
                            
                            //
                            // reconstruct the IN IP string, but
                            // make sure our replacement string is the
                            // same length as the original string
                            // so as not to change the Content-Length string
                        //  while ( *nextChar )
                        //  {   
                                // replace all digits on the line with zeros...
                        //      if ( isdigit( *nextChar ) )
                        //          *nextChar = '0';
                        //      nextChar++;
                        //  
                        //  }
                        }
                        
                        //
                        // put the line in the outgoing buffer
                        num = strlen(lineBuff);
                        
                        assert( num + s->amtInClientOutBuffer <= RTSP_SESSION_BUF_SIZE );
    
                        memcpy(s->coutbuf + s->amtInClientOutBuffer, lineBuff,  (size_t)num);
                        s->amtInClientOutBuffer += num;
                        
                        nextBuffPos = get_line_str( lineBuff, nextBuffPos, MAX_LINE_BUFF );
                    }
                    
                    

                }
                else 
                {
                    assert( s->contentLength + s->amtInClientOutBuffer <= RTSP_SESSION_BUF_SIZE );
                        
                    memcpy(&s->coutbuf[s->amtInClientOutBuffer], pBuf, (size_t) s->contentLength);
                    s->amtInClientOutBuffer += s->contentLength;
                }
            }

            s->state = stSendClientResponse;
            s->amtInServerInBuffer = 0;
            s->totalContentLength = 0;
            s->haveParsedServerReplyHeaders = 0;
            s->contentLength = 0;
            //printf("NEXT: stSendClientResponse\n" );
            gNeedsService++;
            break;

        case stSendClientResponse:
            //printf("stSendClientResponse\n" );
            //
            // check to see that we're still connected (writable) and send the response
            if (s->amtInClientOutBuffer && isWritable(s->client_skt)) 
                        {
                if ((num = send_tcp(s->client_skt, s->coutbuf, s->amtInClientOutBuffer)) == SOCKET_ERROR) 
                                {
                    switch (GetLastSocketError(s->client_skt)) 
                                        {
                        case EPIPE: // connection broke
                        case ENOTCONN:  // shut down
                        case ECONNRESET:
                            s->state = stClientShutdown;
                            break;
                        case EAGAIN:    // was busy - try again
                        case EINTR: // got interrupted - try again
                            break;
                        default:
                            ErrorString1("writing to client error (%d)\n", GetLastSocketError(s->client_skt));
                            s->state = stError;
                            return;
                    }
                }
                else 
                                {
                                    s->amtInClientOutBuffer -= num;
                                    if (s->amtInClientOutBuffer == 0) {
                                        if (s->transaction_type == ttTeardown)
                                            s->state = stClientShutdown;
                                        else
                                            s->state = stRecvClientCommand;
                                    }
                            }
            }

            gNeedsService++;
            break;
            

        case stClientShutdown:
            if (gDebug && s->client_ip != -1) DebugString1("Client shutdown (ip %s)", ip_to_string(s->client_ip));
            s->die = true;
            gNeedsService++;
            break;

        case stBadServerName:
            send_rtsp_error(s->client_skt, kServerNotFound);
            s->state = stServerShutdown;
            break;

        case stCantConnectToServer:
            send_rtsp_error(s->client_skt, kServerNotFound);
            s->state = stServerShutdown;
            break;
            
        case stServerShutdown:
            if (gDebug && s->server_ip != -1) DebugString1("Server shutdown (ip %s)", ip_to_string(s->server_ip));
            s->die = true;
            gNeedsService++;
            break;

        case stError:
            send_rtsp_error(s->client_skt, kUnknownError);
            if (gDebug) DebugString("error condition.\n");
            s->die = true;
            gNeedsService++;
            break;
    }
}

/**********************************************/

void read_config() {
    int     fd, eof, num;
    char        buf, line[MAX_CONFIG_LINE_LEN], temp[MAX_CONFIG_LINE_LEN];
    int     line_pos;
    regmatch_t  pmatch[3];
    regex_t     regexpAllow, regexpComment, regexpUsers;
    regex_t     regexpListen, regexpPortRange;
    regex_t         regexpRTPAddr;

    if ((fd = open(gConfigFilePath, O_RDONLY)) == -1) {
        switch (errno) {
            case EACCES:
                ErrorStringS("Config file %s inaccessible\n", gConfigFilePath);
                break;
            default:
                ErrorString1("Problems opening config file (%d)\n", errno);
                break;
        }
        return;
    }
    
    if (gVerbose)
        printf("Opened config file %s\n", gConfigFilePath);

    regcomp(&regexpAllow, "^[ \t]*allow[ \t]+([0-9\\.]+)/([0-9]+).*$",
            REG_EXTENDED | REG_ICASE);
    regcomp(&regexpUsers, "^[ \t]*users[ \t]+([0-9]+).*$",
            REG_EXTENDED | REG_ICASE);
    regcomp(&regexpListen, "^[ \t]*listen[ \t]+([0-9\\.]+[ \t]+)?([0-9]+).*$",
            REG_EXTENDED | REG_ICASE);

    regcomp(&regexpPortRange, "^[ \t]*port-range[ \t]+([0-9]+)-([0-9]+).*$",
            REG_EXTENDED | REG_ICASE);
    regcomp(&regexpComment, "^[ \t]*#.*$", REG_EXTENDED | REG_ICASE);

    regcomp(&regexpRTPAddr, "^[ \t]*rtp-bind-addr[ \t]+([a-z0-9\\.\\-]+).*$",REG_EXTENDED | REG_ICASE);

    eof = false;
    while (!eof) {
        //
        // read a line from the configuration file
        line_pos = 0;
        while (1) {
            num = read(fd, &buf, 1);
            if (num <= 0) {
                eof = true;
                break;
            }
            if (buf == '\n' || buf == '\r')
                break;
            if (line_pos < MAX_CONFIG_LINE_LEN)
                line[line_pos++] = buf;
        }
        line[line_pos] = '\0';
        if (gDebug) DebugStringS("Read config line: %s", line);

        //
        // if the line has anything, try to parse it
        if (line_pos > 0) {
            if (regexec(&regexpAllow, line, 3, pmatch, 0) == 0) {
                int     ip, range;

                num = pmatch[1].rm_eo - pmatch[1].rm_so;
                memcpy(temp, line + pmatch[1].rm_so, (size_t) num);
                temp[num] = '\0';
                range = atoi(line + pmatch[2].rm_so);
                //name_to_ip_num(temp, &ip, false);
                ip = ntohl(inet_addr(temp));
                if (gVerbose)
                    printf("Allow connections from %s/%d\n", ip_to_string(ip), range);
                add_allow_subnet(ip, range);
            }
            else if (regexec(&regexpUsers, line, 3, pmatch, 0) == 0) {
                gUserLimit = atoi(line + pmatch[1].rm_so);
                if (gVerbose)
                    printf("Limit number of users to %d\n", gUserLimit);
            }
            else if (regexec(&regexpListen, line, 3, pmatch, 0) == 0) {
                    struct in_addr bindaddr;
                int port = atoi(line + pmatch[2].rm_so);

                    if(pmatch[1].rm_so==-1)
                  bindaddr.s_addr= (in_addr_t) htonl(ANY_ADDRESS);
                else
                  {
                    *(line+pmatch[1].rm_eo)='\0';
                    #if __solaris__
                        bindaddr.s_addr = inet_addr(line+pmatch[1].rm_so);
                        if( 0 == bindaddr.s_addr)
                    #else
                        if( inet_aton(line+pmatch[1].rm_so, (void *) &bindaddr)==0 )
                    #endif
                            printf("listen: failed to parse IP address %s\n",line+pmatch[1].rm_so);
              }

                add_rtsp_port_listener( (int) ntohl(bindaddr.s_addr),port);
            }
            else if (regexec(&regexpPortRange, line, 3, pmatch, 0) == 0) {
                int minPort, maxPort;
                minPort = atoi(line + pmatch[1].rm_so);
                maxPort = atoi(line + pmatch[2].rm_so);
                set_udp_port_min_and_max(minPort, maxPort);
                if (gVerbose)
                    printf("Use UDP ports %d through %d\n", minPort, maxPort);
            }
            else if (regexec(&regexpComment, line, 0, NULL, 0) == 0) {
                // comment - do nothing
            }
            else if (regexec(&regexpRTPAddr,line,3,pmatch,0)==0) {
                num = pmatch[1].rm_eo - pmatch[1].rm_so;
                memcpy(temp, line + pmatch[1].rm_so, (size_t) num);
                temp[num] = '\0';
                name_to_ip_num(temp, &gProxyIP, false);
                if (gProxyIP == -1) {
                    printf("unable to configure rtp-bind-addr: %s\n", temp);
                } else if (gVerbose) {
                    printf("configured rtp-bind-addr: %s (%s)\n", temp, ip_to_string(gProxyIP));
                }
            }
            else {
                ErrorStringS("invalid configuration line [%s]\n", line);
            }
        }
    }

    regfree(&regexpAllow);
    regfree(&regexpComment);
    regfree(&regexpUsers);
    regfree(&regexpListen);
    regfree(&regexpPortRange);
    regfree(&regexpRTPAddr);

    close(fd);
}

/**********************************************/
void add_allow_subnet(int ip, int range)
{
    subnet_allow *allow;

    allow = (subnet_allow*)malloc(sizeof(subnet_allow));
        assert(allow != NULL);
    allow->ip = ip;
    allow->range = range;
    allow->next = gAllowedNets;
    gAllowedNets = allow;
}

/**********************************************/
static int bitfill(int bits)
{
    int mask, i;

    /* unroll the bit fill and deal with common subnet masks. */
    if (bits >= 24) {
            mask = 0xffffff00;
        bits -= 24;
        i = 24;
    } else if (bits >= 16) {
            mask = 0xffff0000;
        bits -= 16;
        i = 16;
    } else if (bits >= 8) { 
            mask = 0xff000000;
        bits -= 8;
        i = 8;
    } else {
            mask = i = 0;
    }

    /* only 7 more bits to go! */
    while (bits) {
        mask |= (1 << (32 - i - bits));
        bits--;
    }
    return mask;
}

/**********************************************/
bool allow_ip(int ip)
{
    int             mask;
    subnet_allow    *cur;

    cur = gAllowedNets;
    if (cur == NULL)
        return true;

    while (cur) {
        mask = bitfill(cur->range);
        if ((cur->ip & mask) == (ip & mask))
            return true;
        cur = cur->next;
    }

    return false;
}

/**********************************************/
void send_rtsp_error(int skt, int refusal)
{
    char *refusal_string;

    switch (refusal) {
        case kServerNotFound:
            refusal_string = "RTSP/1.0 462 Destination unreachable\r\n";
            break;
        case kUnknownError:
            refusal_string = "RTSP/1.0 500 Unknown proxy error\r\n";
            break;
        case kPermissionDenied:
            refusal_string = "RTSP/1.0 403 Proxy denied\r\n";
            break;
        case kTooManyUsers:
            refusal_string = "RTSP/1.0 503 Too many proxy users\r\n";
            break;
        default:
            refusal_string = "RTSP/1.0 500 Unknown proxy error\r\n";
            break;
    }
    
    ErrorStringS("RTSP error: %s", refusal_string);
    send_tcp(skt, refusal_string, (int) strlen(refusal_string));
}

/**********************************************/
/**********************************************/
