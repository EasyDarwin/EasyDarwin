/**
 * @Author Awen
 * @Date 2024/06/01
 * @Email wengaolng@gmail.com
 **/

package imagedata

import (
	"image"

	"github.com/wenlng/go-captcha/v2/base/codec"
	"github.com/wenlng/go-captcha/v2/base/option"
)

// PNGImageData .
type PNGImageData interface {
	Get() image.Image
	ToBytes() ([]byte, error)
	ToBase64() (string, error)
	ToBase64Data() (string, error)
	SaveToFile(filepath string) error
}

var _ PNGImageData = (*pngImageDta)(nil)

// pngImageDta .
type pngImageDta struct {
	image image.Image
}

// NewPNGImageData .
func NewPNGImageData(img image.Image) PNGImageData {
	return &pngImageDta{
		image: img,
	}
}

// Get is to get the original picture
func (c *pngImageDta) Get() image.Image {
	return c.image
}

// SaveToFile is to save PNG as a file
func (c *pngImageDta) SaveToFile(filepath string) error {
	if c.image == nil {
		return ImageMissingDataErr
	}

	return saveToFile(c.image, filepath, true, option.QualityNone)
}

// ToBytes is to convert PNG into byte array
func (c *pngImageDta) ToBytes() ([]byte, error) {
	if c.image == nil {
		return []byte{}, ImageEmptyErr
	}
	return codec.EncodePNGToByte(c.image)
}

// ToBase64Data is to convert PNG into base64
func (c *pngImageDta) ToBase64Data() (string, error) {
	if c.image == nil {
		return "", ImageEmptyErr
	}
	return codec.EncodePNGToBase64Data(c.image)
}

// ToBase64 is to convert PNG into base64
func (c *pngImageDta) ToBase64() (string, error) {
	if c.image == nil {
		return "", ImageEmptyErr
	}
	return codec.EncodePNGToBase64(c.image)
}
