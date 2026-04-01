/**
 * @Author Awen
 * @Date 2024/06/01
 * @Email wengaolng@gmail.com
 **/

package codec

import (
	"bytes"
	"encoding/base64"
	"image"
	"image/jpeg"
	"image/png"
)

const pngBasePrefix = "data:image/png;base64,"
const jpegBasePrefix = "data:image/jpeg;base64,"

// EncodePNGToByte is to encode the png into a byte array
func EncodePNGToByte(img image.Image) (ret []byte, err error) {
	var buf bytes.Buffer
	if err = png.Encode(&buf, img); err != nil {
		return
	}
	ret = buf.Bytes()
	buf.Reset()
	return
}

// EncodeJPEGToByte is to encode the image into a byte array
func EncodeJPEGToByte(img image.Image, quality int) (ret []byte, err error) {
	var buf bytes.Buffer
	if err = jpeg.Encode(&buf, img, &jpeg.Options{Quality: quality}); err != nil {
		return
	}
	ret = buf.Bytes()
	buf.Reset()
	return
}

// DecodeByteToJpeg is to decode the byte array into an image
func DecodeByteToJpeg(b []byte) (img image.Image, err error) {
	var buf bytes.Buffer
	buf.Write(b)
	img, err = jpeg.Decode(&buf)
	buf.Reset()
	return
}

// DecodeByteToPng is to decode the byte array into a png
func DecodeByteToPng(b []byte) (img image.Image, err error) {
	var buf bytes.Buffer
	buf.Write(b)
	img, err = png.Decode(&buf)
	buf.Reset()
	return
}

// EncodePNGToBase64 is to encode the png into string
func EncodePNGToBase64(img image.Image) (string, error) {
	base64Str, err := EncodePNGToBase64Data(img)
	if err != nil {
		return "", err
	}

	return pngBasePrefix + base64Str, nil
}

// EncodeJPEGToBase64 is to encode the image into string
func EncodeJPEGToBase64(img image.Image, quality int) (string, error) {
	base64Str, err := EncodeJPEGToBase64Data(img, quality)
	if err != nil {
		return "", err
	}

	return jpegBasePrefix + base64Str, nil
}

// EncodePNGToBase64Data is to encode the png into string
func EncodePNGToBase64Data(img image.Image) (string, error) {
	byteCode, err := EncodePNGToByte(img)
	if err != nil {
		return "", err
	}
	return base64.StdEncoding.EncodeToString(byteCode), nil
}

// EncodeJPEGToBase64Data is to encode the image into string
func EncodeJPEGToBase64Data(img image.Image, quality int) (string, error) {
	byteCode, err := EncodeJPEGToByte(img, quality)
	if err != nil {
		return "", err
	}

	return base64.StdEncoding.EncodeToString(byteCode), nil
}
