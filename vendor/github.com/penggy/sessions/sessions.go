package sessions

import (
	"log"
	"net/http"
	"regexp"
	"strings"

	"github.com/gin-gonic/gin"
	"github.com/gorilla/context"
	"github.com/gorilla/sessions"
)

const (
	DefaultKey    = "github.com/penggy/sessions"
	errorFormat   = "[sessions] ERROR! %s\n"
	defaultMaxAge = 60 * 60 * 24 * 30 // 30 days
	defaultPath   = "/"
)

type Store interface {
	sessions.Store
	RenewID(r *http.Request, w http.ResponseWriter, gsession *sessions.Session) error
	Options(Options)
}

// Options stores configuration for a session or session store.
// Fields are a subset of http.Cookie fields.
type Options struct {
	Path   string
	Domain string
	// MaxAge=0 means no 'Max-Age' attribute specified.
	// MaxAge<0 means delete cookie now, equivalently 'Max-Age: 0'.
	// MaxAge>0 means Max-Age attribute present and given in seconds.
	MaxAge   int
	Secure   bool
	HttpOnly bool
}

// Wraps thinly gorilla-session methods.
// Session stores the values and optional configuration for a session.
type Session interface {
	// Get returns the session value associated to the given key.
	Get(key interface{}) interface{}
	// Set sets the session value associated to the given key.
	Set(key interface{}, val interface{})
	// Delete removes the session value associated to the given key.
	Delete(key interface{})
	// Clear deletes all values in the session.
	Clear()
	// AddFlash adds a flash message to the session.
	// A single variadic argument is accepted, and it is optional: it defines the flash key.
	// If not defined "_flash" is used by default.
	AddFlash(value interface{}, vars ...string)
	// Flashes returns a slice of flash messages from the session.
	// A single variadic argument is accepted, and it is optional: it defines the flash key.
	// If not defined "_flash" is used by default.
	Flashes(vars ...string) []interface{}
	// Options sets confuguration for a session.
	Options(Options)
	// Save saves all sessions used during the current request.
	Save() error

	RenewID() (string, error)

	ID() string

	SetMaxAge(maxAge int)

	Destroy()
}

func Sessions(name string, store Store) gin.HandlerFunc {
	return func(c *gin.Context) {
		s := &session{name, c.Request, store, nil, false, c.Writer, false}
		c.Set(DefaultKey, s)
		defer context.Clear(c.Request)
		defer s.Save()
		http.SetCookie(s.writer, sessions.NewCookie(s.name, s.ID(), s.Session().Options))
		c.Next()
	}
}

func GorillaSessions(name string, store Store) gin.HandlerFunc {
	return func(c *gin.Context) {
		s := &session{name, c.Request, store, nil, false, c.Writer, true}
		c.Set(DefaultKey, s)
		defer context.Clear(c.Request)
		c.Next()
	}
}

type session struct {
	name    string
	request *http.Request
	store   Store
	session *sessions.Session
	written bool
	writer  http.ResponseWriter
	gorilla bool
}

func (s *session) Get(key interface{}) interface{} {
	return s.Session().Values[key]
}

func (s *session) Set(key interface{}, val interface{}) {
	s.Session().Values[key] = val
	s.written = true
}

func (s *session) Delete(key interface{}) {
	delete(s.Session().Values, key)
	s.written = true
}

func (s *session) Clear() {
	for key := range s.Session().Values {
		delete(s.Session().Values, key)
	}
	s.written = true
}

func (s *session) AddFlash(value interface{}, vars ...string) {
	s.Session().AddFlash(value, vars...)
}

func (s *session) Flashes(vars ...string) []interface{} {
	return s.Session().Flashes(vars...)
}

func (s *session) Options(options Options) {
	s.Session().Options = &sessions.Options{
		Path:     options.Path,
		Domain:   options.Domain,
		MaxAge:   options.MaxAge,
		Secure:   options.Secure,
		HttpOnly: options.HttpOnly,
	}
}

func (s *session) Save() error {
	if s.Written() {
		e := s.Session().Save(s.request, s.writer)
		if e == nil {
			s.written = false
		}
		return e
	}
	return nil
}

func (s *session) RenewID() (string, error) {
	e := s.store.RenewID(s.request, s.writer, s.Session())
	return s.ID(), e
}

func (s *session) ID() string {
	return s.Session().ID
}

func (s *session) SetMaxAge(maxAge int) {
	s.Session().Options.MaxAge = maxAge
	if s.gorilla {
		s.written = true
	} else {
		http.SetCookie(s.writer, sessions.NewCookie(s.name, s.Session().ID, s.Session().Options))
	}
}

func (s *session) Destroy() {
	s.SetMaxAge(-1)
	s.Clear()
}

func (s *session) Written() bool {
	return s.written
}

func (s *session) Session() *sessions.Session {
	if s.session == nil {
		var err error
		s.session, err = s.store.Get(s.request, s.name)
		if err != nil {
			log.Printf(errorFormat, err)
		}
	}
	return s.session
}

func (s *session) XHR() bool {
	if strings.EqualFold(s.request.Header.Get("x-requested-with"), "XMLHttpRequest") {
		return true
	}
	return regexp.MustCompile("\\/json$").MatchString(s.request.Header.Get("accept"))
}

// shortcut to get session
func Default(c *gin.Context) Session {
	return c.MustGet(DefaultKey).(Session)
}
