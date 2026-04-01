// Package circular implements "circular numbers". This is a number that can be
// increased (or decreased) indefinitely while only using up a limited amount of
// memory. This feature comes with the limitiation in how distant two such
// numbers can be. Circular numbers have a maximum. The maximum distance is
// half the maximum value. If a number that has the maximum value is
// increased by 1, it becomes 0. If a number that has the value of 0 is
// decreased by 1, it becomes the maximum value. By comparing two circular
// numbers it is not possible to tell how often they wrapped. Therefore these
// two numbers must come from the same domain in order to make sense of the
// camparison.
package circular

// Number represents a "circular number". A Number is immutable. All modification
// to a Number will result in a new instance of a Number.
type Number struct {
	max       uint32
	threshold uint32
	value     uint32
}

// New returns a new circular number with the value of x and the maximum of max.
func New(x, max uint32) Number {
	c := Number{
		value:     0,
		max:       max,
		threshold: max / 2,
	}

	if x > max {
		return c.Add(x)
	}

	c.value = x

	return c
}

// Val returns the current value of the number.
func (a Number) Val() uint32 {
	return a.value
}

// Equals returns whether two circular numbers have the same value.
func (a Number) Equals(b Number) bool {
	return a.value == b.value
}

// Distance returns the distance of two circular numbers.
func (a Number) Distance(b Number) uint32 {
	if a.Equals(b) {
		return 0
	}

	d := uint32(0)

	if a.value > b.value {
		d = a.value - b.value
	} else {
		d = b.value - a.value
	}

	if d >= a.threshold {
		d = a.max - d + 1
	}

	return d
}

// Lt returns whether the circular number is lower than the circular number b.
func (a Number) Lt(b Number) bool {
	if a.Equals(b) {
		return false
	}

	d := uint32(0)
	altb := false

	if a.value > b.value {
		d = a.value - b.value
	} else {
		d = b.value - a.value
		altb = true
	}

	if d < a.threshold {
		return altb
	}

	return !altb
}

// Lte returns whether the circular number is lower than or equal to the circular number b.
func (a Number) Lte(b Number) bool {
	if a.Equals(b) {
		return true
	}

	return a.Lt(b)
}

// Gt returns whether the circular number is greather than the circular number b.
func (a Number) Gt(b Number) bool {
	if a.Equals(b) {
		return false
	}

	d := uint32(0)
	agtb := false

	if a.value > b.value {
		d = a.value - b.value
		agtb = true
	} else {
		d = b.value - a.value
	}

	if d < a.threshold {
		return agtb
	}

	return !agtb
}

// Gte returns whether the circular number is greather than or equal to the circular number b.
func (a Number) Gte(b Number) bool {
	if a.Equals(b) {
		return true
	}

	return a.Gt(b)
}

// Inc returns a new circular number with a value that is increased by 1.
func (a Number) Inc() Number {
	b := a

	if b.value == b.max {
		b.value = 0
	} else {
		b.value++
	}

	return b
}

// Add returns a new circular number with a value that is increased by b.
func (a Number) Add(b uint32) Number {
	c := a
	x := c.max - c.value

	if b <= x {
		c.value += b
	} else {
		c.value = b - x - 1
	}

	return c
}

// Dec returns a new circular number with a value that is decreased by 1.
func (a Number) Dec() Number {
	b := a

	if b.value == 0 {
		b.value = b.max
	} else {
		b.value--
	}

	return b
}

// Sub returns a new circular number with a value that is decreased by b.
func (a Number) Sub(b uint32) Number {
	c := a

	if b <= c.value {
		c.value -= b
	} else {
		c.value = c.max - (b - c.value) + 1
	}

	return c
}
