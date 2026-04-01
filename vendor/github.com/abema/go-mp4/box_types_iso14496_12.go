package mp4

import (
	"errors"
	"fmt"
	"io"

	"github.com/abema/go-mp4/internal/bitio"
	"github.com/abema/go-mp4/internal/util"
)

/*************************** btrt ****************************/

func BoxTypeBtrt() BoxType { return StrToBoxType("btrt") }

func init() {
	AddBoxDef(&Btrt{}, 0)
}

type Btrt struct {
	Box
	BufferSizeDB uint32 `mp4:"0,size=32"`
	MaxBitrate   uint32 `mp4:"1,size=32"`
	AvgBitrate   uint32 `mp4:"2,size=32"`
}

// GetType returns the BoxType
func (*Btrt) GetType() BoxType {
	return BoxTypeBtrt()
}

/*************************** co64 ****************************/

func BoxTypeCo64() BoxType { return StrToBoxType("co64") }

func init() {
	AddBoxDef(&Co64{}, 0)
}

type Co64 struct {
	FullBox     `mp4:"0,extend"`
	EntryCount  uint32   `mp4:"1,size=32"`
	ChunkOffset []uint64 `mp4:"2,size=64,len=dynamic"`
}

// GetType returns the BoxType
func (*Co64) GetType() BoxType {
	return BoxTypeCo64()
}

// GetFieldLength returns length of dynamic field
func (co64 *Co64) GetFieldLength(name string, ctx Context) uint {
	switch name {
	case "ChunkOffset":
		return uint(co64.EntryCount)
	}
	panic(fmt.Errorf("invalid name of dynamic-length field: boxType=co64 fieldName=%s", name))
}

/*************************** colr ****************************/

func BoxTypeColr() BoxType { return StrToBoxType("colr") }

func init() {
	AddBoxDef(&Colr{})
}

type Colr struct {
	Box
	ColourType              [4]byte `mp4:"0,size=8,string"`
	ColourPrimaries         uint16  `mp4:"1,size=16,opt=dynamic"`
	TransferCharacteristics uint16  `mp4:"2,size=16,opt=dynamic"`
	MatrixCoefficients      uint16  `mp4:"3,size=16,opt=dynamic"`
	FullRangeFlag           bool    `mp4:"4,size=1,opt=dynamic"`
	Reserved                uint8   `mp4:"5,size=7,opt=dynamic"`
	Profile                 []byte  `mp4:"6,size=8,opt=dynamic"`
	Unknown                 []byte  `mp4:"7,size=8,opt=dynamic"`
}

func (colr *Colr) IsOptFieldEnabled(name string, ctx Context) bool {
	switch colr.ColourType {
	case [4]byte{'n', 'c', 'l', 'x'}:
		switch name {
		case "ColourType",
			"ColourPrimaries",
			"TransferCharacteristics",
			"MatrixCoefficients",
			"FullRangeFlag",
			"Reserved":
			return true
		default:
			return false
		}
	case [4]byte{'r', 'I', 'C', 'C'}, [4]byte{'p', 'r', 'o', 'f'}:
		return name == "Profile"
	default:
		return name == "Unknown"
	}
}

// GetType returns the BoxType
func (*Colr) GetType() BoxType {
	return BoxTypeColr()
}

/*************************** cslg ****************************/

func BoxTypeCslg() BoxType { return StrToBoxType("cslg") }

func init() {
	AddBoxDef(&Cslg{}, 0, 1)
}

type Cslg struct {
	FullBox                        `mp4:"0,extend"`
	CompositionToDTSShiftV0        int32 `mp4:"1,size=32,ver=0"`
	LeastDecodeToDisplayDeltaV0    int32 `mp4:"2,size=32,ver=0"`
	GreatestDecodeToDisplayDeltaV0 int32 `mp4:"3,size=32,ver=0"`
	CompositionStartTimeV0         int32 `mp4:"4,size=32,ver=0"`
	CompositionEndTimeV0           int32 `mp4:"5,size=32,ver=0"`
	CompositionToDTSShiftV1        int64 `mp4:"6,size=64,nver=0"`
	LeastDecodeToDisplayDeltaV1    int64 `mp4:"7,size=64,nver=0"`
	GreatestDecodeToDisplayDeltaV1 int64 `mp4:"8,size=64,nver=0"`
	CompositionStartTimeV1         int64 `mp4:"9,size=64,nver=0"`
	CompositionEndTimeV1           int64 `mp4:"10,size=64,nver=0"`
}

// GetType returns the BoxType
func (*Cslg) GetType() BoxType {
	return BoxTypeCslg()
}

func (cslg *Cslg) GetCompositionToDTSShift() int64 {
	switch cslg.GetVersion() {
	case 0:
		return int64(cslg.CompositionToDTSShiftV0)
	case 1:
		return cslg.CompositionToDTSShiftV1
	default:
		return 0
	}
}

func (cslg *Cslg) GetLeastDecodeToDisplayDelta() int64 {
	switch cslg.GetVersion() {
	case 0:
		return int64(cslg.LeastDecodeToDisplayDeltaV0)
	case 1:
		return cslg.LeastDecodeToDisplayDeltaV1
	default:
		return 0
	}
}

func (cslg *Cslg) GetGreatestDecodeToDisplayDelta() int64 {
	switch cslg.GetVersion() {
	case 0:
		return int64(cslg.GreatestDecodeToDisplayDeltaV0)
	case 1:
		return cslg.GreatestDecodeToDisplayDeltaV1
	default:
		return 0
	}
}

func (cslg *Cslg) GetCompositionStartTime() int64 {
	switch cslg.GetVersion() {
	case 0:
		return int64(cslg.CompositionStartTimeV0)
	case 1:
		return cslg.CompositionStartTimeV1
	default:
		return 0
	}
}

func (cslg *Cslg) GetCompositionEndTime() int64 {
	switch cslg.GetVersion() {
	case 0:
		return int64(cslg.CompositionEndTimeV0)
	case 1:
		return cslg.CompositionEndTimeV1
	default:
		return 0
	}
}

/*************************** ctts ****************************/

func BoxTypeCtts() BoxType { return StrToBoxType("ctts") }

func init() {
	AddBoxDef(&Ctts{}, 0, 1)
}

type Ctts struct {
	FullBox    `mp4:"0,extend"`
	EntryCount uint32      `mp4:"1,size=32"`
	Entries    []CttsEntry `mp4:"2,len=dynamic,size=64"`
}

type CttsEntry struct {
	SampleCount    uint32 `mp4:"0,size=32"`
	SampleOffsetV0 uint32 `mp4:"1,size=32,ver=0"`
	SampleOffsetV1 int32  `mp4:"2,size=32,ver=1"`
}

// GetType returns the BoxType
func (*Ctts) GetType() BoxType {
	return BoxTypeCtts()
}

// GetFieldLength returns length of dynamic field
func (ctts *Ctts) GetFieldLength(name string, ctx Context) uint {
	switch name {
	case "Entries":
		return uint(ctts.EntryCount)
	}
	panic(fmt.Errorf("invalid name of dynamic-length field: boxType=ctts fieldName=%s", name))
}

func (ctts *Ctts) GetSampleOffset(index int) int64 {
	switch ctts.GetVersion() {
	case 0:
		return int64(ctts.Entries[index].SampleOffsetV0)
	case 1:
		return int64(ctts.Entries[index].SampleOffsetV1)
	default:
		return 0
	}
}

/*************************** dinf ****************************/

func BoxTypeDinf() BoxType { return StrToBoxType("dinf") }

func init() {
	AddBoxDef(&Dinf{})
}

// Dinf is ISOBMFF dinf box type
type Dinf struct {
	Box
}

// GetType returns the BoxType
func (*Dinf) GetType() BoxType {
	return BoxTypeDinf()
}

/*************************** dref ****************************/

func BoxTypeDref() BoxType { return StrToBoxType("dref") }
func BoxTypeUrl() BoxType  { return StrToBoxType("url ") }
func BoxTypeUrn() BoxType  { return StrToBoxType("urn ") }

func init() {
	AddBoxDef(&Dref{}, 0)
	AddBoxDef(&Url{}, 0)
	AddBoxDef(&Urn{}, 0)
}

// Dref is ISOBMFF dref box type
type Dref struct {
	FullBox    `mp4:"0,extend"`
	EntryCount uint32 `mp4:"1,size=32"`
}

// GetType returns the BoxType
func (*Dref) GetType() BoxType {
	return BoxTypeDref()
}

type Url struct {
	FullBox  `mp4:"0,extend"`
	Location string `mp4:"1,string,nopt=0x000001"`
}

func (*Url) GetType() BoxType {
	return BoxTypeUrl()
}

const UrlSelfContained = 0x000001

type Urn struct {
	FullBox  `mp4:"0,extend"`
	Name     string `mp4:"1,string,nopt=0x000001"`
	Location string `mp4:"2,string,nopt=0x000001"`
}

func (*Urn) GetType() BoxType {
	return BoxTypeUrn()
}

const UrnSelfContained = 0x000001

/*************************** edts ****************************/

func BoxTypeEdts() BoxType { return StrToBoxType("edts") }

func init() {
	AddBoxDef(&Edts{})
}

// Edts is ISOBMFF edts box type
type Edts struct {
	Box
}

// GetType returns the BoxType
func (*Edts) GetType() BoxType {
	return BoxTypeEdts()
}

/*************************** elst ****************************/

func BoxTypeElst() BoxType { return StrToBoxType("elst") }

func init() {
	AddBoxDef(&Elst{}, 0, 1)
}

// Elst is ISOBMFF elst box type
type Elst struct {
	FullBox    `mp4:"0,extend"`
	EntryCount uint32      `mp4:"1,size=32"`
	Entries    []ElstEntry `mp4:"2,len=dynamic,size=dynamic"`
}

type ElstEntry struct {
	SegmentDurationV0 uint32 `mp4:"0,size=32,ver=0"`
	MediaTimeV0       int32  `mp4:"1,size=32,ver=0"`
	SegmentDurationV1 uint64 `mp4:"2,size=64,ver=1"`
	MediaTimeV1       int64  `mp4:"3,size=64,ver=1"`
	MediaRateInteger  int16  `mp4:"4,size=16"`
	MediaRateFraction int16  `mp4:"5,size=16,const=0"`
}

// GetType returns the BoxType
func (*Elst) GetType() BoxType {
	return BoxTypeElst()
}

// GetFieldSize returns size of dynamic field
func (elst *Elst) GetFieldSize(name string, ctx Context) uint {
	switch name {
	case "Entries":
		switch elst.GetVersion() {
		case 0:
			return 0 +
				/* segmentDurationV0 */ 32 +
				/* mediaTimeV0       */ 32 +
				/* mediaRateInteger  */ 16 +
				/* mediaRateFraction */ 16
		case 1:
			return 0 +
				/* segmentDurationV1 */ 64 +
				/* mediaTimeV1       */ 64 +
				/* mediaRateInteger  */ 16 +
				/* mediaRateFraction */ 16
		}
	}
	panic(fmt.Errorf("invalid name of dynamic-size field: boxType=elst fieldName=%s", name))
}

// GetFieldLength returns length of dynamic field
func (elst *Elst) GetFieldLength(name string, ctx Context) uint {
	switch name {
	case "Entries":
		return uint(elst.EntryCount)
	}
	panic(fmt.Errorf("invalid name of dynamic-length field: boxType=elst fieldName=%s", name))
}

func (elst *Elst) GetSegmentDuration(index int) uint64 {
	switch elst.GetVersion() {
	case 0:
		return uint64(elst.Entries[index].SegmentDurationV0)
	case 1:
		return elst.Entries[index].SegmentDurationV1
	default:
		return 0
	}
}

func (elst *Elst) GetMediaTime(index int) int64 {
	switch elst.GetVersion() {
	case 0:
		return int64(elst.Entries[index].MediaTimeV0)
	case 1:
		return elst.Entries[index].MediaTimeV1
	default:
		return 0
	}
}

/*************************** emsg ****************************/

func BoxTypeEmsg() BoxType { return StrToBoxType("emsg") }

func init() {
	AddBoxDef(&Emsg{}, 0, 1)
}

// Emsg is ISOBMFF emsg box type
type Emsg struct {
	FullBox               `mp4:"0,extend"`
	SchemeIdUri           string `mp4:"1,string"`
	Value                 string `mp4:"2,string"`
	Timescale             uint32 `mp4:"3,size=32"`
	PresentationTimeDelta uint32 `mp4:"4,size=32,ver=0"`
	PresentationTime      uint64 `mp4:"5,size=64,ver=1"`
	EventDuration         uint32 `mp4:"6,size=32"`
	Id                    uint32 `mp4:"7,size=32"`
	MessageData           []byte `mp4:"8,size=8,string"`
}

func (emsg *Emsg) OnReadField(name string, r bitio.ReadSeeker, leftBits uint64, ctx Context) (rbits uint64, override bool, err error) {
	if emsg.GetVersion() == 0 {
		return
	}
	switch name {
	case "SchemeIdUri", "Value":
		override = true
		return
	case "MessageData":
		emsg.SchemeIdUri, err = util.ReadString(r)
		if err != nil {
			return
		}
		emsg.Value, err = util.ReadString(r)
		if err != nil {
			return
		}
		rbits += uint64(len(emsg.SchemeIdUri)+len(emsg.Value)+2) * 8
		return
	default:
		return
	}
}

func (emsg *Emsg) OnWriteField(name string, w bitio.Writer, ctx Context) (wbits uint64, override bool, err error) {
	if emsg.GetVersion() == 0 {
		return
	}
	switch name {
	case "SchemeIdUri", "Value":
		override = true
		return
	case "MessageData":
		if err = util.WriteString(w, emsg.SchemeIdUri); err != nil {
			return
		}
		if err = util.WriteString(w, emsg.Value); err != nil {
			return
		}
		wbits += uint64(len(emsg.SchemeIdUri)+len(emsg.Value)+2) * 8
		return
	default:
		return
	}
}

// GetType returns the BoxType
func (*Emsg) GetType() BoxType {
	return BoxTypeEmsg()
}

/*************************** fiel ****************************/

func BoxTypeFiel() BoxType { return StrToBoxType("fiel") }

func init() {
	AddBoxDef(&Fiel{})
}

type Fiel struct {
	Box
	FieldCount    uint8 `mp4:"0,size=8"`
	FieldOrdering uint8 `mp4:"1,size=8"`
}

func (Fiel) GetType() BoxType {
	return BoxTypeFiel()
}

/************************ free, skip *************************/

func BoxTypeFree() BoxType { return StrToBoxType("free") }
func BoxTypeSkip() BoxType { return StrToBoxType("skip") }

func init() {
	AddBoxDef(&Free{})
	AddBoxDef(&Skip{})
}

type FreeSpace struct {
	Box
	Data []uint8 `mp4:"0,size=8"`
}

type Free FreeSpace

func (*Free) GetType() BoxType {
	return BoxTypeFree()
}

type Skip FreeSpace

func (*Skip) GetType() BoxType {
	return BoxTypeSkip()
}

/*************************** frma ****************************/

func BoxTypeFrma() BoxType { return StrToBoxType("frma") }

func init() {
	AddBoxDef(&Frma{})
}

// Frma is ISOBMFF frma box type
type Frma struct {
	Box
	DataFormat [4]byte `mp4:"0,size=8,string"`
}

// GetType returns the BoxType
func (*Frma) GetType() BoxType {
	return BoxTypeFrma()
}

/*************************** ftyp ****************************/

func BoxTypeFtyp() BoxType { return StrToBoxType("ftyp") }

func init() {
	AddBoxDef(&Ftyp{})
}

func BrandQT() [4]byte   { return [4]byte{'q', 't', ' ', ' '} }
func BrandISOM() [4]byte { return [4]byte{'i', 's', 'o', 'm'} }
func BrandISO2() [4]byte { return [4]byte{'i', 's', 'o', '2'} }
func BrandISO3() [4]byte { return [4]byte{'i', 's', 'o', '3'} }
func BrandISO4() [4]byte { return [4]byte{'i', 's', 'o', '4'} }
func BrandISO5() [4]byte { return [4]byte{'i', 's', 'o', '5'} }
func BrandISO6() [4]byte { return [4]byte{'i', 's', 'o', '6'} }
func BrandISO7() [4]byte { return [4]byte{'i', 's', 'o', '7'} }
func BrandISO8() [4]byte { return [4]byte{'i', 's', 'o', '8'} }
func BrandISO9() [4]byte { return [4]byte{'i', 's', 'o', '9'} }
func BrandAVC1() [4]byte { return [4]byte{'a', 'v', 'c', '1'} }
func BrandMP41() [4]byte { return [4]byte{'m', 'p', '4', '1'} }
func BrandMP71() [4]byte { return [4]byte{'m', 'p', '7', '1'} }

// Ftyp is ISOBMFF ftyp box type
type Ftyp struct {
	Box
	MajorBrand       [4]byte               `mp4:"0,size=8,string"`
	MinorVersion     uint32                `mp4:"1,size=32"`
	CompatibleBrands []CompatibleBrandElem `mp4:"2,size=32"` // reach to end of the box
}

type CompatibleBrandElem struct {
	CompatibleBrand [4]byte `mp4:"0,size=8,string"`
}

func (ftyp *Ftyp) AddCompatibleBrand(cb [4]byte) {
	if !ftyp.HasCompatibleBrand(cb) {
		ftyp.CompatibleBrands = append(ftyp.CompatibleBrands, CompatibleBrandElem{
			CompatibleBrand: cb,
		})
	}
}

func (ftyp *Ftyp) RemoveCompatibleBrand(cb [4]byte) {
	for i := 0; i < len(ftyp.CompatibleBrands); {
		if ftyp.CompatibleBrands[i].CompatibleBrand != cb {
			i++
			continue
		}
		ftyp.CompatibleBrands[i] = ftyp.CompatibleBrands[len(ftyp.CompatibleBrands)-1]
		ftyp.CompatibleBrands = ftyp.CompatibleBrands[:len(ftyp.CompatibleBrands)-1]
	}
}

func (ftyp *Ftyp) HasCompatibleBrand(cb [4]byte) bool {
	for i := range ftyp.CompatibleBrands {
		if ftyp.CompatibleBrands[i].CompatibleBrand == cb {
			return true
		}
	}
	return false
}

// GetType returns the BoxType
func (*Ftyp) GetType() BoxType {
	return BoxTypeFtyp()
}

/*************************** hdlr ****************************/

func BoxTypeHdlr() BoxType { return StrToBoxType("hdlr") }

func init() {
	AddBoxDef(&Hdlr{}, 0)
}

// Hdlr is ISOBMFF hdlr box type
type Hdlr struct {
	FullBox `mp4:"0,extend"`
	// Predefined corresponds to component_type of QuickTime.
	// pre_defined of ISO-14496 has always zero,
	// however component_type has "mhlr" or "dhlr".
	PreDefined  uint32    `mp4:"1,size=32"`
	HandlerType [4]byte   `mp4:"2,size=8,string"`
	Reserved    [3]uint32 `mp4:"3,size=32,const=0"`
	Name        string    `mp4:"4,string"`
}

// GetType returns the BoxType
func (*Hdlr) GetType() BoxType {
	return BoxTypeHdlr()
}

func (hdlr *Hdlr) OnReadField(name string, r bitio.ReadSeeker, leftBits uint64, ctx Context) (rbits uint64, override bool, err error) {
	switch name {
	case "Name":
		return hdlr.OnReadName(r, leftBits, ctx)
	default:
		return 0, false, nil
	}
}

func (hdlr *Hdlr) OnReadName(r bitio.ReadSeeker, leftBits uint64, ctx Context) (rbits uint64, override bool, err error) {
	size := leftBits / 8
	if size == 0 {
		hdlr.Name = ""
		return 0, true, nil
	}

	if !readerHasSize(r, size) {
		return 0, false, fmt.Errorf("not enough bits")
	}

	buf := make([]byte, size)
	if _, err := io.ReadFull(r, buf); err != nil {
		return 0, false, err
	}

	plen := buf[0]
	if hdlr.PreDefined != 0 && size >= 2 && size == uint64(plen+1) {
		// Pascal-style String
		hdlr.Name = string(buf[1 : plen+1])
	} else {
		// C-style String
		clen := 0
		for _, c := range buf {
			if c == 0x00 {
				break
			}
			clen++
		}
		hdlr.Name = string(buf[:clen])
	}
	return leftBits, true, nil
}

/*************************** hvcC ****************************/

func BoxTypeHvcC() BoxType { return StrToBoxType("hvcC") }

func init() {
	AddBoxDef(&HvcC{})
}

type HEVCNalu struct {
	BaseCustomFieldObject
	Length  uint16 `mp4:"0,size=16"`
	NALUnit []byte `mp4:"1,size=8,len=dynamic"`
}

func (s HEVCNalu) GetFieldLength(name string, ctx Context) uint {
	switch name {
	case "NALUnit":
		return uint(s.Length)
	}
	return 0
}

type HEVCNaluArray struct {
	BaseCustomFieldObject
	Completeness bool       `mp4:"0,size=1"`
	Reserved     bool       `mp4:"1,size=1"`
	NaluType     uint8      `mp4:"2,size=6"`
	NumNalus     uint16     `mp4:"3,size=16"`
	Nalus        []HEVCNalu `mp4:"4,len=dynamic"`
}

func (a HEVCNaluArray) GetFieldLength(name string, ctx Context) uint {
	switch name {
	case "Nalus":
		return uint(a.NumNalus)
	}
	return 0
}

type HvcC struct {
	Box
	ConfigurationVersion        uint8           `mp4:"0,size=8"`
	GeneralProfileSpace         uint8           `mp4:"1,size=2"`
	GeneralTierFlag             bool            `mp4:"2,size=1"`
	GeneralProfileIdc           uint8           `mp4:"3,size=5"`
	GeneralProfileCompatibility [32]bool        `mp4:"4,size=1"`
	GeneralConstraintIndicator  [6]uint8        `mp4:"5,size=8"`
	GeneralLevelIdc             uint8           `mp4:"6,size=8"`
	Reserved1                   uint8           `mp4:"7,size=4,const=15"`
	MinSpatialSegmentationIdc   uint16          `mp4:"8,size=12"`
	Reserved2                   uint8           `mp4:"9,size=6,const=63"`
	ParallelismType             uint8           `mp4:"10,size=2"`
	Reserved3                   uint8           `mp4:"11,size=6,const=63"`
	ChromaFormatIdc             uint8           `mp4:"12,size=2"`
	Reserved4                   uint8           `mp4:"13,size=5,const=31"`
	BitDepthLumaMinus8          uint8           `mp4:"14,size=3"`
	Reserved5                   uint8           `mp4:"15,size=5,const=31"`
	BitDepthChromaMinus8        uint8           `mp4:"16,size=3"`
	AvgFrameRate                uint16          `mp4:"17,size=16"`
	ConstantFrameRate           uint8           `mp4:"18,size=2"`
	NumTemporalLayers           uint8           `mp4:"19,size=2"`
	TemporalIdNested            uint8           `mp4:"20,size=2"`
	LengthSizeMinusOne          uint8           `mp4:"21,size=2"`
	NumOfNaluArrays             uint8           `mp4:"22,size=8"`
	NaluArrays                  []HEVCNaluArray `mp4:"23,len=dynamic"`
}

func (HvcC) GetType() BoxType {
	return BoxTypeHvcC()
}

func (hvcc HvcC) GetFieldLength(name string, ctx Context) uint {
	switch name {
	case "NaluArrays":
		return uint(hvcc.NumOfNaluArrays)
	}
	return 0
}

/*************************** mdat ****************************/

func BoxTypeMdat() BoxType { return StrToBoxType("mdat") }

func init() {
	AddBoxDef(&Mdat{})
}

// Mdat is ISOBMFF mdat box type
type Mdat struct {
	Box
	Data []byte `mp4:"0,size=8"`
}

// GetType returns the BoxType
func (*Mdat) GetType() BoxType {
	return BoxTypeMdat()
}

/*************************** mdhd ****************************/

func BoxTypeMdhd() BoxType { return StrToBoxType("mdhd") }

func init() {
	AddBoxDef(&Mdhd{}, 0, 1)
}

// Mdhd is ISOBMFF mdhd box type
type Mdhd struct {
	FullBox            `mp4:"0,extend"`
	CreationTimeV0     uint32 `mp4:"1,size=32,ver=0"`
	ModificationTimeV0 uint32 `mp4:"2,size=32,ver=0"`
	CreationTimeV1     uint64 `mp4:"3,size=64,ver=1"`
	ModificationTimeV1 uint64 `mp4:"4,size=64,ver=1"`
	Timescale          uint32 `mp4:"5,size=32"`
	DurationV0         uint32 `mp4:"6,size=32,ver=0"`
	DurationV1         uint64 `mp4:"7,size=64,ver=1"`
	//
	Pad        bool    `mp4:"8,size=1,hidden"`
	Language   [3]byte `mp4:"9,size=5,iso639-2"` // ISO-639-2/T language code
	PreDefined uint16  `mp4:"10,size=16"`
}

// GetType returns the BoxType
func (*Mdhd) GetType() BoxType {
	return BoxTypeMdhd()
}

func (mdhd *Mdhd) GetCreationTime() uint64 {
	switch mdhd.GetVersion() {
	case 0:
		return uint64(mdhd.CreationTimeV0)
	case 1:
		return mdhd.CreationTimeV1
	default:
		return 0
	}
}

func (mdhd *Mdhd) GetModificationTime() uint64 {
	switch mdhd.GetVersion() {
	case 0:
		return uint64(mdhd.ModificationTimeV0)
	case 1:
		return mdhd.ModificationTimeV1
	default:
		return 0
	}
}

func (mdhd *Mdhd) GetDuration() uint64 {
	switch mdhd.GetVersion() {
	case 0:
		return uint64(mdhd.DurationV0)
	case 1:
		return mdhd.DurationV1
	default:
		return 0
	}
}

/*************************** mdia ****************************/

func BoxTypeMdia() BoxType { return StrToBoxType("mdia") }

func init() {
	AddBoxDef(&Mdia{})
}

// Mdia is ISOBMFF mdia box type
type Mdia struct {
	Box
}

// GetType returns the BoxType
func (*Mdia) GetType() BoxType {
	return BoxTypeMdia()
}

/*************************** mehd ****************************/

func BoxTypeMehd() BoxType { return StrToBoxType("mehd") }

func init() {
	AddBoxDef(&Mehd{}, 0, 1)
}

// Mehd is ISOBMFF mehd box type
type Mehd struct {
	FullBox            `mp4:"0,extend"`
	FragmentDurationV0 uint32 `mp4:"1,size=32,ver=0"`
	FragmentDurationV1 uint64 `mp4:"2,size=64,ver=1"`
}

// GetType returns the BoxType
func (*Mehd) GetType() BoxType {
	return BoxTypeMehd()
}

func (mdhd *Mehd) GetFragmentDuration() uint64 {
	switch mdhd.GetVersion() {
	case 0:
		return uint64(mdhd.FragmentDurationV0)
	case 1:
		return mdhd.FragmentDurationV1
	default:
		return 0
	}
}

/*************************** meta ****************************/

func BoxTypeMeta() BoxType { return StrToBoxType("meta") }

func init() {
	AddBoxDef(&Meta{}, 0)
}

// Meta is ISOBMFF meta box type
type Meta struct {
	FullBox `mp4:"0,extend"`
}

// GetType returns the BoxType
func (*Meta) GetType() BoxType {
	return BoxTypeMeta()
}

func (meta *Meta) BeforeUnmarshal(r io.ReadSeeker, size uint64, ctx Context) (n uint64, override bool, err error) {
	// for Apple Quick Time
	buf := make([]byte, 4)
	if _, err := io.ReadFull(r, buf); err != nil {
		return 0, false, err
	}
	if _, err := r.Seek(-int64(len(buf)), io.SeekCurrent); err != nil {
		return 0, false, err
	}
	if buf[0]|buf[1]|buf[2]|buf[3] != 0x00 {
		meta.Version = 0
		meta.Flags = [3]byte{0, 0, 0}
		return 0, true, nil
	}
	return 0, false, nil
}

/*************************** mfhd ****************************/

func BoxTypeMfhd() BoxType { return StrToBoxType("mfhd") }

func init() {
	AddBoxDef(&Mfhd{}, 0)
}

// Mfhd is ISOBMFF mfhd box type
type Mfhd struct {
	FullBox        `mp4:"0,extend"`
	SequenceNumber uint32 `mp4:"1,size=32"`
}

// GetType returns the BoxType
func (*Mfhd) GetType() BoxType {
	return BoxTypeMfhd()
}

/*************************** mfra ****************************/

func BoxTypeMfra() BoxType { return StrToBoxType("mfra") }

func init() {
	AddBoxDef(&Mfra{})
}

// Mfra is ISOBMFF mfra box type
type Mfra struct {
	Box
}

// GetType returns the BoxType
func (*Mfra) GetType() BoxType {
	return BoxTypeMfra()
}

/*************************** mfro ****************************/

func BoxTypeMfro() BoxType { return StrToBoxType("mfro") }

func init() {
	AddBoxDef(&Mfro{}, 0)
}

// Mfro is ISOBMFF mfro box type
type Mfro struct {
	FullBox `mp4:"0,extend"`
	Size    uint32 `mp4:"1,size=32"`
}

// GetType returns the BoxType
func (*Mfro) GetType() BoxType {
	return BoxTypeMfro()
}

/*************************** minf ****************************/

func BoxTypeMinf() BoxType { return StrToBoxType("minf") }

func init() {
	AddBoxDef(&Minf{})
}

// Minf is ISOBMFF minf box type
type Minf struct {
	Box
}

// GetType returns the BoxType
func (*Minf) GetType() BoxType {
	return BoxTypeMinf()
}

/*************************** moof ****************************/

func BoxTypeMoof() BoxType { return StrToBoxType("moof") }

func init() {
	AddBoxDef(&Moof{})
}

// Moof is ISOBMFF moof box type
type Moof struct {
	Box
}

// GetType returns the BoxType
func (*Moof) GetType() BoxType {
	return BoxTypeMoof()
}

/*************************** moov ****************************/

func BoxTypeMoov() BoxType { return StrToBoxType("moov") }

func init() {
	AddBoxDef(&Moov{})
}

// Moov is ISOBMFF moov box type
type Moov struct {
	Box
}

// GetType returns the BoxType
func (*Moov) GetType() BoxType {
	return BoxTypeMoov()
}

/*************************** mvex ****************************/

func BoxTypeMvex() BoxType { return StrToBoxType("mvex") }

func init() {
	AddBoxDef(&Mvex{})
}

// Mvex is ISOBMFF mvex box type
type Mvex struct {
	Box
}

// GetType returns the BoxType
func (*Mvex) GetType() BoxType {
	return BoxTypeMvex()
}

/*************************** mvhd ****************************/

func BoxTypeMvhd() BoxType { return StrToBoxType("mvhd") }

func init() {
	AddBoxDef(&Mvhd{}, 0, 1)
}

// Mvhd is ISOBMFF mvhd box type
type Mvhd struct {
	FullBox            `mp4:"0,extend"`
	CreationTimeV0     uint32    `mp4:"1,size=32,ver=0"`
	ModificationTimeV0 uint32    `mp4:"2,size=32,ver=0"`
	CreationTimeV1     uint64    `mp4:"3,size=64,ver=1"`
	ModificationTimeV1 uint64    `mp4:"4,size=64,ver=1"`
	Timescale          uint32    `mp4:"5,size=32"`
	DurationV0         uint32    `mp4:"6,size=32,ver=0"`
	DurationV1         uint64    `mp4:"7,size=64,ver=1"`
	Rate               int32     `mp4:"8,size=32"` // fixed-point 16.16 - template=0x00010000
	Volume             int16     `mp4:"9,size=16"` // template=0x0100
	Reserved           int16     `mp4:"10,size=16,const=0"`
	Reserved2          [2]uint32 `mp4:"11,size=32,const=0"`
	Matrix             [9]int32  `mp4:"12,size=32,hex"` // template={ 0x00010000,0,0,0,0x00010000,0,0,0,0x40000000 }
	PreDefined         [6]int32  `mp4:"13,size=32"`
	NextTrackID        uint32    `mp4:"14,size=32"`
}

// GetType returns the BoxType
func (*Mvhd) GetType() BoxType {
	return BoxTypeMvhd()
}

// StringifyField returns field value as string
func (mvhd *Mvhd) StringifyField(name string, indent string, depth int, ctx Context) (string, bool) {
	switch name {
	case "Rate":
		return util.FormatSignedFixedFloat1616(mvhd.Rate), true
	default:
		return "", false
	}
}

func (mvhd *Mvhd) GetCreationTime() uint64 {
	switch mvhd.GetVersion() {
	case 0:
		return uint64(mvhd.CreationTimeV0)
	case 1:
		return mvhd.CreationTimeV1
	default:
		return 0
	}
}

func (mvhd *Mvhd) GetModificationTime() uint64 {
	switch mvhd.GetVersion() {
	case 0:
		return uint64(mvhd.ModificationTimeV0)
	case 1:
		return mvhd.ModificationTimeV1
	default:
		return 0
	}
}

func (mvhd *Mvhd) GetDuration() uint64 {
	switch mvhd.GetVersion() {
	case 0:
		return uint64(mvhd.DurationV0)
	case 1:
		return mvhd.DurationV1
	default:
		return 0
	}
}

// GetRate returns value of rate as float64
func (mvhd *Mvhd) GetRate() float64 {
	return float64(mvhd.Rate) / (1 << 16)
}

// GetRateInt returns value of rate as int16
func (mvhd *Mvhd) GetRateInt() int16 {
	return int16(mvhd.Rate >> 16)
}

/*************************** saio ****************************/

func BoxTypeSaio() BoxType { return StrToBoxType("saio") }

func init() {
	AddBoxDef(&Saio{}, 0, 1)
}

type Saio struct {
	FullBox              `mp4:"0,extend"`
	AuxInfoType          [4]byte  `mp4:"1,size=8,opt=0x000001,string"`
	AuxInfoTypeParameter uint32   `mp4:"2,size=32,opt=0x000001,hex"`
	EntryCount           uint32   `mp4:"3,size=32"`
	OffsetV0             []uint32 `mp4:"4,size=32,ver=0,len=dynamic"`
	OffsetV1             []uint64 `mp4:"5,size=64,nver=0,len=dynamic"`
}

func (saio *Saio) GetFieldLength(name string, ctx Context) uint {
	switch name {
	case "OffsetV0", "OffsetV1":
		return uint(saio.EntryCount)
	}
	panic(fmt.Errorf("invalid name of dynamic-length field: boxType=saio fieldName=%s", name))
}

func (*Saio) GetType() BoxType {
	return BoxTypeSaio()
}

func (saio *Saio) GetOffset(index int) uint64 {
	switch saio.GetVersion() {
	case 0:
		return uint64(saio.OffsetV0[index])
	case 1:
		return saio.OffsetV1[index]
	default:
		return 0
	}
}

/*************************** saiz ****************************/

func BoxTypeSaiz() BoxType { return StrToBoxType("saiz") }

func init() {
	AddBoxDef(&Saiz{}, 0)
}

type Saiz struct {
	FullBox               `mp4:"0,extend"`
	AuxInfoType           [4]byte `mp4:"1,size=8,opt=0x000001,string"`
	AuxInfoTypeParameter  uint32  `mp4:"2,size=32,opt=0x000001,hex"`
	DefaultSampleInfoSize uint8   `mp4:"3,size=8,dec"`
	SampleCount           uint32  `mp4:"4,size=32"`
	SampleInfoSize        []uint8 `mp4:"5,size=8,opt=dynamic,len=dynamic,dec"`
}

func (saiz *Saiz) IsOptFieldEnabled(name string, ctx Context) bool {
	switch name {
	case "SampleInfoSize":
		return saiz.DefaultSampleInfoSize == 0
	}
	return false
}

func (saiz *Saiz) GetFieldLength(name string, ctx Context) uint {
	switch name {
	case "SampleInfoSize":
		return uint(saiz.SampleCount)
	}
	panic(fmt.Errorf("invalid name of dynamic-length field: boxType=saiz fieldName=%s", name))
}

func (*Saiz) GetType() BoxType {
	return BoxTypeSaiz()
}

/*********************** SampleEntry *************************/

func BoxTypeMp4v() BoxType { return StrToBoxType("mp4v") }
func BoxTypeAvc1() BoxType { return StrToBoxType("avc1") }
func BoxTypeEncv() BoxType { return StrToBoxType("encv") }
func BoxTypeHev1() BoxType { return StrToBoxType("hev1") }
func BoxTypeHvc1() BoxType { return StrToBoxType("hvc1") }
func BoxTypeMp4a() BoxType { return StrToBoxType("mp4a") }
func BoxTypeEnca() BoxType { return StrToBoxType("enca") }
func BoxTypeAvcC() BoxType { return StrToBoxType("avcC") }
func BoxTypePasp() BoxType { return StrToBoxType("pasp") }

func init() {
	AddAnyTypeBoxDef(&VisualSampleEntry{}, BoxTypeMp4v())
	AddAnyTypeBoxDef(&VisualSampleEntry{}, BoxTypeAvc1())
	AddAnyTypeBoxDef(&VisualSampleEntry{}, BoxTypeEncv())
	AddAnyTypeBoxDef(&VisualSampleEntry{}, BoxTypeHev1())
	AddAnyTypeBoxDef(&VisualSampleEntry{}, BoxTypeHvc1())
	AddAnyTypeBoxDef(&AudioSampleEntry{}, BoxTypeMp4a())
	AddAnyTypeBoxDef(&AudioSampleEntry{}, BoxTypeEnca())
	AddAnyTypeBoxDef(&AVCDecoderConfiguration{}, BoxTypeAvcC())
	AddAnyTypeBoxDef(&PixelAspectRatioBox{}, BoxTypePasp())
}

type SampleEntry struct {
	AnyTypeBox
	Reserved           [6]uint8 `mp4:"0,size=8,const=0"`
	DataReferenceIndex uint16   `mp4:"1,size=16"`
}

type VisualSampleEntry struct {
	SampleEntry     `mp4:"0,extend"`
	PreDefined      uint16    `mp4:"1,size=16"`
	Reserved        uint16    `mp4:"2,size=16,const=0"`
	PreDefined2     [3]uint32 `mp4:"3,size=32"`
	Width           uint16    `mp4:"4,size=16"`
	Height          uint16    `mp4:"5,size=16"`
	Horizresolution uint32    `mp4:"6,size=32"`
	Vertresolution  uint32    `mp4:"7,size=32"`
	Reserved2       uint32    `mp4:"8,size=32,const=0"`
	FrameCount      uint16    `mp4:"9,size=16"`
	Compressorname  [32]byte  `mp4:"10,size=8"`
	Depth           uint16    `mp4:"11,size=16"`
	PreDefined3     int16     `mp4:"12,size=16"`
}

// StringifyField returns field value as string
func (vse *VisualSampleEntry) StringifyField(name string, indent string, depth int, ctx Context) (string, bool) {
	switch name {
	case "Compressorname":
		if vse.Compressorname[0] <= 31 {
			return `"` + util.EscapeUnprintables(string(vse.Compressorname[1:vse.Compressorname[0]+1])) + `"`, true
		}
		return "", false
	default:
		return "", false
	}
}

type AudioSampleEntry struct {
	SampleEntry   `mp4:"0,extend,opt=dynamic"`
	EntryVersion  uint16    `mp4:"1,size=16,opt=dynamic"`
	Reserved      [3]uint16 `mp4:"2,size=16,opt=dynamic,const=0"`
	ChannelCount  uint16    `mp4:"3,size=16,opt=dynamic"`
	SampleSize    uint16    `mp4:"4,size=16,opt=dynamic"`
	PreDefined    uint16    `mp4:"5,size=16,opt=dynamic"`
	Reserved2     uint16    `mp4:"6,size=16,opt=dynamic,const=0"`
	SampleRate    uint32    `mp4:"7,size=32,opt=dynamic"` // fixed-point 16.16
	QuickTimeData []byte    `mp4:"8,size=8,opt=dynamic,len=dynamic"`
}

func (ase *AudioSampleEntry) IsOptFieldEnabled(name string, ctx Context) bool {
	if name == "QuickTimeData" {
		return ctx.IsQuickTimeCompatible && (ctx.UnderWave || ase.EntryVersion == 1 || ase.EntryVersion == 2)
	}
	if ctx.IsQuickTimeCompatible && ctx.UnderWave {
		return false
	}
	return true
}

func (ase *AudioSampleEntry) GetFieldLength(name string, ctx Context) uint {
	if name == "QuickTimeData" && ctx.IsQuickTimeCompatible {
		if ctx.UnderWave {
			return LengthUnlimited
		} else if ase.EntryVersion == 1 {
			return 16
		} else if ase.EntryVersion == 2 {
			return 36
		}
	}
	return 0
}

// StringifyField returns field value as string
func (ase *AudioSampleEntry) StringifyField(name string, indent string, depth int, ctx Context) (string, bool) {
	switch name {
	case "SampleRate":
		return util.FormatUnsignedFixedFloat1616(ase.SampleRate), true
	default:
		return "", false
	}
}

func (ase *AudioSampleEntry) GetSampleRate() float64 {
	return float64(ase.SampleRate) / (1 << 16)
}

func (ase *AudioSampleEntry) GetSampleRateInt() uint16 {
	return uint16(ase.SampleRate >> 16)
}

const (
	AVCBaselineProfile uint8 = 66  // 0x42
	AVCMainProfile     uint8 = 77  // 0x4d
	AVCExtendedProfile uint8 = 88  // 0x58
	AVCHighProfile     uint8 = 100 // 0x64
	AVCHigh10Profile   uint8 = 110 // 0x6e
	AVCHigh422Profile  uint8 = 122 // 0x7a
)

type AVCDecoderConfiguration struct {
	AnyTypeBox
	ConfigurationVersion         uint8             `mp4:"0,size=8"`
	Profile                      uint8             `mp4:"1,size=8"`
	ProfileCompatibility         uint8             `mp4:"2,size=8"`
	Level                        uint8             `mp4:"3,size=8"`
	Reserved                     uint8             `mp4:"4,size=6,const=63"`
	LengthSizeMinusOne           uint8             `mp4:"5,size=2"`
	Reserved2                    uint8             `mp4:"6,size=3,const=7"`
	NumOfSequenceParameterSets   uint8             `mp4:"7,size=5"`
	SequenceParameterSets        []AVCParameterSet `mp4:"8,len=dynamic"`
	NumOfPictureParameterSets    uint8             `mp4:"9,size=8"`
	PictureParameterSets         []AVCParameterSet `mp4:"10,len=dynamic"`
	HighProfileFieldsEnabled     bool              `mp4:"11,hidden"`
	Reserved3                    uint8             `mp4:"12,size=6,opt=dynamic,const=63"`
	ChromaFormat                 uint8             `mp4:"13,size=2,opt=dynamic"`
	Reserved4                    uint8             `mp4:"14,size=5,opt=dynamic,const=31"`
	BitDepthLumaMinus8           uint8             `mp4:"15,size=3,opt=dynamic"`
	Reserved5                    uint8             `mp4:"16,size=5,opt=dynamic,const=31"`
	BitDepthChromaMinus8         uint8             `mp4:"17,size=3,opt=dynamic"`
	NumOfSequenceParameterSetExt uint8             `mp4:"18,size=8,opt=dynamic"`
	SequenceParameterSetsExt     []AVCParameterSet `mp4:"19,len=dynamic,opt=dynamic"`
}

func (avcc *AVCDecoderConfiguration) GetFieldLength(name string, ctx Context) uint {
	switch name {
	case "SequenceParameterSets":
		return uint(avcc.NumOfSequenceParameterSets)
	case "PictureParameterSets":
		return uint(avcc.NumOfPictureParameterSets)
	case "SequenceParameterSetsExt":
		return uint(avcc.NumOfSequenceParameterSetExt)
	}
	return 0
}

func (avcc *AVCDecoderConfiguration) IsOptFieldEnabled(name string, ctx Context) bool {
	switch name {
	case "Reserved3",
		"ChromaFormat",
		"Reserved4",
		"BitDepthLumaMinus8",
		"Reserved5",
		"BitDepthChromaMinus8",
		"NumOfSequenceParameterSetExt",
		"SequenceParameterSetsExt":
		return avcc.HighProfileFieldsEnabled
	}
	return false
}

func (avcc *AVCDecoderConfiguration) OnReadField(name string, r bitio.ReadSeeker, leftBits uint64, ctx Context) (rbits uint64, override bool, err error) {
	if name == "HighProfileFieldsEnabled" {
		avcc.HighProfileFieldsEnabled = leftBits >= 32 &&
			(avcc.Profile == AVCHighProfile ||
				avcc.Profile == AVCHigh10Profile ||
				avcc.Profile == AVCHigh422Profile ||
				avcc.Profile == 144)
		return 0, true, nil
	}
	return 0, false, nil
}

func (avcc *AVCDecoderConfiguration) OnWriteField(name string, w bitio.Writer, ctx Context) (wbits uint64, override bool, err error) {
	if name == "HighProfileFieldsEnabled" {
		if avcc.HighProfileFieldsEnabled &&
			avcc.Profile != AVCHighProfile &&
			avcc.Profile != AVCHigh10Profile &&
			avcc.Profile != AVCHigh422Profile &&
			avcc.Profile != 144 {
			return 0, false, errors.New("each values of Profile and HighProfileFieldsEnabled are inconsistent")
		}
		return 0, true, nil
	}
	return 0, false, nil
}

type AVCParameterSet struct {
	BaseCustomFieldObject
	Length  uint16 `mp4:"0,size=16"`
	NALUnit []byte `mp4:"1,size=8,len=dynamic"`
}

func (s *AVCParameterSet) GetFieldLength(name string, ctx Context) uint {
	switch name {
	case "NALUnit":
		return uint(s.Length)
	}
	return 0
}

type PixelAspectRatioBox struct {
	AnyTypeBox
	HSpacing uint32 `mp4:"0,size=32"`
	VSpacing uint32 `mp4:"1,size=32"`
}

/*************************** sbgp ****************************/

func BoxTypeSbgp() BoxType { return StrToBoxType("sbgp") }

func init() {
	AddBoxDef(&Sbgp{}, 0, 1)
}

type Sbgp struct {
	FullBox               `mp4:"0,extend"`
	GroupingType          uint32      `mp4:"1,size=32"`
	GroupingTypeParameter uint32      `mp4:"2,size=32,ver=1"`
	EntryCount            uint32      `mp4:"3,size=32"`
	Entries               []SbgpEntry `mp4:"4,len=dynamic,size=64"`
}

type SbgpEntry struct {
	SampleCount           uint32 `mp4:"0,size=32"`
	GroupDescriptionIndex uint32 `mp4:"1,size=32"`
}

func (sbgp *Sbgp) GetFieldLength(name string, ctx Context) uint {
	switch name {
	case "Entries":
		return uint(sbgp.EntryCount)
	}
	panic(fmt.Errorf("invalid name of dynamic-length field: boxType=sbgp fieldName=%s", name))
}

func (*Sbgp) GetType() BoxType {
	return BoxTypeSbgp()
}

/*************************** schi ****************************/

func BoxTypeSchi() BoxType { return StrToBoxType("schi") }

func init() {
	AddBoxDef(&Schi{})
}

type Schi struct {
	Box
}

func (*Schi) GetType() BoxType {
	return BoxTypeSchi()
}

/*************************** schm ****************************/

func BoxTypeSchm() BoxType { return StrToBoxType("schm") }

func init() {
	AddBoxDef(&Schm{}, 0)
}

type Schm struct {
	FullBox       `mp4:"0,extend"`
	SchemeType    [4]byte `mp4:"1,size=8,string"`
	SchemeVersion uint32  `mp4:"2,size=32,hex"`
	SchemeUri     []byte  `mp4:"3,size=8,opt=0x000001,string"`
}

func (*Schm) GetType() BoxType {
	return BoxTypeSchm()
}

/*************************** sdtp ****************************/

func BoxTypeSdtp() BoxType { return StrToBoxType("sdtp") }

func init() {
	AddBoxDef(&Sdtp{}, 0)
}

type Sdtp struct {
	FullBox `mp4:"0,extend"`
	Samples []SdtpSampleElem `mp4:"1,size=8"`
}

type SdtpSampleElem struct {
	IsLeading           uint8 `mp4:"0,size=2"`
	SampleDependsOn     uint8 `mp4:"1,size=2"`
	SampleIsDependedOn  uint8 `mp4:"2,size=2"`
	SampleHasRedundancy uint8 `mp4:"3,size=2"`
}

func (*Sdtp) GetType() BoxType {
	return BoxTypeSdtp()
}

/*************************** sgpd ****************************/

func BoxTypeSgpd() BoxType { return StrToBoxType("sgpd") }

func init() {
	AddBoxDef(&Sgpd{}, 1, 2) // version 0 is deprecated by ISO/IEC 14496-12
}

type Sgpd struct {
	FullBox                       `mp4:"0,extend"`
	GroupingType                  [4]byte                    `mp4:"1,size=8,string"`
	DefaultLength                 uint32                     `mp4:"2,size=32,ver=1"`
	DefaultSampleDescriptionIndex uint32                     `mp4:"3,size=32,ver=2"`
	EntryCount                    uint32                     `mp4:"4,size=32"`
	RollDistances                 []int16                    `mp4:"5,size=16,opt=dynamic"`
	RollDistancesL                []RollDistanceWithLength   `mp4:"6,size=16,opt=dynamic"`
	AlternativeStartupEntries     []AlternativeStartupEntry  `mp4:"7,size=dynamic,len=dynamic,opt=dynamic"`
	AlternativeStartupEntriesL    []AlternativeStartupEntryL `mp4:"8,len=dynamic,opt=dynamic"`
	VisualRandomAccessEntries     []VisualRandomAccessEntry  `mp4:"9,len=dynamic,opt=dynamic"`
	VisualRandomAccessEntriesL    []VisualRandomAccessEntryL `mp4:"10,len=dynamic,opt=dynamic"`
	TemporalLevelEntries          []TemporalLevelEntry       `mp4:"11,len=dynamic,opt=dynamic"`
	TemporalLevelEntriesL         []TemporalLevelEntryL      `mp4:"12,len=dynamic,opt=dynamic"`
	Unsupported                   []byte                     `mp4:"13,size=8,opt=dynamic"`
}

type RollDistanceWithLength struct {
	DescriptionLength uint32 `mp4:"0,size=32"`
	RollDistance      int16  `mp4:"1,size=16"`
}

type AlternativeStartupEntry struct {
	BaseCustomFieldObject
	RollCount         uint16                       `mp4:"0,size=16"`
	FirstOutputSample uint16                       `mp4:"1,size=16"`
	SampleOffset      []uint32                     `mp4:"2,size=32,len=dynamic"`
	Opts              []AlternativeStartupEntryOpt `mp4:"3,size=32"`
}

type AlternativeStartupEntryL struct {
	DescriptionLength       uint32 `mp4:"0,size=32"`
	AlternativeStartupEntry `mp4:"1,extend,size=dynamic"`
}

type AlternativeStartupEntryOpt struct {
	NumOutputSamples uint16 `mp4:"0,size=16"`
	NumTotalSamples  uint16 `mp4:"1,size=16"`
}

type VisualRandomAccessEntry struct {
	NumLeadingSamplesKnown bool  `mp4:"0,size=1"`
	NumLeadingSamples      uint8 `mp4:"1,size=7"`
}

type VisualRandomAccessEntryL struct {
	DescriptionLength       uint32 `mp4:"0,size=32"`
	VisualRandomAccessEntry `mp4:"1,extend"`
}

type TemporalLevelEntry struct {
	LevelIndependentlyDecodable bool  `mp4:"0,size=1"`
	Reserved                    uint8 `mp4:"1,size=7,const=0"`
}

type TemporalLevelEntryL struct {
	DescriptionLength  uint32 `mp4:"0,size=32"`
	TemporalLevelEntry `mp4:"1,extend"`
}

func (sgpd *Sgpd) GetFieldSize(name string, ctx Context) uint {
	switch name {
	case "AlternativeStartupEntries":
		return uint(sgpd.DefaultLength * 8)
	}
	return 0
}

func (sgpd *Sgpd) GetFieldLength(name string, ctx Context) uint {
	switch name {
	case "RollDistances", "RollDistancesL",
		"AlternativeStartupEntries", "AlternativeStartupEntriesL",
		"VisualRandomAccessEntries", "VisualRandomAccessEntriesL",
		"TemporalLevelEntries", "TemporalLevelEntriesL":
		return uint(sgpd.EntryCount)
	}
	return 0
}

func (sgpd *Sgpd) IsOptFieldEnabled(name string, ctx Context) bool {
	noDefaultLength := sgpd.Version == 1 && sgpd.DefaultLength == 0
	rollDistances := sgpd.GroupingType == [4]byte{'r', 'o', 'l', 'l'} ||
		sgpd.GroupingType == [4]byte{'p', 'r', 'o', 'l'}
	alternativeStartupEntries := sgpd.GroupingType == [4]byte{'a', 'l', 's', 't'}
	visualRandomAccessEntries := sgpd.GroupingType == [4]byte{'r', 'a', 'p', ' '}
	temporalLevelEntries := sgpd.GroupingType == [4]byte{'t', 'e', 'l', 'e'}
	switch name {
	case "RollDistances":
		return rollDistances && !noDefaultLength
	case "RollDistancesL":
		return rollDistances && noDefaultLength
	case "AlternativeStartupEntries":
		return alternativeStartupEntries && !noDefaultLength
	case "AlternativeStartupEntriesL":
		return alternativeStartupEntries && noDefaultLength
	case "VisualRandomAccessEntries":
		return visualRandomAccessEntries && !noDefaultLength
	case "VisualRandomAccessEntriesL":
		return visualRandomAccessEntries && noDefaultLength
	case "TemporalLevelEntries":
		return temporalLevelEntries && !noDefaultLength
	case "TemporalLevelEntriesL":
		return temporalLevelEntries && noDefaultLength
	case "Unsupported":
		return !rollDistances &&
			!alternativeStartupEntries &&
			!visualRandomAccessEntries &&
			!temporalLevelEntries
	default:
		return false
	}
}

func (*Sgpd) GetType() BoxType {
	return BoxTypeSgpd()
}

func (entry *AlternativeStartupEntry) GetFieldLength(name string, ctx Context) uint {
	switch name {
	case "SampleOffset":
		return uint(entry.RollCount)
	}
	return 0
}

func (entry *AlternativeStartupEntryL) GetFieldSize(name string, ctx Context) uint {
	switch name {
	case "AlternativeStartupEntry":
		return uint(entry.DescriptionLength * 8)
	}
	return 0
}

/*************************** sidx ****************************/

func BoxTypeSidx() BoxType { return StrToBoxType("sidx") }

func init() {
	AddBoxDef(&Sidx{}, 0, 1)
}

type Sidx struct {
	FullBox                    `mp4:"0,extend"`
	ReferenceID                uint32          `mp4:"1,size=32"`
	Timescale                  uint32          `mp4:"2,size=32"`
	EarliestPresentationTimeV0 uint32          `mp4:"3,size=32,ver=0"`
	FirstOffsetV0              uint32          `mp4:"4,size=32,ver=0"`
	EarliestPresentationTimeV1 uint64          `mp4:"5,size=64,nver=0"`
	FirstOffsetV1              uint64          `mp4:"6,size=64,nver=0"`
	Reserved                   uint16          `mp4:"7,size=16,const=0"`
	ReferenceCount             uint16          `mp4:"8,size=16"`
	References                 []SidxReference `mp4:"9,size=96,len=dynamic"`
}

type SidxReference struct {
	ReferenceType      bool   `mp4:"0,size=1"`
	ReferencedSize     uint32 `mp4:"1,size=31"`
	SubsegmentDuration uint32 `mp4:"2,size=32"`
	StartsWithSAP      bool   `mp4:"3,size=1"`
	SAPType            uint32 `mp4:"4,size=3"`
	SAPDeltaTime       uint32 `mp4:"5,size=28"`
}

func (*Sidx) GetType() BoxType {
	return BoxTypeSidx()
}

func (sidx *Sidx) GetFieldLength(name string, ctx Context) uint {
	switch name {
	case "References":
		return uint(sidx.ReferenceCount)
	}
	panic(fmt.Errorf("invalid name of dynamic-length field: boxType=sidx fieldName=%s", name))
}

func (sidx *Sidx) GetEarliestPresentationTime() uint64 {
	switch sidx.GetVersion() {
	case 0:
		return uint64(sidx.EarliestPresentationTimeV0)
	case 1:
		return sidx.EarliestPresentationTimeV1
	default:
		return 0
	}
}

func (sidx *Sidx) GetFirstOffset() uint64 {
	switch sidx.GetVersion() {
	case 0:
		return uint64(sidx.FirstOffsetV0)
	case 1:
		return sidx.FirstOffsetV1
	default:
		return 0
	}
}

/*************************** sinf ****************************/

func BoxTypeSinf() BoxType { return StrToBoxType("sinf") }

func init() {
	AddBoxDef(&Sinf{})
}

type Sinf struct {
	Box
}

func (*Sinf) GetType() BoxType {
	return BoxTypeSinf()
}

/*************************** smhd ****************************/

func BoxTypeSmhd() BoxType { return StrToBoxType("smhd") }

func init() {
	AddBoxDef(&Smhd{}, 0)
}

type Smhd struct {
	FullBox  `mp4:"0,extend"`
	Balance  int16  `mp4:"1,size=16"` // fixed-point 8.8 template=0
	Reserved uint16 `mp4:"2,size=16,const=0"`
}

func (*Smhd) GetType() BoxType {
	return BoxTypeSmhd()
}

// StringifyField returns field value as string
func (smhd *Smhd) StringifyField(name string, indent string, depth int, ctx Context) (string, bool) {
	switch name {
	case "Balance":
		return util.FormatSignedFixedFloat88(smhd.Balance), true
	default:
		return "", false
	}
}

// GetBalance returns value of width as float32
func (smhd *Smhd) GetBalance() float32 {
	return float32(smhd.Balance) / (1 << 8)
}

// GetBalanceInt returns value of width as int8
func (smhd *Smhd) GetBalanceInt() int8 {
	return int8(smhd.Balance >> 8)
}

/*************************** stbl ****************************/

func BoxTypeStbl() BoxType { return StrToBoxType("stbl") }

func init() {
	AddBoxDef(&Stbl{})
}

// Stbl is ISOBMFF stbl box type
type Stbl struct {
	Box
}

// GetType returns the BoxType
func (*Stbl) GetType() BoxType {
	return BoxTypeStbl()
}

/*************************** stco ****************************/

func BoxTypeStco() BoxType { return StrToBoxType("stco") }

func init() {
	AddBoxDef(&Stco{}, 0)
}

// Stco is ISOBMFF stco box type
type Stco struct {
	FullBox     `mp4:"0,extend"`
	EntryCount  uint32   `mp4:"1,size=32"`
	ChunkOffset []uint32 `mp4:"2,size=32,len=dynamic"`
}

// GetType returns the BoxType
func (*Stco) GetType() BoxType {
	return BoxTypeStco()
}

// GetFieldLength returns length of dynamic field
func (stco *Stco) GetFieldLength(name string, ctx Context) uint {
	switch name {
	case "ChunkOffset":
		return uint(stco.EntryCount)
	}
	panic(fmt.Errorf("invalid name of dynamic-length field: boxType=stco fieldName=%s", name))
}

/*************************** stsc ****************************/

func BoxTypeStsc() BoxType { return StrToBoxType("stsc") }

func init() {
	AddBoxDef(&Stsc{}, 0)
}

// Stsc is ISOBMFF stsc box type
type Stsc struct {
	FullBox    `mp4:"0,extend"`
	EntryCount uint32      `mp4:"1,size=32"`
	Entries    []StscEntry `mp4:"2,len=dynamic,size=96"`
}

type StscEntry struct {
	FirstChunk             uint32 `mp4:"0,size=32"`
	SamplesPerChunk        uint32 `mp4:"1,size=32"`
	SampleDescriptionIndex uint32 `mp4:"2,size=32"`
}

// GetType returns the BoxType
func (*Stsc) GetType() BoxType {
	return BoxTypeStsc()
}

// GetFieldLength returns length of dynamic field
func (stsc *Stsc) GetFieldLength(name string, ctx Context) uint {
	switch name {
	case "Entries":
		return uint(stsc.EntryCount)
	}
	panic(fmt.Errorf("invalid name of dynamic-length field: boxType=stsc fieldName=%s", name))
}

/*************************** stsd ****************************/

func BoxTypeStsd() BoxType { return StrToBoxType("stsd") }

func init() {
	AddBoxDef(&Stsd{}, 0)
}

// Stsd is ISOBMFF stsd box type
type Stsd struct {
	FullBox    `mp4:"0,extend"`
	EntryCount uint32 `mp4:"1,size=32"`
}

// GetType returns the BoxType
func (*Stsd) GetType() BoxType {
	return BoxTypeStsd()
}

/*************************** stss ****************************/

func BoxTypeStss() BoxType { return StrToBoxType("stss") }

func init() {
	AddBoxDef(&Stss{}, 0)
}

type Stss struct {
	FullBox      `mp4:"0,extend"`
	EntryCount   uint32   `mp4:"1,size=32"`
	SampleNumber []uint32 `mp4:"2,len=dynamic,size=32"`
}

// GetType returns the BoxType
func (*Stss) GetType() BoxType {
	return BoxTypeStss()
}

// GetFieldLength returns length of dynamic field
func (stss *Stss) GetFieldLength(name string, ctx Context) uint {
	switch name {
	case "SampleNumber":
		return uint(stss.EntryCount)
	}
	panic(fmt.Errorf("invalid name of dynamic-length field: boxType=stss fieldName=%s", name))
}

/*************************** stsz ****************************/

func BoxTypeStsz() BoxType { return StrToBoxType("stsz") }

func init() {
	AddBoxDef(&Stsz{}, 0)
}

// Stsz is ISOBMFF stsz box type
type Stsz struct {
	FullBox     `mp4:"0,extend"`
	SampleSize  uint32   `mp4:"1,size=32"`
	SampleCount uint32   `mp4:"2,size=32"`
	EntrySize   []uint32 `mp4:"3,size=32,len=dynamic"`
}

// GetType returns the BoxType
func (*Stsz) GetType() BoxType {
	return BoxTypeStsz()
}

// GetFieldLength returns length of dynamic field
func (stsz *Stsz) GetFieldLength(name string, ctx Context) uint {
	switch name {
	case "EntrySize":
		if stsz.SampleSize == 0 {
			return uint(stsz.SampleCount)
		}
		return 0
	}
	panic(fmt.Errorf("invalid name of dynamic-length field: boxType=stsz fieldName=%s", name))
}

/*************************** stts ****************************/

func BoxTypeStts() BoxType { return StrToBoxType("stts") }

func init() {
	AddBoxDef(&Stts{}, 0)
}

// Stts is ISOBMFF stts box type
type Stts struct {
	FullBox    `mp4:"0,extend"`
	EntryCount uint32      `mp4:"1,size=32"`
	Entries    []SttsEntry `mp4:"2,len=dynamic,size=64"`
}

type SttsEntry struct {
	SampleCount uint32 `mp4:"0,size=32"`
	SampleDelta uint32 `mp4:"1,size=32"`
}

// GetType returns the BoxType
func (*Stts) GetType() BoxType {
	return BoxTypeStts()
}

// GetFieldLength returns length of dynamic field
func (stts *Stts) GetFieldLength(name string, ctx Context) uint {
	switch name {
	case "Entries":
		return uint(stts.EntryCount)
	}
	panic(fmt.Errorf("invalid name of dynamic-length field: boxType=stts fieldName=%s", name))
}

/*************************** styp ****************************/

func BoxTypeStyp() BoxType { return StrToBoxType("styp") }

func init() {
	AddBoxDef(&Styp{})
}

type Styp struct {
	Box
	MajorBrand       [4]byte               `mp4:"0,size=8,string"`
	MinorVersion     uint32                `mp4:"1,size=32"`
	CompatibleBrands []CompatibleBrandElem `mp4:"2,size=32"` // reach to end of the box
}

func (*Styp) GetType() BoxType {
	return BoxTypeStyp()
}

/*************************** tfdt ****************************/

func BoxTypeTfdt() BoxType { return StrToBoxType("tfdt") }

func init() {
	AddBoxDef(&Tfdt{}, 0, 1)
}

// Tfdt is ISOBMFF tfdt box type
type Tfdt struct {
	FullBox               `mp4:"0,extend"`
	BaseMediaDecodeTimeV0 uint32 `mp4:"1,size=32,ver=0"`
	BaseMediaDecodeTimeV1 uint64 `mp4:"2,size=64,ver=1"`
}

// GetType returns the BoxType
func (*Tfdt) GetType() BoxType {
	return BoxTypeTfdt()
}

func (tfdt *Tfdt) GetBaseMediaDecodeTime() uint64 {
	switch tfdt.GetVersion() {
	case 0:
		return uint64(tfdt.BaseMediaDecodeTimeV0)
	case 1:
		return tfdt.BaseMediaDecodeTimeV1
	default:
		return 0
	}
}

/*************************** tfhd ****************************/

func BoxTypeTfhd() BoxType { return StrToBoxType("tfhd") }

func init() {
	AddBoxDef(&Tfhd{}, 0)
}

// Tfhd is ISOBMFF tfhd box type
type Tfhd struct {
	FullBox `mp4:"0,extend"`
	TrackID uint32 `mp4:"1,size=32"`

	// optional
	BaseDataOffset         uint64 `mp4:"2,size=64,opt=0x000001"`
	SampleDescriptionIndex uint32 `mp4:"3,size=32,opt=0x000002"`
	DefaultSampleDuration  uint32 `mp4:"4,size=32,opt=0x000008"`
	DefaultSampleSize      uint32 `mp4:"5,size=32,opt=0x000010"`
	DefaultSampleFlags     uint32 `mp4:"6,size=32,opt=0x000020,hex"`
}

const (
	TfhdBaseDataOffsetPresent         = 0x000001
	TfhdSampleDescriptionIndexPresent = 0x000002
	TfhdDefaultSampleDurationPresent  = 0x000008
	TfhdDefaultSampleSizePresent      = 0x000010
	TfhdDefaultSampleFlagsPresent     = 0x000020
	TfhdDurationIsEmpty               = 0x010000
	TfhdDefaultBaseIsMoof             = 0x020000
)

// GetType returns the BoxType
func (*Tfhd) GetType() BoxType {
	return BoxTypeTfhd()
}

/*************************** tfra ****************************/

func BoxTypeTfra() BoxType { return StrToBoxType("tfra") }

func init() {
	AddBoxDef(&Tfra{}, 0, 1)
}

// Tfra is ISOBMFF tfra box type
type Tfra struct {
	FullBox               `mp4:"0,extend"`
	TrackID               uint32      `mp4:"1,size=32"`
	Reserved              uint32      `mp4:"2,size=26,const=0"`
	LengthSizeOfTrafNum   byte        `mp4:"3,size=2"`
	LengthSizeOfTrunNum   byte        `mp4:"4,size=2"`
	LengthSizeOfSampleNum byte        `mp4:"5,size=2"`
	NumberOfEntry         uint32      `mp4:"6,size=32"`
	Entries               []TfraEntry `mp4:"7,len=dynamic,size=dynamic"`
}

type TfraEntry struct {
	TimeV0       uint32 `mp4:"0,size=32,ver=0"`
	MoofOffsetV0 uint32 `mp4:"1,size=32,ver=0"`
	TimeV1       uint64 `mp4:"2,size=64,ver=1"`
	MoofOffsetV1 uint64 `mp4:"3,size=64,ver=1"`
	TrafNumber   uint32 `mp4:"4,size=dynamic"`
	TrunNumber   uint32 `mp4:"5,size=dynamic"`
	SampleNumber uint32 `mp4:"6,size=dynamic"`
}

// GetType returns the BoxType
func (*Tfra) GetType() BoxType {
	return BoxTypeTfra()
}

// GetFieldSize returns size of dynamic field
func (tfra *Tfra) GetFieldSize(name string, ctx Context) uint {
	switch name {
	case "TrafNumber":
		return (uint(tfra.LengthSizeOfTrafNum) + 1) * 8
	case "TrunNumber":
		return (uint(tfra.LengthSizeOfTrunNum) + 1) * 8
	case "SampleNumber":
		return (uint(tfra.LengthSizeOfSampleNum) + 1) * 8
	case "Entries":
		switch tfra.GetVersion() {
		case 0:
			return 0 +
				/* TimeV0       */ 32 +
				/* MoofOffsetV0 */ 32 +
				/* TrafNumber   */ (uint(tfra.LengthSizeOfTrafNum)+1)*8 +
				/* TrunNumber   */ (uint(tfra.LengthSizeOfTrunNum)+1)*8 +
				/* SampleNumber */ (uint(tfra.LengthSizeOfSampleNum)+1)*8
		case 1:
			return 0 +
				/* TimeV1       */ 64 +
				/* MoofOffsetV1 */ 64 +
				/* TrafNumber   */ (uint(tfra.LengthSizeOfTrafNum)+1)*8 +
				/* TrunNumber   */ (uint(tfra.LengthSizeOfTrunNum)+1)*8 +
				/* SampleNumber */ (uint(tfra.LengthSizeOfSampleNum)+1)*8
		}
	}
	panic(fmt.Errorf("invalid name of dynamic-size field: boxType=tfra fieldName=%s", name))
}

// GetFieldLength returns length of dynamic field
func (tfra *Tfra) GetFieldLength(name string, ctx Context) uint {
	switch name {
	case "Entries":
		return uint(tfra.NumberOfEntry)
	}
	panic(fmt.Errorf("invalid name of dynamic-length field: boxType=tfra fieldName=%s", name))
}

func (tfra *Tfra) GetTime(index int) uint64 {
	switch tfra.GetVersion() {
	case 0:
		return uint64(tfra.Entries[index].TimeV0)
	case 1:
		return tfra.Entries[index].TimeV1
	default:
		return 0
	}
}

func (tfra *Tfra) GetMoofOffset(index int) uint64 {
	switch tfra.GetVersion() {
	case 0:
		return uint64(tfra.Entries[index].MoofOffsetV0)
	case 1:
		return tfra.Entries[index].MoofOffsetV1
	default:
		return 0
	}
}

/*************************** tkhd ****************************/

func BoxTypeTkhd() BoxType { return StrToBoxType("tkhd") }

func init() {
	AddBoxDef(&Tkhd{}, 0, 1)
}

// Tkhd is ISOBMFF tkhd box type
type Tkhd struct {
	FullBox            `mp4:"0,extend"`
	CreationTimeV0     uint32 `mp4:"1,size=32,ver=0"`
	ModificationTimeV0 uint32 `mp4:"2,size=32,ver=0"`
	CreationTimeV1     uint64 `mp4:"3,size=64,ver=1"`
	ModificationTimeV1 uint64 `mp4:"4,size=64,ver=1"`
	TrackID            uint32 `mp4:"5,size=32"`
	Reserved0          uint32 `mp4:"6,size=32,const=0"`
	DurationV0         uint32 `mp4:"7,size=32,ver=0"`
	DurationV1         uint64 `mp4:"8,size=64,ver=1"`
	//
	Reserved1      [2]uint32 `mp4:"9,size=32,const=0"`
	Layer          int16     `mp4:"10,size=16"` // template=0
	AlternateGroup int16     `mp4:"11,size=16"` // template=0
	Volume         int16     `mp4:"12,size=16"` // template={if track_is_audio 0x0100 else 0}
	Reserved2      uint16    `mp4:"13,size=16,const=0"`
	Matrix         [9]int32  `mp4:"14,size=32,hex"` // template={ 0x00010000,0,0,0,0x00010000,0,0,0,0x40000000 };
	Width          uint32    `mp4:"15,size=32"`     // fixed-point 16.16
	Height         uint32    `mp4:"16,size=32"`     // fixed-point 16.16
}

// GetType returns the BoxType
func (*Tkhd) GetType() BoxType {
	return BoxTypeTkhd()
}

// StringifyField returns field value as string
func (tkhd *Tkhd) StringifyField(name string, indent string, depth int, ctx Context) (string, bool) {
	switch name {
	case "Width":
		return util.FormatUnsignedFixedFloat1616(tkhd.Width), true
	case "Height":
		return util.FormatUnsignedFixedFloat1616(tkhd.Height), true
	default:
		return "", false
	}
}

func (tkhd *Tkhd) GetCreationTime() uint64 {
	switch tkhd.GetVersion() {
	case 0:
		return uint64(tkhd.CreationTimeV0)
	case 1:
		return tkhd.CreationTimeV1
	default:
		return 0
	}
}

func (tkhd *Tkhd) GetModificationTime() uint64 {
	switch tkhd.GetVersion() {
	case 0:
		return uint64(tkhd.ModificationTimeV0)
	case 1:
		return tkhd.ModificationTimeV1
	default:
		return 0
	}
}

func (tkhd *Tkhd) GetDuration() uint64 {
	switch tkhd.GetVersion() {
	case 0:
		return uint64(tkhd.DurationV0)
	case 1:
		return tkhd.DurationV1
	default:
		return 0
	}
}

// GetWidth returns value of width as float64
func (tkhd *Tkhd) GetWidth() float64 {
	return float64(tkhd.Width) / (1 << 16)
}

// GetWidthInt returns value of width as uint16
func (tkhd *Tkhd) GetWidthInt() uint16 {
	return uint16(tkhd.Width >> 16)
}

// GetHeight returns value of height as float64
func (tkhd *Tkhd) GetHeight() float64 {
	return float64(tkhd.Height) / (1 << 16)
}

// GetHeightInt returns value of height as uint16
func (tkhd *Tkhd) GetHeightInt() uint16 {
	return uint16(tkhd.Height >> 16)
}

/*************************** traf ****************************/

func BoxTypeTraf() BoxType { return StrToBoxType("traf") }

func init() {
	AddBoxDef(&Traf{})
}

// Traf is ISOBMFF traf box type
type Traf struct {
	Box
}

// GetType returns the BoxType
func (*Traf) GetType() BoxType {
	return BoxTypeTraf()
}

/*************************** trak ****************************/

func BoxTypeTrak() BoxType { return StrToBoxType("trak") }

func init() {
	AddBoxDef(&Trak{})
}

// Trak is ISOBMFF trak box type
type Trak struct {
	Box
}

// GetType returns the BoxType
func (*Trak) GetType() BoxType {
	return BoxTypeTrak()
}

/*************************** trep ****************************/

func BoxTypeTrep() BoxType { return StrToBoxType("trep") }

func init() {
	AddBoxDef(&Trep{}, 0)
}

// Trep is ISOBMFF trep box type
type Trep struct {
	FullBox `mp4:"0,extend"`
	TrackID uint32 `mp4:"1,size=32"`
}

// GetType returns the BoxType
func (*Trep) GetType() BoxType {
	return BoxTypeTrep()
}

/*************************** trex ****************************/

func BoxTypeTrex() BoxType { return StrToBoxType("trex") }

func init() {
	AddBoxDef(&Trex{}, 0)
}

// Trex is ISOBMFF trex box type
type Trex struct {
	FullBox                       `mp4:"0,extend"`
	TrackID                       uint32 `mp4:"1,size=32"`
	DefaultSampleDescriptionIndex uint32 `mp4:"2,size=32"`
	DefaultSampleDuration         uint32 `mp4:"3,size=32"`
	DefaultSampleSize             uint32 `mp4:"4,size=32"`
	DefaultSampleFlags            uint32 `mp4:"5,size=32,hex"`
}

// GetType returns the BoxType
func (*Trex) GetType() BoxType {
	return BoxTypeTrex()
}

/*************************** trun ****************************/

func BoxTypeTrun() BoxType { return StrToBoxType("trun") }

func init() {
	AddBoxDef(&Trun{}, 0, 1)
}

// Trun is ISOBMFF trun box type
type Trun struct {
	FullBox     `mp4:"0,extend"`
	SampleCount uint32 `mp4:"1,size=32"`

	// optional fields
	DataOffset       int32       `mp4:"2,size=32,opt=0x000001"`
	FirstSampleFlags uint32      `mp4:"3,size=32,opt=0x000004,hex"`
	Entries          []TrunEntry `mp4:"4,len=dynamic,size=dynamic"`
}

type TrunEntry struct {
	SampleDuration                uint32 `mp4:"0,size=32,opt=0x000100"`
	SampleSize                    uint32 `mp4:"1,size=32,opt=0x000200"`
	SampleFlags                   uint32 `mp4:"2,size=32,opt=0x000400,hex"`
	SampleCompositionTimeOffsetV0 uint32 `mp4:"3,size=32,opt=0x000800,ver=0"`
	SampleCompositionTimeOffsetV1 int32  `mp4:"4,size=32,opt=0x000800,nver=0"`
}

// GetType returns the BoxType
func (*Trun) GetType() BoxType {
	return BoxTypeTrun()
}

// GetFieldSize returns size of dynamic field
func (trun *Trun) GetFieldSize(name string, ctx Context) uint {
	switch name {
	case "Entries":
		var size uint
		flags := trun.GetFlags()
		if flags&0x100 != 0 {
			size += 32 // SampleDuration
		}
		if flags&0x200 != 0 {
			size += 32 // SampleSize
		}
		if flags&0x400 != 0 {
			size += 32 // SampleFlags
		}
		if flags&0x800 != 0 {
			size += 32 // SampleCompositionTimeOffsetV0 or V1
		}
		return size
	}
	panic(fmt.Errorf("invalid name of dynamic-size field: boxType=trun fieldName=%s", name))
}

// GetFieldLength returns length of dynamic field
func (trun *Trun) GetFieldLength(name string, ctx Context) uint {
	switch name {
	case "Entries":
		return uint(trun.SampleCount)
	}
	panic(fmt.Errorf("invalid name of dynamic-length field: boxType=trun fieldName=%s", name))
}

func (trun *Trun) GetSampleCompositionTimeOffset(index int) int64 {
	switch trun.GetVersion() {
	case 0:
		return int64(trun.Entries[index].SampleCompositionTimeOffsetV0)
	case 1:
		return int64(trun.Entries[index].SampleCompositionTimeOffsetV1)
	default:
		return 0
	}
}

/*************************** udta ****************************/

func BoxTypeUdta() BoxType { return StrToBoxType("udta") }

func init() {
	AddBoxDef(&Udta{})
}

// Udta is ISOBMFF udta box type
type Udta struct {
	Box
}

// GetType returns the BoxType
func (*Udta) GetType() BoxType {
	return BoxTypeUdta()
}

func isUnderUdta(ctx Context) bool {
	return ctx.UnderUdta
}

/*************************** vmhd ****************************/

func BoxTypeVmhd() BoxType { return StrToBoxType("vmhd") }

func init() {
	AddBoxDef(&Vmhd{}, 0)
}

// Vmhd is ISOBMFF vmhd box type
type Vmhd struct {
	FullBox      `mp4:"0,extend"`
	Graphicsmode uint16    `mp4:"1,size=16"` // template=0
	Opcolor      [3]uint16 `mp4:"2,size=16"` // template={0, 0, 0}
}

// GetType returns the BoxType
func (*Vmhd) GetType() BoxType {
	return BoxTypeVmhd()
}

/*************************** wave ****************************/

func BoxTypeWave() BoxType { return StrToBoxType("wave") }

func init() {
	AddBoxDef(&Wave{})
}

// Wave is QuickTime wave box
type Wave struct {
	Box
}

// GetType returns the BoxType
func (*Wave) GetType() BoxType {
	return BoxTypeWave()
}
