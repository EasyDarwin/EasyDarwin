package web

import (
	"sync"
	"time"

	"github.com/gin-gonic/gin"
	"golang.org/x/time/rate"
)

// RateLimiter 限流器
func RateLimiter(r rate.Limit, b int) gin.HandlerFunc {
	l := rate.NewLimiter(rate.Limit(r), b)
	return func(c *gin.Context) {
		if !l.Allow() {
			c.AbortWithStatusJSON(400, gin.H{"msg": "服务器繁忙"})
			return
		}
		c.Next()
	}
}

type client struct {
	limiter    *rate.Limiter
	lastSeenAt time.Time
}

// IPRateLimiter IP 限流器
func IPRateLimiterForGin(r rate.Limit, b int) gin.HandlerFunc {
	limiter := IPRateLimiter(r, b)
	return func(c *gin.Context) {
		ip := c.RemoteIP()
		if !limiter(ip) {
			c.AbortWithStatusJSON(400, gin.H{"msg": "服务器繁忙"})
			return
		}
		c.Next()
	}
}

// IPRateLimiter IP 限流器
func IPRateLimiter(r rate.Limit, b int) func(ip string) bool {
	var m sync.Mutex
	clients := make(map[string]*client)
	// 定时清理
	go func() {
		ticker := time.NewTicker(30 * time.Second)
		defer ticker.Stop()
		for {
			<-ticker.C
			m.Lock()
			for k, v := range clients {
				if time.Since(v.lastSeenAt) > 3*time.Minute {
					delete(clients, k)
				}
			}
			m.Unlock()
		}
	}()
	return func(ip string) bool {
		m.Lock()
		v, exist := clients[ip]
		if !exist {
			v = &client{limiter: rate.NewLimiter(r, b), lastSeenAt: time.Now()}
			clients[ip] = v
		}
		v.lastSeenAt = time.Now()
		m.Unlock()
		return v.limiter.Allow()
	}
}
