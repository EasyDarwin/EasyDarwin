/**
 * @Author Awen
 * @Date 2024/06/01
 * @Email wengaolng@gmail.com
 **/

package slide

import "github.com/wenlng/go-captcha/v2/base/imagedata"

// CaptchaData .
type CaptchaData interface {
	GetData() *Block
	GetMasterImage() imagedata.JPEGImageData
	GetTileImage() imagedata.PNGImageData
}

// CaptData .
type CaptData struct {
	block       *Block
	masterImage imagedata.JPEGImageData
	tileImage   imagedata.PNGImageData
}

var _ CaptchaData = (*CaptData)(nil)

// GetData is to get block
func (c CaptData) GetData() *Block {
	return c.block
}

// GetMasterImage is to get master image
func (c CaptData) GetMasterImage() imagedata.JPEGImageData {
	return c.masterImage
}

// GetTileImage is to get tile image
func (c CaptData) GetTileImage() imagedata.PNGImageData {
	return c.tileImage
}
