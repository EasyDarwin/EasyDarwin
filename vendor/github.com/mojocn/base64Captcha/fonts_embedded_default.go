package base64Captcha

import "embed"

// defaultEmbeddedFontsFS Built-in font storage.
//
//go:embed fonts/*.ttf
//go:embed fonts/*.ttc
var defaultEmbeddedFontsFS embed.FS

var DefaultEmbeddedFonts = NewEmbeddedFontsStorage(defaultEmbeddedFontsFS)
