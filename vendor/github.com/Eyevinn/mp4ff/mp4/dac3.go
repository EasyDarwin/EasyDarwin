package mp4

import (
	"bytes"
	"fmt"
	"io"
	"strings"

	"github.com/Eyevinn/mp4ff/bits"
)

// AC3SampleRates - Sample rates as defined in  ETSI TS 102 366 V1.4.1 (2017) section 4.4.1.3
// Signaled in fscod - Sample rate code - 2 bits
var AC3SampleRates = []int{48000, 44100, 32000}

// AX3acmodChanneTable - channel configurations from ETSI TS 102 366 V1.4.1 (2017) section 4.4.2.3A
// Signaled in acmod - audio coding mode - 3 bits
var AC3acmodChannelTable = []string{
	"L/R", //Ch1 Ch2 dual mono but name them L R
	"C",
	"L/R",
	"L/C/R",
	"L/R/Cs",
	"L/C/R/Cs",
	"L/R/Ls/Rs",
	"L/C/R/Ls/Rs",
}

// AC3BitrateCodesKbps - Bitrates in kbps ETSI TS 102 366 V1.4.1 Table F.4.1 (2017)
var AC3BitrateCodesKbps = []uint16{
	32,
	40,
	48,
	56,
	64,
	80,
	96,
	112,
	128,
	160,
	192,
	224,
	256,
	320,
	384,
	448,
	512,
	576,
	640,
}

// Dac3Box - AC3SpecificBox from ETSI TS 102 366 V1.4.1 F.4 (2017)
type Dac3Box struct {
	FSCod       byte
	BSID        byte
	BSMod       byte
	ACMod       byte
	LFEOn       byte
	BitRateCode byte
}

// DecodeDac3 - box-specific decode
func DecodeDac3(hdr BoxHeader, startPos uint64, r io.Reader) (Box, error) {
	data, err := io.ReadAll(r)
	if err != nil {
		return nil, err
	}
	return decodeDac3FromData(data)
}

// DecodeDac3SR - box-specific decode
func DecodeDac3SR(hdr BoxHeader, startPos uint64, sr bits.SliceReader) (Box, error) {
	data := sr.ReadBytes(hdr.payloadLen())
	if sr.AccError() != nil {
		return nil, sr.AccError()
	}
	return decodeDac3FromData(data)
}

func decodeDac3FromData(data []byte) (Box, error) {
	if len(data) != 3 {
		return nil, fmt.Errorf("not 3 bytes payload in dac3 box")
	}
	buf := bytes.NewBuffer(data)
	br := bits.NewReader(buf)
	b := Dac3Box{}
	b.FSCod = byte(br.Read(2))
	b.BSID = byte(br.Read(5))
	b.BSMod = byte(br.Read(3))
	b.ACMod = byte(br.Read(3))
	b.LFEOn = byte(br.Read(1))
	b.BitRateCode = byte(br.Read(5))
	// 5 bits reserved follows
	_ = br.Read(5)
	return &b, nil
}

// Type - box type
func (b *Dac3Box) Type() string {
	return "dac3"
}

// Size - calculated size of box
func (b *Dac3Box) Size() uint64 {
	return uint64(boxHeaderSize + 3)
}

// Encode - write box to w
func (b *Dac3Box) Encode(w io.Writer) error {
	sw := bits.NewFixedSliceWriter(int(b.Size()))
	err := b.EncodeSW(sw)
	if err != nil {
		return err
	}
	_, err = w.Write(sw.Bytes())
	return err
}

// Encode - write box to sw
func (b *Dac3Box) EncodeSW(sw bits.SliceWriter) error {
	err := EncodeHeaderSW(b, sw)
	if err != nil {
		return err
	}
	sw.WriteBits(uint(b.FSCod), 2)
	sw.WriteBits(uint(b.BSID), 5)
	sw.WriteBits(uint(b.BSMod), 3)
	sw.WriteBits(uint(b.ACMod), 3)
	sw.WriteBits(uint(b.LFEOn), 1)
	sw.WriteBits(uint(b.BitRateCode), 5)
	sw.WriteBits(0, 5) // 5-bits padding
	return sw.AccError()
}

// ChannelInfo - number of channels and channelmap according to E.1.3.1.8
func (b *Dac3Box) ChannelInfo() (nrChannels int, chanmap uint16) {
	speakers := GetChannelListFromACMod(b.ACMod)
	if b.LFEOn == 1 {
		speakers = append(speakers, "LFE")
	}
	nrChannels = len(speakers)
	for _, speaker := range speakers {
		chanmap |= CustomChannelMapLocations[speaker]
	}
	return nrChannels, chanmap
}

func (b *Dac3Box) Info(w io.Writer, specificBoxLevels, indent, indentStep string) error {
	bd := newInfoDumper(w, indent, b, -1, 0)
	bd.write(" - sampleRateCode=%d => sampleRate=%d", b.FSCod, AC3SampleRates[b.FSCod])
	bd.write(" - bitStreamInformation=%d", b.BSID)
	bd.write(" - audioCodingMode=%d => channelConfiguration=%q", b.ACMod, AC3acmodChannelTable[b.ACMod])
	bd.write(" - lowFrequencyEffectsChannelOn=%d", b.LFEOn)
	bd.write(" - bitRateCode=%d => bitrate=%dkbps", b.BitRateCode, AC3BitrateCodesKbps[b.BitRateCode])
	nrChannels, chanmap := b.ChannelInfo()
	bd.write(" - nrChannels=%d, chanmap=%04x", nrChannels, chanmap)
	return bd.err
}

func (b *Dac3Box) BitrateBps() int {
	return int(AC3BitrateCodesKbps[b.BitRateCode]) * 1000
}

func (b *Dac3Box) SamplingFrequency() int {
	return int(AC3SampleRates[b.FSCod])
}

// GetChannelListFromACMod - get list of channels from acmod byte
func GetChannelListFromACMod(acmod byte) []string {
	return strings.Split(AC3acmodChannelTable[acmod], "/")
}
