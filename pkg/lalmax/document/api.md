# HTTP API

lalmax 提供了一些 HTTP 的 API 接口，通过这些接口，可以获取 lalmax 的一些状态，以及控制一些行为。

 lalmax 的 HTTP API 旨在包含 lal 的 API 调用，并补充相关订阅数据。其请求方式，请求参数，响应参数等与 lal API 完全一致。

可参考本文档，也可以参考 lal API 文档。

## 接口列表

接口分为两大类：

- 查询类型的，以 `/api/stat` 开头
- 控制类型的，以 `/api/ctrl` 开头

```bash
1.1. /api/stat/group     // 查询特定group的信息
1.2. /api/stat/all_group // 查询所有group的信息
1.3. /api/stat/lal_info  // 查询服务器信息

2.1. /api/ctrl/start_relay_pull // 控制服务器从远端拉流至本地
2.2. /api/ctrl/stop_relay_pull  // 停止relay pull
2.3. /api/ctrl/kick_session     // 强行踢出关闭指定session，session可以是pub、sub、pull类型
2.4. /api/ctrl/start_rtp_pub    // 打开GB28181接收端口(停止先使用kick_session)
```

## 名词解释

+ `group` lal中的group是群组的概念，lal作为流媒体服务器，通过流名称将每1路输入流转发给`1~n`路输出流，流名称相同的输入输出流被同1个group群组管理。

## 接口规则

1 所有接口的返回结果中，必含的一级参数：

```json
{
    "error_code": 0,
    "desp": "succ",
    "data": ...
}
```

2 `error_code`列表：

| error_code | desp                       | 说明                 |
| ---------- | -------------------------- | -------------------- |
| 0          | succ                       | 调用成功             |
| 1001       | group not found            | group不存在          |
| 1002       | param missing              | 必填参数缺失         |
| 1003       | session not found          | session不存在        |
| 2001       | 多种值，表示失败的具体原因 | start_relay_pull失败 |
| 2002       | 打开gb28181端口失败        | start_rtp_pub失败    |

3 注意，有的接口使用HTTP GET+URL 参数的形式调用，有的接口使用 HTTP POST+JSON body 的形式调用，请仔细查看文档说明。

## 接口详情

### 1.1 `/api/stat/group`

✸ 简要描述： 查询指定group的信息

✸ 请求示例：

```
$curl http://127.0.0.1:8083/api/stat/group?stream_name=test110
```

✸ 请求方式： `HTTP GET`+url参数

✸ 请求参数：

- stream_name | 必填项 | 指定 group 的流名称

✸ 返回值`error_code`可能取值：

- 0 group存在，查询成功
- 1001 group不存在
- 1002 必填参数缺失

✸ 返回示例：

```
{
  "error_code": 0, // 接口返回值，0表示成功
  "desp": "succ",  // 接口返回描述，"succ"表示成功
  "data": {
    "stream_name": "test110", // 流名称
    "app_name":    "live",    // appName
    "audio_codec": "AAC",     // 音频编码格式 "AAC"
    "video_codec": "H264",    // 视频编码格式 "H264" | "H265"
    "video_width": 640,       // 视频宽
    "video_height": 360,      // 视频高
    "pub": {                                   // -----接收推流的信息-----
      "session_id": "RTMPPUBSUB1",             // 会话ID，会话全局唯一标识
      "protocol": "RTMP",                      // 推流协议，取值范围： "RTMP" | "RTSP"
      "base_type": "PUB",                      // 基础类型，该处固定为"PUB"
      "start_time": "2020-10-11 19:17:41.586", // 推流开始时间
      "remote_addr": "127.0.0.1:61353",        // 对端地址
      "read_bytes_sum": 9219247,               // 累计读取数据大小（从推流开始时计算）
      "wrote_bytes_sum": 3500,                 // 累计发送数据大小
      "bitrate_kbits": 436,                    // 最近5秒码率，单位kbit/s。对于pub类型，如无特殊声明，等价于`read_bitrate_kbits`
      "read_bitrate"_kbits: 436,               // 最近5秒读取数据码率
      "write_bitrate_kbits": 0                 // 最近5秒发送数据码率
    },
    "subs": [                                    // -----拉流的信息，可能存在多种协议，每种协议可能存在多个会话连接-----
      {
        "session_id": "FLVSUB1",                 // 会话ID，会话全局唯一标识
        "protocol": "FLV",                       // 拉流协议，取值范围： "RTMP" | "FLV" | "TS"
        "base_type" "SUB"                        // 基础类型，该处固定为"SUB"
        "start_time": "2020-10-11 19:19:21.724", // 拉流开始时间
        "remote_addr": "127.0.0.1:61785",        // 对端地址
        "read_bytes_sum": 134,                   // 累计读取数据大小（从拉流开始时计算）
        "wrote_bytes_sum": 2944020,              // 累计发送数据大小
        "bitrate_kbits": 439,                    // 最近5秒码率，单位kbit/s。对于sub类型，如无特殊声明，等价于`write_bitrate_kbits`
        "read_bitrate_kbits": 0,                 // 最近5秒读取数据码率
        "write_bitrate_kbits": 439               // 最近5秒发送数据码率
      }
    ],
    "pull": {              // -----该节点从其他节点拉流回源信息-----
      "base_type": "PULL", // 该处固定为"PULL"
      ...                  // 其他字段和上面pub的内部字段相同，不再赘述
    },
    "pushs":[] // 主动外连转推信息，暂时不提供
  }
}
```

### 1.2 `/api/stat/all_group`

✸ 简要描述： 查询所有group的信息

✸ 请求示例：

```
$curl http://127.0.0.1:8083/api/stat/all_group
```

✸ 请求方式： `HTTP GET`

✸ 请求参数： 无

✸ 返回值`error_code`可能取值：

- 0 查询成功

✸ 返回示例：

```
{
    "error_code": 0,
    "desp": "succ",
    "data": {
        "groups": [
            ...      // 数组内每个元素的内容格式和/api/stat/group接口中data字段相同，不再赘述
        ]
    }
}
```

### 1.3 `/api/stat/lal_info`

✸ 简要描述： 查询服务器信息

✸ 请求示例：

```
$curl http://127.0.0.1:8083/api/stat/lal_info
```

✸ 请求方式： `HTTP GET`

✸ 请求参数： 无

✸ 返回值`error_code`可能取值：

- 0 查询成功

✸ 返回示例：

```
{
  "error_code": 0,
  "desp": "succ",
  "data": {
    "server_id": "1",
    "bin_info": "GitTag=v0.17.0. GitCommitLog=bbf850aca2d4f3e55380d44ca9c3a16be60c8d39 ${NewVersion} -> version.go. GitStatus= M CHANGELOG.md | M gen_tag.sh | M pkg/base/version.go. BuildTime=2020.11.21.173812. GoVersion=go version go1.14.2 darwin/amd64. runtime=darwin/amd64.",
    "lal_version": "v0.17.0",               // lal可执行文件版本信息
    "api_version": "v0.1.2",                // HTTP API接口版本信息
    "notify_version": "v0.0.4",             // HTTP Notify版本信息
    "start_time": "2020-11-21 17:34:53.973" // lal进程启动时间
  }
}
```

### 2.1 `/api/ctrl/start_relay_pull`

✸ 简要描述： 控制服务器主动从远端拉流至本地

✸ 请求示例：

```
$curl -H "Content-Type:application/json" -X POST -d '{"url": "rtmp://127.0.0.1/live/test110?token=aaa&p2=bbb", "pull_retry_num": 0}' http://127.0.0.1:8083/api/ctrl/start_relay_pull
```

✸ 请求方式： `HTTP POST`

✸ 请求参数：

```
{
    "url": "rtmp://127.0.0.1/live/test110?token=aaa&p2=bbb", //. 必填项，回源拉流的完整url地址，目前支持rtmp和rtsp
                                                             //
    "stream_name": "test110",                                //. 选填项，如果不指定，则从`url`参数中解析获取
                                                             //
    "pull_timeout_ms": 10000,                                //. 选填项，pull建立会话的超时时间，单位毫秒。
                                                             //  默认值是10000
                                                             //
    "pull_retry_num": 0,                                     //. 选填项，pull连接失败或者中途断开连接的重试次数
                                                             //  -1  表示一直重试，直到收到stop请求，或者开启并触发下面的自动关闭功能
                                                             //  = 0 表示不重试
                                                             //  > 0 表示重试次数
                                                             //  默认值是0
                                                             //  提示：不开启自动重连，你可以在收到HTTP-Notify on_relay_pull_stop, on_update等消息时决定是否重连
                                                             //
    "auto_stop_pull_after_no_out_ms": -1,                    //. 选填项，没有观看者时，自动关闭pull会话，节约资源
                                                             //  -1  表示不启动该功能
                                                             //  = 0 表示没有观看者时，立即关闭pull会话
                                                             //  > 0 表示没有观看者持续多长时间，关闭pull会话，单位毫秒
                                                             //  默认值是-1
                                                             //  提示：不开启该功能，你可以在收到HTTP-Notify on_sub_stop, on_update等消息时决定是否关闭relay pull
                                                             //
    "rtsp_mode": 0,                                          //. 选填项，使用rtsp时的连接方式
                                                             //  0 tcp
                                                             //  1 udp
                                                             //  默认值是0
    "debug_dump_packet": ""                                  //. 选填项，将接收的数据存成文件
                                                             //  注意啊，有问题的时候才使用，把存储的文件提供给lal作者分析。没问题时关掉，避免性能下降并且浪费磁盘
                                                             //  值举例："./dump/test110.laldump", "/tmp/test110.laldump"
                                                             //  如果为空字符串""，则不会存文件
                                                             //  默认值是""
}
```

✸ 返回值`error_code`可能取值：

- 0 请求接口成功。
- 1002 参数错误
- 2001 请求接口失败，失败描述参考desp
  - "lal.logic: in stream already exist in group": 输入流已经存在了

> 注意：返回成功表示lalserver收到命令并开始从远端拉流，并不保证从远端拉流成功。判断是否拉流成功，可以使用HTTP-Notify的on_relay_pull_start, on_update等回调事件

✸ 返回示例：

```
{
  "error_code": 0,
  "desp": "succ",
  "data": {
    "stream_name": "test110",
    "session_id": "RTMPPULL1"
  }
}
```

### 2.2 `/api/ctrl/stop_relay_pull`

✸ 简要描述： 关闭特定的relay pull

✸ 请求示例：

```
$curl http://127.0.0.1:8083/api/ctrl/stop_relay_pull?stream_name=test110
```

✸ 请求方式： `HTTP GET`+url参数

✸ 请求参数：

- stream_name | 必填项 | 需要关闭relay pull的流名称

✸ 返回值`error_code`可能取值：

- 0 group存在，查询成功
- 1001 group不存在
- 1002 必填参数缺失
- 1003 pull session不存在

✸ 返回示例：

```
{
  "error_code": 0,
  "desp": "succ",
  "data": {
    "session_id": "RTMPPULL1"
  }
}
```

> 提示，除了stop_relay_pull，也可以使用kick_session关闭relay pull回源拉流。

### 2.3 `/api/ctrl/kick_session`

✸ 简要描述： 强行踢出关闭指定session。session可以是pub、sub、pull类型。

✸ 请求示例：

```
$curl -H "Content-Type:application/json" -X POST -d '{"stream_name": "test110", "session_id": "FLVSUB1"}' http://127.0.0.1:8083/api/ctrl/kick_session
```

✸ 请求方式： `HTTP POST`

✸ 请求参数：

```
{
  "stream_name": "test110", // 必填项，流名称
  "session_id": "FLVSUB1"   // 必填项，会话唯一标识
}
```

✸ 返回值`error_code`可能取值：

- 0 请求接口成功。指定会话被关闭
- 1001 指定流名称对应的group不存在
- 1002 参数错误
- 1003 指定会话不存在

✸ 返回示例：

```
{
  "error_code": 0,
  "desp": "succ"
}
```

### 2.4 `/api/ctrl/start_rtp_pub`

✸ 简要描述： 打开GB28181接收端口

✸ 请求示例：

```
$curl -H "Content-Type:application/json" -X POST -d '{"stream_name": "test110", "port": 0, "timeout_ms": 10000}' http://127.0.0.1:8083/api/ctrl/start_rtp_pub
```

✸ 请求方式： `HTTP POST`

✸ 请求参数：

```
{
  "stream_name": "test110", //. 必填项，流名称，后续这条流都与这个流名称绑定，比如生成的录制文件名，用其他协议拉流的流名称等
                            //
  "port": 0,                //. 选填项，接收端口
                            //  如果为0，lalserver选择一个随机端口，并将端口通过返回值返回给调用方
                            //  默认值是0
                            //
  "timeout_ms": 60000,      //. 选填项，超时时间，单位毫秒，开启时或中途超过这个时长没有收到任何数据，则关闭端口监听
                            //  如果为0，则不会超时关闭
                            //  默认值是60000
                            //
  "is_tcp_flag": 0,         //. 选填项，是否使用tcp传输流媒体音视频数据
                            //  如果为1，使用tcp；如果为0，使用udp
                            //  默认值为0
  "debug_dump_packet": ""   //. 选填项，将接收的udp数据存成文件
                            //  注意啊，有问题的时候才使用，把存储的文件提供给lal作者分析。没问题时关掉，避免性能下降并且浪费磁盘
                            //  值举例："./dump/test110.laldump", "/tmp/test110.laldump"
                            //  如果为空字符串""，则不会存文件
                            //  默认值是""
}
```

✸ 返回值`error_code`可能取值：

- 0 请求接口成功。端口成功打开
- 1002 参数错误
- 2002 绑定监听端口失败

✸ 返回示例：

```
{
  "error_code": 0,
  "desp": "succ",
  "data": {
    "stream_name": "test110",
    "session_id": "PSSUB1",
    "port": 20000
  }
}
```