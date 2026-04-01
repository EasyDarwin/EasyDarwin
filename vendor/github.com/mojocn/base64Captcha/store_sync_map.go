package base64Captcha

import (
	"strings"
	"sync"
	"time"
)

// StoreSyncMap use sync.Map as store
type StoreSyncMap struct {
	liveTime time.Duration
	m        *sync.Map
}

// NewStoreSyncMap new a instance
func NewStoreSyncMap(liveTime time.Duration) *StoreSyncMap {
	return &StoreSyncMap{liveTime: liveTime, m: new(sync.Map)}
}

// smv a value type
type smv struct {
	t     time.Time
	Value string
}

// newSmv create a instance
func newSmv(v string) *smv {
	return &smv{t: time.Now(), Value: v}
}

// rmExpire remove expired items
func (s StoreSyncMap) rmExpire() {
	expireTime := time.Now().Add(-s.liveTime)
	s.m.Range(func(key, value interface{}) bool {
		if sv, ok := value.(*smv); ok && sv.t.Before(expireTime) {
			s.m.Delete(key)
		}
		return true
	})
}

// Get get a string value
func (s StoreSyncMap) Set(id string, value string) {
	s.rmExpire()
	s.m.Store(id, newSmv(value))
}

// Set a string value
func (s StoreSyncMap) Get(id string, clear bool) string {
	v, ok := s.m.Load(id)
	if !ok {
		return ""
	}
	s.m.Delete(id)
	if sv, ok := v.(*smv); ok {
		return sv.Value
	}
	return ""
}

// Verify check a string value
func (s StoreSyncMap) Verify(id, answer string, clear bool) bool {
	vv := s.Get(id, clear)
    return strings.EqualFold(vv, answer)
}
