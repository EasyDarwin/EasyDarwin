package iso8601

import (
	"errors"
	"regexp"
)

//Duration of iso8601
type Duration struct {
	years  string //= number of years
	months string //= number of months
	days   string //= number of days
	// Time section
	hours   string //= the number of hours
	minutes string //= the number of minutes
	seconds string //= the number of seconds
}

//NewDuration return duration
func NewDuration(years, months, days, hours, minutes, seconds string) (*Duration, error) {
	// Pattern for Years, Months, Days, Hours and Minutes components
	pattern1 := "^$|[0-9]+"
	// Pattern for Seconds component
	pattern2 := "^$|[0-9]+(\\.[0-9]+)?"

	matched, err := regexp.MatchString(pattern1, years)
	if err != nil {
		return nil, err
	} else if !matched {
		return nil, errors.New("years value = " + years + " does not match pattern " + pattern1)
	}

	matched, err = regexp.MatchString(pattern1, months)
	if err != nil {
		return nil, err
	} else if !matched {
		return nil, errors.New("months value = " + months + " does not match pattern " + pattern1)
	}

	matched, err = regexp.MatchString(pattern1, days)
	if err != nil {
		return nil, err
	} else if !matched {
		return nil, errors.New("months value = " + days + " does not match pattern " + pattern1)
	}

	matched, err = regexp.MatchString(pattern1, hours)
	if err != nil {
		return nil, err
	} else if !matched {
		return nil, errors.New("months value = " + hours + " does not match pattern " + pattern1)
	}

	matched, err = regexp.MatchString(pattern1, minutes)
	if err != nil {
		return nil, err
	} else if !matched {
		return nil, errors.New("months value = " + minutes + " does not match pattern " + pattern1)
	}

	matched, err = regexp.MatchString(pattern2, seconds)
	if err != nil {
		return nil, err
	} else if !matched {
		return nil, errors.New("years value = " + seconds + " does not match pattern " + pattern2)
	}

	return &Duration{years: years, months: months, hours: hours, days: days, minutes: minutes, seconds: seconds}, nil
}

//ISO8601Duration to string
func (duration Duration) ISO8601Duration() string {
	var result string
	result += "P" // time duration designator
	//years
	if duration.years != "" {
		result += duration.years + "Y"
	}
	if duration.months != "" {
		result += duration.months + "M"
	}
	if duration.days != "" {
		result += duration.days + "D"
	}

	if duration.hours != "" && duration.minutes != "" && duration.seconds != "" {
		result += "T"
		if duration.hours != "" {
			result += duration.hours + "H"
		}
		if duration.minutes != "" {
			result += duration.minutes + "M"
		}
		if duration.seconds != "" {
			result += duration.seconds + "S"
		}
	}

	if len(result) == 1 {
		result += "T0S"
	}

	return result
}
