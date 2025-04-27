package video

import "sync"

// -------------------------------转码进度------------------------------
var TransProgress = MapTransProgress{Data: make(map[string]int), Lock: &sync.RWMutex{}}

type MapTransProgress struct {
	Data map[string]int
	Lock *sync.RWMutex
}

func (d MapTransProgress) Get(k string) (int, bool) {
	d.Lock.RLock()
	defer d.Lock.RUnlock()
	if v, OK := d.Data[k]; OK {
		return v, OK
	}
	return 0, false
}

func (d MapTransProgress) Set(k string, v int) {
	d.Lock.Lock()
	defer d.Lock.Unlock()
	d.Data[k] = v
}

func (d MapTransProgress) Delete(k string) {
	d.Lock.Lock()
	defer d.Lock.Unlock()
	delete(d.Data, k)
}

func (d MapTransProgress) Keys() []string {
	d.Lock.RLock()
	defer d.Lock.RUnlock()
	var keys []string
	for k := range d.Data {
		keys = append(keys, k)
	}
	return keys
}
