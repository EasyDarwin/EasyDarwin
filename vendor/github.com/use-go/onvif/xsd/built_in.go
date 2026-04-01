package xsd

import (
	"encoding/base64"
	"encoding/hex"
	"errors"
	"fmt"
	"log"
	"net/url"
	"regexp"
	"strings"
	"time"

	"github.com/use-go/onvif/xsd/iso8601"
)

/*
	TODO: XML SOURCE: https://www.w3.org/2001/05/datatypes.xsd
*/

// AnyType alias for string
type AnyType string

// AnySimpleType alias for string
type AnySimpleType string

/***********************************************************
  					Non-derived types
***********************************************************/

/*
	The string datatype represents character strings in XML.
	The ·value space· of string is the set of finite-length sequences of characters.
	String has the following constraining facets:
	• length
	• minLength
	• maxLength
	• pattern
	• enumeration
	• whiteSpace

	More info: https://www.w3.org/TR/xmlschema-2/#string

	//TODO: valid/invalid character declaration and process restrictions
*/
type String string

/*
	Construct an instance of xsd String type
*/
func (tp String) NewString(data string) String {
	return String(data)
}

/*
	Boolean has the ·value space· required to support the mathematical concept of binary-valued logic: {true, false}.
	Boolean has the following ·constraining facets·:
	• pattern
	• whiteSpace

	More info: https://www.w3.org/TR/xmlschema-2/#boolean

	//TODO: process restrictions
*/
type Boolean bool

/*
	Construct an instance of xsd Boolean type
*/
func (tp Boolean) NewBool(data bool) Boolean {
	return Boolean(data)
}

/*
	Float is patterned after the IEEE single-precision 32-bit floating point type
	Float has the following ·constraining facets·:
	• pattern
	• enumeration
	• whiteSpace
	• maxInclusive
	• maxExclusive
	• minInclusive
	• minExclusive

	More info: https://www.w3.org/TR/xmlschema-2/#float

	//TODO: process restrictions
*/
type Float float32

/*
	Construct an instance of xsd Float type
*/
func (tp Float) NewFloat(data float32) Float {
	return Float(data)
}

/*
	The double datatype is patterned after the IEEE double-precision 64-bit floating point type
	Double has the following ·constraining facets·:
	• pattern
	• enumeration
	• whiteSpace
	• maxInclusive
	• maxExclusive
	• minInclusive
	• minExclusive

	More info: https://www.w3.org/TR/xmlschema-2/#double

	//TODO: process restrictions
*/
type Double float64

/*
	Construct an instance of xsd Double type
*/
func (tp Double) NewDouble(data float64) Double {
	return Double(data)
}

/*
	The type decimal represents a decimal number of arbitrary precision.
	Schema processors vary in the number of significant digits they support,
	but a conforming processor must support a minimum of 18 significant digits.
	The format of xsd:decimal is a sequence of digits optionally preceded by a sign ("+" or "-")
	and optionally containing a period. The value may start or end with a period.
	If the fractional part is 0 then the period and trailing zeros may be omitted.
	Leading and trailing zeros are permitted, but they are not considered significant.
	That is, the decimal values 3.0 and 3.0000 are considered equal.

	Source: http://www.datypic.com/sc/xsd/t-xsd_decimal.html

	Decimal has the following ·constraining facets·:
	• totalDigits
	• fractionDigits
	• pattern
	• whiteSpace
	• enumeration
	• maxInclusive
	• maxExclusive
	• minInclusive
	• minExclusive

	More info: https://www.w3.org/TR/xmlschema-2/#decimal

	//TODO: process restrictions, valid/invalid characters(commas are not permitted; the decimal separator must be a period)

*/
type Decimal string

/*
	Construct an instance of xsd Decimal type
*/
func (tp Decimal) NewDecimal(data string) Decimal {
	return Decimal(data)
}

/*
	Duration is the [ISO 8601] extended format PnYn MnDTnH nMnS,
	where nY represents the number of years, nM the number of months,
	nD the number of days, 'T' is the date/time separator,
	nH the number of hours, nM the number of minutes and nS the number of seconds.
	The number of seconds can include decimal digits to arbitrary precision.
	PnYnMnDTnHnMnS

	Duration has the following ·constraining facets·:

	• pattern
	• enumeration
	• whiteSpace
	• maxInclusive
	• maxExclusive
	• minInclusive
	• minExclusive

	More info: https://www.w3.org/TR/xmlschema-2/#duration

	TODO: process restrictions
	TODO: Look at time.Duration go type
*/

//Duration alias for AnySimpleType
type Duration AnySimpleType

/*
	Construct an instance of xsd duration type
*/
func (tp Duration) NewDateTime(years, months, days, hours, minutes, seconds string) Duration {
	i, err := iso8601.NewDuration(
		years,
		months,
		days,
		hours,
		minutes,
		seconds,
	)

	if err != nil {
		log.Fatalln(err)
	}

	//fmt.Println(i.ISO8601Duration())

	return Duration(i.ISO8601Duration())
}

/*
	DateTime values may be viewed as objects with integer-valued year, month, day, hour
	and minute properties, a decimal-valued second property, and a boolean timezoned property.


	The ·lexical space· of dateTime consists of finite-length sequences of characters of the form:
	 '-'? yyyy '-' mm '-' dd 'T' hh ':' mm ':' ss ('.' s+)? (zzzzzz)?,


	DateTime has the following ·constraining facets·:

	• pattern
	• enumeration
	• whiteSpace
	• maxInclusive
	• maxExclusive
	• minInclusive
	• minExclusive

	More info: https://www.w3.org/TR/xmlschema-2/#dateTime

	TODO: decide good type for time with proper format
	TODO: process restrictions
*/
type DateTime AnySimpleType

/*
	Construct an instance of xsd dateTime type
*/
func (tp DateTime) NewDateTime(time time.Time) DateTime {
	return DateTime(time.Format("2002-10-10T12:00:00-05:00"))
}

/*
	Time represents an instant of time that recurs every day.
	The ·value space· of time is the space of time of day values
	as defined in § 5.3 of [ISO 8601]. Specifically, it is a set
	of zero-duration daily time instances.

	Time has the following ·constraining facets·:

	• pattern
	• enumeration
	• whiteSpace
	• maxInclusive
	• maxExclusive
	• minInclusive
	• minExclusive

	More info: https://www.w3.org/TR/xmlschema-2/#time

	TODO: process restrictions
*/
type Time AnySimpleType

/*
	Construct an instance of xsd time type
*/
func (tp DateTime) NewTime(time time.Time) DateTime {
	return DateTime(time.Format("15:04:05"))
}

/*
	The ·value space· of date consists of top-open intervals of
	exactly one day in length on the timelines of dateTime, beginning
	on the beginning moment of each day (in each timezone),
	i.e. '00:00:00', up to but not including '24:00:00'
	(which is identical with '00:00:00' of the next day).
	For nontimezoned values, the top-open intervals disjointly
	cover the nontimezoned timeline, one per day. For timezoned
	values, the intervals begin at every minute and therefore overlap.
*/
type Date AnySimpleType

/*
	Construct an instance of xsd date type
*/
func (tp Date) NewDate(time time.Time) Date {
	return Date(time.Format("2004-04-12-05:00"))
}

/*
	The type xsd:gYearMonth represents a specific month of a specific
	year. The letter g signifies "Gregorian." The format of
	xsd:gYearMonth is CCYY-MM. No left truncation is allowed on
	either part. To represents years later than 9999, additional
	digits can be added to the left of the year value.
	To represent years before 0001, a preceding minus sign ("-")
	is permitted.

	Source: http://www.datypic.com/sc/xsd/t-xsd_gYearMonth.html

	More info: https://www.w3.org/TR/xmlschema-2/#gYearMonth
*/
type GYearMonth AnySimpleType

/*
	Construct an instance of xsd GYearMonth type
*/
func (tp GYearMonth) NewGYearMonth(time time.Time) GYearMonth {
	return GYearMonth(fmt.Sprintf("", time.Year(), "-", time.Month()))
	//return GYearMonth(time.Format("2004-04-05:00"))
}

/*
	The type xsd:gYear represents a specific calendar year.
	The letter g signifies "Gregorian." The format of xsd:gYear
	is CCYY. No left truncation is allowed. To represent years
	later than 9999, additional digits can be added to the left
	of the year value. To represent years before 0001, a preceding
	minus sign ("-") is allowed.

	Source: http://www.datypic.com/sc/xsd/t-xsd_gYear.html

	More info: https://www.w3.org/TR/xmlschema-2/#gYear
*/
type GYear AnySimpleType

/*
	Construct an instance of xsd GYear type
*/
func (tp GYear) NewGYear(time time.Time) GYear {
	return GYear(fmt.Sprintf("", time.Year()))
	//return GYearMonth(time.Format("2004-04-05:00"))
}

/*
	The type xsd:gMonthDay represents a specific day that recurs
	every year. The letter g signifies "Gregorian." xsd:gMonthDay
	can be used to say, for example, that your birthday is on the
	14th of April every year. The format of xsd:gMonthDay is --MM-DD.

	Source: http://www.datypic.com/sc/xsd/t-xsd_gMonthDay.html

	More info: https://www.w3.org/TR/xmlschema-2/#gMonthDay
*/
type GMonthDay AnySimpleType

/*
	Construct an instance of xsd GMonthDay type
*/
func (tp GMonthDay) NewGMonthDay(time time.Time) GMonthDay {
	return GMonthDay(fmt.Sprintf("--", time.Month(), "-", time.Day()))
}

/*
	The type xsd:gDay represents a day that recurs every month.
	The letter g signifies "Gregorian." xsd:gDay can be used to say,
	for example, that checks are paid on the 5th of each month.
	To represent a duration of days, use the duration type instead.
	The format of gDay is ---DD.

	Source: http://www.datypic.com/sc/xsd/t-xsd_gDay.html

	More info: https://www.w3.org/TR/xmlschema-2/#gDay
*/
type GDay AnySimpleType

/*
	Construct an instance of xsd GDay type
*/
func (tp GDay) NewGDay(time time.Time) GDay {
	return GDay(fmt.Sprintf("---", time.Day()))
}

/*
	The type xsd:gMonth represents a specific month that recurs
	every year. The letter g signifies "Gregorian." xsd:gMonth
	can be used to indicate, for example, that fiscal year-end
	processing occurs in September of every year. To represent
	a duration of months, use the duration type instead. The format
	of xsd:gMonth is --MM.

	Source: http://www.datypic.com/sc/xsd/t-xsd_gMonth.html

	More info: https://www.w3.org/TR/xmlschema-2/#gMonth
*/
type GMonth AnySimpleType

func (tp GMonth) NewGMonth(time time.Time) GMonth {
	return GMonth(fmt.Sprintf("--", time.Month()))
}

/*
	The xsd:hexBinary type represents binary data as a sequence
	of binary octets. It uses hexadecimal encoding, where each
	binary octet is a two-character hexadecimal number.
	Lowercase and uppercase letters A through F are permitted.
	For example, 0FB8 and 0fb8 are two equal xsd:hexBinary
	representations consisting of two octets.

	Source: http://www.datypic.com/sc/xsd/t-xsd_hexBinary.html

	More info: https://www.w3.org/TR/xmlschema-2/#hexBinary
*/
type HexBinary AnySimpleType

func (tp HexBinary) NewHexBinary(data []byte) HexBinary {
	return HexBinary(hex.EncodeToString(data))
}

/*
	base64Binary represents Base64-encoded arbitrary binary data.
	The ·value space· of base64Binary is the set of finite-length sequences of binary octets.
	For base64Binary data the entire binary stream is encoded using the Base64 Alphabet in [RFC 2045].

	base64Binary has the following ·constraining facets·:
	• length
	• minLength
	• maxLength
	• pattern
	• enumeration
	• whiteSpace

	More info: https://www.w3.org/TR/xmlschema-2/#base64Binary
*/
type Base64Binary AnySimpleType

func (tp Base64Binary) NewBase64Binary(data []byte) Base64Binary {
	return Base64Binary(base64.StdEncoding.EncodeToString(data))
}

/*
	anyURI represents a Uniform Resource Identifier Reference (URI).
	An anyURI value can be absolute or relative, and may have an optional
	fragment identifier (i.e., it may be a URI Reference).
	This type should be used to specify the intention that the
	value fulfills the role of a URI as defined by [RFC 2396], as amended by [RFC 2732].

	anyURI has the following ·constraining facets·:
	• length
	• minLength
	• maxLength
	• pattern
	• enumeration
	• whiteSpace

	More info: https://www.w3.org/TR/xmlschema-2/#anyURI
*/
type AnyURI AnySimpleType

func (tp AnyURI) NewAnyURI(data url.URL) AnyURI {
	return AnyURI(data.String())
}

/*
	QName represents XML qualified names. The ·value space· of QName is the set of tuples
	{namespace name, local part}, where namespace name is an anyURI and local part is an NCName.
	The ·lexical space· of QName is the set of strings that ·match· the QName production of [Namespaces in XML].

	QName has the following ·constraining facets·:
	• length
	• minLength
	• maxLength
	• pattern
	• enumeration
	• whiteSpace

	More info: https://www.w3.org/TR/xmlschema-2/#QName
*/
type QName AnySimpleType

func (tp QName) NewQName(prefix, local string) QName {
	var result string
	if len(prefix) == 0 {
		result += local
	} else {
		result = prefix + ":" + local
	}
	return QName(result)
}

//TODO: NOTATION datatype
//type NOTATION AnySimpleType

/*
   Derived types
*/

type NormalizedString String

//TODO: check normalization
func (tp NormalizedString) NewNormalizedString(data string) (NormalizedString, error) {
	if strings.ContainsAny(data, "\r\n\t<>&") {
		return NormalizedString(""), errors.New("String " + data + "  contains forbidden symbols")
	}
	return NormalizedString(data), nil
}

type Token NormalizedString

func (tp Token) NewToken(data NormalizedString) (Token, error) {
	trailing_leading_whitespaces := regexp.MustCompile(`^[\s\p{Zs}]+|[\s\p{Zs}]+$`)
	multiple_whitespaces := regexp.MustCompile(`[\s\p{Zs}]{2,}`)
	//Removing trailing and leading whitespaces and multiple spaces
	/*final := re_leadclose_whtsp.ReplaceAllString(data, "")
	final = re_inside_whtsp.ReplaceAllString(final, " ")*/
	if strings.ContainsAny(string(data), "\r\n\t<>&") || trailing_leading_whitespaces.MatchString(string(data)) || multiple_whitespaces.MatchString(string(data)) {
		return Token(""), errors.New("String " + string(data) + "  contains forbidden symbols or whitespaces")
	}

	return Token(data), nil
}

type Language Token

func (tp Language) NewLanguage(data Token) (Language, error) {
	//Pattern was given from https://www.w3.org/2001/05/datatypes.xsd
	rgxp := regexp.MustCompile(`([a-zA-Z]{2}|[iI]-[a-zA-Z]+|[xX]-[a-zA-Z]{1,8})(-[a-zA-Z]{1,8})*`)
	if rgxp.MatchString(string(data)) {
		return Language(""), errors.New("String does not match pattern ([a-zA-Z]{2}|[iI]-[a-zA-Z]+|[xX]-[a-zA-Z]{1,8})(-[a-zA-Z]{1,8})*")
	}
	return Language(data), nil
}

type NMTOKEN Token

//TODO: check for valid symbols: https://www.w3.org/TR/xml/#NT-Nmtoken
func (tp NMTOKEN) NewNMTOKEN(data string) NMTOKEN {
	return NMTOKEN(data)
}

type NMTOKENS []NMTOKEN

func (tp NMTOKENS) NewNMTOKENS(data []NMTOKEN) NMTOKENS {
	result := make(NMTOKENS, len(data))
	for i, j := range data {
		result[i] = j
	}
	return result
}

type Name Token

//TODO: implements https://www.w3.org/TR/xml/#NT-Name
func (tp Name) NewName(data Token) Name {
	return Name(data)
}

type NCName Name

//TODO: https://www.w3.org/TR/REC-xml/#NT-Name and https://www.w3.org/TR/xml-names/#NT-NCName
func (tp NCName) NewNCName(data Name) NCName {
	return NCName(data)
}

//TODO: improve next types to correspond to XMLSchema
type ID NCName

func (tp ID) NewID(data NCName) ID {
	return ID(data)
}

type IDREF NCName

func (tp IDREF) NewIDREF(data NCName) IDREF {
	return IDREF(data)
}

type IDREFS []IDREF

func (tp IDREFS) NewIDREFS(data []IDREF) IDREFS {
	result := make(IDREFS, len(data))
	for i, j := range data {
		result[i] = j
	}
	return result
}

type ENTITY NCName

func (tp ENTITY) NewENTITY(data NCName) ENTITY {
	return ENTITY(data)
}

type ENTITIES []ENTITY

func (tp ENTITIES) NewENTITIES(data []ENTITY) ENTITIES {
	result := make(ENTITIES, len(data))
	for i, j := range data {
		result[i] = j
	}
	return result
}

type Integer int64

func (tp Integer) NewInteger(data int64) Integer {
	return Integer(data)
}

type NonPositiveInteger int64

func (tp NonPositiveInteger) NewNonPositiveInteger(data int64) (NonPositiveInteger, error) {
	if data > 0 {
		return 0, errors.New("Value must be less or equal to 0")
	}
	return NonPositiveInteger(data), nil
}

type NegativeInteger int64

func (tp NegativeInteger) NewNegativeInteger(data int64) (NegativeInteger, error) {
	if data >= 0 {
		return 0, errors.New("Value must be less than 0")
	}
	return NegativeInteger(data), nil
}

type Long int64

func (tp Long) NewLong(data int64) Long {
	return Long(data)
}

type Int int32

func (tp Int) NewInt(data int32) Int {
	return Int(data)
}

type Short int16

func (tp Short) NewShort(data int16) Short {
	return Short(data)
}

type Byte int8

func (tp Byte) NewByte(data int8) Byte {
	return Byte(data)
}

type NonNegativeInteger int64

func (tp NonNegativeInteger) NewNonNegativeInteger(data int64) (NonNegativeInteger, error) {
	if data > 0 {
		return 0, errors.New("Value must be more or equal to 0")
	}
	return NonNegativeInteger(data), nil
}

type UnsignedLong uint64

func (tp UnsignedLong) NewUnsignedLong(data uint64) UnsignedLong {
	return UnsignedLong(data)
}

type UnsignedInt uint32

func (tp UnsignedInt) NewUnsignedInt(data uint32) UnsignedInt {
	return UnsignedInt(data)
}

type UnsignedShort uint16

func (tp UnsignedShort) NewUnsignedShort(data uint16) UnsignedShort {
	return UnsignedShort(data)
}

type UnsignedByte uint8

func (tp UnsignedByte) NewUnsignedByte(data uint8) UnsignedByte {
	return UnsignedByte(data)
}

type PositiveInteger int64

func (tp PositiveInteger) NewPositiveInteger(data int64) (PositiveInteger, error) {
	if data >= 0 {
		return 0, errors.New("Value must be more than 0")
	}
	return PositiveInteger(data), nil
}
