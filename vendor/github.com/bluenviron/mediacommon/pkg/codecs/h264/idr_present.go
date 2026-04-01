package h264

// IDRPresent check whether there's an IDR inside the access unit.
func IDRPresent(au [][]byte) bool {
	for _, nalu := range au {
		typ := NALUType(nalu[0] & 0x1F)
		if typ == NALUTypeIDR {
			return true
		}
	}
	return false
}
