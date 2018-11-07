package sessions

import (
	"net/http"
	"time"

	"github.com/gorilla/context"
	"github.com/gorilla/securecookie"
	gsessions "github.com/gorilla/sessions"
	"github.com/jinzhu/gorm"
	"github.com/teris-io/shortid"
)

// Options for gormstore
type GormStoreOptions struct {
	TableName       string
	SkipCreateTable bool
}

// Store represent a gormstore
type GormStore struct {
	db          *gorm.DB
	opts        GormStoreOptions
	Codecs      []securecookie.Codec
	SessionOpts *gsessions.Options
}

type gormSession struct {
	ID        string `sql:"unique_index"`
	Data      string `sql:"type:text"`
	CreatedAt time.Time
	UpdatedAt time.Time
	ExpiresAt time.Time `sql:"index"`

	tableName string `sql:"-"` // just for convenience instead of db.Table(...)
}

// Define a type for context keys so that they can't clash with anything else stored in context
type contextKey string

func (gs *gormSession) TableName() string {
	return gs.tableName
}

// New creates a new gormstore session
func NewGormStore(db *gorm.DB, keyPairs ...[]byte) *GormStore {
	return NewGormStoreWithOptions(db, GormStoreOptions{}, keyPairs...)
}

// NewOptions creates a new gormstore session with options
func NewGormStoreWithOptions(db *gorm.DB, opts GormStoreOptions, keyPairs ...[]byte) *GormStore {
	st := &GormStore{
		db:     db,
		opts:   opts,
		Codecs: securecookie.CodecsFromPairs(keyPairs...),
		SessionOpts: &gsessions.Options{
			Path:   defaultPath,
			MaxAge: defaultMaxAge,
		},
	}
	if st.opts.TableName == "" {
		st.opts.TableName = "t_sessions"
	}

	if !st.opts.SkipCreateTable {
		st.db.AutoMigrate(&gormSession{tableName: st.opts.TableName})
	}
	st.Cleanup()
	return st
}

// Get returns a session for the given name after adding it to the registry.
func (st *GormStore) Get(r *http.Request, name string) (*gsessions.Session, error) {
	return gsessions.GetRegistry(r).Get(st, name)
}

// New creates a session with name without adding it to the registry.
func (st *GormStore) New(r *http.Request, name string) (*gsessions.Session, error) {
	session := gsessions.NewSession(st, name)
	opts := *st.SessionOpts
	session.Options = &opts
	session.IsNew = true
	st.MaxAge(st.SessionOpts.MaxAge)

	// try fetch from db if there is a cookie
	if cookie, err := r.Cookie(name); err == nil {
		session.ID = cookie.Value
		s := &gormSession{tableName: st.opts.TableName}
		if err := st.db.Where("id = ? AND expires_at > ?", session.ID, gorm.NowFunc()).First(s).Error; err != nil {
			return session, nil
		}
		if err := securecookie.DecodeMulti(session.Name(), s.Data, &session.Values, st.Codecs...); err != nil {
			return session, nil
		}
		session.IsNew = false
		context.Set(r, contextKey(name), s)
	} else {
		session.ID = shortid.MustGenerate()
	}

	return session, nil
}

func (st *GormStore) RenewID(r *http.Request, w http.ResponseWriter, session *gsessions.Session) error {
	_id := session.ID
	session.ID = shortid.MustGenerate()
	st.db.Exec("UPDATE "+st.opts.TableName+" SET id=? WHERE id=?", session.ID, _id)
	http.SetCookie(w, gsessions.NewCookie(session.Name(), session.ID, session.Options))
	return nil
}

// Save session and set cookie header
func (st *GormStore) Save(r *http.Request, w http.ResponseWriter, session *gsessions.Session) error {
	s, _ := context.Get(r, contextKey(session.Name())).(*gormSession)

	// delete if max age is < 0
	if session.Options.MaxAge < 0 || len(session.Values) == 0 {
		if s != nil {
			if err := st.db.Delete(s).Error; err != nil {
				return err
			}
		}
		return nil
	}

	data, err := securecookie.EncodeMulti(session.Name(), session.Values, st.Codecs...)
	if err != nil {
		return err
	}
	now := time.Now()
	expire := now.Add(time.Second * time.Duration(session.Options.MaxAge))

	if s == nil {
		// generate random session ID key suitable for storage in the db
		if session.ID == "" {
			session.ID = shortid.MustGenerate()
		}
		s = &gormSession{
			ID:        session.ID,
			Data:      data,
			CreatedAt: now,
			UpdatedAt: now,
			ExpiresAt: expire,
			tableName: st.opts.TableName,
		}
		if err := st.db.Create(s).Error; err != nil {
			return err
		}
		context.Set(r, contextKey(session.Name()), s)
	} else {
		s.Data = data
		s.UpdatedAt = now
		s.ExpiresAt = expire
		if err := st.db.Save(s).Error; err != nil {
			return err
		}
	}
	return nil
}

// MaxAge sets the maximum age for the store and the underlying cookie
// implementation. Individual sessions can be deleted by setting
// Options.MaxAge = -1 for that session.
func (st *GormStore) MaxAge(age int) {
	st.SessionOpts.MaxAge = age
	for _, codec := range st.Codecs {
		if sc, ok := codec.(*securecookie.SecureCookie); ok {
			sc.MaxAge(age)
		}
	}
}

func (st *GormStore) Options(options Options) {
	st.SessionOpts = &gsessions.Options{
		Path:     options.Path,
		Domain:   options.Domain,
		MaxAge:   options.MaxAge,
		Secure:   options.Secure,
		HttpOnly: options.HttpOnly,
	}
}

// MaxLength restricts the maximum length of new sessions to l.
// If l is 0 there is no limit to the size of a session, use with caution.
// The default is 4096 (default for securecookie)
func (st *GormStore) MaxLength(l int) {
	for _, c := range st.Codecs {
		if codec, ok := c.(*securecookie.SecureCookie); ok {
			codec.MaxLength(l)
		}
	}
}

// Cleanup deletes expired sessions
func (st *GormStore) Cleanup() {
	st.db.Delete(&gormSession{tableName: st.opts.TableName}, "expires_at <= ?", gorm.NowFunc())
	time.AfterFunc(15*time.Second, func() {
		st.Cleanup()
	})
}
