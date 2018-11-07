package cors

import (
	"errors"
	"strings"
	"time"

	"github.com/gin-gonic/gin"
)

// Config represents all available options for the middleware.
type Config struct {
	AllowAllOrigins bool

	// AllowedOrigins is a list of origins a cross-domain request can be executed from.
	// If the special "*" value is present in the list, all origins will be allowed.
	// Default value is []
	AllowOrigins []string

	// AllowOriginFunc is a custom function to validate the origin. It take the origin
	// as argument and returns true if allowed or false otherwise. If this option is
	// set, the content of AllowedOrigins is ignored.
	AllowOriginFunc func(origin string) bool

	// AllowedMethods is a list of methods the client is allowed to use with
	// cross-domain requests. Default value is simple methods (GET and POST)
	AllowMethods []string

	// AllowedHeaders is list of non simple headers the client is allowed to use with
	// cross-domain requests.
	AllowHeaders []string

	// AllowCredentials indicates whether the request can include user credentials like
	// cookies, HTTP authentication or client side SSL certificates.
	AllowCredentials bool

	// ExposedHeaders indicates which headers are safe to expose to the API of a CORS
	// API specification
	ExposeHeaders []string

	// MaxAge indicates how long (in seconds) the results of a preflight request
	// can be cached
	MaxAge time.Duration
}

// AddAllowMethods is allowed to add custom methods
func (c *Config) AddAllowMethods(methods ...string) {
	c.AllowMethods = append(c.AllowMethods, methods...)
}

// AddAllowHeaders is allowed to add custom headers
func (c *Config) AddAllowHeaders(headers ...string) {
	c.AllowHeaders = append(c.AllowHeaders, headers...)
}

// AddExposeHeaders is allowed to add custom expose headers
func (c *Config) AddExposeHeaders(headers ...string) {
	c.ExposeHeaders = append(c.ExposeHeaders, headers...)
}

// Validate is check configuration of user defined.
func (c Config) Validate() error {
	if c.AllowAllOrigins && (c.AllowOriginFunc != nil || len(c.AllowOrigins) > 0) {
		return errors.New("conflict settings: all origins are allowed. AllowOriginFunc or AllowedOrigins is not needed")
	}
	if !c.AllowAllOrigins && c.AllowOriginFunc == nil && len(c.AllowOrigins) == 0 {
		return errors.New("conflict settings: all origins disabled")
	}
	for _, origin := range c.AllowOrigins {
		if origin != "*" && !strings.HasPrefix(origin, "http://") && !strings.HasPrefix(origin, "https://") {
			return errors.New("bad origin: origins must either be '*' or include http:// or https://")
		}
	}
	return nil
}

// DefaultConfig returns a generic default configuration mapped to localhost.
func DefaultConfig() Config {
	return Config{
		AllowMethods:     []string{"GET", "POST", "PUT", "PATCH", "DELETE", "HEAD"},
		AllowHeaders:     []string{"Origin", "Content-Length", "Content-Type"},
		AllowCredentials: true,
		AllowOriginFunc:  func(origin string) bool { return true },
		MaxAge:           12 * time.Hour,
	}
}

// Default returns the location middleware with default configuration.
func Default() gin.HandlerFunc {
	config := DefaultConfig()
	return New(config)
}

// New returns the location middleware with user-defined custom configuration.
func New(config Config) gin.HandlerFunc {
	cors := newCors(config)
	return func(c *gin.Context) {
		cors.applyCors(c)
	}
}
