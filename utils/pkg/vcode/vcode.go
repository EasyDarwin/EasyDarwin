// Author: xiexu
// Date: 2022-09-20

package vcode

import (
	"image/color"
	"log"

	"github.com/mojocn/base64Captcha"
	"github.com/wenlng/go-captcha-assets/resources/images"
	"github.com/wenlng/go-captcha-assets/resources/tiles"
	"github.com/wenlng/go-captcha/v2/slide"
)

type VerifyCode struct {
	d *base64Captcha.DriverMath
}

func NewVerifyCode() *VerifyCode {
	return &VerifyCode{
		d: base64Captcha.NewDriverMath(
			42, 100, 0, 0, &color.RGBA{
				R: 255,
				G: 255,
				B: 255,
				A: 255,
			}, nil, []string{"chromohv.ttf"},
		),
	}
}

// GenerateIdQuestionAnswer 生成问题和答案
func (v *VerifyCode) GenerateIdQuestionAnswer() (string, string) {
	_, q, a := v.d.GenerateIdQuestionAnswer()
	return q, a
}

// DrawCaptcha 将答案绘制成图片
func (v *VerifyCode) DrawCaptcha(q string) (string, error) {
	item, err := v.d.DrawCaptcha(q)
	if err != nil {
		return "", err
	}
	return item.EncodeB64string(), nil
}

type SlideCaptcha struct {
	d slide.Captcha
}

func NewSlideCaptcha() (*SlideCaptcha, error) {
	builder := slide.NewBuilder(
		slide.WithGenGraphNumber(1), // 几个填充框?
		slide.WithEnableGraphVerticalRandom(true),
	)
	imgs, err := images.GetImages()
	if err != nil {
		return nil, err
	}

	graphs, err := tiles.GetTiles()
	if err != nil {
		log.Fatalln(err)
	}

	newGraphs := make([]*slide.GraphImage, 0, len(graphs))
	for i := 0; i < len(graphs); i++ {
		graph := graphs[i]
		newGraphs = append(newGraphs, &slide.GraphImage{
			OverlayImage: graph.OverlayImage,
			MaskImage:    graph.MaskImage,
			ShadowImage:  graph.ShadowImage,
		})
	}

	// set resources
	builder.SetResources(
		slide.WithGraphImages(newGraphs),
		slide.WithBackgrounds(imgs),
	)

	return &SlideCaptcha{
		d: builder.MakeWithRegion(),
	}, nil
}

// GenerateIdQuestionAnswer 生成问题和答案
func (s *SlideCaptcha) GenerateIdQuestionAnswer() (*slide.Block, string, string, error) {
	capdata, err := s.d.Generate()
	if err != nil {
		return nil, "", "", err
	}
	data := capdata.GetData()
	m, err := capdata.GetMasterImage().ToBase64WithQuality(95)
	if err != nil {
		return nil, "", "", err
	}
	t, err := capdata.GetTileImage().ToBase64()
	return data, m, t, err
}
