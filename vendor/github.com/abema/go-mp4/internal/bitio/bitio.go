package bitio

import "errors"

var (
	ErrInvalidAlignment  = errors.New("invalid alignment")
	ErrDiscouragedReader = errors.New("discouraged reader implementation")
)
