package aac

import (
	"bytes"
	"fmt"
	"io"

	"github.com/Eyevinn/mp4ff/bits"
)

// ADTSHeader - data for an unencrypted ADTS Header with one AAC frame.
// Not used in mp4 files, but in MPEG-2 TS.
// Defined in ISO/IEC 13818-7
type ADTSHeader struct {
	ID                     byte // 0 is MPEG-4, 1 is MPEG-2
	ObjectType             byte
	SamplingFrequencyIndex byte
	ChannelConfig          byte
	HeaderLength           byte // Should be 7 or 9
	PayloadLength          uint16
	BufferFullness         uint16
}

// NewADTSHeader - create a new ADTS header
func NewADTSHeader(samplingFrequency int, channelConfig byte, objectType byte, plLen uint16) (*ADTSHeader, error) {
	if objectType != AAClc {
		return nil, fmt.Errorf("Must use AAC-LC (type 2) not %d", objectType)
	}
	sfi, ok := ReverseFrequencies[samplingFrequency]
	if !ok {
		return nil, fmt.Errorf("Sampling frequency %d not supported", samplingFrequency)
	}
	return &ADTSHeader{
		ObjectType:             objectType,
		SamplingFrequencyIndex: sfi,
		ChannelConfig:          channelConfig,
		HeaderLength:           7,
		PayloadLength:          plLen,
		BufferFullness:         0x7ff, // variable bitrate
	}, nil
}

// Encode - encode ADTSHeader into byte slice
func (a ADTSHeader) Encode() []byte {
	buf := bytes.Buffer{}
	bw := bits.NewWriter(&buf)
	bw.Write(0xfff, 12)                         // sync word
	bw.Write(0x01, 4)                           //ID=0 for MPEG-4 + layer + protection absent
	bw.Write(uint(a.ObjectType)-1, 2)           // profile
	bw.Write(uint(a.SamplingFrequencyIndex), 4) // sampling frequency index (3 = 48KHz)
	bw.Write(0, 1)                              // private
	bw.Write(uint(a.ChannelConfig), 3)          // Channel configuration
	bw.Write(0, 4)                              // Copyright etc
	bw.Write(uint(a.PayloadLength+7), 13)       // The length should include this 7-byte header
	bw.Write(uint(a.BufferFullness), 11)        // Buffer fullness value
	bw.Write(0, 2)                              // Nr AAC frames in ADTS frame minus 1
	return buf.Bytes()
}

// DecodeADTSHeader by first looking for sync word
func DecodeADTSHeader(r io.Reader) (header *ADTSHeader, offset int, err error) {
	br := bits.NewReader(r)
	tsPacketSize := 188 // Find sync 0xfff in first 188 bytes (MPEG-TS related)
	syncFound := false
	offset = 0
	var sync1, sync2, mpegID, layer, protectionAbsent byte
	for i := 0; i < tsPacketSize; i++ {
		if sync2 != 0xff {
			sync1 = byte(br.Read(8))
		} else {
			sync1 = sync2
			offset--
		}
		if sync1 == 0xff {
			sync2 = byte(br.Read(8))
			startPattern := sync2 >> 4
			mpegID = (sync2 >> 3) & 1
			layer = (sync2 >> 1) & 3
			protectionAbsent = sync2 & 1
			if startPattern == 0xf && layer == 0 {
				syncFound = true
				break
			}
			offset++
		}
		offset++
	}
	if br.AccError() != nil {
		return nil, 0, fmt.Errorf("searching for 0xfff: %w", br.AccError())
	}

	if !syncFound {
		return nil, 0, fmt.Errorf("no 0xfff sync found")
	}
	if layer != 0 {
		return nil, 0, fmt.Errorf("non-permitted layer value %d", layer)
	}
	ah := &ADTSHeader{ID: mpegID, HeaderLength: 7}
	if protectionAbsent != 1 {
		ah.HeaderLength += 2 // 16-bit CRC
	}

	profile := br.Read(2)
	ah.ObjectType = byte(profile + 1)
	ah.SamplingFrequencyIndex = byte(br.Read(4))
	_ = br.Read(1) // ignore private
	ah.ChannelConfig = byte(br.Read(3))
	_ = br.Read(4) // ignore original/copy, home, copyright
	frameLength := br.Read(13)
	ah.PayloadLength = uint16(frameLength) - uint16(ah.HeaderLength)
	ah.BufferFullness = uint16(br.Read(11))
	nrRawBlocksMinus1 := br.Read(2)
	if nrRawBlocksMinus1 != 0 {
		return nil, 0, fmt.Errorf("only 1 raw block supported")
	}
	if protectionAbsent != 1 {
		_ = br.Read(16) // CRC
	}

	if br.AccError() != nil {
		return nil, 0, br.AccError()
	}

	return ah, offset, nil
}

// Frequency looks up the sampling frequency for index in ADTSHeader
func (a ADTSHeader) Frequency() uint16 {
	return uint16(FrequencyTable[a.SamplingFrequencyIndex])
}
