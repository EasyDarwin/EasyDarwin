package cloud

import (
	"fmt"
	"io"

	"github.com/abema/go-mp4"
)

type Fmp4 struct {
	lastDuration uint32
	sum          uint64
	firstMoov    bool
	firstFtyp    bool
	wr           *mp4.Writer

	num uint32
	// first               bool
	baseMediaDecodeTime map[uint32]uint64

	isStart bool

	lastSegmentBaseMediaDecodeTime uint64
}

func NewFmp4(w io.WriteSeeker) *Fmp4 {
	return &Fmp4{
		wr:                  mp4.NewWriter(w),
		baseMediaDecodeTime: make(map[uint32]uint64),
	}
}

type Info struct {
	baseMediaDecodeTime uint64
	length              int
}

func (f *Fmp4) ProcessMP4(r io.ReadSeeker) error {
	var currentTrackID uint32
	firstBaseMediaDecodeTimes := make(map[uint32]uint64)
	// 累计的 baseMediaDecodeTime
	totalBaseMediaDecodeTimes := make(map[uint32]Info)
	// 计算一个 segment 的平均值

	f.isStart = true
	_, err := mp4.ReadBoxStructure(r, func(h *mp4.ReadHandle) (interface{}, error) {
		switch h.BoxInfo.Type.String() {
		case "moov":
			var err error
			if !f.firstMoov {
				f.firstMoov = !f.firstMoov
				err = f.wr.CopyBox(r, &h.BoxInfo)
			}
			return nil, err
		case "ftyp":
			var err error
			if !f.firstFtyp {
				f.firstFtyp = !f.firstFtyp
				err = f.wr.CopyBox(r, &h.BoxInfo)
			}
			return nil, err
		case "traf":
			if _, err := f.wr.StartBox(&h.BoxInfo); err != nil {
				return nil, err
			}
			children, err := h.Expand()
			if err != nil {
				return nil, err
			}
			_, err = f.wr.EndBox()
			return children, err
		case "moof":
			if _, err := f.wr.StartBox(&h.BoxInfo); err != nil {
				return nil, err
			}
			children, err := h.Expand()
			if err != nil {
				return nil, err
			}
			_, err = f.wr.EndBox()
			return children, err
		case "mfhd":
			if _, err := f.wr.StartBox(&h.BoxInfo); err != nil {
				return nil, err
			}
			box, _, err := h.ReadPayload()
			if err != nil {
				return nil, err
			}
			tfdt, ok := box.(*mp4.Mfhd)
			if !ok {
				return nil, fmt.Errorf("invalid tfdt box")
			}
			f.num++
			tfdt.SequenceNumber = f.num
			if _, err := mp4.Marshal(f.wr, tfdt, h.BoxInfo.Context); err != nil {
				return nil, err
			}
			_, err = f.wr.EndBox()
			return nil, err
		case "trun":
			box, _, err := h.ReadPayload()
			if err != nil {
				return nil, err
			}
			turn := box.(*mp4.Trun)
			f.lastDuration = 0
			for _, v := range turn.Entries {
				f.lastDuration += v.SampleDuration
			}
			return nil, f.wr.CopyBox(r, &h.BoxInfo)
		case "tfhd":
			if _, err := f.wr.StartBox(&h.BoxInfo); err != nil {
				return nil, err
			}
			box, _, err := h.ReadPayload()
			if err != nil {
				return nil, err
			}
			tfhd, ok := box.(*mp4.Tfhd)
			if !ok {
				return nil, fmt.Errorf("invalid tfhd box")
			}

			// 更新当前的 track_id
			currentTrackID = tfhd.TrackID

			if _, err := mp4.Marshal(f.wr, tfhd, h.BoxInfo.Context); err != nil {
				return nil, err
			}
			_, err = f.wr.EndBox()
			return nil, err
		case "tfdt":
			if _, err := f.wr.StartBox(&h.BoxInfo); err != nil {
				return nil, err
			}
			box, _, err := h.ReadPayload()
			if err != nil {
				return nil, err
			}
			tfdt, ok := box.(*mp4.Tfdt)
			if !ok {
				return nil, fmt.Errorf("invalid tfdt box")
			}

			total := f.baseMediaDecodeTime[currentTrackID]
			// 记录首个 traf 的 baseMediaDecodeTime
			v := firstBaseMediaDecodeTimes[currentTrackID]
			if v == 0 {
				v = tfdt.BaseMediaDecodeTimeV1
				firstBaseMediaDecodeTimes[currentTrackID] = v
			}
			// 本次 baseMediaDecodeTime 是 当前 baseMediaDecodeTime-firstBaseMediaDecodeTime
			sub := tfdt.BaseMediaDecodeTimeV1 - v
			tfdt.BaseMediaDecodeTimeV1 = sub + total

			totalBaseMediaDecodeTimes[currentTrackID] = Info{
				baseMediaDecodeTime: sub,
				length:              totalBaseMediaDecodeTimes[currentTrackID].length + 1,
			}
			if _, err := mp4.Marshal(f.wr, tfdt, h.BoxInfo.Context); err != nil {
				return nil, err
			}
			_, err = f.wr.EndBox()
			return nil, err
		default:
			return nil, f.wr.CopyBox(r, &h.BoxInfo)
		}
	})

	for k, v := range totalBaseMediaDecodeTimes {
		f.baseMediaDecodeTime[k] += v.baseMediaDecodeTime
		if v.length > 0 {
			f.baseMediaDecodeTime[k] += v.baseMediaDecodeTime / uint64(v.length)
		}
	}
	return err
}
