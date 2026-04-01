package mp4

type IAnyType interface {
	IBox
	SetType(BoxType)
}

type AnyTypeBox struct {
	Box
	Type BoxType
}

func (e *AnyTypeBox) GetType() BoxType {
	return e.Type
}

func (e *AnyTypeBox) SetType(boxType BoxType) {
	e.Type = boxType
}
