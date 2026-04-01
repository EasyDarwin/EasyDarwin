/**
 * @Author Awen
 * @Date 2024/06/01
 * @Email wengaolng@gmail.com
 **/

package slide

import (
	"image"
)

// Block .
type Block struct {
	X      int `json:"x"`
	Y      int `json:"y"`
	Width  int `json:"width"`
	Height int `json:"height"`
	Angle  int `json:"angle"`
	TileX  int `json:"tile_x"`
	TileY  int `json:"tile_y"`
}

// DrawBlock .
type DrawBlock struct {
	Block  *Block
	X      int
	Y      int
	Image  image.Image
	Width  int
	Height int
	Angle  int
}
