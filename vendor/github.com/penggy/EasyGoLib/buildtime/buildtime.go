package buildtime

/*
const char* utils_build_time(void)
{
    static const char* psz_build_time = __DATE__ " " __TIME__;
    return psz_build_time;
}
*/
import "C"

import "time"

const (
	CBuildTimeLayout = "Jan  _2 2006 15:04:05"
)

var BuildTime = GetBuildTime()

func GetBuildTime() time.Time {
	t, _ := time.ParseInLocation(CBuildTimeLayout, C.GoString(C.utils_build_time()), time.Local)
	return t
}
