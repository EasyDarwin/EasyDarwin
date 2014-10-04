ADMIN PROTOCOL SPECIFICATION
DarwinStreamingServer 3.0 Beta Release 
(This document is subject to change)

SERVER DATA ORGANIZATION
The server's internals are mapped to a hierarchical tree of element arrays. Each element is a named type including a container type for retrieval of sub-node elements.

PROTOCOL
The protocol relies upon the URI mechanism as defined by RFC2396 for specifying a container entity using a path and HTTP 1.0 RFC 1945 for specifying the Request and Response mechanisms.

REQUEST METHODS
HTTP GET is the current request and response method.

SESSION STATE
The session is closed at the end of each HTTP request response.

SUPPORTED REQUEST HEADER FEATURES 
Authorization

SERVER DATA ACCESS
All data on the server is specified using a URI

DEFINITION OF SERVER URI
The following URI references the top level of the Streaming Server's hierarchical data tree using a simple HTTP GET request.

Example:
GET /modules/admin 

URI REQUESTS

A valid request is an absolute reference (a URL beginning with "/") followed by the Server URI: 
[absolute URL]?[parameters="value"(s)]+[command="value"]+
["option"="value"]

Example:
GET /modules/admin/server/qtssSvrClientSessions?parameters=rt+command=get

Design Goals:

Concept: 
The server state machine and database can be accessed through a regular expression. The Admin protocol abstracts the QTSS module API to handle data access and in some cases to provide data access triggers for execution of server functions.

Flexibility: 
Four basic functions provide all of the administrative functions used by the server: add, set, del or get. 

Server Performance: 
Server streaming threads are blocked during admin accesses to internal data. To minimize the blocking of the server's activities, the protocol allows scoped access to the server's data structures by allowing specific URL paths to any element.

Query Functionality: 
Queries can contain an array iterator, a name lookup, a recursive tree walk, and a filtered response. All functions can execute in a single URI query.

Example of a query for the stream time scale and stream payload name from every stream in every session
GET /modules/admin/server/qtssSvrClientSessions/*/qtssCliSesStreamObjects?parameters=r+command=get+filter1=qtssRTPStrTimescale+filter2=qtssRTPStrPayloadName

"*" = array iterator
"parameters=rt" = 'r' recursive walk and 't' show data types in result.
"filter1=qtssRTPStrTimescale" = return the stream time scale
"filter2=qtssRTPStrPayloadName" = return the stream payload
 
Example of a query for all server module names and their descriptions 
GET /modules/admin/server/qtssSvrModuleObjects?parameters=r+command=get+filter2=qtssModDesc+filter1=qtssModName

URI RULES
/path = absolute reference
* = iterate each element in current URL location
path/* = is defined as all elements contained in the "path" container
. = not supported
.. = not supported
; = not supported
? query options follow ("+" delimited name="value" pairs)
spaces and tabs are stop characters.
"" are supported for values and required for values containing spaces and tabs.

PATH DEFINITION
A path represents a server's virtual hierarchical data structure of containers and is expressed as a URL. 

DATA REFERENCES
All Elements are arrays. Single element arrays may be referenced by "path/element", "path/element/", "path/element/*", and"path/element/1" are evaluated as the same query.


QUERY OPTIONS
URIs without a '?' default to a get request.
URIs containing a '?' designator must contain a "command=[get|set|del|add] " query option. 

Query options are not case sensitive. 
Query option values except for the command options are case sensitive.
Unknown query options are ignored.
Query options not required by a command are ignored.

COMMAND OPTION:
command=[GET | SET | DEL | ADD]
Unknown commands are reported as an error.

command=GET <- get data identified by URI
Command GET does not require other query options.
Example: GET /modules/admin/example_count

command=SET <- set data identified by URI
Value checking is not performed. Conversion between the text value and the actual value is type specific. 
Example: GET /modules/admin/example_count?command=SET+value=5

OPTIONAL QUERY OPTIONS
type= <- if defined then type checking of the server element type and the set type is performed. If a match of the stored type and the request type fails an error is returned and the command fails.
Example: GET /modules/admin/maxcount?command=SET+value=5+type=SInt32

command=DEL <- delete data identified by URI
The command deletes the element referenced by the URL. 
Example: GET /modules/admin/maxcount?command=DEL

command=ADD <- add data identified by URI
If the element at the end of the URL is an element then Add performs an add to the array of elements reference the element name.
required query options are
value=
type=
Example: GET /modules/admin/example_count ? command=ADD+value=6type=SInt16

If the element at the end of the URL is a QTSS_Object container then a "command=add" performs a named element add to the container.
required query options are
value=
type=
name=
Example: GET /modules/admin/?command=ADD+value=5+name=maxcount+type=SInt16


"parameters=":
'r' = recurse -> walk downward in hierarchy starting at end of URL. Recursion should be avoided if "*" iterators or direct URL access to elements can be used. 
'v' = verbose -> return full path in name
'a' = access -> return read/write access 
't' = type -> return date type of value 
'd' = debug -> return debugging info with error
'c' = count -> return count of elements in path
Parameters are always single characters with no delimiters. 
Parameter options follow the URL [URL]?parameters=[p][p]
example= path/path?parameters=rvat

ACCESS TYPES
r = read
w = write
p = pre-emptive safe

DATA TYPES
Data types can be any server allowed text value. New data types can be defined and returned by the server so data types are not limited to the basic set below.

	"UInt8"
	"SInt8"
	"UInt16"
	"SInt16"
	"UInt32"
	"SInt32"
	"UInt64",
	"SInt64"
	"Float32"
	"Float64"
	"Bool8"
	"Bool16"
	"CharArray"
	"QTSS_Object"
	"void_pointer"	

QTSS_Objects, pointers and unknown data types always converted to a host ordered string of hex values. Example of hex value result: unknown_pointer=DEADBEEF; type=void_pointer


URI POST FILTERS
Filters specify a subset of data to be returned on each request.
Multiple filters are evaluated in order with each result placed in the response. 


RESPONSES

Example: Unauthorized

HTTP/1.1 401 Unauthorized  
WWW-Authenticate: Basic realm="QTSS/modules/admin"  
Server: QTSS  
Connection: Close  
Content-Type: text/plain  

Example: OK result

HTTP/1.0 200 OK  
Server: QTSS/3.0 [v252]-Linux  
Connection: Close  
Content-Type: text/plain  
   
Container="/" 
admin/ 
error:(0) 

RESPONSE END

Each OK response ends with
error:(0)


RESPONSE DATA
All entity references follow the form [NAME=VALUE] ; [attribute="value"] , [attribute="value"].
NAME=VALUE
NAME=VALUE;attribute="value"
NAME=VALUE;attribute="value",attribute="value"

All container references follow the form [NAME/] ; [attribute="value"] , [attribute="value"].
NAME/
NAME/;attribute="value"
NAME;attribute="value",attribute="value"

The order of appearance of container references and the container’s entity references is important. This is especially true when the response is a recursive walk of a container hierarchy.
The "Container=" reference must appear at the beginning of each new level in the hierarchy. Each Container list of elements must be a complete list of the contained elements and any containers. The appearance of a "Container=" reference indicates the end of a previous container’s contents and the start of a new container.

The example below shows how each new container is identified with the unique path.

Container="/level1/"
field1="value"
field2="value"
level2a/
level2b/
Container="/level1/level2a/"
field1="value"
level3a/
level3b/
Container="/level1/level2a/level3a"
field1="value"
Container="/level1/level2a/level3b"
Container="/level1/level2b/"
field1="value"
level3a/
Container="/level1/level2b/level3a/"
field1="value"

ARRAY VALUES
Arrays of elements are handled in the response by using a numerical value to represent the index. Arrays are containers.

Container="/level1/"
field1="value"
field2="value"
array1/
Container="/level1/array1/"
1=value
2=value

Array elements may be containers.
Container="/level1/array1/"
1/
2/
3/

Container="/level1/array1/1/"
field1="value"
field2="value"
Container="/level1/array1/2/"
Container="/level1/array1/3/"
field1="value"


ROOT VALUE
/admin

ERRORS IN RESPONSE
The Error state for the Request is always reported with each response at the end of the data.
Error:(0) <- no Error
Error:(404) <- data not found.

The number appearing in the parenthesis is an HTTP error code followed by an error string when debugging is turned on using the "parameters=d" query option.
error:(404);reason="No data found" 

SETTING ENTITY VALUES

When changing server values the entity names and their values are located in the request body. If a match is made on an entity name including the URL base at the current container level then the value is set in the server provided the read write attribute allows the set.

base = /base/container
name = value
/base/container/name="value"



EXAMPLES
================================
This first example uses basic authentication and shows the HTTP response headers. Later examples will focus on the content of the response and URI of the request.  Note a simple method for performing a request is to use a web browser using the URL 

http://17.221.45.238:554/modules/admin/?parameters=a+command=get

----------------Request----------------
GET /modules/admin?parameters=a+command=get
Authorization: Basic QWXtaW5pT3RXYXRvcjXkZWZhdWx0
----------------Response----------------
HTTP/1.0 200 OK  
Server: QTSS/3.0 [v252]-Linux  
Connection: Close  
Content-Type: text/plain  
   
Container="/" 
admin/;a=r 
error:(0)  
 

================================
3 Examples Request

----------------Request 1----------------
GET /modules/admin?command=get+parameters=r <--recurse  *get everything*

----------------Request 2----------------
GET /modules/admin?command=get+parameters=rat <- recurse, access, type

----------------Request 3----------------
GET /modules/admin/*<- request elements in admin command not required if no query options are present.


========================================

An admin client may wish to just monitor the session list like so

----------------Request----------------
GET /modules/admin/server/qtssSvrClientSessions/*
----------------Response-----------------
Container="/admin/server/qtssSvrClientSessions/"
12/
2/
4/
8/
error:(0)

Response is a qtssSvrClientSessions list of unique session ids

----------------Request----------------
GET /modules/admin/server/qtssSvrClientSessions/*/qtssCliSesStreamObjects/*
----------------Response-----------------
Container="/admin/server/qtssSvrClientSessions/3/qtssCliSesStreamObjects/"
0/
1/
error:(0)

qtssCliSesStreamObjects are an indexed array of streams
========================================

GET /modules/admin/server/qtssSvrClientSessions/3/qtssCliSesStreamObjects/0/*
----------------Response-----------------
qtssRTPStrTrackID="4"
qtssRTPStrSSRC="683618521"
qtssRTPStrPayloadName="X-QT/600"
qtssRTPStrPayloadType="1"
qtssRTPStrFirstSeqNumber="-7111"
qtssRTPStrFirstTimestamp="433634204"
qtssRTPStrTimescale="600"
qtssRTPStrQualityLevel="0"
qtssRTPStrNumQualityLevels="3"
qtssRTPStrBufferDelayInSecs="3.000000"
qtssRTPStrFractionLostPackets="0"
qtssRTPStrTotalLostPackets="52"
qtssRTPStrJitter="0"
qtssRTPStrRecvBitRate="1526072"
qtssRTPStrAvgLateMilliseconds="501"
qtssRTPStrPercentPacketsLost="0"
qtssRTPStrAvgBufDelayInMsec="30"
qtssRTPStrGettingBetter="0"
qtssRTPStrGettingWorse="0"
qtssRTPStrNumEyes="0"
qtssRTPStrNumEyesActive="0"
qtssRTPStrNumEyesPaused="0"
qtssRTPStrTotPacketsRecv="6763"
qtssRTPStrTotPacketsDropped="0"
qtssRTPStrTotPacketsLost="0"
qtssRTPStrClientBufFill="0"
qtssRTPStrFrameRate="0"
qtssRTPStrExpFrameRate="3903"
qtssRTPStrAudioDryCount="0"
qtssRTPStrIsTCP="false"
qtssRTPStrStreamRef="18861508"
qtssRTPStrCurrentPacketDelay="-2"
qtssRTPStrTransportType="0"
qtssRTPStrStalePacketsDropped="0"
qtssRTPStrTimeFlowControlLifted="974373815109"
qtssRTPStrCurrentAckTimeout="0"
qtssRTPStrCurPacketsLostInRTCPInterval="52"
qtssRTPStrPacketCountInRTCPInterval="689"
QTSSReflectorModuleStreamCookie=(null)
qtssNextSeqNum=(null)
qtssSeqNumOffset=(null)
QTSSSplitterModuleStreamCookie=(null)
QTSSFlowControlModuleLossAboveTol="0"
QTSSFlowControlModuleLossBelowTol="3"
QTSSFlowControlModuleGettingWorses="0"
error:(0)


========================================

Here is an example of monitoring just the IP addresses of connected clients. Only the IP addresses are polled and returned.

----------------Request----------------
modules/admin/server/qtssSvrClientSessions/*/qtssCliRTSPSessRemoteAddrStr
---------------Response----------------
Container="/admin/server/qtssSvrClientSessions/5/"qtssCliRTSPSessRemoteAddrStr=17.221.40.1
Container="/admin/server/qtssSvrClientSessions/6/"qtssCliRTSPSessRemoteAddrStr=17.221.40.2
Container="/admin/server/qtssSvrClientSessions/8/"qtssCliRTSPSessRemoteAddrStr=17.221.40.3
Container="/admin/server/qtssSvrClientSessions/14/"qtssCliRTSPSessRemoteAddrStr=17.221.40.4
error:(0)
========================================


SPECIAL PATHS

PREFERENCES
Setting a server or module preference value causes the value to be flushed to the server's xml preference file and the new value will take affect immediately.

/modules/admin/server/qtssSvrPreferences <-- server preferences
/modules/admin/server/qtssSvrModuleObjects/*/qtssModPrefs/ <-- module preferences

The elements defined in qtssSvrPreferences are modify-only elements. 
The elements in qtssModPrefs containers may be added, deleted and modified. Some deleted elements may be restored automatically by a module if the server or module requires them. The add, del, and set commands on a qtssModPrefs element will cause the streaming server.xml file to be rewritten.

SERVER STATE
/modules/admin/server/qtssSvrState

Controls the sever state. It can be modified as a UInt32 with the following values.
	qtssStartingUpState 			= 0,
	qtssRunningState 				= 1,
	qtssRefusingConnectionsState 	= 2,
	qtssFatalErrorState 			= 3,//a fatal error has occurred, not shutting down yet
	qtssShuttingDownState			= 4,
	qtssIdleState					= 5 // Like refusing connections state, but will also kill any currently connected clients

See the QTSS.h API documentation for information about other documented server attributes.

