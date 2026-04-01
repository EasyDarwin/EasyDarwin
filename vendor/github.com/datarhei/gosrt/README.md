# GoSRT

Implementation of the SRT protocol in pure Go with minimal dependencies.

<p align="left">
  <a href="http://srtalliance.org/">
    <img alt="SRT" src="https://github.com/datarhei/misc/blob/main/img/gosrt.png?raw=true" width="600"/>
  </a>
</p>

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
![Tests](https://github.com/datarhei/gosrt/actions/workflows/go-tests.yml/badge.svg)
[![codecov](https://codecov.io/gh/datarhei/gosrt/branch/main/graph/badge.svg?token=90YMPZRAFK)](https://codecov.io/gh/datarhei/gosrt)
[![Go Report Card](https://goreportcard.com/badge/github.com/datarhei/gosrt)](https://goreportcard.com/report/github.com/datarhei/gosrt)
[![PkgGoDev](https://pkg.go.dev/badge/github.com/datarhei/gosrt)](https://pkg.go.dev/github.com/datarhei/gosrt)

-   [SRT reference implementation](https://github.com/Haivision/srt)
-   [SRT RFC](https://haivision.github.io/srt-rfc/draft-sharabayko-srt.html)
-   [SRT Technical Overview](https://github.com/Haivision/srt/files/2489142/SRT_Protocol_TechnicalOverview_DRAFT_2018-10-17.pdf)

## Implementations

This implementation of the SRT protocol has live streaming of video/audio in mind. Because of this, the buffer mode and File Transfer
Congestion Control (FileCC) are not implemented.

|     |                                           |
| --- | ----------------------------------------- |
| ✅  | Handshake v4 and v5                       |
| ✅  | Message mode                              |
| ✅  | Caller-Listener Handshake                 |
| ✅  | Timestamp-Based Packet Delivery (TSBPD)   |
| ✅  | Too-Late Packet Drop (TLPKTDROP)          |
| ✅  | Live Congestion Control (LiveCC)          |
| ✅  | NAK and Peridoc NAK                       |
| ✅  | Encryption                                |
| ❌  | Buffer mode                               |
| ❌  | Rendezvous Handshake                      |
| ❌  | File Transfer Congestion Control (FileCC) |
| ❌  | Connection Bonding                        |

The parts that are implemented are based on what has been published in the SRT RFC.

## Requirements

A Go version of 1.20+ is required.

## Installation

```shell
go get github.com/datarhei/gosrt
```

## Caller example

```go
import "github.com/datarhei/gosrt"

conn, err := srt.Dial("srt", "golang.org:6000", srt.Config{
    StreamId: "...",
})
if err != nil {
    // handle error
}

buffer := make([]byte, 2048)

for {
    n, err := conn.Read(buffer)
    if err != nil {
        // handle error
    }

    // handle received data
}

conn.Close()
```

In the `contrib/client` directory you'll find a complete example of a SRT client.

## Listener example

```go
import "github.com/datarhei/gosrt"

ln, err := srt.Listen("srt", ":6000", srt.Config{...})
if err != nil {
    // handle error
}

for {
    req, err := ln.Accept2()
    if err != nil {
        // handle error
    }

    go func(req ConnRequest) {
        // check connection request by inspecting the connection request
        // and either rejecting it ...

        if somethingIsWrong {
            req.Reject(srt.REJ_PEER)
            return
        }

        // ... or accepting it ...

        conn, err := req.Accept()
        if err != nil {
            return
        }

        // ... and decide whether it is a publishing or subscribing connection.

        if publish {
            handlePublish(conn)
        } else {
            handleSubscribe(conn)
        }
    }(req)
}
```

In the `contrib/server` directory you'll find a complete example of a SRT server. For your convenience
this module provides the `Server` type which is a light framework for creating your own SRT server. The
example server is based on this type.

## Contributed client

In the `contrib/client` directory you'll find an example implementation of a SRT client.

Build the client application with

```shell
cd contrib/client && go build
```

The application requires only two options:

| Option  | Description          |
| ------- | -------------------- |
| `-from` | Address to read from |
| `-to`   | Address to write to  |

Both options accept an address. Valid addresses are: `-` for `stdin`, resp. `stdout`, a `srt://` address, or an `udp://` address.

### SRT URL

A SRT URL is of the form `srt://[host]:[port]/?[options]` where options are in the form of a `HTTP` query string. These are the
known options (similar to [srt-live-transmit](https://github.com/Haivision/srt/blob/master/docs/apps/srt-live-transmit.md)):

| Option               | Values                 | Description                                                             |
| -------------------- | ---------------------- | ----------------------------------------------------------------------- |
| `mode`               | `listener` or `caller` | Enforce listener or caller mode.                                        |
| `congestion`         | `live`                 | Congestion control. Currently only `live` is supported.                 |
| `conntimeo`          | `ms`                   | Connection timeout.                                                     |
| `drifttracer`        | `bool`                 | Enable drift tracer. Not implemented.                                   |
| `enforcedencryption` | `bool`                 | Accept connection only if both parties have encryption enabled.         |
| `fc`                 | `bytes`                | Flow control window size.                                               |
| `inputbw`            | `bytes`                | Input bandwidth. Ignored.                                               |
| `iptos`              | 0...255                | IP socket type of service. Broken.                                      |
| `ipttl`              | 1...255                | Defines IP socket "time to live" option. Broken.                        |
| `ipv6only`           | `bool`                 | Use IPv6 only. Not implemented.                                         |
| `kmpreannounce`      | `packets`              | Duration of Stream Encryption key switchover.                           |
| `kmrefreshrate`      | `packets`              | Stream encryption key refresh rate.                                     |
| `latency`            | `ms`                   | Maximum accepted transmission latency.                                  |
| `lossmaxttl`         | `ms`                   | Packet reorder tolerance. Not implemented.                              |
| `maxbw`              | `bytes`                | Bandwidth limit. Ignored.                                               |
| `mininputbw`         | `bytes`                | Minimum allowed estimate of `inputbw`.                                  |
| `messageapi`         | `bool`                 | Enable SRT message mode. Must be `false`.                               |
| `mss`                | 76...                  | MTU size.                                                               |
| `nakreport`          | `bool`                 | Enable periodic NAK reports.                                            |
| `oheadbw`            | 10...100               | Limits bandwidth overhead. Percents. Ignored.                           |
| `packetfilter`       | `string`               | Set up the packet filter. Not implemented.                              |
| `passphrase`         | `string`               | Password for the encrypted transmission.                                |
| `payloadsize`        | `bytes`                | Maximum payload size.                                                   |
| `pbkeylen`           | `16`, `24`, or `32`    | Crypto key length in bytes.                                             |
| `peeridletimeo`      | `ms`                   | Peer idle timeout.                                                      |
| `peerlatency`        | `ms`                   | Minimum receiver latency to be requested by sender.                     |
| `rcvbuf`             | `bytes`                | Receiver buffer size.                                                   |
| `rcvlatency`         | `ms`                   | Receiver-side latency.                                                  |
| `sndbuf`             | `bytes`                | Sender buffer size.                                                     |
| `snddropdelay`       | `ms`                   | Sender's delay before dropping packets.                                 |
| `streamid`           | `string`               | Stream ID (settable in caller mode only, visible on the listener peer). |
| `tlpktdrop`          | `bool`                 | Drop too late packets.                                                  |
| `transtype`          | `live`                 | Transmission type. Must be `live`.                                      |
| `tsbpdmode`          | `bool`                 | Enable timestamp-based packet delivery mode.                            |

### Usage

Reading from a SRT sender and play with `ffplay`:

```shell
./client -from "srt://127.0.0.1:6001/?mode=listener&streamid=..." -to - | ffplay -f mpegts -i -
```

Reading from UDP and sending to a SRT server:

```shell
./client -from udp://:6000 -to "srt://127.0.0.1:6001/?mode=caller&streamid=..."
```

Simulate point-to-point transfer on localhost. Open one console and start `ffmpeg` (you need at least version 4.3.2, built with SRT enabled) to send to an UDP address:

```shell
ffmpeg \
    -f lavfi \
    -re \
    -i testsrc2=rate=25:size=640x360 \
    -codec:v libx264 \
    -b:v 1024k \
    -maxrate:v 1024k \
    -bufsize:v 1024k \
    -preset ultrafast \
    -r 25 \
    -g 50 \
    -pix_fmt yuv420p \
    -flags2 local_header \
    -f mpegts \
    "udp://127.0.0.1:6000?pkt_size=1316"
```

In another console read from the UDP and start a SRT listenr:

```shell
./client -from udp://:6000 -to "srt://127.0.0.1:6001/?mode=listener&streamid=foobar"
```

In the third console connect to that stream and play the video with `ffplay`:

```shell
./client -from "srt://127.0.0.1:6001/?mode=caller&streamid=foobar" -to - | ffplay -f mpegts -i -
```

## Contributed server

In the `contrib/server` directory you'll find an example implementation of a SRT server. This server allows you to publish
a stream that can be read by many clients.

Build the client application with

```shell
cd contrib/server && go build
```

The application has these options:

| Option        | Default   | Description                                |
| ------------- | --------- | ------------------------------------------ |
| `-addr`       | required  | Address to listen on                       |
| `-app`        | `/`       | Path prefix for streamid                   |
| `-token`      | (not set) | Token query param for streamid             |
| `-passphrase` | (not set) | Passphrase for de- and enrcypting the data |
| `-logtopics`  | (not set) | Topics for the log output                  |
| `-profile`    | `false`   | Enable profiling                           |

This example server expects the streamID (without any prefix) to be an URL path with optional query parameter, e.g. `/live/stream`. If the `-app`
option is used, then the path must start with that path, e.g. the value is `/live` then the streamID must start with that value. The `-token`
option can be used to define a token for that stream as some kind of access control, e.g. with `-token foobar` the streamID might look like
`/live/stream?token=foobar`.

Use `-passphrase` in order to enable and enforce encryption.

Use `-logtopics` in order to write debug output. The value are a comma separated list of topics you want to be written to `stderr`, e.g. `connection,listen`. Check the [Logging](#logging) section in order to find out more about the different topics.

Use `-profile` in order to write a CPU profile.

### StreamID

In SRT the StreamID is used to transport somewhat arbitrary information from the caller to the listener. The provided example server uses this
machanism to decide who is the sender and who is the receiver. The server must know if the connecting client wants to publish a stream or
if it wants to subscribe to a stream.

The example server looks for the `publish:` prefix in the StreamID. If this prefix is present, the server assumes that it is the receiver
and the client will send the data. The subcribing clients must use the same StreamID (withouth the `publish:` prefix) in order to be able to
receive data.

If you implement your own server you are free to interpret the streamID as you wish.

### Usage

Running a server listening on port 6001 with defaults:

```shell
./server -addr ":6001"
```

Now you can use the contributed client to publish a stream:

```shell
./client -from ... -to "srt://127.0.0.1:6001/?mode=caller&streamid=publish:/live/stream"
```

or directly from `ffmpeg`:

```shell
ffmpeg \
    -f lavfi \
    -re \
    -i testsrc2=rate=25:size=640x360 \
    -codec:v libx264 \
    -b:v 1024k \
    -maxrate:v 1024k \
    -bufsize:v 1024k \
    -preset ultrafast \
    -r 25 \
    -g 50 \
    -pix_fmt yuv420p \
    -flags2 local_header \
    -f mpegts \
    -transtype live \
    "srt://127.0.0.1:6001?streamid=publish:/live/stream"
```

If the server is not on localhost, you might adjust the `peerlatency` in order to avoid packet loss: `-peerlatency 1000000`.

Now you can play the stream:

```shell
ffplay -f mpegts -transtype live -i "srt://127.0.0.1:6001?streamid=/live/stream"
```

You will most likely first see some error messages from `ffplay` because it tries to make sense of the received data until a keyframe arrives. If you
get more errors during playback, you might increase the receive buffer by adding e.g. `-rcvlatency 1000000` to the command line.

### Encryption

The stream can be encrypted with a passphrase. First start the server with a passphrase. If you are using `srt-live-transmit`, the passphrase has to be at least 10 characters long otherwise it will not be accepted.

```shell
./server -addr :6001 -passphrase foobarfoobar
```

Send an encrpyted stream to the server:

```shell
ffmpeg \
    -f lavfi \
    -re \
    -i testsrc2=rate=25:size=640x360 \
    -codec:v libx264 \
    -b:v 1024k \
    -maxrate:v 1024k \
    -bufsize:v 1024k \
    -preset ultrafast \
    -r 25 \
    -g 50 \
    -pix_fmt yuv420p \
    -flags2 local_header \
    -f mpegts \
    -transtype live \
    "srt://127.0.0.1:6001?streamid=publish:/live/stream&passphrase=foobarfoobar"
```

Receive an encrypted stream from the server:

```shell
ffplay -f mpegts -transtype live -i "srt://127.0.0.1:6001?streamid=/live/stream&passphrase=foobarfoobar"
```

You will most likely first see some error messages from `ffplay` because it tries to make sense of the received data until a keyframe arrives. If you
get more errors during playback, you might increase the receive buffer by adding e.g. `-rcvlatency 1000000` to the command line.

## Logging

This SRT module has a built-in logging facility for debugging purposes. Check the `Logger` interface and the `NewLogger(topics []string)` function. Because logging everything would be too much output if you wonly want to debug something specific, you have the possibility to limit the logging to specific areas like everything regarding a connection or only the handshake. That's why there are various topics.

In the contributed server you see an example of how logging is used. Here's the essence:

```go
logger := srt.NewLogger([]string{"connection", "handshake"})

config := srt.DefaultConfig
config.Logger = logger

ln, err := srt.Listen("udp", ":6000", config)
if err != nil {
    // handle error
}

go func() {
    for m := range logger.Listen() {
        fmt.Fprintf(os.Stderr, "%#08x %s (in %s:%d)\n%s \n", m.SocketId, m.Topic, m.File, m.Line, m.Message)
    }
}()

for {
    conn, mode, err := ln.Accept(acceptFn)
    ...
}
```

Currently known topics are:

```
connection:close
connection:error
connection:filter
connection:new
connection:rtt
connection:tsbpd
control:recv:ACK:cif
control:recv:ACK:dump
control:recv:ACK:error
control:recv:ACKACK:dump
control:recv:ACKACK:error
control:recv:KM:cif
control:recv:KM:dump
control:recv:KM:error
control:recv:NAK:cif
control:recv:NAK:dump
control:recv:NAK:error
control:recv:keepalive:dump
control:recv:shutdown:dump
control:send:ACK:cif
control:send:ACK:dump
control:send:ACKACK:dump
control:send:KM:cif
control:send:KM:dump
control:send:KM:error
control:send:NAK:cif
control:send:NAK:dump
control:send:keepalive:dump
control:send:shutdown:cif
control:send:shutdown:dump
data:recv:dump
data:send:dump
dial
handshake:recv:cif
handshake:recv:dump
handshake:recv:error
handshake:send:cif
handshake:send:dump
listen
packet:recv:dump
packet:send:dump
```

You can run `make logtopics` in order to extract the list of topics.

## Docker

The docker image you can build with `docker build -t srt .` provides the example SRT client and server as mentioned in the paragraph above.
E.g. run the server with `docker run -it --rm -p 6001:6001/udp srt srt-server -addr :6001`.
