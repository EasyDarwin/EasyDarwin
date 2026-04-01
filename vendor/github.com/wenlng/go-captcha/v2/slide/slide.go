/**
 * @Author Awen
 * @Date 2024/06/01
 * @Email wengaolng@gmail.com
 **/

package slide

import (
	"errors"
	"image"
	"math"

	"github.com/wenlng/go-captcha/v2/base/helper"
	"github.com/wenlng/go-captcha/v2/base/imagedata"
	"github.com/wenlng/go-captcha/v2/base/logger"
	"github.com/wenlng/go-captcha/v2/base/option"
	"github.com/wenlng/go-captcha/v2/base/randgen"
	"github.com/wenlng/go-captcha/v2/base/random"
)

type Mode int

const (
	ModeBasic Mode = iota
	ModeRegion
)

// Captcha .
type Captcha interface {
	setOptions(opts ...Option)
	setResources(resources ...Resource)
	GetOptions() *Options
	Generate() (CaptchaData, error)
}

var _ Captcha = (*captcha)(nil)

var GraphImageErr = errors.New("graph image is incorrect")
var GenerateDataErr = errors.New("generate data failed")
var ImageTypeErr = errors.New("tile image must be is image.Image type")
var ShadowImageTypeErr = errors.New("tile shadow image must be is image.Image type")
var MaskImageTypeErr = errors.New("tile shadow image must be is image.Image type")
var EmptyBackgroundImageErr = errors.New("no background image")

// captcha .
type captcha struct {
	version   string
	logger    logger.Logger
	drawImage DrawImage
	opts      *Options
	resources *Resources
	mode      Mode
}

// newWithMode .
func newWithMode(mode Mode, opts ...Option) Captcha {
	capt := &captcha{
		logger:    logger.New(),
		drawImage: NewDrawImage(),
		opts:      NewOptions(),
		resources: NewResources(),
		mode:      mode,
	}

	defaultOptions()(capt.opts)
	defaultResource()(capt.resources)

	capt.setOptions(opts...)

	if mode == ModeBasic {
		capt.opts.rangeDeadZoneDirections = []DeadZoneDirectionType{DeadZoneDirectionTypeLeft}
		capt.opts.enableGraphVerticalRandom = false
	}

	return capt
}

// setOptions is to set option
func (c *captcha) setOptions(opts ...Option) {
	for _, opt := range opts {
		opt(c.opts)
	}
}

// setResources is to set resource
func (c *captcha) setResources(resources ...Resource) {
	for _, resource := range resources {
		resource(c.resources)
	}
}

// GetOptions is to get options
func (c *captcha) GetOptions() *Options {
	return c.opts
}

// Generate is to generate the captcha data
func (c *captcha) Generate() (CaptchaData, error) {
	if err := c.check(); err != nil {
		return nil, err
	}

	overlayImage, shadowImage, maskImage := c.genGraph()
	if overlayImage == nil || shadowImage == nil || maskImage == nil {
		return nil, GraphImageErr
	}

	blocks, tilePoint := c.genGraphBlocks(c.opts.imageSize, c.opts.rangeGraphSize, c.opts.genGraphNumber)
	var block *Block
	if len(blocks) > 1 {
		index := helper.RandIndex(len(blocks))
		if index < 0 {
			index = 0
		}
		block = blocks[index]
	} else {
		block = blocks[0]
	}

	if block == nil {
		return nil, GenerateDataErr
	}

	var masterImage, masterBgImage, tileImage image.Image
	var err error

	masterImage, masterBgImage, err = c.genMasterImage(c.opts.imageSize, shadowImage, blocks)
	if err != nil {
		return nil, err
	}

	tileImage, err = c.genTileImage(maskImage, masterBgImage, overlayImage, block)
	if err != nil {
		return nil, err
	}

	if c.mode == ModeBasic {
		block.TileY = block.Y
	} else {
		block.TileY = tilePoint.Y
	}
	block.TileX = tilePoint.X

	return &CaptData{
		block:       block,
		masterImage: imagedata.NewJPEGImageData(masterImage),
		tileImage:   imagedata.NewPNGImageData(tileImage),
	}, nil
}

// generateWithShape is to generate the master image
func (c *captcha) genMasterImage(size *option.Size, shadowImage image.Image, blocks []*Block) (image.Image, image.Image, error) {
	var drawBlocks = make([]*DrawBlock, 0, len(blocks))
	for i := 0; i < len(blocks); i++ {
		block := blocks[i]
		drawBlocks = append(drawBlocks, &DrawBlock{
			X:      block.X,
			Y:      block.Y,
			Width:  block.Width,
			Height: block.Height,
			Angle:  block.Angle,
			Block:  block,
			Image:  shadowImage,
		})
	}

	return c.drawImage.DrawWithNRGBA(&DrawImageParams{
		Width:             size.Width,
		Height:            size.Height,
		Background:        randgen.RandImage(c.resources.rangBackgrounds),
		Alpha:             c.opts.imageAlpha,
		CaptchaDrawBlocks: drawBlocks,
	})
}

// genThumbImage is to generate a tile image
func (c *captcha) genTileImage(maskImage image.Image, bgImage image.Image, overlayImage image.Image, block *Block) (image.Image, error) {
	return c.drawImage.DrawWithTemplate(&DrawTplImageParams{
		Background: bgImage,
		MaskImage:  maskImage,
		Alpha:      c.opts.imageAlpha,
		Width:      block.Width,
		Height:     block.Height,
		CaptchaDrawBlock: &DrawBlock{
			X:      block.X,
			Y:      block.Y,
			Width:  block.Width,
			Height: block.Height,
			Angle:  block.Angle,
			Block:  block,
			Image:  overlayImage,
		},
	})
}

// randDeadZoneDirection is to generate random zone direction
func (c *captcha) randDeadZoneDirection() DeadZoneDirectionType {
	dirs := c.opts.rangeDeadZoneDirections

	index := helper.RandIndex(len(dirs))
	if index < 0 {
		return 0
	}

	res := dirs[index]
	return res
}

// randGraphAngle is to generate random angle
func (c *captcha) randGraphAngle() int {
	angles := c.opts.rangeGraphAnglePos

	index := helper.RandIndex(len(angles))
	if index < 0 {
		return 0
	}

	angle := angles[index]
	res := random.RandInt(angle.Min, angle.Max)

	return res
}

// genGraphBlocks is to generate blocks
func (c *captcha) genGraphBlocks(imageSize *option.Size, size *option.RangeVal, length int) ([]*Block, *option.Point) {
	var blocks = make([]*Block, 0, length)
	width := imageSize.Width
	height := imageSize.Height

	randAngle := c.randGraphAngle()
	randSize := random.RandInt(size.Min, size.Max)
	cHeight := randSize
	cWidth := randSize

	dzdType := c.randDeadZoneDirection()
	dp := cWidth / 2
	blockWidth := (width - cWidth - 20) / length
	y := c.calcYWithDeadZone(5, height-cHeight-5, cHeight, dzdType)

	for i := 0; i < length; i++ {
		var block = &Block{}
		start, end := c.calcXWithDeadZone((i*blockWidth)+dp+5, ((i+1)*blockWidth)-dp, cWidth, dzdType)

		start = int(math.Max(float64(start), float64(dp+5)))
		block.X = random.RandInt(start+20, end+20) - dp

		if c.opts.enableGraphVerticalRandom {
			y = c.calcYWithDeadZone(5, height-cHeight-5, cHeight, dzdType)
		}

		block.Y = y
		block.Width = cWidth
		block.Height = cHeight
		block.Angle = randAngle

		blocks = append(blocks, block)
	}

	point := &option.Point{}
	if c.mode == ModeBasic {
		point.X = random.RandInt(5, dp)
		point.Y = y
		return blocks, point
	}

	if dzdType == DeadZoneDirectionTypeTop {
		point.X = random.RandInt(5, width-cWidth-5)
		point.Y = 5
	} else if dzdType == DeadZoneDirectionTypeBottom {
		point.X = random.RandInt(5, width-cWidth-5)
		point.Y = height - cHeight - 5
	} else if dzdType == DeadZoneDirectionTypeLeft {
		point.X = 5
		point.Y = random.RandInt(5, height-cHeight-5)
	} else if dzdType == DeadZoneDirectionTypeRight {
		point.X = width - cWidth - 5
		point.Y = random.RandInt(5, height-cHeight-5)
	}

	return blocks, point
}

// calcXWithDeadZone .
func (c *captcha) calcXWithDeadZone(start, end, value int, dzdType DeadZoneDirectionType) (int, int) {
	if dzdType == DeadZoneDirectionTypeLeft {
		start += value
		end += value
	}
	return start, end
}

// calcYWithDeadZone .
func (c *captcha) calcYWithDeadZone(start, end, value int, dzdType DeadZoneDirectionType) int {
	if dzdType == DeadZoneDirectionTypeTop {
		start += value
	} else if dzdType == DeadZoneDirectionTypeBottom {
		end -= value
	}
	return random.RandInt(start, end)
}

// genGraph is to generate random graph
func (c *captcha) genGraph() (maskImage, shadowImage, templateImage image.Image) {
	index := helper.RandIndex(len(c.resources.rangGraphImage))
	if index < 0 {
		return nil, nil, nil
	}

	graphImage := c.resources.rangGraphImage[index]

	return graphImage.OverlayImage, graphImage.ShadowImage, graphImage.MaskImage
}

// check is to check the captcha parameter
func (c *captcha) check() error {
	for _, tile := range c.resources.rangGraphImage {
		if tile.OverlayImage == nil {
			return ImageTypeErr
		} else if tile.ShadowImage == nil {
			return ShadowImageTypeErr
		} else if tile.MaskImage == nil {
			return MaskImageTypeErr
		}
	}

	if len(c.resources.rangBackgrounds) == 0 {
		return EmptyBackgroundImageErr
	}

	return nil
}
