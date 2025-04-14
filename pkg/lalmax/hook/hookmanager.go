package hook

import (
	"sync"

	"github.com/q191201771/naza/pkg/nazalog"
)

type HookSessionMangaer struct {
	sessionMap sync.Map
}

var (
	manager *HookSessionMangaer
	once    sync.Once
)

func GetHookSessionManagerInstance() *HookSessionMangaer {
	once.Do(func() {
		manager = &HookSessionMangaer{}
	})

	return manager
}

func (m *HookSessionMangaer) SetHookSession(streamName string, session *HookSession) {
	nazalog.Info("SetHookSession, streamName:", streamName)
	m.sessionMap.Store(streamName, session)
}

func (m *HookSessionMangaer) RemoveHookSession(streamName string) {
	nazalog.Info("RemoveHookSession, streamName:", streamName)
	// s, ok := m.sessionMap.Load(streamName)
	// if ok {
	m.sessionMap.Delete(streamName)
	// }
}

func (m *HookSessionMangaer) GetHookSession(streamName string) (bool, *HookSession) {
	s, ok := m.sessionMap.Load(streamName)
	if ok {
		return true, s.(*HookSession)
	}

	return false, nil
}
