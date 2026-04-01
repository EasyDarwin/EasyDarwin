/**
 * @Author Awen
 * @Date 2024/06/01
 * @Email wengaolng@gmail.com
 **/

package slide

import (
	"image"
)

// GraphImage .
type GraphImage struct {
	OverlayImage image.Image
	ShadowImage  image.Image
	MaskImage    image.Image
}

// Resources .
type Resources struct {
	rangBackgrounds []image.Image
	rangGraphImage  []*GraphImage
}

// NewResources .
func NewResources() *Resources {
	return &Resources{}
}

type Resource func(*Resources)

// WithBackgrounds is to set background image
func WithBackgrounds(images []image.Image) Resource {
	return func(resources *Resources) {
		resources.rangBackgrounds = images
	}
}

// WithGraphImages is to set graphic image
func WithGraphImages(images []*GraphImage) Resource {
	return func(resources *Resources) {
		resources.rangGraphImage = images
	}
}
