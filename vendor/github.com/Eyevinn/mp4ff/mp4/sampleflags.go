package mp4

import "fmt"

// SampleFlags according to 14496-12 Sec. 8.8.3.1
type SampleFlags struct {
	IsLeading                 byte
	SampleDependsOn           byte
	SampleIsDependedOn        byte
	SampleHasRedundancy       byte
	SamplePaddingValue        byte
	SampleIsNonSync           bool
	SampleDegradationPriority uint16
}

func (sf SampleFlags) String() string {
	return fmt.Sprintf("isLeading=%d dependsOn=%d isDependedOn=%d hasRedundancy=%d padding=%d isNonSync=%t degradationPriority=%d",
		sf.IsLeading, sf.SampleDependsOn, sf.SampleIsDependedOn, sf.SampleHasRedundancy, sf.SamplePaddingValue,
		sf.SampleIsNonSync, sf.SampleDegradationPriority)
}

// Encode - convert sampleflags to uint32 bit pattern
func (sf SampleFlags) Encode() uint32 {
	sfBin := uint32(sf.IsLeading)<<26 | uint32(sf.SampleDependsOn)<<24 | uint32(sf.SampleIsDependedOn)<<22
	sfBin |= uint32(sf.SampleHasRedundancy)<<20 | uint32(sf.SamplePaddingValue)<<17
	if sf.SampleIsNonSync {
		sfBin |= 1 << 16
	}
	sfBin |= uint32(sf.SampleDegradationPriority)
	return sfBin
}

// SyncSampleFlags - flags for I-frame or other sync sample
const SyncSampleFlags uint32 = 0x02000000

// NonSyncSampleFlags - flags for non-sync sample
const NonSyncSampleFlags uint32 = 0x00010000

// IsSyncSampleFlags - flags is set correctly for sync sample
func IsSyncSampleFlags(flags uint32) bool {
	return flags&SyncSampleFlags == SyncSampleFlags
}

// SetSyncSampleFlags - return flags with syncsample pattern
func SetSyncSampleFlags(flags uint32) uint32 {
	return flags & SyncSampleFlags & ^NonSyncSampleFlags
}

// SetNonSyncSampleFlags - return flags with nonsyncsample pattern
func SetNonSyncSampleFlags(flags uint32) uint32 {
	return flags & ^SyncSampleFlags & NonSyncSampleFlags
}

// DecodeSampleFlags - decode a uint32 flags field
func DecodeSampleFlags(u uint32) SampleFlags {
	sf := SampleFlags{
		IsLeading:                 byte((u >> 26) & 0x3),
		SampleDependsOn:           byte((u >> 24) & 0x3),
		SampleIsDependedOn:        byte((u >> 22) & 0x3),
		SampleHasRedundancy:       byte((u >> 20) & 0x3),
		SamplePaddingValue:        byte((u >> 17) & 0x7),
		SampleIsNonSync:           (u>>16)&0x1 == 1,
		SampleDegradationPriority: uint16(u & 0xffff),
	}
	return sf
}
