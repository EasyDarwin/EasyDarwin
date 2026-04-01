package astits

import "sync"

// bytesPool global variable is used to ease access to pool from any place of the code
var bytesPool = &bytesPooler{
	sp: sync.Pool{
		New: func() interface{} {
			// Prepare the slice of somewhat sensible initial size to minimize calls to runtime.growslice
			return &bytesPoolItem{
				s: make([]byte, 0, 1024),
			}
		},
	},
}

// bytesPoolItem is an object containing payload slice
type bytesPoolItem struct {
	s []byte
}

// bytesPooler is a pool for temporary payload in parseData()
// Don't use it anywhere else to avoid pool pollution
type bytesPooler struct {
	sp sync.Pool
}

// get returns the bytesPoolItem object with byte slice of a 'size' length
func (bp *bytesPooler) get(size int) (payload *bytesPoolItem) {
	payload = bp.sp.Get().(*bytesPoolItem)
	// Reset slice length or grow it to requested size for use with copy
	if cap(payload.s) >= size {
		payload.s = payload.s[:size]
	} else {
		n := size - cap(payload.s)
		payload.s = append(payload.s[:cap(payload.s)], make([]byte, n)...)[:size]
	}
	return
}

// put returns reference to the payload slice back to pool
// Don't use the payload after a call to put
func (bp *bytesPooler) put(payload *bytesPoolItem) {
	bp.sp.Put(payload)
}
