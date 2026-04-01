package mp4

// https://www.webmproject.org/vp9/mp4/

/*************************** vp08 ****************************/

func BoxTypeVp08() BoxType { return StrToBoxType("vp08") }

func init() {
	AddAnyTypeBoxDef(&VisualSampleEntry{}, BoxTypeVp08())
}

/*************************** vp09 ****************************/

func BoxTypeVp09() BoxType { return StrToBoxType("vp09") }

func init() {
	AddAnyTypeBoxDef(&VisualSampleEntry{}, BoxTypeVp09())
}

/*************************** VpcC ****************************/

func BoxTypeVpcC() BoxType { return StrToBoxType("vpcC") }

func init() {
	AddBoxDef(&VpcC{})
}

type VpcC struct {
	FullBox                     `mp4:"0,extend"`
	Profile                     uint8   `mp4:"1,size=8"`
	Level                       uint8   `mp4:"2,size=8"`
	BitDepth                    uint8   `mp4:"3,size=4"`
	ChromaSubsampling           uint8   `mp4:"4,size=3"`
	VideoFullRangeFlag          uint8   `mp4:"5,size=1"`
	ColourPrimaries             uint8   `mp4:"6,size=8"`
	TransferCharacteristics     uint8   `mp4:"7,size=8"`
	MatrixCoefficients          uint8   `mp4:"8,size=8"`
	CodecInitializationDataSize uint16  `mp4:"9,size=16"`
	CodecInitializationData     []uint8 `mp4:"10,size=8,len=dynamic"`
}

func (VpcC) GetType() BoxType {
	return BoxTypeVpcC()
}

func (vpcc VpcC) GetFieldLength(name string, ctx Context) uint {
	switch name {
	case "CodecInitializationData":
		return uint(vpcc.CodecInitializationDataSize)
	}
	return 0
}
