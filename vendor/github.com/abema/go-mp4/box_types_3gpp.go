package mp4

var udta3GppMetaBoxTypes = []BoxType{
	StrToBoxType("titl"),
	StrToBoxType("dscp"),
	StrToBoxType("cprt"),
	StrToBoxType("perf"),
	StrToBoxType("auth"),
	StrToBoxType("gnre"),
}

func init() {
	for _, bt := range udta3GppMetaBoxTypes {
		AddAnyTypeBoxDefEx(&Udta3GppString{}, bt, isUnderUdta, 0)
	}
}

type Udta3GppString struct {
	AnyTypeBox
	FullBox  `mp4:"0,extend"`
	Pad      bool    `mp4:"1,size=1,hidden"`
	Language [3]byte `mp4:"2,size=5,iso639-2"` // ISO-639-2/T language code
	Data     []byte  `mp4:"3,size=8,string"`
}
