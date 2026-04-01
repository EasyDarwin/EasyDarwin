package h265

// IsRandomAccess checks whether the access unit is a random access point.
func IsRandomAccess(au [][]byte) bool {
	for _, nalu := range au {
		typ := NALUType((nalu[0] >> 1) & 0b111111)
		switch typ {
		case NALUType_IDR_W_RADL, NALUType_IDR_N_LP, NALUType_CRA_NUT:
			return true
		}
	}
	return false
}
