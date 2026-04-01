/**
 * @Author Awen
 * @Date 2024/06/01
 * @Email wengaolng@gmail.com
 **/

package slide

import (
	"image"

	"github.com/wenlng/go-captcha/v2/base/canvas"
	"github.com/wenlng/go-captcha/v2/base/randgen"
	"golang.org/x/image/draw"
)

// DrawImageParams .
type DrawImageParams struct {
	Width             int
	Height            int
	Background        image.Image
	Alpha             float32
	CaptchaDrawBlocks []*DrawBlock
}

// DrawTplImageParams .
type DrawTplImageParams struct {
	X                int
	Y                int
	Width            int
	Height           int
	Background       image.Image
	MaskImage        image.Image
	Alpha            float32
	CaptchaDrawBlock *DrawBlock
}

// DrawImage .
type DrawImage interface {
	DrawWithNRGBA(params *DrawImageParams) (img image.Image, bgImg image.Image, err error)
	DrawWithTemplate(params *DrawTplImageParams) (image.Image, error)
}

var _ DrawImage = (*drawImage)(nil)

// NewDrawImage .
func NewDrawImage() DrawImage {
	return &drawImage{}
}

// drawImage .
type drawImage struct {
}

// DrawWithTemplate is to draw with a template
func (d *drawImage) DrawWithTemplate(params *DrawTplImageParams) (image.Image, error) {
	block := params.CaptchaDrawBlock
	bgImage := params.Background
	cvs := canvas.CreateNRGBACanvas(params.Width, params.Height, true)
	bgCvs := canvas.CreateNRGBACanvas(params.Width, params.Height, true)

	tplImage, err := d.drawGraphImage(params.Width, params.Height, params.MaskImage)
	if err != nil {
		return nil, err
	}

	draw.Draw(bgCvs.Get(), bgCvs.Bounds(), bgImage, image.Pt(block.X, block.Y), draw.Src)
	draw.DrawMask(cvs.Get(), tplImage.Bounds(), bgCvs.Get(), image.Point{}, tplImage, image.Point{}, draw.Over)

	maskImage, err := d.drawGraphImage(params.Width, params.Height, block.Image)
	if err != nil {
		return nil, err
	}
	draw.Draw(cvs.Get(), maskImage.Bounds(), maskImage, image.Point{}, draw.Over)

	return cvs, nil
}

// DrawWithNRGBA is to draw with a NRGBA
func (d *drawImage) DrawWithNRGBA(params *DrawImageParams) (img image.Image, bgImg image.Image, err error) {
	blocks := params.CaptchaDrawBlocks
	cvs := canvas.CreateNRGBACanvas(params.Width, params.Height, true)

	for i := 0; i < len(blocks); i++ {
		block := blocks[i]
		var graphImage canvas.NRGBA
		graphImage, err = d.drawGraphImage(block.Width, block.Height, block.Image)
		if err != nil {
			return nil, nil, err
		}

		graphBounds := graphImage.Bounds()
		draw.Draw(cvs.Get(), image.Rect(block.X, block.Y, block.X+graphBounds.Dx(), block.Y+graphBounds.Dy()), graphImage.Get(), image.Point{}, draw.Over)
	}

	var rcm = canvas.CreateNRGBACanvas(params.Width, params.Height, true)
	if params.Background != nil {
		bgImage := params.Background
		b := bgImage.Bounds()
		m := canvas.CreateNRGBACanvas(b.Dx(), b.Dy(), true)
		point := randgen.RangCutImagePos(params.Width, params.Height, bgImage)
		draw.Draw(m.Get(), b, bgImage, point, draw.Src)
		m.SubImage(image.Rect(0, 0, params.Width, params.Height))

		draw.Draw(rcm.Get(), rcm.Bounds(), m.Get(), image.Point{}, draw.Over)
		draw.Draw(m.Get(), cvs.Bounds(), cvs, image.Point{}, draw.Over)
		return m.Get(), rcm, nil
	}

	return cvs, rcm, nil
}

// drawGraphImage is to draw graphics
func (d *drawImage) drawGraphImage(width, height int, img image.Image) (canvas.NRGBA, error) {
	cvs := canvas.CreateNRGBACanvas(width, height, true)
	draw.BiLinear.Scale(cvs.Get(), cvs.Bounds(), img, img.Bounds(), draw.Over, nil)
	return cvs, nil
}
