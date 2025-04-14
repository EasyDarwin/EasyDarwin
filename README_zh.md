<p align="center">
    <img src="./logo.png#gh-light-mode-only" alt="Goyave Logo" width="550"/>
    <img src="./logo_dark.png#gh-dark-mode-only" alt="Goyave Logo" width="550"/>
</p>

<p align="center">
    <a href="https://github.com/ixugo/goweb/releases"><img src="https://img.shields.io/github/v/release/ixugo/goweb?include_prereleases" alt="Version"/></a>
    <a href="https://github.com/ixugo/goweb/blob/master/LICENSE.txt"><img src="https://img.shields.io/dub/l/vibe-d.svg" alt="License"/></a>
	<a href="https://gin-gonic.com"><img width=30px  src="https://avatars.githubusercontent.com/u/7894478?s=48&v=4" alt="GIN"/></a>
    <a href="https://gorm.io"><img width=70px src="https://gorm.io/gorm.svg" alt="GORM"/></a>

</p>

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
│       └── api					RESTful API
└── pkg						依赖库
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

## 快速开始

业务说明:

假设我们要做一个版本管理的业务，curd 步骤如下:

在 「internal」-「core」 创建 「version」 目录，创建「model.go」写入领域模型，该模型为数据库表结构映射。

创建「core.go」 写入如下内容

```go
package version

import (
	"fmt"
	"strings"
)

// Storer 依赖反转的数据持久化接口
type Storer interface {
	First(*Version) error
	Add(*Version) error
}

// Core 业务对象
type Core struct {
	Storer    Storer
}

// NewCore 创建业务对象
func NewCore(store Storer) *Core {
	return &Core{
		Storer: store,
	}
}

// IsAutoMigrate 是否需要进行表迁移
// 判断硬编码在代码中的数据库表版本号，与数据库存储的版本号做对比
func (c *Core) IsAutoMigrate(currentVer, remark string) bool {
	var ver Version
	if err := c.Storer.First(&ver); err != nil {
		isMigrate := true
		c.IsMigrate = &isMigrate
		return isMigrate
	}
	isMigrate := compareVersionFunc(currentVer, ver.Version, func(a, b string) bool {
		return a > b
	})
	c.IsMigrate = &isMigrate
	return isMigrate
}

func compareVersionFunc(a, b string, f func(a, b string) bool) bool {
	s1 := versionToStr(a)
	s2 := versionToStr(b)
	if len(s1) != len(s2) {
		return true
	}
	return f(s1, s2)
}

func versionToStr(str string) string {
	var result strings.Builder
	arr := strings.Split(str, ".")
	for _, item := range arr {
		if idx := strings.Index(item, "-"); idx != -1 {
			item = item[0:idx]
		}
		result.WriteString(fmt.Sprintf("%03s", item))
	}
	return result.String()
}
```

创建 「store/versiondb」 目录，创建「db.go」 文件写入

```go
type DB struct {
	db *gorm.DB
}

func NewDB(db *gorm.DB) DB {
	return DB{db: db}
}

// AutoMigrate 表迁移
func (d DB) AutoMigrate(ok bool) DB {
	if !ok {
		return d
	}
	if err := d.db.AutoMigrate(
		new(version.Version),
	); err != nil {
		panic(err)
	}
	return d
}

func (d DB) First(v *version.Version) error {
	return d.db.Order("id DESC").First(v).Error
}

func (d DB) Add(v *version.Version) error {
	return d.db.Create(v).Error
}
```

在 API 层做依赖注入，对 「web/api/provider.go」 写入函数，往 Usecase 中注入业务对象

```go
var ProviderSet = wire.NewSet(
	wire.Struct(new(Usecase), "*"),
	NewHTTPHandler,
	NewVersion,
)

func NewVersion(db *gorm.DB) *version.Core {
	vdb := versiondb.NewDB(db)
	core := version.NewCore(vdb)
	isOK := core.IsAutoMigrate(dbVersion, dbRemark)
	vdb.AutoMigrate(isOK)
	if isOK {
		slog.Info("更新数据库表结构")
		if err := core.RecordVersion(dbVersion, dbRemark); err != nil {
			slog.Error("RecordVersion", "err", err)
		}
	}
	return core
}
```

在 API 层新建「version.go」文件，写入

```go
// version 业务函数命名空间
type VersionAPI struct {
	ver *version.Core
}

func NewVersionAPI(ver *version.Core) VersionAPI {
	return VersionAPI{ver: ver}
}
// registerVersion 向路由注册业务接口
func registerVersion(r gin.IRouter, verAPI VersionAPI, handler ...gin.HandlerFunc) {
	ver := r.Group("/version", handler...)
	ver.GET("", web.WarpH(verAPI.getVersion))
}

func (v VersionAPI) getVersion(_ *gin.Context, _ *struct{}) (any, error) {
	return gin.H{"msg": "test"}, nil
}
```


## 常见问题

> 为什么不在每一层分别定义模型?

开发效率与解耦的取舍，在代码通俗易懂和效率之间取的平衡。

> 那 api 层参数模型，表映射模型到底应该定义在哪里?

要清楚各层之间的依赖关系，api 直接依赖 core，db 依赖反转 core。故而领域模型定义在 core 中，api 的入参和出参也可以定义在 core，当然 core 层用不上的结构体，定义在 API 层也无妨。

> 为什么 api 层直接依赖 core 层，而不是依赖接口?

接口的目的是为了解耦，在实际开发过程中，更多是替换 api 层，而不是替换 core 层。

API 只做参数获取，返回响应参数，只做最少的事情，方便从 HTTP 快速过度的 GRPC。

面向未来设计，面向当下编程，先提高搬砖效率，等未来需要的那天会有更好的方式重构。

> 为什么 db 依赖反转 core?

数据持久化不是独立的，它为业务而服务。即持久化服务于业务，依赖于业务。

通过依赖反转，业务可以在中间穿插 redis cache 等其它 db 。

> 为什么入参/出参模型以  Input/Output 单词结尾

约定大于配置，类似有些项目以 Request/Response 单词结尾，只是为了有一个统一的，大家都明确的参数。

当然，有可能出参也是入参，你可以定义别名，也可以直接使用。

很多时候，我们都想明确自己在做什么，为什么这样做，这个「常见问题」希望能提供一点解惑思路。

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

## 表迁移

每次程序启动都执行一遍，太慢了。

所以通过 version 表来控制，是否要进行表迁移操作。

当发现数据库表版本已经是最新时，即不执行。通过修改 api/db.go 文件中 dbVersion 控制版本号。

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