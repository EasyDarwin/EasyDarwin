package images

import (
	"image"

	assets_1 "github.com/wenlng/go-captcha-assets/bindata/images/image_1"
	assets_2 "github.com/wenlng/go-captcha-assets/bindata/images/image_2"
	assets_3 "github.com/wenlng/go-captcha-assets/bindata/images/image_3"
	assets_4 "github.com/wenlng/go-captcha-assets/bindata/images/image_4"
	assets_5 "github.com/wenlng/go-captcha-assets/bindata/images/image_5"
	"github.com/wenlng/go-captcha-assets/helper"
)

func GetImages() ([]image.Image, error) {
	var images []image.Image
	var asset []byte
	var img image.Image
	var err error

	//
	asset, err = assets_1.Asset("sourcedata/images/image-1/image.jpg")
	if err != nil {
		return images, err
	}
	img, err = helper.DecodeByteToJpeg(asset)
	if err != nil {
		return images, err
	}
	images = append(images, img)

	//
	asset, err = assets_2.Asset("sourcedata/images/image-2/image.jpg")
	if err != nil {
		return images, err
	}
	img, err = helper.DecodeByteToJpeg(asset)
	if err != nil {
		return images, err
	}
	images = append(images, img)

	//
	asset, err = assets_3.Asset("sourcedata/images/image-3/image.jpg")
	if err != nil {
		return images, err
	}
	img, err = helper.DecodeByteToJpeg(asset)
	if err != nil {
		return images, err
	}
	images = append(images, img)

	//
	asset, err = assets_4.Asset("sourcedata/images/image-4/image.jpg")
	if err != nil {
		return images, err
	}
	img, err = helper.DecodeByteToJpeg(asset)
	if err != nil {
		return images, err
	}
	images = append(images, img)

	//
	asset, err = assets_5.Asset("sourcedata/images/image-5/image.jpg")
	if err != nil {
		return images, err
	}
	img, err = helper.DecodeByteToJpeg(asset)
	if err != nil {
		return images, err
	}
	images = append(images, img)
	
	return images, nil
}
