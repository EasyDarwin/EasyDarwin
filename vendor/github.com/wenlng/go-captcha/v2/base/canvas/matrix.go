/**
 * @Author Awen
 * @Date 2024/06/01
 * @Email wengaolng@gmail.com
 **/

package canvas

import "math"

// Matrix .
type Matrix struct {
	XX, YX, XY, YY, X0, Y0 float64
}

// Translate is to the matrix calculation of translate
func (a Matrix) Translate(x, y float64) Matrix {
	return Matrix{
		1, 0,
		0, 1,
		x, y,
	}.Multiply(a)
}

// Multiply is to the matrix calculation of multiply
func (a Matrix) Multiply(b Matrix) Matrix {
	return Matrix{
		a.XX*b.XX + a.YX*b.XY,
		a.XX*b.YX + a.YX*b.YY,
		a.XY*b.XX + a.YY*b.XY,
		a.XY*b.YX + a.YY*b.YY,
		a.X0*b.XX + a.Y0*b.XY + b.X0,
		a.X0*b.YX + a.Y0*b.YY + b.Y0,
	}
}

// Rotate is to the matrix calculation of rotation
func (a Matrix) Rotate(angle float64) Matrix {
	c := math.Cos(angle)
	s := math.Sin(angle)
	return Matrix{
		c, s,
		-s, c,
		0, 0,
	}.Multiply(a)
}
