package main

import "C"
import (
	"strconv"
	"time"
)

var buildTimeAt string
var now = time.Now()

func GetBuildTime() time.Time {
	if buildTimeAt == "" {
		return now
	}
	v, err := strconv.ParseInt(buildTimeAt, 10, 64)
	if err != nil {
		return now
	}
	return time.Unix(v, 0)
}
