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
	Copyleft (c) 2013-2015 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.EasyDarwin.org
*/

#ifndef QTSS_H
#define QTSS_H
#include "OSHeaders.h"
#ifdef __cplusplus
extern "C" {
#endif

#ifndef __Win32__
#include <sys/uio.h>
#endif
#define QTSS_API_VERSION                0x00050000
#define QTSS_MAX_MODULE_NAME_LENGTH     64
#define QTSS_MAX_SESSION_ID_LENGTH      64
#define QTSS_MAX_ATTRIBUTE_NAME_SIZE    64
#define EASY_MAX_URL_LENGTH				512
#define EASY_MAX_SERIAL_LENGTH			32
#define EASY_REQUEST_BUFFER_SIZE_LEN	64*1024

//*******************************
// ENUMERATED TYPES

/**********************************/
// Error Codes

enum
{
    QTSS_NoErr              = 0,
    QTSS_RequestFailed      = -1,
    QTSS_Unimplemented      = -2,
    QTSS_RequestArrived     = -3,
    QTSS_OutOfState         = -4,
    QTSS_NotAModule         = -5,
    QTSS_WrongVersion       = -6,
    QTSS_IllegalService     = -7,
    QTSS_BadIndex           = -8,
    QTSS_ValueNotFound      = -9,
    QTSS_BadArgument        = -10,
    QTSS_ReadOnly           = -11,
    QTSS_NotPreemptiveSafe  = -12,
    QTSS_NotEnoughSpace     = -13,
    QTSS_WouldBlock         = -14,
    QTSS_NotConnected       = -15,
    QTSS_FileNotFound       = -16,
    QTSS_NoMoreData         = -17,
    QTSS_AttrDoesntExist    = -18,
    QTSS_AttrNameExists     = -19,
    QTSS_InstanceAttrsNotAllowed= -20
};
typedef SInt32 QTSS_Error;

// Flags for QTSS_Write when writing to a QTSS_ClientSessionObject.
enum 
{
    qtssWriteFlagsNoFlags           = 0x00000000, 
    qtssWriteFlagsWriteBurstBegin   = 0x00000001,
    qtssWriteFlagsBufferData        = 0x00000002
};
typedef UInt32 QTSS_WriteFlags;

// Flags for the qtssRTSPReqAction attribute in a QTSS_RTSPRequestObject.
enum 
{
    qtssActionFlagsNoFlags      = 0x00000000,
    qtssActionFlagsRead         = 0x00000001,
    qtssActionFlagsWrite        = 0x00000002,
    qtssActionFlagsAdmin        = 0x00000004,
    qtssActionFlagsExtended     = 0x40000000,
    qtssActionQTSSExtended      = 0x80000000,
};
typedef UInt32 QTSS_ActionFlags;

// Events
enum
{
    QTSS_ReadableEvent      = 1,
    QTSS_WriteableEvent     = 2
};
typedef UInt32  QTSS_EventType;


/**********************************/
//HTTP SESSION TYPES
enum
{
    EasyHTTPSession			= 0,
	EasyCameraSession		= 1,
	EasyNVRSessioin			= 2
};
typedef UInt32 Easy_SessionType;

/**********************************/
// PAYLOAD TYPES
//
// When a module adds an RTP stream to a client session, it must specify
// the stream's payload type. This is so that other modules can find out
// this information in a generalized fashion. Here are the currently
// defined payload types
enum
{
    qtssUnknownPayloadType  = 0,
    qtssVideoPayloadType    = 1,
    qtssAudioPayloadType    = 2
};
typedef UInt32 QTSS_RTPPayloadType;

/**********************************/
// QTSS API OBJECT TYPES
enum
{
    qtssDynamicObjectType           = FOUR_CHARS_TO_INT('d', 'y', 'm', 'c'), //dymc
    qtssRTSPSessionObjectType       = FOUR_CHARS_TO_INT('s', 's', 'e', 'o'), //sseo
    qtssServerObjectType            = FOUR_CHARS_TO_INT('s', 'e', 'r', 'o'), //sero
    qtssPrefsObjectType             = FOUR_CHARS_TO_INT('p', 'r', 'f', 'o'), //prfo
    qtssTextMessagesObjectType      = FOUR_CHARS_TO_INT('t', 'x', 't', 'o'), //txto
    qtssFileObjectType              = FOUR_CHARS_TO_INT('f', 'i', 'l', 'e'), //file
    qtssModuleObjectType            = FOUR_CHARS_TO_INT('m', 'o', 'd', 'o'), //modo
    qtssModulePrefsObjectType       = FOUR_CHARS_TO_INT('m', 'o', 'd', 'p'), //modp
    qtssAttrInfoObjectType          = FOUR_CHARS_TO_INT('a', 't', 't', 'r'), //attr
    qtssUserProfileObjectType       = FOUR_CHARS_TO_INT('u', 's', 'p', 'o'), //uspo
    qtssConnectedUserObjectType     = FOUR_CHARS_TO_INT('c', 'u', 's', 'r'), //cusr    
};
typedef UInt32 QTSS_ObjectType;

/**********************************/
// ERROR LOG VERBOSITIES
//
// This provides some information to the module on the priority or
// type of this error message.
//
// When modules write to the error log stream (see below),
// the verbosity is qtssMessageVerbosity.
enum
{
    qtssFatalVerbosity              = 0,
    qtssWarningVerbosity            = 1,
    qtssMessageVerbosity            = 2,
    qtssAssertVerbosity             = 3,
    qtssDebugVerbosity              = 4,
    
    qtssIllegalVerbosity            = 5
};
typedef UInt32 QTSS_ErrorVerbosity;

enum
{
    qtssOpenFileNoFlags =       0,
    qtssOpenFileAsync =         1,  // File stream will be asynchronous (read may return QTSS_WouldBlock)
    qtssOpenFileReadAhead =     2   // File stream will be used for a linear read through the file.
};
typedef UInt32 QTSS_OpenFileFlags;


/**********************************/
// SERVER STATES
//
//  An attribute in the QTSS_ServerObject returns the server state
//  as a QTSS_ServerState. Modules may also set the server state.
//
//  Setting the server state to qtssFatalErrorState, or qtssShuttingDownState
//  will cause the server to quit.
//
//  Setting the state to qtssRefusingConnectionsState will cause the server
//  to start refusing new connections.
enum
{
    qtssStartingUpState             = 0,
    qtssRunningState                = 1,
    qtssRefusingConnectionsState    = 2,
    qtssFatalErrorState             = 3,//a fatal error has occurred, not shutting down yet
    qtssShuttingDownState           = 4,
    qtssIdleState                   = 5 // Like refusing connections state, but will also kill any currently connected clients
};
typedef UInt32 QTSS_ServerState;

/**********************************/
// ILLEGAL ATTRIBUTE ID
enum
{
    qtssIllegalAttrID               = -1,
    qtssIllegalServiceID            = -1
};

//*********************************/
// QTSS DON'T CALL SENDPACKETS AGAIN
// If this time is specified as the next packet time when returning
// from QTSS_SendPackets_Role, the module won't get called again in
// that role until another QTSS_Play is issued
enum
{
    qtssDontCallSendPacketsAgain    = -1
};

// DATA TYPES
enum
{
    qtssAttrDataTypeUnknown         = 0,
    qtssAttrDataTypeCharArray       = 1,
    qtssAttrDataTypeBool16          = 2,
    qtssAttrDataTypeSInt16          = 3,
    qtssAttrDataTypeUInt16          = 4,
    qtssAttrDataTypeSInt32          = 5,
    qtssAttrDataTypeUInt32          = 6,
    qtssAttrDataTypeSInt64          = 7,
    qtssAttrDataTypeUInt64          = 8,
    qtssAttrDataTypeQTSS_Object     = 9,
    qtssAttrDataTypeQTSS_StreamRef  = 10,
    qtssAttrDataTypeFloat32         = 11,
    qtssAttrDataTypeFloat64         = 12,
    qtssAttrDataTypeVoidPointer     = 13,
    qtssAttrDataTypeTimeVal         = 14,
    
    qtssAttrDataTypeNumTypes        = 15
};
typedef UInt32 QTSS_AttrDataType;

enum
{
    qtssAttrModeRead                = 1,
    qtssAttrModeWrite               = 2,
    qtssAttrModePreempSafe          = 4,
    qtssAttrModeInstanceAttrAllowed = 8,
    qtssAttrModeCacheable           = 16,
    qtssAttrModeDelete              = 32
};
typedef UInt32 QTSS_AttrPermission;

enum
{
    qtssAttrRightNone           = 0,    
    qtssAttrRightRead           = 1 << 0,
    qtssAttrRightWrite          = 1 << 1,
    qtssAttrRightAdmin          = 1 << 2,
    qtssAttrRightExtended       = 1 << 30, // Set this flag in the qtssUserRights when defining a new right. The right is a string i.e. "myauthmodule.myright" store the string in the QTSS_UserProfileObject attribute qtssUserExtendedRights
    qtssAttrRightQTSSExtended   = 1 << 31  // This flag is reserved for future use by the server. Additional rights are stored in qtssUserQTSSExtendedRights.
};
typedef UInt32 QTSS_AttrRights; // see QTSS_UserProfileObject


/**********************************/
//BUILT IN SERVER ATTRIBUTES

//The server maintains many attributes internally, and makes these available to plug-ins.
//Each value is a standard attribute, with a name and everything. Plug-ins may resolve the id's of
//these values by name if they'd like, but in the initialize role they will receive a struct of
//all the ids of all the internally maintained server parameters. This enumerated type block defines the indexes
//in that array for the id's.

enum
{
    //QTSS_RTSPSessionObject parameters
    
    //Valid in any role that receives a QTSS_RTSPSessionObject
    EasyHTTPSesIndex		= 0,        //read      //UInt32        //This is a unique ID for each session since the server started up.
    EasyHTTPSesLocalAddr    = 1,        //read      //UInt32        //Local IP address for this RTSP connection
    EasyHTTPSesLocalAddrStr = 2,        //read      //char array	//Ditto, in dotted-decimal format.
    EasyHTTPSesLocalDNS     = 3,        //read      //char array	//DNS name of local IP address for this RTSP connection.
    EasyHTTPSesRemoteAddr   = 4,        //read      //UInt32        //IP address of client.
    EasyHTTPSesRemoteAddrStr= 5,        //read      //char array	//IP address addr of client, in dotted-decimal format.
    EasyHTTPSesEventCntxt   = 6,        //read      //QTSS_EventContextRef		//An event context for the HTTP connection to the client. This should primarily be used to wait for EV_WR events if flow-controlled when responding to a client. 
    EasyHTTPSesType         = 7,        //read      //QTSS_RTSPSession			//Is this a normal RTSP session, or is it a HTTP tunnelled RTSP session?
    EasyHTTPSesSerial	    = 8,        //read      //QTSS_RTSPSessionStream	// A QTSS_StreamRef used for sending data to the RTSP client.

    EasyHTTPSessionID		= 9,		//read     //char array			// Private
    EasyHTTPSesLocalPort    = 10,       //read      //UInt16			// This is the local port for the connection
    EasyHTTPSesRemotePort   = 11,       //read      //UInt16			// This is the client port for the connection

	EasyHTTPSesContentBody	= 12,		//read		//char array
	EasyHTTPSesContentBodyOffset = 13,	//read		//int

    EasyHTTPSesNumParams    = 14
};
typedef UInt32 Easy_HTTPSessionAttributes;

enum
{
    //QTSS_ServerObject parameters
    
    // These parameters ARE pre-emptive safe.
    
    qtssServerAPIVersion            = 0,    //read  //UInt32            //The API version supported by this server (format 0xMMMMmmmm, where M=major version, m=minor version)
    qtssSvrDefaultDNSName           = 1,    //read  //char array        //The "default" DNS name of the server
    qtssSvrDefaultIPAddr            = 2,    //read  //UInt32            //The "default" IP address of the server
    qtssSvrServerName               = 3,    //read  //char array        //Name of the server
    qtssSvrServerVersion            = 4,    //read  //char array        //Version of the server
    qtssSvrServerBuildDate          = 5,    //read  //char array        //When was the server built?
    qtssSvrRTSPPorts                = 6,    //read  // NOT PREEMPTIVE SAFE!//UInt16         //Indexed parameter: all the ports the server is listening on
    qtssSvrRTSPServerHeader         = 7,    //read  //char array        //Server: header that the server uses to respond to RTSP clients

    // These parameters are NOT pre-emptive safe, they cannot be accessed
    // via. QTSS_GetValuePtr. Some exceptions noted below
    
    qtssSvrState                    = 8,    //r/w   //QTSS_ServerState  //The current state of the server. If a module sets the server state, the server will respond in the appropriate fashion. Setting to qtssRefusingConnectionsState causes the server to refuse connections, setting to qtssFatalErrorState or qtssShuttingDownState causes the server to quit.
    qtssSvrIsOutOfDescriptors       = 9,    //read  //Bool16            //true if the server has run out of file descriptors, false otherwise
    qtssCurrentSessionCount			= 10,   //read  //UInt32            //Current number of connected clients over standard RTSP
    
    qtssSvrHandledMethods           = 11,   //r/w       //QTSS_RTSPMethod   //The methods that the server supports. Modules should append the methods they support to this attribute in their QTSS_Initialize_Role.
    qtssSvrModuleObjects            = 12,   //read  // this IS PREMPTIVE SAFE!  //QTSS_ModuleObject // A module object representing each module
    qtssSvrStartupTime              = 13,   //read      //QTSS_TimeVal  //Time the server started up
    qtssSvrGMTOffsetInHrs           = 14,   //read      //SInt32        //Server time zone (offset from GMT in hours)
    qtssSvrDefaultIPAddrStr         = 15,   //read      //char array    //The "default" IP address of the server as a string

    qtssSvrPreferences              = 16,   //read      //QTSS_PrefsObject  // An object representing each the server's preferences
    qtssSvrMessages                 = 17,   //read      //QTSS_Object   // An object containing the server's error messages.
    qtssSvrClientSessions           = 18,   //read      //QTSS_Object // An object containing all client sessions stored as indexed QTSS_ClientSessionObject(s).
    qtssSvrCurrentTimeMilliseconds  = 19,   //read      //QTSS_TimeVal  //Server's current time in milliseconds. Retrieving this attribute is equivalent to calling QTSS_Milliseconds
    qtssSvrCPULoadPercent           = 20,   //read      //Float32       //Current % CPU being used by the server

	qtssSvrConnectedUsers           = 21,   //r/w       //QTSS_Object   //List of connected user sessions (updated by modules for their sessions)

	qtssSvrServerBuild              = 22,   //read      //char array //build of the server
    qtssSvrServerPlatform           = 23,   //read      //char array //Platform (OS) of the server
    qtssSvrRTSPServerComment        = 24,   //read      //char array //RTSP comment for the server header    
    qtssSvrNumThinned               = 25,   //read      //SInt32    //Number of thinned sessions
    qtssSvrNumThreads               = 26,   //read     //UInt32    //Number of task threads // see also qtssPrefsRunNumThreads
    qtssSvrNumParams                = 27
};
typedef UInt32 QTSS_ServerAttributes;

enum
{
    //QTSS_PrefsObject parameters

    // Valid in all methods. None of these are pre-emptive safe, so the version
    // of QTSS_GetAttribute that copies data must be used.
    
    // All of these parameters are read-write. 
    
    qtssPrefsRTSPTimeout            = 0,    //"rtsp_timeout"                //UInt32    //RTSP timeout in seconds sent to the client.
    qtssPrefsSessionTimeout			= 1,	//"session_timeout"				//UInt32    //Amount of time in seconds the server will wait before disconnecting idle RTSP clients. 0 means no timeout
    qtssPrefsRTPTimeout             = 2,    //"rtp_timeout"                 //UInt32    //Amount of time in seconds the server will wait before disconnecting idle RTP clients. 0 means no timeout
    qtssPrefsMaximumConnections     = 3,    //"maximum_connections"         //SInt32    //Maximum # of concurrent RTP connections allowed by the server. -1 means unlimited.
    qtssPrefsMaximumBandwidth       = 4,    //"maximum_bandwidth"           //SInt32    //Maximum amt of bandwidth the server is allowed to serve in K bits. -1 means unlimited.
    qtssPrefsCMSIPAddr				= 5,    //"cms_ip_addr"                //char array    //Path to the root movie folder
    qtssPrefsBindIPAddr             = 6,    //"bind_ip_addr"                //char array    //IP address the server should accept RTSP connections on. 0.0.0.0 means all addresses on the machine.
    qtssPrefsBreakOnAssert          = 7,    //"break_on_assert"             //Bool16        //If true, the server will break in the debugger when an assert fails.
    qtssPrefsAutoRestart            = 8,    //"auto_restart"                //Bool16        //If true, the server will automatically restart itself if it crashes.
    qtssPrefsTotalBytesUpdate       = 9,    //"total_bytes_update"          //UInt32    //Interval in seconds between updates of the server's total bytes and current bandwidth statistics
    qtssPrefsAvgBandwidthUpdate     = 10,   //"average_bandwidth_update"    //UInt32    //Interval in seconds between computations of the server's average bandwidth
    qtssPrefsSafePlayDuration       = 11,   //"safe_play_duration"          //UInt32    //Hard to explain... see streamingserver.conf
    qtssPrefsModuleFolder           = 12,   //"module_folder"               //char array    //Path to the module folder

    // There is a compiled-in error log module that loads before all the other modules
    // (so it can log errors from the get-go). It uses these prefs.
    
    qtssPrefsErrorLogName           = 13,   //"error_logfile_name"          //char array        //Name of error log file
    qtssPrefsErrorLogDir            = 14,   //"error_logfile_dir"           //char array        //Path to error log file directory
    qtssPrefsErrorRollInterval      = 15,   //"error_logfile_interval"      //UInt32    //Interval in days between error logfile rolls
    qtssPrefsMaxErrorLogSize        = 16,   //"error_logfile_size"          //UInt32    //Max size in bytes of the error log
    qtssPrefsErrorLogVerbosity      = 17,   //"error_logfile_verbosity"     //UInt32    //Max verbosity level of messages the error logger will log
    qtssPrefsScreenLogging          = 18,   //"screen_logging"              //Bool16        //Should the error logger echo messages to the screen?
    qtssPrefsErrorLogEnabled        = 19,   //"error_logging"               //Bool16        //Is error logging enabled?

    qtssPrefsServiceID				= 20,   //"service_id"    //SInt32 // Don't send video packets later than this
    qtssPrefsStartThinningDelayInMsec       = 21,   //"start_thinning_delay"    //SInt32 // lateness at which we might start thinning
    qtssPrefsLargeWindowSizeInK             = 22,   //"large_window_size"  // UInt32    //default size that will be used for high bitrate movies
    qtssPrefsWindowSizeThreshold            = 23,   //"window_size_threshold"  // UInt32    //bitrate at which we switch to larger window size
    
    qtssPrefsMinTCPBufferSizeInBytes        = 24,   //"min_tcp_buffer_size" //UInt32    // When streaming over TCP, this is the minimum size the TCP socket send buffer can be set to
    qtssPrefsMaxTCPBufferSizeInBytes        = 25,   //"max_tcp_buffer_size" //UInt32    // When streaming over TCP, this is the maximum size the TCP socket send buffer can be set to
    qtssPrefsTCPSecondsToBuffer             = 26,   //"tcp_seconds_to_buffer" //Float32 // When streaming over TCP, the size of the TCP send buffer is scaled based on the bitrate of the movie. It will fit all the data that gets sent in this amount of time.
    
    qtssPrefsCMSPort						= 27,   //"cms_port"    //UInt16    // when behind a round robin DNS, the client needs to be told the specific ip address of the maching handling its request. this pref tells the server to repot its IP address in the reply to the HTTP GET request when tunneling RTSP through HTTP

    qtssPrefsRedisIPAddr					= 28,   // "redis_ip_addr" //char array   //
    
    qtssPrefsRunUserName                    = 29,   //"run_user_name"       //char array        //Run under this user's account
    qtssPrefsRunGroupName                   = 30,   //"run_group_name"      //char array        //Run under this group's account
    
    qtssPrefsSrcAddrInTransport             = 31,   //"append_source_addr_in_transport" // Bool16   //If true, the server will append the src address to the Transport header responses
    qtssPrefsRedisPorts						= 32,   //"redis_port"          // UInt16   

    qtssPrefsMaxRetransDelayInMsec          = 33,   //"max_retransmit_delay" // UInt32  //maximum interval between when a retransmit is supposed to be sent and when it actually gets sent. Lower values means smoother flow but slower server performance
    qtssPrefsSmallWindowSizeInK             = 34,   //"small_window_size"  // UInt32    //default size that will be used for low bitrate movies
    qtssPrefsAckLoggingEnabled              = 35,   //"ack_logging_enabled"  // Bool16  //Debugging only: turns on detailed logging of 
    qtssPrefsSnapLocalPath					= 36,   //"snap_local_path"		// char array   //快照本地存储路径
    qtssPrefsSnapWebPath			        = 37,   //"snap_web_path"		// char array   //快照网络存储路径
    qtssPrefsSendInterval                   = 38,   //"send_interval"  // UInt32    //
    qtssPrefsThickAllTheWayDelayInMsec      = 39,   //"thick_all_the_way_delay"     // UInt32   //
    qtssPrefsAltTransportIPAddr             = 40,   //"alt_transport_src_ipaddr"// char     //If empty, the server uses its own IP addr in the source= param of the transport header. Otherwise, it uses this addr.
    qtssPrefsMaxAdvanceSendTimeInSec        = 41,   //"max_send_ahead_time"     // UInt32   //This is the farthest in advance the server will send a packet to a client that supports overbuffering.

	
	qtssPrefsAuthenticationScheme           = 42,   //"authentication_scheme" // char   //Set this to be the authentication scheme you want the server to use. "basic", "digest", and "none" are the currently supported values

	qtssPrefsAutoStart                      = 43,   //"auto_start" //Bool16 //If true, streaming server likes to be started at system startup
       
	qtssPrefsEnableMSGDebugPrintfs         = 44,	//"MSG_debug_printfs" //Boo1l6 // printfs incoming RTSPRequests and Outgoing RTSP responses.

    qtssPrefsEnableMonitorStatsFile         = 45,   //"enable_monitor_stats_file" //Bool16 //write server stats to the monitor file
    qtssPrefsMonitorStatsFileIntervalSec    = 46,   //"monitor_stats_file_interval_seconds" // private
    qtssPrefsMonitorStatsFileName           = 47,   //"monitor_stats_file_name" // private

	qtssPrefsOverbufferRate                 = 48,    //"overbuffer_rate"    //Float32
    qtssPrefsMediumWindowSizeInK            = 49,    //"medium_window_size" // UInt32    //default size that will be used for medium bitrate movies
    qtssPrefsWindowSizeMaxThreshold         = 50,    //"window_size_threshold"  // UInt32    //bitrate at which we switch from medium to large window size
    qtssPrefsEnableCMSServerInfo			= 51,   //"CMS_server_info" //Boo1l6 // Adds server info to the RTSP responses.
    qtssPrefsRunNumThreads                  = 52,   //"run_num_threads" //UInt32 // if value is non-zero, will  create that many task threads; otherwise a thread will be created for each processor
    qtssPrefsPidFile                        = 53,    //"pid_file" //Char Array //path to pid file
    qtssPrefsCloseLogsOnWrite               = 54,   // "force_logs_close_on_write" //Bool16 // force log files to close after each write.
    qtssPrefsDisableThinning                = 55,   // "disable_thinning" //Bool16 // Usually used for performance testing. Turn off stream thinning from packet loss or stream lateness.
    qtssPrefsMonitorLANPort					= 56,   // "monitor_lan_port" //UInt16 // localhost destination port of reflected stream
    qtssPrefsMonitorWANPort					= 57,   // "monitor_wan_port" //UInt16 // localhost destination port of reflected stream
    qtssPrefsMonitorLANIPAddr				= 58,   // "monitor_lan_ip"    //char array    //IP address the server should send RTP monitor reflected streams. 
    qtssPrefsMonitorWANIPAddr				= 59,   // "monitor_wan_ip"    //char array    //client IP address the server monitor should reflect. *.*.*.* means all client addresses.
    qtssPrefsEnableAllowGuestDefault        = 60,   // "enable_allow_guest_authorize_default" //Boo1l6 // server hint to access modules to allow guest access as the default (can be overriden in a qtaccess file or other means)
    qtssPrefsNumMsgThreads					= 61,   // "run_num_msg_threads" //UInt32 // if value is non-zero, the server will  create that many task threads; otherwise a single thread will be created.
	qtssPrefsDssIP							= 62,	// "dss_ip"		 //char array 
	qtssPrefsDssPort						= 63,	// "dss_port"	//UInt16 

    qtssPrefsNumParams                      = 64
};

typedef UInt32 QTSS_PrefsAttributes;

enum
{
    //QTSS_TextMessagesObject parameters
    
    // All of these parameters are read-only, char*'s, and preemptive-safe.
    
    qtssMsgNoMessage                = 0,    //"NoMessage"
    qtssMsgNoURLInRequest           = 1,
    qtssMsgBadRTSPMethod            = 2,
    qtssMsgNoRTSPVersion            = 3,
    qtssMsgNoRTSPInURL              = 4,
    qtssMsgURLTooLong               = 5,
    qtssMsgURLInBadFormat           = 6,
    qtssMsgNoColonAfterHeader       = 7,
    qtssMsgNoEOLAfterHeader         = 8,
    qtssMsgRequestTooLong           = 9,
    qtssMsgNoModuleFolder           = 10,
    qtssMsgCouldntListen            = 11,
    qtssMsgInitFailed               = 12,
    qtssMsgNotConfiguredForIP       = 13,
    qtssMsgDefaultRTSPAddrUnavail   = 14,
    qtssMsgBadModule                = 15,
    qtssMsgRegFailed                = 16,
    qtssMsgRefusingConnections      = 17,
    qtssMsgTooManyClients           = 18,
    qtssMsgTooMuchThruput           = 19,
    qtssMsgNoSessionID              = 20,
    qtssMsgFileNameTooLong          = 21,
    qtssMsgNoClientPortInTransport  = 22,


    qtssMsgOutOfPorts               = 23,
    qtssMsgNoModuleForRequest       = 24,
    qtssMsgAltDestNotAllowed        = 25,
    qtssMsgCantSetupMulticast       = 26,
    qtssListenPortInUse             = 27,
    qtssListenPortAccessDenied      = 28,
    qtssListenPortError             = 29,
    qtssMsgBadBase64                = 30,
    qtssMsgSomePortsFailed          = 31,
    qtssMsgNoPortsSucceeded         = 32,
    qtssMsgCannotCreatePidFile      = 33,
    qtssMsgCannotSetRunUser         = 34,
    qtssMsgCannotSetRunGroup        = 35,
    qtssMsgNoSesIDOnDescribe        = 36,
    qtssServerPrefMissing           = 37,
    qtssServerPrefWrongType         = 38,
    qtssMsgCantWriteFile            = 39,
    qtssMsgSockBufSizesTooLarge     = 40,
    qtssMsgBadFormat                = 41,
    qtssMsgNumParams                = 42
    
};
typedef UInt32 QTSS_TextMessagesAttributes;

enum
{
    //QTSS_FileObject parameters
    
    // All of these parameters are preemptive-safe.
    
    qtssFlObjStream                 = 0,    // read // QTSS_FileStream. Stream ref for this file object
    qtssFlObjFileSysModuleName      = 1,    // read // char array. Name of the file system module handling this file object
    qtssFlObjLength                 = 2,    // r/w  // UInt64. Length of the file
    qtssFlObjPosition               = 3,    // read // UInt64. Current position of the file pointer in the file.
    qtssFlObjModDate                = 4,    // r/w  // QTSS_TimeVal. Date & time of last modification

    qtssFlObjNumParams              = 5
};
typedef UInt32 QTSS_FileObjectAttributes;

enum
{
    //QTSS_ModuleObject parameters
    
    qtssModName                 = 0,    //read      //preemptive-safe       //char array        //Module name. 
    qtssModDesc                 = 1,    //r/w       //not preemptive-safe   //char array        //Text description of what the module does
    qtssModVersion              = 2,    //r/w       //not preemptive-safe   //UInt32            //Version of the module. UInt32 format should be 0xMM.m.v.bbbb M=major version m=minor version v=very minor version b=build #
    qtssModRoles                = 3,    //read      //preemptive-safe       //QTSS_Role         //List of all the roles this module has registered for.
    qtssModPrefs                = 4,    //read      //preemptive-safe       //QTSS_ModulePrefsObject //An object containing as attributes the preferences for this module
    qtssModAttributes           = 5,    //read      //preemptive-safe       //QTSS_Object
            
    qtssModNumParams            = 6
};
typedef UInt32 QTSS_ModuleObjectAttributes;

enum
{
    //QTSS_AttrInfoObject parameters
    
    // All of these parameters are preemptive-safe.

    qtssAttrName                    = 0,    //read //char array             //Attribute name
    qtssAttrID                      = 1,    //read //QTSS_AttributeID       //Attribute ID
    qtssAttrDataType                = 2,    //read //QTSS_AttrDataType      //Data type
    qtssAttrPermissions             = 3,    //read //QTSS_AttrPermission    //Permissions

    qtssAttrInfoNumParams           = 4
};
typedef UInt32 QTSS_AttrInfoObjectAttributes;

enum
{
    //QTSS_UserProfileObject parameters
    
    // All of these parameters are preemptive-safe.
    
    qtssUserName                = 0, //read  //char array
    qtssUserPassword            = 1, //r/w   //char array
    qtssUserGroups              = 2, //r/w   //char array -  multi-valued attribute, all values should be C strings padded with \0s to                                         //              make them all of the same length 
    qtssUserRealm               = 3, //r/w   //char array -  the authentication realm for username
    qtssUserRights              = 4, //r/w   //QTSS_AttrRights - rights granted this user
    qtssUserExtendedRights      = 5, //r/w   //qtssAttrDataTypeCharArray - a list of strings with extended rights granted to the user.
    qtssUserQTSSExtendedRights  = 6, //r/w   //qtssAttrDataTypeCharArray - a private list of strings with extended rights granted to the user and reserved by QTSS/Apple.
    qtssUserNumParams           = 7,
};
typedef UInt32 QTSS_UserProfileObjectAttributes;

enum
{
    //QTSS_ConnectedUserObject parameters
    
    //All of these are preemptive safe
    
    qtssConnectionType                  = 0,    //read      //char array    // type of user connection (e.g. "RTP reflected" or "MP3")
    qtssConnectionCreateTimeInMsec      = 1,    //read      //QTSS_TimeVal  //Time in milliseconds the session was created.
    qtssConnectionTimeConnectedInMsec   = 2,    //read      //QTSS_TimeVal  //Time in milliseconds the session was created.
    qtssConnectionBytesSent             = 3,    //read      //UInt32        //Number of RTP bytes sent so far on this session.
    qtssConnectionMountPoint            = 4,    //read      //char array    //Presentation URL for this session. This URL is the "base" URL for the session. RTSP requests to this URL are assumed to affect all streams on the session.
    qtssConnectionHostName              = 5,    //read      //char array    //host name for this request

    qtssConnectionSessRemoteAddrStr     = 6,    //read      //char array        //IP address addr of client, in dotted-decimal format.
    qtssConnectionSessLocalAddrStr      = 7,    //read      //char array        //Ditto, in dotted-decimal format.

    qtssConnectionCurrentBitRate        = 8,    //read          //UInt32    //Current bit rate of all the streams on this session. This is not an average. In bits per second.
    qtssConnectionPacketLossPercent     = 9,    //read          //Float32   //Current percent loss as a fraction. .5 = 50%. This is not an average.

    qtssConnectionTimeStorage           = 10,   //read          //QTSS_TimeVal  //Internal, use qtssConnectionTimeConnectedInMsec above

    qtssConnectionNumParams             = 11
};
typedef UInt32 QTSS_ConnectedUserObjectAttributes;


/********************************************************************/
// QTSS API ROLES
//
// Each role represents a unique situation in which a module may be
// invoked. Modules must specify which roles they want to be invoked for. 

enum
{
    //Global
    QTSS_Register_Role =             FOUR_CHARS_TO_INT('r', 'e', 'g', ' '), //reg  //All modules get this once at startup
    QTSS_Initialize_Role =           FOUR_CHARS_TO_INT('i', 'n', 'i', 't'), //init //Gets called once, later on in the startup process
    QTSS_Shutdown_Role =             FOUR_CHARS_TO_INT('s', 'h', 'u', 't'), //shut //Gets called once at shutdown
    
    QTSS_ErrorLog_Role =             FOUR_CHARS_TO_INT('e', 'l', 'o', 'g'), //elog //This gets called when the server wants to log an error.
    QTSS_RereadPrefs_Role =          FOUR_CHARS_TO_INT('p', 'r', 'e', 'f'), //pref //This gets called when the server rereads preferences.
    QTSS_StateChange_Role =          FOUR_CHARS_TO_INT('s', 't', 'a', 't'), //stat //This gets called whenever the server changes state.
    
    QTSS_Interval_Role =             FOUR_CHARS_TO_INT('t', 'i', 'm', 'r'), //timr //This gets called whenever the module's interval timer times out calls.
    
    //File system roles
    QTSS_OpenFilePreProcess_Role =  FOUR_CHARS_TO_INT('o', 'p', 'p', 'r'),  //oppr
    QTSS_OpenFile_Role =            FOUR_CHARS_TO_INT('o', 'p', 'f', 'l'),  //opfl
    QTSS_AdviseFile_Role =          FOUR_CHARS_TO_INT('a', 'd', 'f', 'l'),  //adfl
    QTSS_ReadFile_Role =            FOUR_CHARS_TO_INT('r', 'd', 'f', 'l'),  //rdfl
    QTSS_CloseFile_Role =           FOUR_CHARS_TO_INT('c', 'l', 'f', 'l'),  //clfl
    QTSS_RequestEventFile_Role =    FOUR_CHARS_TO_INT('r', 'e', 'f', 'l'),  //refl
    //add,角色的添加
	QTSS_NONCE_ROLE    =            FOUR_CHARS_TO_INT('o','n','c','e'),//once
	QTSS_AUTH_ROLE     =            FOUR_CHARS_TO_INT('a','u','t','r')//autr,auth已经使用
};
typedef UInt32 QTSS_Role;


//***********************************************/
// TYPEDEFS

typedef void*           QTSS_StreamRef;
typedef void*           QTSS_Object;
typedef void*           QTSS_ServiceFunctionArgsPtr;
typedef SInt32          QTSS_AttributeID;
typedef SInt32          QTSS_ServiceID;
typedef SInt64          QTSS_TimeVal;

typedef QTSS_Object             QTSS_RTSPSessionObject;
typedef QTSS_Object             QTSS_RTSPRequestObject;
typedef QTSS_Object             QTSS_RTSPHeaderObject;
typedef QTSS_Object             QTSS_ServerObject;
typedef QTSS_Object             QTSS_PrefsObject;
typedef QTSS_Object             QTSS_TextMessagesObject;
typedef QTSS_Object             QTSS_FileObject;
typedef QTSS_Object             QTSS_ModuleObject;
typedef QTSS_Object             QTSS_ModulePrefsObject;
typedef QTSS_Object             QTSS_AttrInfoObject;
typedef QTSS_Object             QTSS_UserProfileObject;
typedef QTSS_Object             QTSS_ConnectedUserObject;

typedef QTSS_StreamRef          QTSS_ErrorLogStream;
typedef QTSS_StreamRef          QTSS_FileStream;
typedef QTSS_StreamRef          QTSS_SocketStream;

//***********************************************/
// ROLE PARAMETER BLOCKS
//
// Each role has a unique set of parameters that get passed
// to the module.

typedef struct
{
    char outModuleName[QTSS_MAX_MODULE_NAME_LENGTH];
} QTSS_Register_Params;

typedef struct
{
    QTSS_ServerObject           inServer;           // Global dictionaries
    QTSS_PrefsObject            inPrefs;
    QTSS_TextMessagesObject     inMessages;
    QTSS_ErrorLogStream         inErrorLogStream;   // Writing to this stream causes modules to
                                                    // be invoked in the QTSS_ErrorLog_Role
    QTSS_ModuleObject           inModule;
} QTSS_Initialize_Params;

typedef struct
{
    QTSS_ErrorVerbosity         inVerbosity;
    char*                       inBuffer;
    
} QTSS_ErrorLog_Params;

typedef struct
{
    QTSS_ServerState            inNewState;
} QTSS_StateChange_Params;

typedef struct 
{
    QTSS_RTSPSessionObject      inRTSPSession;
    QTSS_RTSPRequestObject      inRTSPRequest;
    char**                      outNewRequest;

} QTSS_Filter_Params;

typedef struct
{
    char*                       inPath;
    QTSS_OpenFileFlags          inFlags;
    QTSS_Object                 inFileObject;
} QTSS_OpenFile_Params;

typedef struct
{
    QTSS_Object                 inFileObject;
    UInt64                      inPosition;
    UInt32                      inSize;
} QTSS_AdviseFile_Params;

typedef struct
{
    QTSS_Object                 inFileObject;
    UInt64                      inFilePosition;
    void*                       ioBuffer;
    UInt32                      inBufLen;
    UInt32*                     outLenRead;
} QTSS_ReadFile_Params;

typedef struct
{
    QTSS_Object                 inFileObject;
} QTSS_CloseFile_Params;

typedef struct
{
    QTSS_Object                 inFileObject;
    QTSS_EventType              inEventMask;
} QTSS_RequestEventFile_Params;

typedef struct
{
	QTSS_Object					inStreamName;
	QTSS_Object					inRTSPSession;
	QTSS_Object					inStreamingParam;
} QTSS_Streaming_Params;
typedef struct//add  
{
	char *pNonce;//用于存放随机数
	char *pResult;//存放验证随机数的结构
}QTSS_Nonce_Params;

typedef struct
{
	UInt32						currentPlaying;
} QTSS_Sync_Params;

typedef union
{
    QTSS_Register_Params                regParams;
    QTSS_Initialize_Params              initParams;
    QTSS_ErrorLog_Params                errorParams;
    QTSS_StateChange_Params             stateChangeParams;

    QTSS_Filter_Params                  rtspFilterParams;
    
    QTSS_OpenFile_Params                openFilePreProcessParams;
    QTSS_OpenFile_Params                openFileParams;
    QTSS_AdviseFile_Params              adviseFileParams;
    QTSS_ReadFile_Params                readFileParams;
    QTSS_CloseFile_Params               closeFileParams;
    QTSS_RequestEventFile_Params        reqEventFileParams;

	// EasyCMS
	QTSS_Streaming_Params				StreamingParams;
	QTSS_Sync_Params					SyncParams;
    QTSS_Nonce_Params                   NonceParams;
} QTSS_RoleParams, *QTSS_RoleParamPtr;


/********************************************************************/
// ENTRYPOINTS & FUNCTION TYPEDEFS

// MAIN ENTRYPOINT FOR MODULES
//
// Every QTSS API must implement two functions: a main entrypoint, and a dispatch
// function. The main entrypoint gets called by the server at startup to do some
// initialization. Your main entrypoint must follow the convention established below
//
// QTSS_Error mymodule_main(void* inPrivateArgs)
// {
//      return _stublibrary_main(inPrivateArgs, MyDispatchFunction);
// }
//
//

typedef QTSS_Error (*QTSS_MainEntryPointPtr)(void* inPrivateArgs);
typedef QTSS_Error (*QTSS_DispatchFuncPtr)(QTSS_Role inRole, QTSS_RoleParamPtr inParamBlock);

// STUB LIBRARY MAIN
QTSS_Error _stublibrary_main(void* inPrivateArgs, QTSS_DispatchFuncPtr inDispatchFunc);

/********************************************************************/
//  QTSS_New
//  QTSS_Delete
//
//  These should be used for all dynamic memory allocation done from
//  within modules. The memoryIdentifier is used for debugging:
//  the server can track this memory to make memory leak debugging easier.
void*   QTSS_New(FourCharCode inMemoryIdentifier, UInt32 inSize);
void    QTSS_Delete(void* inMemory);

/********************************************************************/
//  QTSS_Milliseconds
//
//  The server maintains a millisecond timer internally. The current
//  value of that timer can be obtained from this function. This value
//  is not valid between server executions.
//
//  All millisecond values used in QTSS API use this timer, unless otherwise noted
QTSS_TimeVal    QTSS_Milliseconds();


/********************************************************************/
//  QTSS_MilliSecsTo1970Secs
//
//  Convert milliseconds from the QTSS_Milliseconds call to 
//  second's since 1970
//
time_t  QTSS_MilliSecsTo1970Secs(QTSS_TimeVal inQTSS_MilliSeconds);

/********************************************************************/
//  QTSS_AddRole
//
//  Only available from QTSS_Initialize role. Call this for all the roles you
//  would like your module to operate on.
//
//  Returns:    QTSS_NoErr
//              QTSS_OutOfState: If this function isn't being called from the Register role
//              QTSS_RequestFailed:     If module is registering for the QTSS_RTSPRequest_Role
//                                      and there already is such a module.
//              QTSS_BadArgument:   Registering for a nonexistent role.
QTSS_Error QTSS_AddRole(QTSS_Role inRole);


/*****************************************/
//  ATTRIBUTE / OBJECT CALLBACKS
//


/********************************************************************/
//  QTSS_LockObject
//
//  Grabs the mutex for this object so that accesses to the objects attributes
//  from other threads will block.  Note that objects created through QTSS_CreateObjectValue
//  will share a mutex with the parent object.
//
//
//  Returns:    QTSS_NoErr
//              QTSS_BadArgument:   bad object
QTSS_Error QTSS_LockObject(QTSS_Object inObject);
                                    
/********************************************************************/
//  QTSS_UnlockObject
//
//  Releases the mutex for this object.
//
//
//  Returns:    QTSS_NoErr
//              QTSS_BadArgument:   bad object
QTSS_Error QTSS_UnlockObject(QTSS_Object inObject);
                                    
/********************************************************************/
//  QTSS_CreateObjectType
//
//  Creates a new object type.  Attributes can be added to this object type and then it can
//  be passed into QTSS_CreateObjectValue.
//
//  This may only be called from the QTSS_Register role.
//
//  Returns:    QTSS_NoErr
//              QTSS_RequestFailed: Too many object types already exist.
QTSS_Error QTSS_CreateObjectType(QTSS_ObjectType* outType);
                                    
/********************************************************************/
//  QTSS_AddStaticAttribute
//
//  Adds a new static attribute to a predefined object type. All added attributes implicitly have
//  qtssAttrModeRead, qtssAttrModeWrite, and qtssAttrModePreempSafe permissions. "inUnused" should
//  always be NULL. Specify the data type and name of the attribute.
//
//  This may only be called from the QTSS_Register role.
//
//  Returns:    QTSS_NoErr
//              QTSS_OutOfState: If this function isn't being called from the Register role
//              QTSS_BadArgument:   Adding an attribute to a nonexistent object type, attribute
//                      name too long, or NULL arguments.
//              QTSS_AttrNameExists: The name must be unique.
QTSS_Error QTSS_AddStaticAttribute( QTSS_ObjectType inObjectType, char* inAttrName,
                void* inUnused, QTSS_AttrDataType inAttrDataType);
                
/********************************************************************/
//  QTSS_AddInstanceAttribute
//
//  Adds a new instance attribute to a predefined object type. All added attributes implicitly have
//  qtssAttrModeRead, qtssAttrModeWrite, and qtssAttrModePreempSafe permissions. "inUnused" should
//  always be NULL. Specify the data type and name of the attribute.
//
//  This may be called at any time.
//
//  Returns:    QTSS_NoErr
//              QTSS_OutOfState: If this function isn't being called from the Register role
//              QTSS_BadArgument:   Adding an attribute to a nonexistent object type, attribute
//                      name too long, or NULL arguments.
//              QTSS_AttrNameExists: The name must be unique.
QTSS_Error QTSS_AddInstanceAttribute(   QTSS_Object inObject, char* inAttrName,
        void* inUnused, QTSS_AttrDataType inAttrDataType);
                                        
/********************************************************************/
//  QTSS_RemoveInstanceAttribute
//
//  Removes an existing instance attribute. This may be called at any time
//
//  Returns:    QTSS_NoErr
//              QTSS_OutOfState: If this function isn't being called from the Register role
//              QTSS_BadArgument:   Bad object type.
//              QTSS_AttrDoesntExist: Bad attribute ID
QTSS_Error QTSS_RemoveInstanceAttribute(QTSS_Object inObject, QTSS_AttributeID inID);

/********************************************************************/
//  Getting attribute information
//
//  The following callbacks allow modules to discover at runtime what
//  attributes exist in which objects and object types, and discover
//  all attribute meta-data

/********************************************************************/
//  QTSS_IDForAttr
//
//  Given an attribute name, this returns its accompanying attribute ID.
//  The ID can in turn be used to retrieve the attribute value from
//  a object. This callback applies only to static attributes 
//
//  Returns:    QTSS_NoErr
//              QTSS_BadArgument: Bad argument
QTSS_Error QTSS_IDForAttr(QTSS_ObjectType inObjectType, const char* inAttributeName,
                            QTSS_AttributeID* outID);

/********************************************************************/
//  QTSS_GetAttrInfoByID
//
//  Searches for an attribute with the specified ID in the specified object.
//  If found, this function returns a QTSS_AttrInfoObject describing the attribute.
//
//  Returns:    QTSS_NoErr
//              QTSS_BadArgument
//              QTSS_AttrDoesntExist
QTSS_Error QTSS_GetAttrInfoByID(QTSS_Object inObject, QTSS_AttributeID inAttrID,
                                    QTSS_AttrInfoObject* outAttrInfoObject);

/********************************************************************/
//  QTSS_GetAttrInfoByName
//
//  Searches for an attribute with the specified name in the specified object.
//  If found, this function returns a QTSS_AttrInfoObject describing the attribute.
//
//  Returns:    QTSS_NoErr
//              QTSS_BadArgument
//              QTSS_AttrDoesntExist
QTSS_Error QTSS_GetAttrInfoByName(QTSS_Object inObject, char* inAttrName,
                                    QTSS_AttrInfoObject* outAttrInfoObject);

/********************************************************************/
//  QTSS_GetAttrInfoByIndex
//
//  Allows caller to iterate over all the attributes in the specified object.
//  Returns a QTSS_AttrInfoObject for the attribute with the given index (0.. num attributes).
//
//  Returns:    QTSS_NoErr
//              QTSS_BadArgument
//              QTSS_AttrDoesntExist
QTSS_Error QTSS_GetAttrInfoByIndex(QTSS_Object inObject, UInt32 inIndex,
                                    QTSS_AttrInfoObject* outAttrInfoObject);

/********************************************************************/
//  QTSS_GetNumAttributes
//
//  Returns the number of attributes in the specified object.
//
//  Returns:    QTSS_NoErr
//              QTSS_BadArgument: Bad argument
//
QTSS_Error QTSS_GetNumAttributes (QTSS_Object inObject, UInt32* outNumAttributes);

/********************************************************************/
//  QTSS_GetValuePtr
//
//  NOT TO BE USED WITH NON-PREEMPTIVE-SAFE attributes (or provide your own locking
//  using QTSS_LockObject).
//
//  Returns:    QTSS_NoErr
//              QTSS_BadArgument: Bad argument
//              QTSS_NotPreemptiveSafe: Attempt to get a non-preemptive safe attribute
//              QTSS_BadIndex: Attempt to get non-existent index.
QTSS_Error QTSS_GetValuePtr (QTSS_Object inObject, QTSS_AttributeID inID, UInt32 inIndex,
                                void** outBuffer, UInt32* outLen);

/********************************************************************/
//  QTSS_GetValue
//
//  Copies the data into provided buffer. If QTSS_NotEnoughSpace is returned, outLen is still set.
//
//  Returns:    QTSS_NoErr
//              QTSS_BadArgument: Bad argument
//              QTSS_NotEnoughSpace: Value is too big for buffer provided.
//              QTSS_BadIndex: Attempt to get non-existent index.
QTSS_Error QTSS_GetValue (QTSS_Object inObject, QTSS_AttributeID inID, UInt32 inIndex,
                            void* ioBuffer, UInt32* ioLen);

/********************************************************************/
//  QTSS_GetValueAsString
//
//  Returns the specified attribute converted to a C-string. This call allocates
//  memory for the string which should be disposed of using QTSS_Delete.
//
//  Returns:    QTSS_NoErr
//              QTSS_BadArgument: Bad argument
//              QTSS_BadIndex: Attempt to get non-existent index.
QTSS_Error QTSS_GetValueAsString (QTSS_Object inObject, QTSS_AttributeID inID, UInt32 inIndex,
                                    char** outString);


/********************************************************************/
//  QTSS_TypeStringToType
//  QTSS_TypeToTypeString
//
//  Returns a text name for the specified QTSS_AttrDataType, or vice-versa
//
//  Returns:    QTSS_NoErr
//              QTSS_BadArgument
QTSS_Error  QTSS_TypeStringToType(const char* inTypeString, QTSS_AttrDataType* outType);
QTSS_Error  QTSS_TypeToTypeString(const QTSS_AttrDataType inType, char** outTypeString);


/********************************************************************/
//  QTSS_StringToValue
//
//  Given a C-string and a QTSS_AttrDataType, this function converts the C-string
//  to the specified type and puts the result in ioBuffer. ioBuffer must be allocated
//  by the caller and must be big enough to contain the converted value.
//
//  Returns:    QTSS_NoErr
//              QTSS_BadArgument: Bad argument
//              QTSS_NotEnoughSpace: Value is too big for buffer provided.
//
//  QTSS_ValueToString
//
//  Given a buffer containing a value of the specified type, this function converts
//  the value to a C-string. This string is allocated internally and must be disposed of
//  using QTSS_Delete
//
//  Returns:    QTSS_NoErr
//              QTSS_BadArgument: Bad argument
QTSS_Error  QTSS_StringToValue(const char* inValueAsString, const QTSS_AttrDataType inType, void* ioBuffer, UInt32* ioBufSize);
QTSS_Error  QTSS_ValueToString(const void* inValue, const UInt32 inValueLen, const QTSS_AttrDataType inType, char** outString);

/********************************************************************/
//  QTSS_SetValue
//
//  Returns:    QTSS_NoErr
//              QTSS_BadArgument: Bad argument
//              QTSS_ReadOnly: Attribute is read only.
//              QTSS_BadIndex: Attempt to set non-0 index of attribute with a param retrieval function.
//
QTSS_Error QTSS_SetValue (QTSS_Object inObject, QTSS_AttributeID inID, UInt32 inIndex, const void* inBuffer,  UInt32 inLen);

/********************************************************************/
//  QTSS_SetValuePtr
//
//  This allows you to have an attribute that simply reflects the value of a variable in your module.
//  If the update to this variable is not atomic, you should protect updates using QTSS_LockObject.
//  This can't be used with indexed attributes.  Make sure the inBuffer provided exists as long as this
//  attribute exists.
//
//  Returns:    QTSS_NoErr
//              QTSS_BadArgument: Bad argument
//              QTSS_ReadOnly: Attribute is read only.
//
QTSS_Error QTSS_SetValuePtr (QTSS_Object inObject, QTSS_AttributeID inID, const void* inBuffer,  UInt32 inLen);

/********************************************************************/
//  QTSS_CreateObjectValue
//
//  Returns:    QTSS_NoErr
//                              QTSS_BadArgument: Bad argument
//                              QTSS_ReadOnly: Attribute is read only.
//
QTSS_Error QTSS_CreateObjectValue (QTSS_Object inObject, QTSS_AttributeID inID, QTSS_ObjectType inType, UInt32* outIndex, QTSS_Object* outCreatedObject);

/********************************************************************/
//  QTSS_GetNumValues
//
//  Returns:    QTSS_NoErr
//              QTSS_BadArgument: Bad argument
//
QTSS_Error QTSS_GetNumValues (QTSS_Object inObject, QTSS_AttributeID inID, UInt32* outNumValues);

/********************************************************************/
//  QTSS_RemoveValue
//
//  This function removes the value with the specified index. If there
//  are any values following this index, they will be reordered.
//
//  Returns:    QTSS_NoErr
//              QTSS_BadArgument: Bad argument
//              QTSS_ReadOnly: Attribute is read only.
//              QTSS_BadIndex: Attempt to set non-0 index of attribute with a param retrieval function.
//
QTSS_Error QTSS_RemoveValue (QTSS_Object inObject, QTSS_AttributeID inID, UInt32 inIndex);

/*****************************************/
//  STREAM CALLBACKS
//
//  The QTSS API provides QTSS_StreamRefs as a generalized stream abstraction. Mostly,
//  QTSS_StreamRefs are used for communicating with the client. For instance,
//  in the QTSS_RTSPRequest_Role, modules receive a QTSS_StreamRef which can be
//  used for reading RTSP data from the client, and sending RTSP response data to the client.
//
//  Additionally, QTSS_StreamRefs are generalized enough to be used in many other situations.
//  For instance, modules receive a QTSS_StreamRef for the error log. When modules want
//  to report errors, they can use these same routines, passing in the error log StreamRef.

/********************************************************************/
//  QTSS_Write
//
//  Writes data to a stream.
//
//  Returns:    QTSS_NoErr
//              QTSS_WouldBlock: The stream cannot accept any data at this time.
//              QTSS_NotConnected: The stream receiver is no longer connected.
//              QTSS_BadArgument:   NULL argument.
QTSS_Error  QTSS_Write(QTSS_StreamRef inRef, const void* inBuffer, UInt32 inLen, UInt32* outLenWritten, QTSS_WriteFlags inFlags);

/********************************************************************/
//  QTSS_WriteV
//
//  Works similar to the POSIX WriteV, and takes a POSIX iovec.
//  THE FIRST ENTRY OF THE IOVEC MUST BE BLANK!!!
//
//  Returns:    QTSS_NoErr
//              QTSS_WouldBlock: The stream cannot accept any data at this time.
//              QTSS_NotConnected: The stream receiver is no longer connected.
//              QTSS_BadArgument:   NULL argument.
QTSS_Error  QTSS_WriteV(QTSS_StreamRef inRef, iovec* inVec, UInt32 inNumVectors, UInt32 inTotalLength, UInt32* outLenWritten);

/********************************************************************/
//  QTSS_Flush
//
//  Some QTSS_StreamRefs (QTSS_RequestRef, for example) buffers data before sending it
//  out. Calling this forces the stream to write the data immediately.
//
//  Returns:    QTSS_NoErr
//              QTSS_WouldBlock: Stream cannot be completely flushed at this time.
//              QTSS_NotConnected: The stream receiver is no longer connected.
//              QTSS_BadArgument:   NULL argument.
QTSS_Error  QTSS_Flush(QTSS_StreamRef inRef);

/********************************************************************/
//  QTSS_Read
//
//  Reads data out of the stream
//
//  Arguments   inRef:      The stream to read from.
//              ioBuffer:   A buffer to place the read data
//              inBufLen:   The length of ioBuffer.
//              outLengthRead:  If function returns QTSS_NoErr, on output this will be set to the
//                              amount of data actually read.
//
//  Returns:    QTSS_NoErr
//              QTSS_WouldBlock
//              QTSS_RequestFailed
//              QTSS_BadArgument
QTSS_Error  QTSS_Read(QTSS_StreamRef inRef, void* ioBuffer, UInt32 inBufLen, UInt32* outLengthRead);

/********************************************************************/
//  QTSS_Seek
//
//  Sets the current stream position to inNewPosition
//
//  Arguments   inRef:      The stream to read from.
//              inNewPosition:  Offset from the start of the stream.
//
//  Returns:    QTSS_NoErr
//              QTSS_RequestFailed
//              QTSS_BadArgument
QTSS_Error  QTSS_Seek(QTSS_StreamRef inRef, UInt64 inNewPosition);

/********************************************************************/
//  QTSS_Advise
//
//  Lets the stream know that the specified section of the stream will be read soon.
//
//  Arguments   inRef:          The stream to advise.
//              inPosition:     Offset from the start of the stream of the advise region.
//              inAdviseSize:   Size of the advise region.
//
//  Returns:    QTSS_NoErr
//              QTSS_RequestFailed
//              QTSS_BadArgument
QTSS_Error  QTSS_Advise(QTSS_StreamRef inRef, UInt64 inPosition, UInt32 inAdviseSize);


/*****************************************/
//  SERVICES
//
//  Oftentimes modules have functionality that they want accessable from other
//  modules. An example of this might be a logging module that allows other
//  modules to write messages to the log.
//
//  Modules can use the following callbacks to register and invoke "services".
//  Adding & finding services works much like adding & finding attributes in
//  an object. A service has a name. In order to invoke a service, the calling
//  module must know the name of the service and resolve that name into an ID.
//
//  Each service has a parameter block format that is specific to that service.
//  Modules that are exporting services should carefully document the services they
//  export, and modules calling services should take care to fail gracefully
//  if the service isn't present or returns an error.

typedef QTSS_Error (*QTSS_ServiceFunctionPtr)(QTSS_ServiceFunctionArgsPtr);

/********************************************************************/
//  QTSS_AddService
//
//  This function registers a service with the specified name, and
//  associates it with the specified function pointer.
//  QTSS_AddService may only be called from the QTSS_Register role
//
//  Returns:    QTSS_NoErr
//              QTSS_OutOfState: If this function isn't being called from the Register role
//              QTSS_BadArgument:   Service name too long, or NULL arguments.
QTSS_Error QTSS_AddService(const char* inServiceName, QTSS_ServiceFunctionPtr inFunctionPtr);


/********************************************************************/
//  QTSS_IDForService
//
//  Much like QTSS_IDForAttr, this resolves a service name into its
//  corresponding QTSS_ServiceID. The QTSS_ServiceID can then be used to
//  invoke the service.
//
//  Returns:    QTSS_NoErr
//              QTSS_BadArgument: Bad argument
QTSS_Error QTSS_IDForService(const char* inTag, QTSS_ServiceID* outID);

/********************************************************************/
//  QTSS_DoService
//
//  Invokes the service. Return value from this function comes from the service
//  function itself, unless the QTSS_IllegalService errorcode is returned,
//  which is returned when the QTSS_ServiceID is bad.
QTSS_Error QTSS_DoService(QTSS_ServiceID inID, QTSS_ServiceFunctionArgsPtr inArgs);

/********************************************************************/
//  BUILT-IN SERVICES
//
//  The server registers some built-in services when it starts up.
//  Here are macros for their names & descriptions of what they do

// Rereads the preferences, also causes the QTSS_RereadPrefs_Role to be invoked
#define QTSS_REREAD_PREFS_SERVICE   "RereadPreferences"

/*****************************************/
//  FILE SYSTEM CALLBACKS
//
//  All modules that interact with the local file system should use these APIs instead
//  of the direct operating system calls.
//
//  This is for two reasons: 1) to ensure portability of your module across different
//  platforms such as Win32 and different versions of the UNIX operating system.
//
//  2)  To ensure your module will work properly if there is a 3rd party file system
//      or database that contains media files.

/********************************************************************/
//  QTSS_OpenFileObject
//
//  Arguments   inPath: a NULL-terminated C-string containing a full path to the file to open.
//                      inPath must be in the local (operating system) file system path style.
//              inFlags: desired flags.
//              outFileObject:  If function returns QTSS_NoErr, on output this will be a QTSS_Object
//                              for the file.
//
//  Returns:    QTSS_NoErr
//              QTSS_FileNotFound
//              QTSS_RequestFailed
//              QTSS_BadArgument
QTSS_Error  QTSS_OpenFileObject(char* inPath, QTSS_OpenFileFlags inFlags, QTSS_Object* outFileObject);

/********************************************************************/
//  QTSS_CloseFileObject
//
//  Closes the file object.
//
//  Arguments:  inFileObject: the file to close
//
//  Returns:    QTSS_NoErr
//              QTSS_BadArgument
QTSS_Error  QTSS_CloseFileObject(QTSS_Object inFileObject);


/*****************************************/
//  SOCKET CALLBACKS
//
//  It is not necessary for a module that internally uses network I/O to go through
//  the QTSS API for their networking APIs. However, it is highly recommended
//  to use nonblocking network I/O from a module. With nonblocking network I/O, it
//  is very important to be able to receive socket events.
//
//  To facilitate this, QTSS API provides the following two callbacks to link external
//  sockets into the QTSS API streams framework.
//
//  Once a module has created a QTSS stream out of its socket, it is possible to use the
//  QTSS_RequestEvent callback to receive events on the socket. 


/********************************************************************/
//  QTSS_CreateStreamFromSocket
//
//  Creates a socket stream.
//
//  Arguments:  inFileDesc: the socket
//
//  Returns:    QTSS_NoErr
QTSS_Error  QTSS_CreateStreamFromSocket(int inFileDesc, QTSS_SocketStream* outStream);


/********************************************************************/
//  QTSS_DestroySocketStream
//
//  Creates a socket stream.
//
//  Arguments:  inFileDesc: the socket
//
//  Returns:    QTSS_NoErr
QTSS_Error  QTSS_DestroySocketStream(QTSS_SocketStream inStream);


/*****************************************/
//  ASYNC I/O CALLBACKS
//
//  QTSS modules must be kind in how they use the CPU. The server doesn't
//  prevent a poorly implemented QTSS module from hogging the processing
//  capability of the server, at the expense of other modules and other clients.
//
//  It is therefore imperitive that a module use non-blocking, or async, I/O.
//  If a module were to block, say, waiting to read file data off disk, this stall
//  would affect the entire server.
//
//  This problem is resolved in QTSS API in a number of ways.
//
//  Firstly, all QTSS_StreamRefs provided to modules are non-blocking, or async.
//  Modules should be prepared to receive EWOULDBLOCK errors in response to
//  QTSS_Read, QTSS_Write, & QTSS_WriteV calls, with certain noted exceptions
//  in the case of responding to RTSP requests.
//
//  Modules that open their own file descriptors for network or file I/O can
//  create separate threads for handling I/O. In this case, these descriptors
//  can remain blocking, as long as they always block on the private module threads.
//
//  In most cases, however, creating a separate thread for I/O is not viable for the
//  kind of work the module would like to do. For instance, a module may wish
//  to respond to a RTSP DESCRIBE request, but can't immediately because constructing
//  the response would require I/O that would block.
//
//  The problem is once the module returns from the QTSS_RTSPProcess_Role, the
//  server will mistakenly consider the request handled, and move on. It won't
//  know that the module has more work to do before it finishes processing the DESCRIBE.
//
//  In this case, the module needs to tell the server to delay processing of the
//  DESCRIBE request until the file descriptor's blocking condition is lifted.
//  The module can do this by using the provided "event" callback routines.

//  Returns:    QTSS_NoErr
//              QTSS_BadArgument: Bad argument
//              QTSS_OutOfState: if this callback is made from a role that doesn't allow async I/O events
//              QTSS_RequestFailed: Not currently possible to request an event. 

QTSS_Error  QTSS_RequestEvent(QTSS_StreamRef inStream, QTSS_EventType inEventMask);
QTSS_Error  QTSS_SignalStream(QTSS_StreamRef inStream, QTSS_EventType inEventMask);

QTSS_Error  QTSS_SetIdleTimer(SInt64 inIdleMsec);
QTSS_Error  QTSS_SetIntervalRoleTimer(SInt64 inIdleMsec);

QTSS_Error  QTSS_RequestGlobalLock();
Bool16      QTSS_IsGlobalLocked();
QTSS_Error  QTSS_GlobalUnLock();

void        QTSS_LockStdLib();
void        QTSS_UnlockStdLib();

//DispatchCenter消息中心调用
QTSS_Error	QTSS_SendHTTPPacket(QTSS_RTSPSessionObject inServiceSession, char* inValue, UInt32 inValueLen, Bool16 connectionClose = false, Bool16 decrement = true);
QTSS_Error	QTSS_RegDevSession(QTSS_RTSPSessionObject inServiceSession, char* inValue, UInt32 inValueLen);    
QTSS_Error	QTSS_UpdateDevRedis(QTSS_RTSPSessionObject inServiceSession);
QTSS_Error	QTSS_UpdateDevSnap(QTSS_RTSPSessionObject inServiceSession, const char* inSnapTime, const char* inSnapJpg); 

#ifdef __cplusplus
}
#endif

#endif
