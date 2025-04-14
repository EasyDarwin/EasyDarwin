# srt_config
主要用于设置srt相关的配置
- enable: srt服务使能配置,设置为true才可以使用srt功能

*类型*: bool

*值举例*: true

- addr[string]: srt服务监听地址,srt服务监听的是UDP端口

*类型*: string

*值举例*: ":6001"

# rtc_config
主要用于设置rtc相关的配置,目前rtc只实现了WHIP/WHEP,需要配合http_config一起使用
- enable: rtc服务使能配置,设置为true才可以使用rtc功能

*类型*: bool

*值举例*: true

- iceHostNatToIps: rtc服务内穿ip,具体为SDP中的candidate信息,不设置的话,会输出全部网卡的地址

*类型*: []string

*举例*: ["192.168.0.1"]

- iceUdpMuxPort: rtc udp复用端口

*类型*: int

*值举例*: 4888

- iceTcpMuxPort: rtc tcp复用端口

*类型*: int

*值举例*: 4888

# http_config
主要用于设置http相关的配置,依赖http的协议均需要设置,涉及的协议有rtc、http-fmp4、hls(fmp4/llhls)
- http_listen_addr: http服务监听地址

*类型*: string

*值举例*: ":1290"

- enable_https: https使能

*类型*: bool

*值举例*: true

- https_listen_addr: https监听地址

*类型*: string

*值举例*: ":1233"

- https_cert_file: https cert文件路径

*类型*: string

*值举例*: "./conf/cert.pem"

- https_key_file: https key文件路径

*类型*: string

*值举例*: "./conf/key.pem"

- ctrl_auth_whitelist: 统计控制类接口鉴权，用于访问以 `/api/stat` 和 `/api/ctrl` 前缀的接口，无权限访问时 http status 将会响应 200，其 error_code 为 401。多种鉴权方式都不是零值时，必须同时满足才会通过鉴权。

*类型*: object

- secrets: 用户请求鉴权的方式是增加 query 参数 `token`，例如 `token=secret`，满足数组中任意匹配则通过。

*类型*: []string

*值举例*: ["EC3D1536-5D93-4BD6-9FBD-96A52CB1596D"]

- ips: 远程 IP 白名单，空数组表示允许任意 IP 访问，无权限访问时 http status 将会响应 200，其 error_code 为 401。

*类型*: []string

*值举例*: ["192.168.1.2","192.168.1.3"]


# http-fmp4配置
主要用于设置http-fmp4相关的配置,需要配合http_config一起使用
- enable: http-fmp4服务使能配置

*类型*: bool

*值举例*: true

# hls_config
主要用于设置hls-fmp4/llhls相关的配置,需要配合http_config一起使用,hls-ts的能力请使用lal,这里不做过多描述
- enable: hls-fmp4/llhls服务使能配置

*类型*: bool

*值举例*: true

- segmentCount: hls-fmp4 m3u8返回的切片个数,默认为7, llhls默认设置为7个(gohlslib要求)

*类型*: int

*值举例*: 3

- segmentDuration: hls-fmp4 切片时长,默认为1s

*类型*: int

*值举例*: 3

- partDuration:llhls part部分的时长,默认为200ms

*类型*: int

*值举例*: 100

- lowLatency: llhls使能配置,开启此配置后则都走llhls

*类型*: bool

*值举例*: true

# hook_config
主要用于 hook 相关的配置。

- gop_cache_num: gop 缓存的数量，默认为 1

*类型*: int

*值举例*: 3

- single_gop_max_frame_num: 一个 gop 的缓存帧数，0 表示智能识别

*类型*: int

*值举例*: 120


# gb28181_config

- enable: gb28181使能配置

*类型*: bool

*值举例*: true

- listenAddr: gb28181监听地址

*类型*: string

*值举例*: "0.0.0.0"

- sipNetwork: 传输协议

*类型*: string

*值举例*: "udp"

- sipIp: sip服务器公网IP

*类型*: string

*值举例*: "100.100.100.101"

- sipPort: sip服务器公网端口

*类型*: uint16

*值举例*: 5060

- serial: sip服务器ID

*类型*: string

*值举例*: "34020000002000000001"

- realm: sip服务器域

*类型*: string

*值举例*: "3402000000"

- username: sip服务器账号

*类型*: string

*值举例*: "admin"

- password: sip服务器密码

*类型*: string

*值举例*: "admin123"

# onvif_config
- enable: onvif使能配置

*类型*: bool

*值举例*: true

# lal_config_path
主要设置lal配置文件的路径
