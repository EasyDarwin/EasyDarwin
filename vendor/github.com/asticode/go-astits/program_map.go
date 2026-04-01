package astits

// programMap represents a program ids map
type programMap struct {
	// We use map[uint32] instead map[uint16] as go runtime provide optimized hash functions for (u)int32/64 keys
	p map[uint32]uint16 // map[ProgramMapID]ProgramNumber
}

// newProgramMap creates a new program ids map
func newProgramMap() *programMap {
	return &programMap{
		p: make(map[uint32]uint16),
	}
}

// existsUnlocked checks whether the program with this pid exists
func (m programMap) existsUnlocked(pid uint16) (ok bool) {
	_, ok = m.p[uint32(pid)]
	return
}

// setUnlocked sets a new program id
func (m programMap) setUnlocked(pid, number uint16) {
	m.p[uint32(pid)] = number
}

func (m programMap) unsetUnlocked(pid uint16) {
	delete(m.p, uint32(pid))
}

func (m programMap) toPATDataUnlocked() *PATData {
	d := &PATData{
		Programs:          make([]*PATProgram, 0, len(m.p)),
		TransportStreamID: uint16(PSITableIDPAT),
	}

	for pid, pnr := range m.p {
		d.Programs = append(d.Programs, &PATProgram{
			ProgramMapID:  uint16(pid),
			ProgramNumber: pnr,
		})
	}

	return d
}
