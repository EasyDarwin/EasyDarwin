package mp4

/*************************** ac-3 ****************************/

// https://www.etsi.org/deliver/etsi_ts/102300_102399/102366/01.04.01_60/ts_102366v010401p.pdf

func BoxTypeAC3() BoxType { return StrToBoxType("ac-3") }

func init() {
	AddAnyTypeBoxDef(&AudioSampleEntry{}, BoxTypeAC3())
}

/*************************** dac3 ****************************/

// https://www.etsi.org/deliver/etsi_ts/102300_102399/102366/01.04.01_60/ts_102366v010401p.pdf

func BoxTypeDAC3() BoxType { return StrToBoxType("dac3") }

func init() {
	AddBoxDef(&Dac3{})
}

type Dac3 struct {
	Box
	Fscod       uint8 `mp4:"0,size=2"`
	Bsid        uint8 `mp4:"1,size=5"`
	Bsmod       uint8 `mp4:"2,size=3"`
	Acmod       uint8 `mp4:"3,size=3"`
	LfeOn       uint8 `mp4:"4,size=1"`
	BitRateCode uint8 `mp4:"5,size=5"`
	Reserved    uint8 `mp4:"6,size=5,const=0"`
}

func (Dac3) GetType() BoxType {
	return BoxTypeDAC3()
}
