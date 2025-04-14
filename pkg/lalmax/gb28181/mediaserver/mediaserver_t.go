package mediaserver

type MediaInfo struct {
	IsInvite     bool
	Ssrc         uint32
	StreamName   string
	SinglePort   bool
	DumpFileName string
	MediaKey     string
}

func (m *MediaInfo) Clear() (err error) {
	m.IsInvite = false
	m.Ssrc = 0
	m.StreamName = ""
	m.SinglePort = false
	m.DumpFileName = ""

	return
}
