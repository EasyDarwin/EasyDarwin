/**
 * @Author Awen
 * @Date 2024/06/01
 * @Email wengaolng@gmail.com
 **/

package slide

import (
	"github.com/wenlng/go-captcha/v2/base/option"
)

// defaultOptions is to the default configuration
func defaultOptions() Option {
	return func(opts *Options) {
		opts.imageSize = &option.Size{Width: 300, Height: 220}
		opts.imageAlpha = 1
		opts.rangeDeadZoneDirections = []DeadZoneDirectionType{
			DeadZoneDirectionTypeLeft,
			DeadZoneDirectionTypeRight,
			DeadZoneDirectionTypeBottom,
			DeadZoneDirectionTypeTop,
			3,
		}

		opts.genGraphNumber = 1
		opts.rangeGraphAnglePos = []*option.RangeVal{
			{Min: 0, Max: 0},
		}
		opts.rangeGraphSize = &option.RangeVal{Min: 60, Max: 70}
	}
}

// defaultResource is to the default resource
func defaultResource() Resource {
	return func(resources *Resources) {
		// ...
	}
}
