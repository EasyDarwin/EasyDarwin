Support For Stream Caching in Streaming Server 3

Introduction

Streaming Server 3 adds new features to RTSP and RTP in order to make
it as easy as possible for a caching proxy server to capture and
manage a pristine copy of a media stream. Some of these features are
elements of the RTSP protocol that were not supported in previous
versions. Some are additions to RTSP and RTP - these extensions have
already been, or will be soon, submitted to the IETF for consideration
as additions to the standard.

This document provides a complete description of the stream caching
support added to Streaming Server 3. It is intended as a guide for
RTSP / RTP caching vendors to use.

Four major features have been added:

1. Speed RTSP Header

Streaming Server 3 supports the RTSP Speed header wherever possible.
This allows a caching proxy server to request a stream to be delivered
faster than real time, so it can move a stream into the cache as
quickly as possible. For complete documentation on the Speed header,
see the RTSP RFC.

2. x-Transport-Options RTSP Header

Streaming Server 3 supports a non-standard RTSP header,
x-Transport-Options. This header has one currently supported argument,
"late-tolerance". This argument allows a caching proxy to tell the
server how late packets may be sent and have them still be useful to
the cache. A complete description of this header, with examples, is
included in this document as Appendix A.

3. RTP-Meta-Info RTP Payload

Streaming Server 3 fully supports the RTP-Meta-Info RTP payload
format, described separately in an Internet Draft (draft-serenyi-avt-rtp-meta-00).
RTP packets of this payload type include important information such as
the packet transmission time, unique packet number, and video frame
type. Caching proxies can use this information to provide the same
quality of service to clients as the origin server.

4. x-Packet-Range RTSP Header

Streaming Server 3 supports a non-standard RTSP header,
x-Packet-Range. This header is similar to the Range RTSP header, but
allows the client to specify a specific range of packets instead of a
range of time. This allows a caching proxy to tell the origin server
to selectively retransmit only those packets which it needs to fill in
holes in its cached copy of the stream. A complete description of this
header, with examples, is included in this document as Appendix B.


Appendix A: Description of the x-Transport-Options RTSP header.

This optional header should be sent from a client to server on a
SETUP, and echoed by the server. If the header is not echoed the
client must assume that the server does not support this header.

The body of this header contains one or more semi-colon delimited
arguments. Each argument modifies the contents or delivery RTP packets
for the RTP stream specified by the SETUP in a particular way. In the
SETUP request, the arguments represent the options the client would
like applied to the stream. In the SETUP response, the arguments
represent what the server will actually apply to the RTP stream. The
response may contain a subset of the arguments requested by the
client. This may happen in the event the server doesn't support all
the requested arguments, or some don't apply to the RTP stream
specified by the SETUP. The server may also modify the values of any
arguments it returns.

There is currently only one possible argument for the X-Transport-Options
header. More arguments may be added later in the event that more
fine-grain control is required for the RTSP server's RTP streams.

Argument 1: late-tolerance

The value of this argument should be a positive integer, representing
the number of seconds late a media packet may be sent by the server
and still have it be useful to the client. It should be used as a
guide to the server to make a best-effort attempt to deliver all media
data not older than late-tolerance value.

Example:

x-Transport-Options: late-tolerance=30

If this is being passed to an RTSP server as part of a SETUP for a
video stream, the server should attempt to deliver all video frames
unless they are more than 30 seconds old. Frames that are older than
30 seconds can be dropped by the server because they are stale.

This can be used by a caching proxy to prevent the media server from
dropping frames or lowering the stream bit rate in the event it falls
behind sending media data. If the caching proxy knows the duration of
the media, setting late-tolerance to that value will prevent the
server from ever dropping frames, allowing the cache to receive a
complete copy of the media data. For a live broadcast, a caching proxy
may want to do extra buffering to improve quality for its clients. The
proxy could advertise the length of its buffer to the server this way.


Appendix B: Description of the x-Packet-Range RTSP Header

This header should be sent from a client to server on a PLAY in place
of the Range header. If the server does not support this header it
should respond with a 501 Header Not Implemented.

The body of this header contains a start and stop packet number for
this PLAY request. The specified packet numbers must be based on the
packet number RTP-Meta-Info field. For information on how to request
packet numbers as part of the RTP stream, see the RTP-Meta-Info
payload format Internet Draft (draft-serenyi-avt-rtp-meta-00).

The header format consists of two semi-colon delimited arguments. The
first argument must be the packet number range, with the start and
stop packet numbers separated by a '-'. The second argument must be
the stream URL to which these packet numbers belong.

Example:

x-Packet-Range: pn=4551-4689;url=trackID3

The stop packet number must be equal to or greater than the start
packet number. Otherwise, the server may return an error or simply not
send any media data following the PLAY response.
