# sessions

[![Build Status](https://travis-ci.org/gin-contrib/sessions.svg)](https://travis-ci.org/gin-contrib/sessions)
[![codecov](https://codecov.io/gh/gin-contrib/sessions/branch/master/graph/badge.svg)](https://codecov.io/gh/gin-contrib/sessions)
[![Go Report Card](https://goreportcard.com/badge/github.com/penggy/sessions)](https://goreportcard.com/report/github.com/penggy/sessions)
[![GoDoc](https://godoc.org/github.com/penggy/sessions?status.svg)](https://godoc.org/github.com/penggy/sessions)
[![Join the chat at https://gitter.im/gin-gonic/gin](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/gin-gonic/gin)

Gin middleware for session management with multi-backend support (currently cookie, Redis, Memcached, MongoDB, memstore).

## Usage

### Start using it

Download and install it:

```bash
$ go get github.com/penggy/sessions
```

Import it in your code:

```go
import "github.com/penggy/sessions"
```

## Examples

#### cookie-based

[embedmd]:# (example_cookie/main.go go)
```go
package main

import (
	"github.com/penggy/sessions"
	"github.com/penggy/sessions/cookie"
	"github.com/gin-gonic/gin"
)

func main() {
	r := gin.Default()
	store := cookie.NewStore([]byte("secret"))
	r.Use(sessions.Sessions("mysession", store))

	r.GET("/incr", func(c *gin.Context) {
		session := sessions.Default(c)
		var count int
		v := session.Get("count")
		if v == nil {
			count = 0
		} else {
			count = v.(int)
			count++
		}
		session.Set("count", count)
		session.Save()
		c.JSON(200, gin.H{"count": count})
	})
	r.Run(":8000")
}
```

#### Redis

[embedmd]:# (example_redis/main.go go)
```go
package main

import (
	"github.com/penggy/sessions"
	"github.com/penggy/sessions/redis"
	"github.com/gin-gonic/gin"
)

func main() {
	r := gin.Default()
	store, _ := redis.NewStore(10, "tcp", "localhost:6379", "", []byte("secret"))
	r.Use(sessions.Sessions("mysession", store))

	r.GET("/incr", func(c *gin.Context) {
		session := sessions.Default(c)
		var count int
		v := session.Get("count")
		if v == nil {
			count = 0
		} else {
			count = v.(int)
			count++
		}
		session.Set("count", count)
		session.Save()
		c.JSON(200, gin.H{"count": count})
	})
	r.Run(":8000")
}
```

#### Memcached

[embedmd]:# (example_memcached/main.go go)
```go
package main

import (
	"github.com/bradfitz/gomemcache/memcache"
	"github.com/penggy/sessions"
	"github.com/penggy/sessions/memcached"
	"github.com/gin-gonic/gin"
)

func main() {
	r := gin.Default()
	store := memcached.NewStore(memcache.New("localhost:11211"), "", []byte("secret"))
	r.Use(sessions.Sessions("mysession", store))

	r.GET("/incr", func(c *gin.Context) {
		session := sessions.Default(c)
		var count int
		v := session.Get("count")
		if v == nil {
			count = 0
		} else {
			count = v.(int)
			count++
		}
		session.Set("count", count)
		session.Save()
		c.JSON(200, gin.H{"count": count})
	})
	r.Run(":8000")
}
```

#### MongoDB

[embedmd]:# (example_mongo/main.go go)
```go
package main

import (
	"github.com/penggy/sessions"
	"github.com/penggy/sessions/mongo"
	"github.com/gin-gonic/gin"
	"gopkg.in/mgo.v2"
)

func main() {
	r := gin.Default()
	session, err := mgo.Dial("localhost:27017/test")
	if err != nil {
		// handle err
	}

	c := session.DB("").C("sessions")
	store := mongo.NewStore(c, 3600, true, []byte("secret"))
	r.Use(sessions.Sessions("mysession", store))

	r.GET("/incr", func(c *gin.Context) {
		session := sessions.Default(c)
		var count int
		v := session.Get("count")
		if v == nil {
			count = 0
		} else {
			count = v.(int)
			count++
		}
		session.Set("count", count)
		session.Save()
		c.JSON(200, gin.H{"count": count})
	})
	r.Run(":8000")
}
```

#### memstore

[embedmd]:# (example_memstore/main.go go)
```go
package main

import (
	"github.com/penggy/sessions"
	"github.com/penggy/sessions/memstore"
	"github.com/gin-gonic/gin"
)

func main() {
	r := gin.Default()
	store := memstore.NewStore([]byte("secret"))
	r.Use(sessions.Sessions("mysession", store))

	r.GET("/incr", func(c *gin.Context) {
		session := sessions.Default(c)
		var count int
		v := session.Get("count")
		if v == nil {
			count = 0
		} else {
			count = v.(int)
			count++
		}
		session.Set("count", count)
		session.Save()
		c.JSON(200, gin.H{"count": count})
	})
	r.Run(":8000")
}
```
