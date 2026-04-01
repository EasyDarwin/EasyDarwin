/**
 * @Author Awen
 * @Date 2024/06/01
 * @Email wengaolng@gmail.com
 **/

package imagedata

import (
	"errors"
	"image"
	"image/jpeg"
	"image/png"
	"os"
	"path"
)

var ImageEmptyErr = errors.New("image is empty")
var ImageMissingDataErr = errors.New("missing image data")

// saveToFile .
func saveToFile(img image.Image, filepath string, isTransparent bool, quality int) error {
	var file *os.File
	var err error

	err = os.MkdirAll(path.Dir(filepath), os.ModePerm)
	if err != nil {
		return err
	}

	if _, err = os.Stat(filepath); os.IsNotExist(err) {
		file, err = os.Create(filepath)
	} else {
		file, err = os.OpenFile(filepath, os.O_RDWR, 0666)
	}
	if err != nil {
		return err
	}
	defer file.Close()

	if isTransparent {
		err = png.Encode(file, img)
	} else {
		err = jpeg.Encode(file, img, &jpeg.Options{Quality: quality})
	}

	return err
}
