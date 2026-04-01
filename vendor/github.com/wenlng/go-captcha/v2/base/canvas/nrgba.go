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
	"golang.org/x/image/draw"
	"golang.org/x/image/font"
	"golang.org/x/image/math/f64"
	"golang.org/x/image/math/fixed"
)

// NRGBA .
type NRGBA interface {
	image.Image
	Get() *image.NRGBA
	DrawImage(img Palette, dotRect *PositionRect, posRect *AreaRect)
	DrawString(params *DrawStringParams, pt fixed.Point26_6) error
	CalcMarginBlankArea() *AreaRect
	Rotate(angle int, overCrop bool)
	Scale(zoomSize int, keepRatio, centerAlign bool)
	CropCircle(x, y, radius int)
	CropScaleCircle(x, y, radius int, zoomSize int)
	SubImage(r image.Rectangle)
}

var _ NRGBA = (*nRGBA)(nil)

// NewNRGBA .
func NewNRGBA(r image.Rectangle, isAlpha bool) NRGBA {
	nrgba := image.NewNRGBA(r)
	for y := 0; y < r.Max.Y; y++ {
		for x := 0; x < r.Max.X; x++ {
			if isAlpha {
				nrgba.Set(x, y, color.Alpha{A: uint8(0)})
			} else {
				nrgba.Set(x, y, color.RGBA{R: 255, G: 255, B: 255, A: 255})
			}
		}
	}

	return &nRGBA{
		NRGBA: nrgba,
	}
}

// nRGBA .
type nRGBA struct {
	*image.NRGBA
}

// Get is to get the NRGBA
func (n *nRGBA) Get() *image.NRGBA {
	return n.NRGBA
}

// DrawString is to draws a string
func (n *nRGBA) DrawString(params *DrawStringParams, pt fixed.Point26_6) error {
	dc := freetype.NewContext()
	dc.SetDPI(float64(params.FontDPI))
	dc.SetFont(params.Font)
	dc.SetClip(n.Bounds())
	dc.SetDst(n.Get())

	dc.SetFontSize(float64(params.Size))
	dc.SetHinting(font.HintingFull)

	fontColor := image.NewUniform(params.Color)
	dc.SetSrc(fontColor)

	if _, err := dc.DrawString(params.Text, pt); err != nil {
		return err
	}

	return nil
}

// DrawImage is to draws a picture
func (n *nRGBA) DrawImage(img Palette, dotRect *PositionRect, posRect *AreaRect) {
	nW := img.Bounds().Max.X
	nH := img.Bounds().Max.Y

	dX := dotRect.X
	dY := dotRect.Y
	dHeight := dotRect.Height

	pMinX := posRect.MinX
	pMinY := posRect.MinY
	pMaxX := posRect.MaxX
	pMaxY := posRect.MaxY

	for x := 0; x < nW; x++ {
		for y := 0; y < nH; y++ {
			co := img.At(x, y)
			if _, _, _, a := co.RGBA(); a > 0 {
				if x >= pMinX && x <= pMaxX && y >= pMinY && y <= pMaxY {
					n.Set(dX+(x-pMinX), dY-dHeight+(y-pMinY), img.At(x, y))
				}
			}
		}
	}
}

// CalcMarginBlankArea is to the calculation of margin space
func (n *nRGBA) CalcMarginBlankArea() *AreaRect {
	nW := n.Bounds().Max.X
	nH := n.Bounds().Max.Y
	minX := nW
	maxX := 0
	minY := nH
	maxY := 0
	for x := 0; x < nW; x++ {
		for y := 0; y < nH; y++ {
			co := n.At(x, y)
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

// Rotate is to rotation at any Angle
func (n *nRGBA) Rotate(a int, overCrop bool) {
	if a == 0 {
		return
	}

	angle := float64(a) * math.Pi / 180

	sW := n.Get().Bounds().Dx()
	sH := n.Get().Bounds().Dy()
	w, h := RotatedSize(sW, sH, float64(a))
	img := image.NewNRGBA(image.Rect(0, 0, w, h))

	centerX := float64(w) / 2
	centerY := float64(h) / 2

	matrix := Matrix{
		1, 0,
		0, 1,
		0, 0,
	}
	matrix = matrix.Translate(centerX, centerY)
	matrix = matrix.Rotate(angle)
	matrix = matrix.Translate(-centerX, -centerY)

	x := (w - n.Get().Bounds().Dx()) / 2
	y := (h - n.Get().Bounds().Dy()) / 2
	fx, fy := float64(x), float64(y)

	m := matrix.Translate(fx, fy)
	s2d := f64.Aff3{m.XX, m.XY, m.X0, m.YX, m.YY, m.Y0}

	draw.BiLinear.Transform(img, s2d, n.Get(), n.Get().Bounds(), draw.Over, nil)
	n.NRGBA = img

	if overCrop {
		xx := w - sW
		yy := h - sH
		dx := xx / 2
		dy := yy / 2
		n.SubImage(image.Rect(dx, dy, sW+dx, sH+dy))
	}
}

// CropCircle is to cut the circle
func (n *nRGBA) CropCircle(x, y, radius int) {
	bounds := n.Get().Bounds()
	mask := image.NewNRGBA(bounds)
	for py := bounds.Min.Y; py < bounds.Max.Y; py++ {
		for px := bounds.Min.X; px < bounds.Max.X; px++ {
			dist := math.Hypot(float64(px-x), float64(py-y))
			if dist <= float64(radius) {
				mask.Set(px, py, color.White)
			} else {
				mask.Set(px, py, color.Transparent)
			}
		}
	}

	draw.DrawMask(mask, mask.Bounds(), n.Get(), image.Point{X: 0, Y: 0}, mask, image.Point{}, draw.Over)
	n.NRGBA = mask
}

// CropScaleCircle is to scale and crop the circle
func (n *nRGBA) CropScaleCircle(x, y, radius int, zoomSize int) {
	bounds := n.Get().Bounds()
	mask := image.NewNRGBA(bounds)

	for py := bounds.Min.Y; py < bounds.Max.Y; py++ {
		for px := bounds.Min.X; px < bounds.Max.X; px++ {
			dist := math.Hypot(float64(px-x), float64(py-y))
			if dist <= float64(radius) {
				mask.Set(px, py, color.White)
			} else {
				mask.Set(px, py, color.Transparent)
			}
		}
	}

	if zoomSize > 0 {
		subtract := zoomSize * 2
		scaleMask := image.NewNRGBA(image.Rect(0, 0, n.Bounds().Dx()-subtract, n.Bounds().Dy()-subtract))
		draw.BiLinear.Scale(scaleMask, scaleMask.Bounds(), mask, mask.Bounds(), draw.Over, nil)
		mask = scaleMask
	}

	draw.DrawMask(mask, mask.Bounds(), n.Get(), image.Point{X: zoomSize, Y: zoomSize}, mask, image.Point{}, draw.Over)
	n.NRGBA = mask
}

// Scale is to scale the image
func (n *nRGBA) Scale(zoomSize int, keepRatio, centerAlign bool) {
	img := n.NRGBA
	if zoomSize > 0 {
		subtract := zoomSize * 2
		newW := n.Get().Bounds().Dx() - subtract
		newH := n.Get().Bounds().Dy() - subtract
		outImg := image.NewNRGBA(image.Rect(0, 0, newW, newH))

		if !keepRatio {
			draw.BiLinear.Scale(outImg, outImg.Bounds(), n.Get(), n.Get().Bounds(), draw.Over, nil)
		} else {
			dst := CalcResizedRect(n.Get().Bounds(), newW, newH, centerAlign)
			draw.ApproxBiLinear.Scale(outImg, dst.Bounds(), n.Get(), n.Get().Bounds(), draw.Over, nil)
		}

		img = outImg
	}

	n.NRGBA = img
}

// SubImage is to capture the image
func (n *nRGBA) SubImage(r image.Rectangle) {
	n.NRGBA = n.Get().SubImage(r).(*image.NRGBA)
}
