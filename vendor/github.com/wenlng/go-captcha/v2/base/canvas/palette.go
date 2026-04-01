/**
 * @Author Awen
 * @Date 2024/06/01
 * @Email wengaolng@gmail.com
 **/

package canvas

import (
	"image"
	"image/color"
	"math"

	"github.com/golang/freetype"
	"golang.org/x/image/font"
	"golang.org/x/image/math/fixed"
)

// Palette .
type Palette interface {
	image.Image
	Get() *image.Paletted
	CalcMarginBlankArea() *AreaRect
	Rotate(angle int)
	Distort(amplude float64, period float64)
	DrawBeeline(point1 image.Point, point2 image.Point, lineColor color.RGBA)
	DrawCircle(x, y, radius int, c color.RGBA)
	DrawHorizLine(fromX, toX, y int, c color.RGBA)
	AngleSwapPoint(x, y, r, angle float64) (tarX, tarY float64)
	DrawString(params *DrawStringParams, pt fixed.Point26_6) error
}

var _ Palette = (*palette)(nil)

// NewPalette .
func NewPalette(r image.Rectangle, p color.Palette) Palette {
	return &palette{
		image.NewPaletted(r, p),
	}
}

// Palette .
type palette struct {
	*image.Paletted
}

// Get is to get the Palette
func (p *palette) Get() *image.Paletted {
	return p.Paletted
}

// Rotate is to rotation at any Angle
func (p *palette) Rotate(angle int) {
	if angle == 0 {
		return
	}

	tarImg := p
	width := tarImg.Bounds().Max.X
	height := tarImg.Bounds().Max.Y
	r := width / 2
	retImg := image.NewPaletted(image.Rect(0, 0, width, height), tarImg.Palette)
	for x := 0; x <= retImg.Bounds().Max.X; x++ {
		for y := 0; y <= retImg.Bounds().Max.Y; y++ {
			tx, ty := p.AngleSwapPoint(float64(x), float64(y), float64(r), float64(angle))
			retImg.SetColorIndex(x, y, tarImg.ColorIndexAt(int(tx), int(ty)))
		}
	}

	nW := retImg.Bounds().Max.X
	nH := retImg.Bounds().Max.Y
	for x := 0; x < nW; x++ {
		for y := 0; y < nH; y++ {
			p.SetColorIndex(x, y, retImg.ColorIndexAt(x, y))
		}
	}
}

// DrawCircle is drawing circle
func (p *palette) DrawCircle(x, y, radius int, c color.RGBA) {
	f := 1 - radius
	dfx := 1
	dfy := -2 * radius
	xo := 0
	yo := radius

	p.Set(x, y+radius, c)
	p.Set(x, y-radius, c)
	p.DrawHorizLine(x-radius, x+radius, y, c)

	for xo < yo {
		if f >= 0 {
			yo--
			dfy += 2
			f += dfy
		}
		xo++
		dfx += 2
		f += dfx
		p.DrawHorizLine(x-xo, x+xo, y+yo, c)
		p.DrawHorizLine(x-xo, x+xo, y-yo, c)
		p.DrawHorizLine(x-yo, x+yo, y+xo, c)
		p.DrawHorizLine(x-yo, x+yo, y-xo, c)
	}
}

// DrawHorizLine is to draw horiz line
func (p *palette) DrawHorizLine(fromX, toX, y int, c color.RGBA) {
	for x := fromX; x <= toX; x++ {
		p.Set(x, y, c)
	}
}

// Distort is to distort the image
func (p *palette) Distort(amplude float64, period float64) {
	w := p.Bounds().Max.X
	h := p.Bounds().Max.Y
	newP := NewPalette(image.Rect(0, 0, w, h), p.Palette)
	dx := 2.0 * math.Pi / period
	for x := 0; x < w; x++ {
		for y := 0; y < h; y++ {
			xo := amplude * math.Sin(float64(y)*dx)
			yo := amplude * math.Cos(float64(x)*dx)
			newP.Get().SetColorIndex(x, y, p.ColorIndexAt(x+int(xo), y+int(yo)))
		}
	}

	nW := newP.Bounds().Max.X
	nH := newP.Bounds().Max.Y
	for x := 0; x < nW; x++ {
		for y := 0; y < nH; y++ {
			p.SetColorIndex(x, y, newP.Get().ColorIndexAt(x, y))
		}
	}

	newP.Get().Palette = nil
}

// DrawBeeline is to draw beelines
func (p *palette) DrawBeeline(point1 image.Point, point2 image.Point, lineColor color.RGBA) {
	dx := math.Abs(float64(point1.X - point2.X))
	dy := math.Abs(float64(point2.Y - point1.Y))
	sx, sy := 1, 1
	if point1.X >= point2.X {
		sx = -1
	}
	if point1.Y >= point2.Y {
		sy = -1
	}
	err := dx - dy
	for {
		p.Set(point1.X, point1.Y, lineColor)
		p.Set(point1.X+1, point1.Y, lineColor)
		p.Set(point1.X-1, point1.Y, lineColor)
		p.Set(point1.X+2, point1.Y, lineColor)
		p.Set(point1.X-2, point1.Y, lineColor)
		if point1.X == point2.X && point1.Y == point2.Y {
			return
		}
		e2 := err * 2
		if e2 > -dy {
			err -= dy
			point1.X += sx
		}
		if e2 < dx {
			err += dx
			point1.Y += sy
		}
	}
}

// AngleSwapPoint is to the angular conversion point coordinate
func (p *palette) AngleSwapPoint(x, y, r, angle float64) (tarX, tarY float64) {
	x -= r
	y = r - y
	sinVal := math.Sin(angle * (math.Pi / 180))
	cosVal := math.Cos(angle * (math.Pi / 180))
	tarX = x*cosVal + y*sinVal
	tarY = -x*sinVal + y*cosVal
	tarX += r
	tarY = r - tarY
	return
}

// CalcMarginBlankArea is to the calculation of margin space
func (p *palette) CalcMarginBlankArea() *AreaRect {
	nW := p.Bounds().Max.X
	nH := p.Bounds().Max.Y
	minX := nW
	maxX := 0
	minY := nH
	maxY := 0
	for x := 0; x < nW; x++ {
		for y := 0; y < nH; y++ {
			co := p.At(x, y)
			if _, _, _, a := co.RGBA(); a > 0 {
				if x < minX {
					minX = x
				}
				if x > maxX {
					maxX = x
				}

				if y < minY {
					minY = y
				}
				if y > maxY {
					maxY = y
				}
			}
		}
	}

	minX = int(math.Max(0, float64(minX-2)))
	maxX = int(math.Min(float64(nW), float64(maxX+2)))
	minY = int(math.Max(0, float64(minY-2)))
	maxY = int(math.Min(float64(nH), float64(maxY+2)))

	return &AreaRect{
		minX,
		maxX,
		minY,
		maxY,
	}
}

// DrawString is to draw a string
func (p *palette) DrawString(params *DrawStringParams, pt fixed.Point26_6) error {
	dc := freetype.NewContext()
	dc.SetDPI(float64(params.FontDPI))
	dc.SetFont(params.Font)
	dc.SetClip(p.Bounds())
	dc.SetDst(p.Get())

	dc.SetFontSize(float64(params.Size))
	dc.SetHinting(font.HintingFull)

	fontColor := image.NewUniform(params.Color)
	dc.SetSrc(fontColor)

	if _, err := dc.DrawString(params.Text, pt); err != nil {
		return err
	}

	return nil
}
