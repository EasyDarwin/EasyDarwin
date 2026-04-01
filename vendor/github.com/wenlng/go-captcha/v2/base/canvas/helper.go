/**
 * @Author Awen
 * @Date 2024/06/01
 * @Email wengaolng@gmail.com
 **/

package canvas

import (
	"image"
	"math"
)

// RotatePoint is to the point of rotation
func RotatePoint(x, y, sin, cos float64) (float64, float64) {
	return x*cos - y*sin, x*sin + y*cos
}

// RotatedSize is to the size of rotation
func RotatedSize(w, h int, angle float64) (int, int) {
	if w <= 0 || h <= 0 {
		return 0, 0
	}

	sin, cos := math.Sincos(math.Pi * angle / 180)
	x1, y1 := RotatePoint(float64(w-1), 0, sin, cos)
	x2, y2 := RotatePoint(float64(w-1), float64(h-1), sin, cos)
	x3, y3 := RotatePoint(0, float64(h-1), sin, cos)

	minX := math.Min(x1, math.Min(x2, math.Min(x3, 0)))
	maxX := math.Max(x1, math.Max(x2, math.Max(x3, 0)))
	minY := math.Min(y1, math.Min(y2, math.Min(y3, 0)))
	maxY := math.Max(y1, math.Max(y2, math.Max(y3, 0)))

	width := maxX - minX + 1
	if width-math.Floor(width) > 0.1 {
		width++
	}
	height := maxY - minY + 1
	if height-math.Floor(height) > 0.1 {
		height++
	}

	return int(width), int(height)
}

func CalcResizedRect(src image.Rectangle, width int, height int, centerAlign bool) image.Rectangle {
	var dst image.Rectangle
	if width*src.Dy() < height*src.Dx() {
		ratio := float64(width) / float64(src.Dx())

		tH := int(float64(src.Dy()) * ratio)
		pad := 0
		if centerAlign {
			pad = (height - tH) / 2
		}
		dst = image.Rect(0, pad, width, pad+tH)
	} else {
		ratio := float64(height) / float64(src.Dy())
		tW := int(float64(src.Dx()) * ratio)
		pad := 0
		if centerAlign {
			pad = (width - tW) / 2
		}
		dst = image.Rect(pad, 0, pad+tW, height)
	}

	return dst
}
