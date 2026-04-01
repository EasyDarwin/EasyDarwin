package aac

import (
	"fmt"
	"io"

	"github.com/Eyevinn/mp4ff/bits"
)

const (
	// AAClc - AAC-LC Low Complexity
	AAClc = 2
	// HEAACv1 - HE-AAC version 1 with SBR
	HEAACv1 = 5
	// HEAACv2 - HE-AAC version 2 with SBR and PS
	HEAACv2 = 29
)

// AudioSpecificConfig according to ISO/IEC 14496-3
// Syntax specified in Table 1.15
type AudioSpecificConfig struct {
	ObjectType           byte
	ChannelConfiguration byte // Defined in Table 1.19
	SamplingFrequency    int
	ExtensionFrequency   int
	SBRPresentFlag       bool
	PSPresentFlag        bool
}

// FrequencyTable maps frequency index to sample rate in Hz
var FrequencyTable = map[byte]int{
	0:  96000,
	1:  88200,
	2:  64000,
	3:  48000,
	4:  44100,
	5:  32000,
	6:  24000,
	7:  22050,
	8:  16000,
	9:  12000,
	10: 11025,
	11: 8000,
	12: 7350,
}

// ReverseFrequencies converts sample frequency to index
var ReverseFrequencies = map[int]byte{
	96000: 0,
	88200: 1,
	64000: 2,
	48000: 3,
	44100: 4,
	32000: 5,
	24000: 6,
	22050: 7,
	16000: 8,
	12000: 9,
	11025: 10,
	8000:  11,
	7350:  12,
}

/* Channel configurations according to table 1.19 in ISO/IEC 14496-3
0: Defined in AOT Specific Config
1: 1 channel: front-center
2: 2 channels: front-left, front-right
3: 3 channels: front-center, front-left, front-right
4: 4 channels: front-center, front-left, front-right, back-center
5: 5 channels: front-center, front-left, front-right, back-left, back-right
6: 6 channels: front-center, front-left, front-right, back-left, back-right, LFE-channel
7: 8 channels: front-center, front-left, front-right, side-left, side-right, back-left, back-right, LFE-channel
8-15: Reserved
*/

// DecodeAudioSpecificConfig -
func DecodeAudioSpecificConfig(r io.Reader) (*AudioSpecificConfig, error) {
	br := bits.NewReader(r)

	asc := &AudioSpecificConfig{}
	audioObjectType := byte(br.Read(5))
	asc.ObjectType = audioObjectType
	switch audioObjectType {
	case AAClc:
		// do nothing
	case HEAACv1:
		asc.SBRPresentFlag = true
	case HEAACv2:
		asc.SBRPresentFlag = true
		asc.PSPresentFlag = true
	default:
		return asc, fmt.Errorf("unsupported object type: %d", audioObjectType)
	}
	frequency, ok := getFrequency(br)
	if !ok {
		return asc, fmt.Errorf("strange frequency index")
	}
	asc.SamplingFrequency = frequency
	asc.ChannelConfiguration = byte(br.Read(4))
	switch audioObjectType {
	case HEAACv1, HEAACv2:
		extFrequency, ok := getFrequency(br)
		if !ok {
			return asc, fmt.Errorf("strange frequency index")
		}
		asc.ExtensionFrequency = extFrequency
		audioObjectType = byte(br.Read(5)) // Shall be set to AAC-LC here again
	}
	if audioObjectType != AAClc {
		return nil, fmt.Errorf("base audioObjectType is %d instead of AAC-LC (2)", audioObjectType)
	}
	//GASpecificConfig()
	_ = br.Read(3) //GASpecificConfig
	// Done (there may be trailing bits)
	return asc, nil
}

// Encode - write AudioSpecificConfig to w for AAC-LC and HE-AAC
func (a *AudioSpecificConfig) Encode(w io.Writer) error {
	switch a.ObjectType {
	case AAClc, HEAACv1, HEAACv2:
		// fine
	default:
		return fmt.Errorf("audioObjectType %d not supported", a.ObjectType)
	}
	bw := bits.NewWriter(w)
	bw.Write(uint(a.ObjectType), 5)
	samplingIndex, ok := ReverseFrequencies[a.SamplingFrequency]
	if ok {
		bw.Write(uint(samplingIndex), 4)
	} else {
		bw.Write(0x0f, 4)
		bw.Write(uint(a.SamplingFrequency), 24)
	}
	bw.Write(uint(a.ChannelConfiguration), 4)
	switch a.ObjectType {
	case HEAACv1, HEAACv2:
		samplingIndex, ok := ReverseFrequencies[a.ExtensionFrequency]
		if ok {
			bw.Write(uint(samplingIndex), 4)
		} else {
			bw.Write(0x0f, 4)
			bw.Write(uint(a.ExtensionFrequency), 24)
		}
		bw.Write(AAClc, 5) // base audioObjectType
	}
	bw.Write(0x00, 3) // GASpecificConfig
	bw.Flush()
	return bw.AccError()
}

// getFrequency - either from 4-bit index or 24-bit value
func getFrequency(br *bits.Reader) (frequency int, ok bool) {
	frequencyIndex := br.Read(4)
	if frequencyIndex == 0x0f {
		f := br.Read(24)
		if br.AccError() != nil {
			return 0, false
		}
		return int(f), true
	}
	if br.AccError() != nil {
		return 0, false
	}
	frequency, ok = FrequencyTable[byte(frequencyIndex)]
	return frequency, ok
}
