package mp4

import "time"

const (
	NTPEpochOffset = 2208988800 // NTP epoch is 1900, Unix epoch is 1970
)

// NTP64 is NTP timestamp in RFC 5905 64-bit format: uint32 seconds since 1900-01-01, and uint32 fraction.
type NTP64 uint64

// Seconds returns integral seconds part of NTP64
func (n NTP64) Seconds() uint32 {
	return uint32(n >> 32)
}

// UTCSeconds returns seconds of NTP64 shifted to UNIX epoch.
func (n NTP64) UTCSeconds() uint64 {
	return uint64(n.Seconds()) - NTPEpochOffset
}

// Fraction returns 32-bit fractional part of NTP64.
func (n NTP64) Fraction() uint32 {
	return uint32(n & 0xffffffff)
}

// UTC returns NTP64 as UTC time in seconds.
func (n NTP64) UTC() float64 {
	return float64(n.UTCSeconds()) + float64(n.Fraction())/float64(1<<32)
}

// Time returns NTP64 as time.Time in UTC.
func (n NTP64) Time() time.Time {
	return time.Unix(int64(n.UTCSeconds()), int64(float64(n.Fraction())*1.e9/float64((1<<32)))).UTC()
}

// NewNTP64 creates NTP64 from UTC time in seconds.
func NewNTP64(utcTime float64) NTP64 {
	seconds := uint64(utcTime)
	fraction := uint64((utcTime - float64(seconds)) * float64(1<<32))
	return NTP64((seconds+NTPEpochOffset)<<32 | fraction)
}

// String returns NTP64 as UTC time in string format.
func (n NTP64) String() string {
	return n.Time().String()
}
