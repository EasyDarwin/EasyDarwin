package sessions

import (
	"encoding/json"
	"errors"
	"fmt"
	"net/http"
	"time"

	"github.com/go-redis/redis"
	"github.com/gorilla/securecookie"
	gsessions "github.com/gorilla/sessions"
	"github.com/teris-io/shortid"
)

// RediStore stores sessions in a redis backend.
type RediStore struct {
	RedisClient    *redis.Client
	Codecs         []securecookie.Codec
	SessionOptions *gsessions.Options // default configuration
	DefaultMaxAge  int                // default Redis TTL for a MaxAge == 0 session
	maxLength      int
	keyPrefix      string
	serializer     SessionSerializer
}

func NewRediStore(client *redis.Client, keyPrefix string, keyPairs ...[]byte) *RediStore {
	if keyPrefix == "" {
		keyPrefix = "session_"
	}
	rs := &RediStore{
		RedisClient: client,
		Codecs:      securecookie.CodecsFromPairs(keyPairs...),
		SessionOptions: &gsessions.Options{
			Path:   defaultPath,
			MaxAge: defaultMaxAge,
		},
		DefaultMaxAge: 60 * 20, // 20 minutes seems like a reasonable default
		maxLength:     4096,
		keyPrefix:     keyPrefix,
		serializer:    JSONSerializer{},
	}
	return rs
}

// SessionSerializer provides an interface hook for alternative serializers
type SessionSerializer interface {
	Deserialize(d []byte, ss *gsessions.Session) error
	Serialize(ss *gsessions.Session) ([]byte, error)
}

// JSONSerializer encode the session map to JSON.
type JSONSerializer struct{}

// Serialize to JSON. Will err if there are unmarshalable key values
func (s JSONSerializer) Serialize(ss *gsessions.Session) ([]byte, error) {
	m := make(map[string]interface{}, len(ss.Values))
	for k, v := range ss.Values {
		ks, ok := k.(string)
		if !ok {
			err := fmt.Errorf("Non-string key value, cannot serialize session to JSON: %v", k)
			fmt.Printf("redistore.JSONSerializer.serialize() Error: %v", err)
			return nil, err
		}
		m[ks] = v
	}
	return json.Marshal(m)
}

// Deserialize back to map[string]interface{}
func (s JSONSerializer) Deserialize(d []byte, ss *gsessions.Session) error {
	m := make(map[string]interface{})
	err := json.Unmarshal(d, &m)
	if err != nil {
		fmt.Printf("redistore.JSONSerializer.deserialize() Error: %v", err)
		return err
	}
	for k, v := range m {
		ss.Values[k] = v
	}
	return nil
}

// SetMaxLength sets RediStore.maxLength if the `l` argument is greater or equal 0
// maxLength restricts the maximum length of new sessions to l.
// If l is 0 there is no limit to the size of a session, use with caution.
// The default for a new RediStore is 4096. Redis allows for max.
// value sizes of up to 512MB (http://redis.io/topics/data-types)
// Default: 4096,
func (s *RediStore) SetMaxLength(l int) {
	if l >= 0 {
		s.maxLength = l
	}
}

func (s *RediStore) Options(options Options) {
	s.SessionOptions = &gsessions.Options{
		Path:     options.Path,
		Domain:   options.Domain,
		MaxAge:   options.MaxAge,
		Secure:   options.Secure,
		HttpOnly: options.HttpOnly,
	}
}

// SetSerializer sets the serializer
func (s *RediStore) SetSerializer(ss SessionSerializer) {
	s.serializer = ss
}

// SetMaxAge restricts the maximum age, in seconds, of the session record
// both in database and a browser. This is to change session storage configuration.
// If you want just to remove session use your session `s` object and change it's
// `Options.MaxAge` to -1, as specified in
//    http://godoc.org/github.com/gorilla/sessions#Options
//
// Default is the one provided by this package value - `sessionExpire`.
// Set it to 0 for no restriction.
// Because we use `MaxAge` also in SecureCookie crypting algorithm you should
// use this function to change `MaxAge` value.
func (s *RediStore) SetMaxAge(v int) {
	var c *securecookie.SecureCookie
	var ok bool
	s.SessionOptions.MaxAge = v
	for i := range s.Codecs {
		if c, ok = s.Codecs[i].(*securecookie.SecureCookie); ok {
			c.MaxAge(v)
		} else {
			fmt.Printf("Can't change MaxAge on codec %v\n", s.Codecs[i])
		}
	}
}

// Get returns a session for the given name after adding it to the registry.
//
// See gorilla/sessions FilesystemStore.Get().
func (s *RediStore) Get(r *http.Request, name string) (*gsessions.Session, error) {
	return gsessions.GetRegistry(r).Get(s, name)
}

// New returns a session for the given name without adding it to the registry.
//
// See gorilla/sessions FilesystemStore.New().
func (s *RediStore) New(r *http.Request, name string) (*gsessions.Session, error) {
	var (
		err error
		ok  bool
	)
	session := gsessions.NewSession(s, name)
	// make a copy
	options := *s.SessionOptions
	session.Options = &options
	session.IsNew = true
	if c, errCookie := r.Cookie(name); errCookie == nil {
		session.ID = c.Value
		ok, err = s.load(session)
		session.IsNew = !(err == nil && ok)
	} else {
		session.ID = shortid.MustGenerate()
	}
	return session, err
}

func (s *RediStore) RenewID(r *http.Request, w http.ResponseWriter, session *gsessions.Session) error {
	_id := session.ID
	data, err := s.RedisClient.Get(s.keyPrefix + _id).Result()
	if err != nil {
		return err
	}
	session.ID = shortid.MustGenerate()
	age := session.Options.MaxAge
	if age == 0 {
		age = s.DefaultMaxAge
	}
	_, err = s.RedisClient.Set(s.keyPrefix+session.ID, data, time.Duration(age)*time.Second).Result()
	if err != nil {
		return err
	}
	http.SetCookie(w, gsessions.NewCookie(session.Name(), session.ID, session.Options))
	return nil
}

// Save adds a single session to the response.
func (s *RediStore) Save(r *http.Request, w http.ResponseWriter, session *gsessions.Session) error {
	// Marked for deletion.
	if session.Options.MaxAge < 0 || len(session.Values) == 0 {
		if err := s.delete(session); err != nil {
			return err
		}
		return nil
	} else {
		// Build an alphanumeric key for the redis store.
		if session.ID == "" {
			session.ID = shortid.MustGenerate()
		}
		if err := s.save(session); err != nil {
			return err
		}
	}
	return nil
}

// save stores the session in redis.
func (s *RediStore) save(session *gsessions.Session) error {
	b, err := s.serializer.Serialize(session)
	if err != nil {
		return err
	}
	if s.maxLength != 0 && len(b) > s.maxLength {
		return errors.New("SessionStore: the value to store is too big")
	}
	age := session.Options.MaxAge
	if age == 0 {
		age = s.DefaultMaxAge
	}
	_, err = s.RedisClient.Set(s.keyPrefix+session.ID, b, time.Duration(age)*time.Second).Result()
	return err
}

// load reads the session from redis.
// returns true if there is a sessoin data in DB
func (s *RediStore) load(session *gsessions.Session) (bool, error) {
	data, err := s.RedisClient.Get(s.keyPrefix + session.ID).Result()
	if err != nil {
		return false, err
	}
	if data == "" {
		return false, nil // no data was associated with this key
	}
	return true, s.serializer.Deserialize([]byte(data), session)
}

// delete removes keys from redis if MaxAge<0
func (s *RediStore) delete(session *gsessions.Session) error {
	_, err := s.RedisClient.Del(s.keyPrefix + session.ID).Result()
	return err
}
