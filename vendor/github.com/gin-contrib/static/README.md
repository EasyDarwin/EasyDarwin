# static middleware

[![Run Tests](https://github.com/gin-contrib/static/actions/workflows/go.yml/badge.svg)](https://github.com/gin-contrib/static/actions/workflows/go.yml)
[![codecov](https://codecov.io/gh/gin-contrib/static/branch/master/graph/badge.svg)](https://codecov.io/gh/gin-contrib/static)
[![Go Report Card](https://goreportcard.com/badge/github.com/gin-contrib/static)](https://goreportcard.com/report/github.com/gin-contrib/static)
[![GoDoc](https://godoc.org/github.com/gin-contrib/static?status.svg)](https://godoc.org/github.com/gin-contrib/static)

Static middleware

## Usage

### Start using it

Download and install it:

```sh
go get github.com/gin-contrib/static
```

Import it in your code:

```go
import "github.com/gin-contrib/static"
```

### Canonical example

See the [example](_example)

#### Serve local file

```go
package main

import (
  "github.com/gin-contrib/static"
  "github.com/gin-gonic/gin"
)

func main() {
  r := gin.Default()

  // if Allow DirectoryIndex
  //r.Use(static.Serve("/", static.LocalFile("/tmp", true)))
  // set prefix
  //r.Use(static.Serve("/static", static.LocalFile("/tmp", true)))

  r.Use(static.Serve("/", static.LocalFile("/tmp", false)))
  r.GET("/ping", func(c *gin.Context) {
    c.String(200, "test")
  })
  // Listen and Server in 0.0.0.0:8080
  r.Run(":8080")
}
```

#### Serve embed folder

```go
package main

import (
  "embed"
  "fmt"
  "net/http"

  "github.com/gin-contrib/static"
  "github.com/gin-gonic/gin"
)

//go:embed data
var server embed.FS

func main() {
  r := gin.Default()
  r.Use(static.Serve("/", static.EmbedFolder(server, "data/server")))
  r.GET("/ping", func(c *gin.Context) {
    c.String(200, "test")
  })
  r.NoRoute(func(c *gin.Context) {
    fmt.Printf("%s doesn't exists, redirect on /\n", c.Request.URL.Path)
    c.Redirect(http.StatusMovedPermanently, "/")
  })
  // Listen and Server in 0.0.0.0:8080
  r.Run(":8080")
  if err := r.Run(":8080"); err != nil {
    log.Fatal(err)
  }
}
```
