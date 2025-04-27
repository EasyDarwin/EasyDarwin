package etime

import (
	"database/sql/driver"
	"fmt"
	"log"
	"time"
)

type DateTime time.Time

const (
	DateYYYYMMDDLayout       = "20060102"
	DateYYYYMMDDHHmmssLayout = "20060102150405"
	DateYYYYMMDDHHLayout     = "2006010215"
	DateLayout               = "2006-01-02"
	DateTimeLayout           = "2006-01-02 15:04:05"
	BuildTimeLayout          = "2006.0102.150405"
	CBuildTimeLayout         = "Jan  _2 2006 15:04:05"
)

var StartTime = time.Now()

func (dt *DateTime) UnmarshalJSON(data []byte) (err error) {
	now, err := time.ParseInLocation(DateTimeLayout, string(data), time.Local)
	*dt = DateTime(now)
	return
}

func (dt DateTime) MarshalJSON() ([]byte, error) {
	b := make([]byte, 0, len(DateTimeLayout)+2)
	b = append(b, '"')
	b = time.Time(dt).AppendFormat(b, DateTimeLayout)
	b = append(b, '"')
	return b, nil
}

func (dt *DateTime) Value() (driver.Value, error) {
	if dt == nil {
		return nil, nil
	}
	var zeroTime time.Time
	time.LoadLocation("Local")
	ti := time.Time(*dt)
	if ti.UnixNano() == zeroTime.UnixNano() {
		return nil, nil
	}
	return ti, nil
}

func (dt *DateTime) Scan(v interface{}) error {
	if value, ok := v.(time.Time); ok {
		*dt = DateTime(value)
		return nil
	}
	return nil
}

func (dt DateTime) String() string {
	return time.Time(dt).Format(DateTimeLayout)
}

func (dt *DateTime) ToString() string {
	if dt == nil {
		return ""
	}
	return time.Time(*dt).Format(DateTimeLayout)
}

func UpTime() time.Duration {
	return time.Since(StartTime)
}
func UpTimeString() string {
	d := UpTime()
	days := d / (time.Hour * 24)
	d -= days * 24 * time.Hour
	hours := d / time.Hour
	d -= hours * time.Hour
	minutes := d / time.Minute
	d -= minutes * time.Minute
	seconds := d / time.Second
	return fmt.Sprintf("%d Days %d Hours %d Mins %d Secs", days, hours, minutes, seconds)
}

// 格式化输出
func FormatTimeString(startTime time.Time, format string) string {
	d := time.Since(startTime)
	days := d / (time.Hour * 24)
	d -= days * 24 * time.Hour
	hours := d / time.Hour
	d -= hours * time.Hour
	minutes := d / time.Minute
	d -= minutes * time.Minute
	seconds := d / time.Second
	return fmt.Sprintf(format, days, hours, minutes, seconds)
}

func TimeNowStr() string {
	return time.Now().Format(DateTimeLayout)
}

func TimeToStr(time time.Time) string {
	if time.IsZero() {
		return ""
	}
	return time.Format(DateTimeLayout)
}

func TimeStrToTime(value string) time.Time {
	tm, err := time.ParseInLocation(DateTimeLayout, value, time.Local)
	if err != nil {
		log.Println(err)
	}
	return tm
}

func (dt *DateTime) ToTime() time.Time {
	if dt == nil {
		return time.Now()
	}
	tm, err := time.ParseInLocation(DateTimeLayout, dt.String(), time.Local)
	if err != nil {
		log.Println(err)
	}
	return tm
}

func TimeYYYYMMDD(t time.Time) string {
	return t.Format(DateYYYYMMDDLayout)
}
func StrYYYYMMDDToTime(value string) time.Time {
	tm, err := time.ParseInLocation(DateYYYYMMDDLayout, value, time.Local)
	if err != nil {
		log.Println(err)
	}
	return tm
}

func TimeYYYYMMDDHHmmss(t time.Time) string {
	return t.Format(DateYYYYMMDDHHmmssLayout)
}
func StrYYYYMMDDHHmmssToTime(value string) time.Time {
	tm, err := time.ParseInLocation(DateYYYYMMDDHHmmssLayout, value, time.Local)
	if err != nil {
		log.Println(err)
	}
	return tm
}
func TimeYYYYMMDDHH(t time.Time) string {
	return t.Format(DateYYYYMMDDHHLayout)
}
func StrYYYYMMDDHHToTime(value string) time.Time {
	tm, err := time.ParseInLocation(DateYYYYMMDDHHLayout, value, time.Local)
	if err != nil {
		log.Println(err)
	}
	return tm
}

func Now() *DateTime {
	n := DateTime(time.Now())
	return &n
}

func ToDateTime(time time.Time) *DateTime {
	n := DateTime(time)
	return &n
}

func StrToDateTime(strtime string) *DateTime {
	if strtime == "" {
		return nil
	}
	n := DateTime(TimeStrToTime(strtime))
	return &n
}

func GetBeforeTime(seconds time.Duration) time.Time {
	s, _ := time.ParseDuration("-1s")
	return time.Now().Add(s * seconds)
}

// 当前时间在 begin 和 end 之前
func TimeIsBetween(begin *DateTime, end *DateTime) bool {
	now := time.Now()

	if begin == nil && end == nil {
		return true
	}

	if begin != nil && end == nil {
		return true
	}

	if begin == nil && end != nil {
		if now.Before(end.ToTime()) {
			return true
		} else {
			return false
		}
	}

	if begin != nil && end != nil {
		if now.After(begin.ToTime()) && now.Before(end.ToTime()) {
			return true
		} else {
			return false
		}
	}

	return false
}
