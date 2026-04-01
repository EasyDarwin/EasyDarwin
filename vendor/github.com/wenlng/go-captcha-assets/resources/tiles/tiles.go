package tiles

import (
	"image"

	assets_1 "github.com/wenlng/go-captcha-assets/bindata/tiles/tile_1"
	assets_2 "github.com/wenlng/go-captcha-assets/bindata/tiles/tile_2"
	assets_3 "github.com/wenlng/go-captcha-assets/bindata/tiles/tile_3"
	assets_4 "github.com/wenlng/go-captcha-assets/bindata/tiles/tile_4"
	"github.com/wenlng/go-captcha-assets/helper"
)

type GraphImage struct {
	OverlayImage image.Image
	ShadowImage  image.Image
	MaskImage    image.Image
}

func GetTiles() ([]*GraphImage, error) {
	var graphs []*GraphImage
	var err error
	var tileImg image.Image
	var tileShadowImg image.Image
	var tileMaskImg image.Image
	var asset []byte

	// .....
	asset, err = assets_1.Asset("sourcedata/tiles/tile-1/tile.png")
	if err != nil {
		return graphs, err
	}
	tileImg, err = helper.DecodeByteToPng(asset)
	if err != nil {
		return graphs, err
	}

	asset, err = assets_1.Asset("sourcedata/tiles/tile-1/tile-shadow.png")
	if err != nil {
		return graphs, err
	}
	tileShadowImg, err = helper.DecodeByteToPng(asset)
	if err != nil {
		return graphs, err
	}

	asset, err = assets_1.Asset("sourcedata/tiles/tile-1/tile-mask.png")
	if err != nil {
		return graphs, err
	}
	tileMaskImg, err = helper.DecodeByteToPng(asset)
	if err != nil {
		return graphs, err
	}

	graphs = append(graphs, &GraphImage{
		OverlayImage: tileImg,
		ShadowImage:  tileShadowImg,
		MaskImage:    tileMaskImg,
	})

	// .....
	asset, err = assets_2.Asset("sourcedata/tiles/tile-2/tile.png")
	if err != nil {
		return graphs, err
	}
	tileImg, err = helper.DecodeByteToPng(asset)
	if err != nil {
		return graphs, err
	}

	asset, err = assets_2.Asset("sourcedata/tiles/tile-2/tile-shadow.png")
	if err != nil {
		return graphs, err
	}
	tileShadowImg, err = helper.DecodeByteToPng(asset)
	if err != nil {
		return graphs, err
	}

	asset, err = assets_2.Asset("sourcedata/tiles/tile-2/tile-mask.png")
	if err != nil {
		return graphs, err
	}
	tileMaskImg, err = helper.DecodeByteToPng(asset)
	if err != nil {
		return graphs, err
	}

	graphs = append(graphs, &GraphImage{
		OverlayImage: tileImg,
		ShadowImage:  tileShadowImg,
		MaskImage:    tileMaskImg,
	})

	// .....
	asset, err = assets_3.Asset("sourcedata/tiles/tile-3/tile.png")
	if err != nil {
		return graphs, err
	}
	tileImg, err = helper.DecodeByteToPng(asset)
	if err != nil {
		return graphs, err
	}

	asset, err = assets_3.Asset("sourcedata/tiles/tile-3/tile-shadow.png")
	if err != nil {
		return graphs, err
	}
	tileShadowImg, err = helper.DecodeByteToPng(asset)
	if err != nil {
		return graphs, err
	}

	asset, err = assets_3.Asset("sourcedata/tiles/tile-3/tile-mask.png")
	if err != nil {
		return graphs, err
	}
	tileMaskImg, err = helper.DecodeByteToPng(asset)
	if err != nil {
		return graphs, err
	}

	// .....
	asset, err = assets_4.Asset("sourcedata/tiles/tile-4/tile.png")
	if err != nil {
		return graphs, err
	}
	tileImg, err = helper.DecodeByteToPng(asset)
	if err != nil {
		return graphs, err
	}

	asset, err = assets_4.Asset("sourcedata/tiles/tile-4/tile-shadow.png")
	if err != nil {
		return graphs, err
	}
	tileShadowImg, err = helper.DecodeByteToPng(asset)
	if err != nil {
		return graphs, err
	}

	asset, err = assets_4.Asset("sourcedata/tiles/tile-4/tile-mask.png")
	if err != nil {
		return graphs, err
	}
	tileMaskImg, err = helper.DecodeByteToPng(asset)
	if err != nil {
		return graphs, err
	}

	graphs = append(graphs, &GraphImage{
		OverlayImage: tileImg,
		ShadowImage:  tileShadowImg,
		MaskImage:    tileMaskImg,
	})

	graphs = append(graphs, &GraphImage{
		OverlayImage: tileImg,
		ShadowImage:  tileShadowImg,
		MaskImage:    tileMaskImg,
	})

	return graphs, nil
}
