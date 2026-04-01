package base64Captcha

import "github.com/golang/freetype/truetype"

// FontsStorage interface for working with fonts
type FontsStorage interface {
	// LoadFontByName returns the font from the storage
	LoadFontByName(name string) *truetype.Font

	// LoadFontsByNames returns multiple fonts from storage
	LoadFontsByNames(assetFontNames []string) []*truetype.Font
}
