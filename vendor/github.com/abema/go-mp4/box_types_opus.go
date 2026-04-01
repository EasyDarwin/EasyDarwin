package mp4

/*************************** Opus ****************************/

// https://opus-codec.org/docs/opus_in_isobmff.html

func BoxTypeOpus() BoxType { return StrToBoxType("Opus") }

func init() {
	AddAnyTypeBoxDef(&AudioSampleEntry{}, BoxTypeOpus())
}

/*************************** dOps ****************************/

// https://opus-codec.org/docs/opus_in_isobmff.html

func BoxTypeDOps() BoxType { return StrToBoxType("dOps") }

func init() {
	AddBoxDef(&DOps{})
}

type DOps struct {
	Box
	Version              uint8   `mp4:"0,size=8"`
	OutputChannelCount   uint8   `mp4:"1,size=8"`
	PreSkip              uint16  `mp4:"2,size=16"`
	InputSampleRate      uint32  `mp4:"3,size=32"`
	OutputGain           int16   `mp4:"4,size=16"`
	ChannelMappingFamily uint8   `mp4:"5,size=8"`
	StreamCount          uint8   `mp4:"6,opt=dynamic,size=8"`
	CoupledCount         uint8   `mp4:"7,opt=dynamic,size=8"`
	ChannelMapping       []uint8 `mp4:"8,opt=dynamic,size=8,len=dynamic"`
}

func (DOps) GetType() BoxType {
	return BoxTypeDOps()
}

func (dops DOps) IsOptFieldEnabled(name string, ctx Context) bool {
	switch name {
	case "StreamCount", "CoupledCount", "ChannelMapping":
		return dops.ChannelMappingFamily != 0
	}
	return false
}

func (ops DOps) GetFieldLength(name string, ctx Context) uint {
	switch name {
	case "ChannelMapping":
		return uint(ops.OutputChannelCount)
	}
	return 0
}
