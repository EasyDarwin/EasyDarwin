package mp4

// SampleInterval - an interval of samples including reference to or concatenated binary media data
type SampleInterval struct {
	FirstDecodeTime uint64
	Samples         []Sample
	OffsetInMdat    uint32 // Offset relative start of mdat box
	Size            uint32 // total size of all samples in interval
	Data            []byte // If set, should be relevant mdat range
}

// Reset resets sample interval while retaining allocated slice-backing arrays
func (s *SampleInterval) Reset() {
	s.FirstDecodeTime = 0
	s.Samples = s.Samples[:0]
	s.OffsetInMdat = 0
	s.Size = 0
	s.Data = s.Data[:0]
}
