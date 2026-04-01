/**
 * @Author Awen
 * @Date 2024/06/01
 * @Email wengaolng@gmail.com
 **/

package canvas

import (
	"image/color"

	"github.com/golang/freetype/truetype"
)

// AreaRect .
type AreaRect struct {
	MinX, MaxX, MinY, MaxY int
}

// MakeAreaRect .
func MakeAreaRect(x0, y0, x1, y1 int) *AreaRect {
	return &AreaRect{
		MinX: x0,
		MaxX: x1,
		MinY: y0,
		MaxY: y1,
	}
}

// PositionRect .
type PositionRect struct {
	X, Y, Width, Height int
}

// MakePositionRect .
func MakePositionRect(x, y, h, w int) *PositionRect {
	return &PositionRect{
		X:      x,
		Y:      y,
		Height: h,
		Width:  w,
	}
}

// DrawStringParams .
type DrawStringParams struct {
	Color   color.Color
	Size    int
	Width   int
	Height  int
	FontDPI int
	Text    string
	Font    *truetype.Font
}
