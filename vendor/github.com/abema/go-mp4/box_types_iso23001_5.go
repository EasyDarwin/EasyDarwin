package mp4

/*************************** ipcm ****************************/

func BoxTypeIpcm() BoxType { return StrToBoxType("ipcm") }

func init() {
	AddAnyTypeBoxDef(&AudioSampleEntry{}, BoxTypeIpcm())
}

/*************************** fpcm ****************************/

func BoxTypeFpcm() BoxType { return StrToBoxType("fpcm") }

func init() {
	AddAnyTypeBoxDef(&AudioSampleEntry{}, BoxTypeFpcm())
}

/*************************** pcmC ****************************/

func BoxTypePcmC() BoxType { return StrToBoxType("pcmC") }

func init() {
	AddBoxDef(&PcmC{}, 0, 1)
}

type PcmC struct {
	FullBox       `mp4:"0,extend"`
	FormatFlags   uint8 `mp4:"1,size=8"`
	PCMSampleSize uint8 `mp4:"1,size=8"`
}

func (PcmC) GetType() BoxType {
	return BoxTypePcmC()
}
