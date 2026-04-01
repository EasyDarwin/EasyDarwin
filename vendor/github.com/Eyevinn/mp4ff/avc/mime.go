package avc

import "fmt"

// CodecString - sub-parameter for MIME type "codecs" parameter like avc1.42E00C where avc1 is sampleEntry.
// Defined in ISO/IEC 14496-15 2017.
func CodecString(sampleEntry string, sps *SPS) string {
	return fmt.Sprintf("%s.%02X%02X%02X", sampleEntry, sps.Profile, sps.ProfileCompatibility, sps.Level)
}
