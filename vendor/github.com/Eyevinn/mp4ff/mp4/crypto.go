package mp4

import (
	"crypto/aes"
	"crypto/cipher"
	"encoding/binary"
	"fmt"
	"log"

	"github.com/Eyevinn/mp4ff/avc"
)

type cryptoDir int

const (
	minClearSize           = 96 // to generate same output as Bento4
	naluHdrLen             = 4
	dirEnc       cryptoDir = iota
	dirDec
)

// GetAVCProtectRanges for common encryption from a sample with 4-byt NALU lengths.
// THe spsMap and ppsMap are only needed for CBCS mode.
// For scheme cenc, protection ranges must be a multiple of 16 bytes leaving header and some more in the clear
// For scheme cbcs, protection range must start after the slice header.
func GetAVCProtectRanges(spsMap map[uint32]*avc.SPS, ppsMap map[uint32]*avc.PPS, sample []byte,
	scheme string) ([]SubSamplePattern, error) {
	var ssps []SubSamplePattern
	length := len(sample)
	if length < 4 {
		return nil, fmt.Errorf("less than 4 bytes, No NALUs")
	}
	var pos uint32 = 0
	clearStart := uint32(0)
	clearEnd := uint32(0)
	for pos < uint32(length-4) {
		naluLength := binary.BigEndian.Uint32(sample[pos : pos+4])
		pos += 4
		if int(pos+naluLength) > len(sample) {
			return nil, fmt.Errorf("NALU length fields are bad")
		}
		naluType := avc.GetNaluType(sample[pos])
		var bytesToProtect uint32 = 0
		clearEnd = pos + naluLength
		if naluType < avc.NALU_SEI {
			//VCL NALU
			nalu := sample[pos : pos+naluLength]
			switch scheme {
			case "cenc":
				if naluLength+naluHdrLen >= minClearSize+16 {
					// Calculate a multiple of 16 bytes to protect
					bytesToProtect = (naluLength + naluHdrLen - minClearSize) & 0xfffffff0
					if bytesToProtect > 0 {
						clearEnd -= bytesToProtect
					}
				}
			case "cbcs":
				sh, err := avc.ParseSliceHeader(nalu, spsMap, ppsMap)
				if err != nil {
					return nil, err
				}
				clearHeadSize := uint32(sh.Size)
				clearEnd = pos + clearHeadSize
				bytesToProtect = naluLength - clearHeadSize
			default:
				return nil, fmt.Errorf("unknown protect scheme %s", scheme)
			}
		}
		if bytesToProtect > 0 {
			ssps = appendProtectRange(ssps, clearEnd-clearStart, bytesToProtect)
			clearStart = clearEnd + bytesToProtect
			clearEnd = clearStart
		}

		pos += naluLength
	}
	if clearEnd > clearStart {
		ssps = appendProtectRange(ssps, clearEnd-clearStart, 0)
	}
	return ssps, nil
}

// appendProtectRange appends a SubSamplePattern to a slice of SubSamplePattern, splitting into multiple if needed.
func appendProtectRange(ssps []SubSamplePattern, nrClear, nrProtected uint32) []SubSamplePattern {
	for {
		if nrClear < 65536 {
			break
		}
		ssps = append(ssps, SubSamplePattern{65535, 0})
		nrClear -= 65535
	}
	ssps = append(ssps, SubSamplePattern{uint16(nrClear), nrProtected})
	return ssps
}

func getAudioProtectRanges(sample []byte, scheme string) ([]SubSamplePattern, error) {
	return nil, nil
}

// CryptSampleCenc encrypts/decrypts cenc-schema sample in place provided key, iv, and subSamplePatterns.
func CryptSampleCenc(sample []byte, key []byte, iv []byte, subSamplePatterns []SubSamplePattern) error {
	block, err := aes.NewCipher(key)
	if err != nil {
		return err
	}

	stream := cipher.NewCTR(block, iv)
	if len(subSamplePatterns) != 0 {
		var pos uint32 = 0
		for j := 0; j < len(subSamplePatterns); j++ {
			ss := subSamplePatterns[j]
			nrClear := uint32(ss.BytesOfClearData)
			if nrClear > 0 {
				pos += nrClear
			}
			nrEnc := ss.BytesOfProtectedData
			if nrEnc > 0 {
				stream.XORKeyStream(sample[pos:pos+nrEnc], sample[pos:pos+nrEnc])
				pos += nrEnc
			}
		}
	} else {
		stream.XORKeyStream(sample, sample)
	}
	return nil
}

// DecryptSampleCenc does in-place decryption of cbcs-schema encrypted sample.
// Each protected byte range is striped with with pattern defined by pattern in tenc.
func DecryptSampleCbcs(sample []byte, key []byte, iv []byte, subSamplePatterns []SubSamplePattern, tenc *TencBox) error {
	return cryptSampleCbcs(dirDec, sample, key, iv, subSamplePatterns, tenc)
}

// EncryptSampleCenc does in-place encryption using cbcs schema.
// Each protected byte range is striped with with pattern defined by pattern in tenc.
func EncryptSampleCbcs(sample []byte, key []byte, iv []byte, subSamplePatterns []SubSamplePattern, tenc *TencBox) error {
	return cryptSampleCbcs(dirEnc, sample, key, iv, subSamplePatterns, tenc)
}

// cryptSampleCbcs does either encryption of decryption of a sample using cbcs scheme.
func cryptSampleCbcs(dir cryptoDir, sample []byte, key []byte, iv []byte, subSamplePatterns []SubSamplePattern, tenc *TencBox) error {
	nrInCryptBlock := int(tenc.DefaultCryptByteBlock) * 16
	nrInSkipBlock := int(tenc.DefaultSkipByteBlock) * 16
	var pos uint32 = 0
	if len(subSamplePatterns) != 0 {
		for j := 0; j < len(subSamplePatterns); j++ {
			ss := subSamplePatterns[j]
			nrClear := uint32(ss.BytesOfClearData)
			pos += nrClear
			if ss.BytesOfProtectedData > 0 {
				err := cbcsCrypt(dir, sample[pos:pos+ss.BytesOfProtectedData], key,
					iv, nrInCryptBlock, nrInSkipBlock)
				if err != nil {
					return err
				}
			}
			pos += ss.BytesOfProtectedData
		}
	} else { // Full encryption as used for audio
		err := cbcsCrypt(dir, sample, key, iv, nrInCryptBlock, nrInSkipBlock)
		if err != nil {
			return err
		}
	}
	return nil
}

// cbcsCrypt does one in-place CBC encryption/decryption. Full if nrInSkipBlock == 0.
// The normal case is that nrInCryptBlock == 16 and nrInSkipBlock == 144.
func cbcsCrypt(dir cryptoDir, data []byte, key []byte, iv []byte, nrInCryptBlock, nrInSkipBlock int) error {
	pos := 0
	size := len(data) // This is the bytes that we should stripe decrypt
	aesCbcCrypto, err := aes.NewCipher(key)
	if err != nil {
		return err
	}
	var cph cipher.BlockMode
	switch dir {
	case dirDec:
		cph = cipher.NewCBCDecrypter(aesCbcCrypto, iv)
	case dirEnc:
		cph = cipher.NewCBCEncrypter(aesCbcCrypto, iv)
	default:
		return fmt.Errorf("unknown crypto direction %d", dir)
	}

	if nrInSkipBlock == 0 {
		nrToCrypt := size & ^0xf // Drops 4 last bits -> multiple of 16
		cph.CryptBlocks(data[:nrToCrypt], data[:nrToCrypt])
		return nil
	}
	for {
		if size-pos < nrInCryptBlock { // Leave the rest
			break
		}
		cph.CryptBlocks(data[pos:pos+nrInCryptBlock], data[pos:pos+nrInCryptBlock])
		pos += nrInCryptBlock
		if size-pos < nrInSkipBlock {
			break
		}
		pos += nrInSkipBlock
	}
	return nil
}

// incrementIV increments the IV by the number of encrypted blocks and return a new IV.
func incrementIV(inIV []byte, subsamplePatterns []SubSamplePattern, sampleLen int) []byte {
	nrEncBlocks := 0
	if len(subsamplePatterns) == 0 {
		nrEncBlocks = (sampleLen + 15) / 16
	} else {
		for _, s := range subsamplePatterns {
			nrEncBlocks += int(s.BytesOfProtectedData / 16)
		}
	}
	iv := make([]byte, len(inIV))
	copy(iv, inIV)
	incrementIVInPlace(iv, nrEncBlocks)
	return iv
}

func incrementIVInPlace(iv []byte, nrSteps int) {
	rest := nrSteps
	for i := len(iv) - 1; i >= 0; i-- {
		sum := int(iv[i]) + rest
		if sum < 256 {
			iv[i] = byte(sum)
			break
		}
		iv[i] = byte(sum % 256)
		rest = sum / 256
	}
}

type ProtectionRangeFunc func(sample []byte, scheme string) ([]SubSamplePattern, error)
type InitProtectData struct {
	Tenc     *TencBox
	ProtFunc ProtectionRangeFunc
	Trex     *TrexBox
	Scheme   string
}

// InitProtect modifies the init segment to add protection information and return what is needed to encrypt fragments.
func InitProtect(init *InitSegment, key, iv []byte, scheme string, kid UUID, psshBoxes []*PsshBox) (*InitProtectData, error) {
	ipd := InitProtectData{Scheme: scheme}
	moov := init.Moov
	if len(moov.Traks) != 1 {
		return nil, fmt.Errorf("only one track supported")
	}
	stsd := moov.Trak.Mdia.Minf.Stbl.Stsd
	if len(stsd.Children) != 1 {
		return nil, fmt.Errorf("only one stsd child supported")
	}

	if len(iv) == 8 {
		// Convert to 16 bytes
		iv8 := iv
		iv = make([]byte, 16)
		copy(iv, iv8)
	}

	var err error
	ipd.Trex = moov.Mvex.Trex
	sinf := SinfBox{}
	switch se := stsd.Children[0].(type) {
	case *VisualSampleEntryBox:
		veType := se.Type()
		se.SetType("encv")
		frma := FrmaBox{DataFormat: veType}
		sinf.AddChild(&frma)
		se.AddChild(&sinf)
		switch veType {
		case "avc1", "avc3":
			ipd.ProtFunc, err = getAVCProtFunc(se.AvcC)
			if err != nil {
				return nil, fmt.Errorf("get avc protect func: %w", err)
			}
			switch scheme {
			case "cenc":
				ipd.Tenc = &TencBox{Version: 0, DefaultIsProtected: 1, DefaultPerSampleIVSize: 16, DefaultKID: kid}
			case "cbcs":
				ipd.Tenc = &TencBox{Version: 1, DefaultCryptByteBlock: 1, DefaultSkipByteBlock: 9,
					DefaultIsProtected: 1, DefaultPerSampleIVSize: 0, DefaultKID: kid,
					DefaultConstantIV: iv}
			default:
				return nil, fmt.Errorf("unknown protection mode %s", scheme)
			}

		default:
			return nil, fmt.Errorf("visual sample entry type %s not yet supported", veType)
		}
	case *AudioSampleEntryBox:
		aeType := se.Type()
		se.SetType("enca")
		frma := FrmaBox{DataFormat: aeType}
		sinf.AddChild(&frma)
		se.AddChild(&sinf)
		switch scheme {
		case "cenc":
			ipd.Tenc = &TencBox{Version: 0, DefaultIsProtected: 1, DefaultPerSampleIVSize: 16, DefaultKID: kid}
		case "cbcs":
			ipd.Tenc = &TencBox{Version: 1, DefaultCryptByteBlock: 0, DefaultSkipByteBlock: 0,
				DefaultIsProtected: 1, DefaultPerSampleIVSize: 0, DefaultKID: kid,
				DefaultConstantIV: iv}
		default:
			return nil, fmt.Errorf("unknown protection scheme %s", scheme)
		}
		ipd.ProtFunc = getAudioProtectRanges
	default:
		return nil, fmt.Errorf("sample entry type %s should not be encrypted", se.Type())
	}
	schi := SchiBox{}
	switch scheme {
	case "cenc":
		sinf.AddChild(&SchmBox{SchemeType: "cenc", SchemeVersion: 65536})
	case "cbcs":
		sinf.AddChild(&SchmBox{SchemeType: "cbcs", SchemeVersion: 65536})
	default:
		return nil, fmt.Errorf("unknown protection scheme %s", scheme)
	}
	schi.AddChild(ipd.Tenc)
	sinf.AddChild(&schi)
	for _, pssh := range psshBoxes {
		init.Moov.AddChild(pssh)
	}
	return &ipd, nil
}

func getSPSMap(spss [][]byte) map[uint32]*avc.SPS {
	spsMap := make(map[uint32]*avc.SPS, 1)
	for _, spsNalu := range spss {
		sps, err := avc.ParseSPSNALUnit(spsNalu, false)
		if err != nil {
			log.Fatal(err)
		}
		spsMap[sps.ParameterID] = sps
	}
	return spsMap
}

func getPPSMap(ppss [][]byte, spsMap map[uint32]*avc.SPS) map[uint32]*avc.PPS {
	ppsMap := make(map[uint32]*avc.PPS, 1)
	for _, ppsNalu := range ppss {
		pps, err := avc.ParsePPSNALUnit(ppsNalu, spsMap)
		if err != nil {
			log.Fatal(err)
		}
		ppsMap[pps.PicParameterSetID] = pps
	}
	return ppsMap
}

func getAVCProtFunc(avcC *AvcCBox) (ProtectionRangeFunc, error) {
	spsMap := getSPSMap(avcC.SPSnalus)
	ppsMap := getPPSMap(avcC.PPSnalus, spsMap)
	return func(sample []byte, scheme string) ([]SubSamplePattern, error) {
		return GetAVCProtectRanges(spsMap, ppsMap, sample, scheme)
	}, nil
}

func EncryptFragment(f *Fragment, key, iv []byte, ipd *InitProtectData) error {
	if ipd == nil {
		return fmt.Errorf("no protection data")
	}
	if len(iv) == 8 {
		// Convert to 16 bytes
		iv8 := iv
		iv = make([]byte, 16)
		copy(iv, iv8)
	}
	if len(iv) != 16 {
		return fmt.Errorf("iv must be 16 bytes")
	}
	if len(f.Moof.Trafs) != 1 {
		return fmt.Errorf("only one traf supported")
	}
	traf := f.Moof.Traf
	if len(traf.Truns) != 1 {
		return fmt.Errorf("only one trun supported")
	}
	nrSamples := int(f.Moof.Traf.Trun.SampleCount())
	saiz := NewSaizBox(nrSamples)
	_ = traf.AddChild(saiz)
	saio := NewSaioBox()
	_ = traf.AddChild(saio)
	var senc *SencBox
	switch ipd.Scheme {
	case "cenc":
		senc = NewSencBox(nrSamples, nrSamples)
	case "cbcs":
		senc = NewSencBox(0, nrSamples)
	default:
		return fmt.Errorf("unknown scheme %s", ipd.Scheme)
	}
	_ = traf.AddChild(senc)
	fss, err := f.GetFullSamples(ipd.Trex)
	if err != nil {
		return fmt.Errorf("get full samples: %w", err)
	}

	for _, fs := range fss {
		sample := fs.Data
		subsamplePatterns, err := ipd.ProtFunc(sample, ipd.Scheme)
		if err != nil {
			return fmt.Errorf("get avc protect ranges: %w", err)
		}
		switch ipd.Scheme {
		case "cenc":
			err = CryptSampleCenc(sample, key, iv, subsamplePatterns)
			if err != nil {
				return fmt.Errorf("crypt sample cenc: %w", err)
			}
			// Store IVs in the senc box and update depending on blocks of encrypted data
			_ = senc.AddSample(SencSample{IV: iv, SubSamples: subsamplePatterns})
			saiz.AddSampleInfo(iv, subsamplePatterns)
			iv = incrementIV(iv, subsamplePatterns, len(sample))
		case "cbcs":
			err = EncryptSampleCbcs(sample, key, iv, subsamplePatterns, ipd.Tenc)
			if err != nil {
				return fmt.Errorf("crypt sample cbcs: %w", err)
			}
			// iv is constant and not sent t senc
			_ = senc.AddSample(SencSample{IV: nil, SubSamples: subsamplePatterns})
			saiz.AddSampleInfo(nil, subsamplePatterns)
		default:
			return fmt.Errorf("unknown scheme %s", ipd.Scheme)
		}
	}
	moof := f.Moof
	offset := uint64(8)
	sencDataOffset := uint64(0) // Offset to the senc box data to be set in saio
	for _, c := range moof.Children {
		if c.Type() != "traf" {
			offset += c.Size()
			continue
		}
		traf := c.(*TrafBox)
		offset += 8
		for _, tc := range traf.Children {
			if tc.Type() == "senc" {
				sencDataOffset = offset + 12 + 4 // 12 for full box and 4 for sample count
			}
			offset += tc.Size()
		}
		break
	}
	saio.Offset[0] = int64(sencDataOffset)
	return nil
}

type DecryptInfo struct {
	Psshs      []*PsshBox
	TrackInfos []DecryptTrackInfo
}

type DecryptTrackInfo struct {
	TrackID uint32
	Sinf    *SinfBox
	Trex    *TrexBox
	Psshs   []*PsshBox
}

func (d DecryptInfo) findTrackInfo(trackID uint32) DecryptTrackInfo {
	for _, ti := range d.TrackInfos {
		if ti.TrackID == trackID {
			return ti
		}
	}
	return DecryptTrackInfo{}
}

// DecryptInit modifies init segment in place and returns decryption info and a clean init segment.
func DecryptInit(init *InitSegment) (DecryptInfo, error) {
	moov := init.Moov
	di := DecryptInfo{
		TrackInfos: make([]DecryptTrackInfo, 0, len(moov.Traks)),
	}
	for _, trak := range moov.Traks {
		trackID := trak.Tkhd.TrackID
		stsd := trak.Mdia.Minf.Stbl.Stsd
		var encv *VisualSampleEntryBox
		var enca *AudioSampleEntryBox
		var schemeType string
		var err error

		for _, child := range stsd.Children {
			var sinf *SinfBox
			switch child.Type() {
			case "encv":
				encv = child.(*VisualSampleEntryBox)
				sinf, err = encv.RemoveEncryption()
				if err != nil {
					return di, err
				}
				schemeType = sinf.Schm.SchemeType
			case "enca":
				enca = child.(*AudioSampleEntryBox)
				sinf, err = enca.RemoveEncryption()
				if err != nil {
					return di, err
				}
				schemeType = sinf.Schm.SchemeType
			default:
				continue
			}
			di.TrackInfos = append(di.TrackInfos, DecryptTrackInfo{
				TrackID: trackID,
				Sinf:    sinf,
			})
		}
		if schemeType != "" && schemeType != "cenc" && schemeType != "cbcs" {
			return di, fmt.Errorf("scheme type %s not supported", schemeType)
		}
		if schemeType == "" {
			// Should be track in the clear
			di.TrackInfos = append(di.TrackInfos, DecryptTrackInfo{
				TrackID: trackID,
				Sinf:    nil,
			})
		}
	}

	for _, trex := range moov.Mvex.Trexs {
		for i := range di.TrackInfos {
			if di.TrackInfos[i].TrackID == trex.TrackID {
				di.TrackInfos[i].Trex = trex
				break
			}
		}
	}
	di.Psshs = moov.RemovePsshs()
	return di, nil
}

// DecryptSegment decrypts a media segment in place
func DecryptSegment(seg *MediaSegment, di DecryptInfo, key []byte) error {
	for _, frag := range seg.Fragments {
		err := DecryptFragment(frag, di, key)
		if err != nil {
			return err
		}
	}
	if len(seg.Sidxs) > 0 {
		seg.Sidx = nil // drop sidx inside segment, since not modified properly
		seg.Sidxs = nil
	}
	return nil
}

// DecryptFragment decrypts a fragment in place
func DecryptFragment(frag *Fragment, di DecryptInfo, key []byte) error {
	moof := frag.Moof
	var nrBytesRemoved uint64 = 0
	for _, traf := range moof.Trafs {
		ti := di.findTrackInfo(traf.Tfhd.TrackID)
		if ti.Sinf != nil {
			schemeType := ti.Sinf.Schm.SchemeType
			if schemeType != "cenc" && schemeType != "cbcs" {
				return fmt.Errorf("scheme type %s not supported", schemeType)
			}
			hasSenc, isParsed := traf.ContainsSencBox()
			if !hasSenc {
				return fmt.Errorf("no senc box in traf")
			}
			if !isParsed {
				defaultPerSampleIVSize := ti.Sinf.Schi.Tenc.DefaultPerSampleIVSize
				err := traf.ParseReadSenc(defaultPerSampleIVSize, moof.StartPos)
				if err != nil {
					return fmt.Errorf("parseReadSenc: %w", err)
				}
			}

			tenc := ti.Sinf.Schi.Tenc
			samples, err := frag.GetFullSamples(ti.Trex)
			if err != nil {
				return err
			}
			var senc *SencBox
			if traf.Senc != nil {
				senc = traf.Senc
			} else {
				senc = traf.UUIDSenc.Senc
			}

			err = decryptSamplesInPlace(schemeType, samples, key, tenc, senc)
			if err != nil {
				return err
			}
			nrBytesRemoved += traf.RemoveEncryptionBoxes()
		}
	}
	_, psshBytesRemoved := moof.RemovePsshs()
	nrBytesRemoved += psshBytesRemoved
	for _, traf := range moof.Trafs {
		for _, trun := range traf.Truns {
			trun.DataOffset -= int32(nrBytesRemoved)
		}
	}
	if frag.Mdat.StartPos > frag.Moof.StartPos {
		frag.Mdat.StartPos -= nrBytesRemoved
	}

	return nil
}

// decryptSample - decrypt samples inplace
func decryptSamplesInPlace(schemeType string, samples []FullSample, key []byte, tenc *TencBox, senc *SencBox) error {

	// TODO. Interpret saio and saiz to get to the right place
	// Saio tells where the IV starts relative to moof start
	// It typically ends up inside senc (16 bytes after start)

	iv := make([]byte, 16)
	if tenc.DefaultConstantIV != nil {
		copy(iv, tenc.DefaultConstantIV)
	}

	for i := range samples {
		if len(senc.IVs) == len(samples) {
			if len(senc.IVs[i]) < 16 {
				for i := 0; i < 16; i++ {
					iv[i] = 0
				}
			}
			copy(iv, senc.IVs[i])
		}
		if len(iv) == 0 {
			return fmt.Errorf("iv has length 0")
		}

		var subSamplePatterns []SubSamplePattern
		if len(senc.SubSamples) != 0 {
			subSamplePatterns = senc.SubSamples[i]
		}
		switch schemeType {
		case "cenc":
			err := CryptSampleCenc(samples[i].Data, key, iv, subSamplePatterns)
			if err != nil {
				return err
			}
		case "cbcs":
			err := DecryptSampleCbcs(samples[i].Data, key, iv, subSamplePatterns, tenc)
			if err != nil {
				return err
			}
		}
	}
	return nil
}

// ExtractInitProtectData extracts protection data from init segment
func ExtractInitProtectData(inSeg *InitSegment) (*InitProtectData, error) {
	if len(inSeg.Moov.Traks) != 1 {
		return nil, fmt.Errorf("only one track supported")
	}
	ipd := InitProtectData{}
	ipd.Trex = inSeg.Moov.Mvex.Trex
	stsd := inSeg.Moov.Trak.Mdia.Minf.Stbl.Stsd
	var sinf *SinfBox
	var err error
	for _, c := range stsd.Children {
		switch box := c.(type) {
		case *VisualSampleEntryBox:
			sinf = box.Sinf
			frma := sinf.Frma
			if frma.DataFormat == "avc1" {
				ipd.ProtFunc, err = getAVCProtFunc(box.AvcC)
				if err != nil {
					return nil, fmt.Errorf("get AVC protect func: %w", err)
				}
			} else {
				return nil, fmt.Errorf("unsupported video codec descriptor %s", frma.DataFormat)
			}
		case *AudioSampleEntryBox:
			sinf = box.Sinf
			ipd.ProtFunc = getAudioProtectRanges
		default:
			continue
		}
		for _, c2 := range sinf.Children {
			switch box := c2.(type) {
			case *SchmBox:
				ipd.Scheme = box.SchemeType
			case *SchiBox:
				ipd.Tenc = box.Tenc
			default:
				continue
			}
		}
	}

	return &ipd, nil
}
