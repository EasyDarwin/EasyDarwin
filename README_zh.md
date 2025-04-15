[English](./README.md) | [简体中文](./README_zh.md)

# 企业 REST API 模板

这是一个专注于 REST API 的完整 CURD 解决方案。

Goweb 目标是:

+ 整洁架构，适用于中小型项目
+ 提供积木套装，快速开始项目，专注于业务开发
+ 令项目更简单，令研发心情更好

如果你觉得以上描述符合你的需求，那就基于此模板开始吧。此项目会源源不断补充如何充分使用的文档指南。

支持[代码自动生成](github.com/ixugo/gowebx)

## 引用文章

[Google API Design Guide](https://google-cloud.gitbook.io/api-design-guide)



## 目录说明


```bash
.
├── cmd						可执行程序
│   └── server
├── configs					配置文件
├── docs					设计文档/用户文档
├── internal					私有业务
│   ├── conf					配置模型
│   ├── core					业务领域
│   │   └── version				实际业务
│   │       └── store
│   │           └── versiondb 		数据库操作
│   ├── data					数据库初始化
│   └── web
│   |   └── api					RESTful API
|   └—— pkg                 依赖库
|   └—— utils               工具
```


## 项目说明

1. 程序启动强依赖的组件，发生异常时主动 panic，尽快崩溃尽快解决错误。

2. core 为业务领域，包含领域模型，领域业务功能

3. store 为数据库操作模块，需要依赖模型，此处依赖反转 core，避免每一层都定义模型。

4. api 层的入参/出参，可以正向依赖 core 层定义模型，参数模型以 `Input/Output` 来简单区分入参出数。

## Makefile

Windows 系统使用 makefile 时，请使用 git bash 终端，不要使用系统默认的 cmd/powershell 终端，否则可能会出现异常情况。

执行 `make` 或 `make help` 来获取更多帮助

在编写 makefile 时，应主动在命令上面增加注释，以 `## <命令>: <描述>` 格式书写，具体参数 Makefile 文件已有命令。其目的是 `make help` 时提供更多信息。

makefile 中提供了一些默认的操作便于快速编写

`make confirm` 用于确认下一步

`make title content=标题`  用于重点突出输出标题

`make info` 获取构建版本相关信息

**makefile 构建的版本号规则说明**

1. 版本号使用 Git tag，格式为 v1.0.0。

2. 如果当前提交没有 tag，找到最近的 tag，计算从该 tag 到当前提交的提交次数。例如，最近的 tag 为 v1.0.1，当前提交距离它有 10 次提交，则版本号为 v1.0.11（v1.0.1 + 10 次提交）。

3. 如果没有任何 tag，则默认版本号为 v0.0.0，后续提交次数作为版本号的次版本号。


## 常见问题

> 为什么不在每一层分别定义模型?

开发效率与解耦的取舍，在代码通俗易懂和效率之间取的平衡。

> 那 api 层参数模型，表映射模型到底应该定义在哪里?

要清楚各层之间的依赖关系，api 直接依赖 data目录，db 依赖反转 data目录。故而领域模型定义在 data 中，api 的入参和出参也可以定义在 data，当然 data 层用不上的结构体，定义在 API 层也无妨。

> 如何为 goweb 编写业务插件?

```go
// RegisterVersion 有一些通用的业务，它们被其它业务依赖，属于业务的基层模块，例如表版本控制，字典，验证码，定时任务，用户管理等等。
// 约定以 Register<Core> 方式编写函数，注入 gin 路由，命名空间，中间件三个参数。
// 具体可以参考项目代码
func RegisterVersion(r gin.IRouter, verAPI VersionAPI, handler ...gin.HandlerFunc) {
	ver := r.Group("/version", handler...)
	ver.GET("", web.WarpH(verAPI.getVersion))
}
```

## 错误处理

core 层导出的函数或 API 层返回的错误，应该返回 web.Error 类型的错误。

在封装的 web.WarpH 中，会正确记录错误到日志并返回给前端。

```go
type Error struct {
	reason  string   // 错误原因
	msg     string   // 错误信息，用户可读
	details []string // 错误扩展，开发可读
}
```

reason 是预定义的错误原因，以英文单词定义，同时用于区分返回的 http response status code。

msg 是展示给用户看的内容。

details 仅开发模式使用，将完整的错误内容暴露给开发者，方便前后端开发调试。

## 自定义配置目录

默认配置目录为可执行文件同目录下的 configs，也可以指定其它配置目录

`./bin -conf ./configs`



## 项目主要依赖

+ gin
+ gorm
+ slog / zap
+ wire
+ lal