package conc

import (
	"strconv"
	"testing"
	"time"
)

func TestTTLMap(t *testing.T) {
	cache := NewTTLMap[string, string]()
	cache.Store("a", "1", time.Second)
	v, ok := cache.Load("a")
	if !ok {
		t.Fatal("expect ok")
	}
	if v != "1" {
		t.Fatal("expect 1")
	}
	time.Sleep(time.Second)
	_, ok = cache.Load("a")
	if ok {
		t.Fatal("expect not ok")
	}
}

func TestDel(t *testing.T) {
	cache := NewTTLMap[string, string]()
	for i := range 10 {
		cache.Store(strconv.Itoa(i), "1", time.Second)
	}
	if l := cache.Len(); l != 10 {
		t.Fatal("expect 10, got", l)
	}
	time.Sleep(1 * time.Second)
	if l := cache.Len(); l != 0 {
		t.Fatal("expect 0, got", l)
	}
}

func TestClear(t *testing.T) {
	cache := NewTTLMap[string, string]().SwichFixedTimeClear(func() time.Duration { return 2 * time.Second })
	for i := range 10 {
		cache.Store(strconv.Itoa(i), "1", time.Second)
	}
	if l := cache.Len(); l != 10 {
		t.Fatal("expect 10, got", l)
	}
	time.Sleep(time.Second)
	if l := cache.Len(); l != 10 {
		t.Fatal("expect 10, got", l)
	}
	time.Sleep(2 * time.Second)
	if l := cache.Len(); l != 0 {
		t.Fatal("expect 0, got", l)
	}
}
