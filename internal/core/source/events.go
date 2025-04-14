package source

import "sync"

type EventListener struct {
	Name       string
	HandleFunc func(...interface{}) bool
}
type EventDispatcher struct {
	listeners     map[*EventListener]bool
	listenersLock *sync.RWMutex
}

func NewListener(name string, f func(...interface{}) bool) (el *EventListener) {
	el = &EventListener{
		Name:       name,
		HandleFunc: f,
	}
	return
}

func NewDispatcher() (ed *EventDispatcher) {
	ed = &EventDispatcher{
		listeners:     make(map[*EventListener]bool),
		listenersLock: &sync.RWMutex{},
	}
	return
}

func (ed *EventDispatcher) AddListener(el *EventListener) {
	ed.listenersLock.Lock()
	defer ed.listenersLock.Unlock()
	ed.listeners[el] = true
}

func (ed *EventDispatcher) RemoveListener(el *EventListener) {
	ed.listenersLock.Lock()
	defer ed.listenersLock.Unlock()
	delete(ed.listeners, el)
}

func (ed *EventDispatcher) ClearListeners() {
	ed.listenersLock.Lock()
	defer ed.listenersLock.Unlock()
	for el, _ := range ed.listeners {
		delete(ed.listeners, el)
	}
}

func (ed *EventDispatcher) GetListeners(name string) (els []*EventListener) {
	els = make([]*EventListener, 0)
	ed.listenersLock.RLock()
	defer ed.listenersLock.RUnlock()
	for el, _ := range ed.listeners {
		if el.Name == name {
			els = append(els, el)
		}
	}
	return
}

func (ed *EventDispatcher) Dispatch(name string, msg ...interface{}) {
	els := ed.GetListeners(name)
	for _, el := range els {
		if !el.HandleFunc(msg...) {
			break
		}
	}
}
