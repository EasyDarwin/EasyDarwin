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
 * proxy.h
 *
 * 
 */

#ifndef __PROXY_H__
#define __PROXY_H__

#include "shared_udp.h"
/**********************************************/
enum {
    stIdle,
    stError,

    stRecvClientCommand,
    stWaitingForIPAddress,
    stParseClientCommand,
    stSendClientResponse,
    stServerTransactionSend,
    stServerTransactionRecv,
    stClientShutdown,
    stServerShutdown,
    stBadServerName,
    stCantConnectToServer,
    stDone
};      // rtsp session states

enum {
    ttNone,
    ttDescribe,
    ttSetup,
    ttPlay,
    ttPause,
    ttStop,
    ttTeardown,
    ttOptions,
    ttAnnounce,
    ttRedirect,
    ttGetParameter,
    ttSetParameter
};      // rtsp command types

enum {
    kPermissionDenied,
    kTooManyUsers,
    kServerNotFound,
    kUnknownError
};      // refusal type

typedef struct {
    char    *cmd;
    int type;
} t_cmd_map;

#define MAX_TRACKS  32

typedef struct {
    int     ID;
    shok        *RTP_S2P;
    shok        *RTCP_S2P;
    shok        *RTP_P2C;
    shok        *RTCP_P2C;
    int     ClientRTPPort;
    int     ServerRTPPort;
    trans_pb    RTP_S2C_tpb;
    trans_pb    RTCP_S2C_tpb;
    trans_pb    RTCP_C2S_tpb;
} track_info;

/* This size will fit nicely in a standard ethernet frame */
#define RTSP_SESSION_BUF_SIZE   4096

typedef struct rtsp_session {
    struct rtsp_session *next;
    int     die;
    int     newSession;
    int     client_skt;
    int     client_ip;
    char        *server_address;
    int     server_skt;
    int     server_interface_addr;
    int     client_interface_addr;
    int     server_ip;
    int     server_port;
    int     server_skt_pending_connection;
    int     state;
    int     transaction_type;
    char        *sessionID;

    int     cur_trk;
    int     numTracks;
    track_info  trk[MAX_TRACKS];

    char        cinbuf[RTSP_SESSION_BUF_SIZE];
    int     amtInClientInBuffer;
    char        coutbuf[RTSP_SESSION_BUF_SIZE];
    int     amtInClientOutBuffer;
    char        sinbuf[RTSP_SESSION_BUF_SIZE];
    int     amtInServerInBuffer;
    char        soutbuf[RTSP_SESSION_BUF_SIZE];
    int     amtInServerOutBuffer;
    int     totalContentLength;
    int     haveParsedServerReplyHeaders;
    int         contentLength;
    char*       responseBodyP;

    int     tempIP;
} rtsp_session;

typedef struct subnet_allow {
    struct subnet_allow *next;
    int     ip;
    int     range;
} subnet_allow;

typedef struct rtsp_listener {
    struct rtsp_listener *next;
    int     port;
    int     skt;
} rtsp_listener;

/**********************************************/
int service_listeners();
int service_sessions();

void add_rtsp_port_listener(int address,int port);
void cleanup_listeners(void);
void answer_new_connection(rtsp_listener *listener);
void add_session(rtsp_session *session);
void remove_session(rtsp_session *session);
rtsp_session *new_session(void);
void cleanup_sessions(void);
void cleanup_session(rtsp_session *session);
void service_session(rtsp_session *session);
void service_session_rtp(rtsp_session *session);
void read_config(void);
void add_allow_subnet(int ip, int range);
bool allow_ip(int ip);
void send_rtsp_error(int skt, int refusal);

#endif // __PROXY_H__

