// Author: xiexu
// Date: 2022-09-20

package vcode

import (
	"fmt"
	"strings"
	"testing"

	"github.com/stretchr/testify/require"
)

func TestNewVerifyCode(t *testing.T) {
	vc := NewVerifyCode()
	q, a := vc.GenerateIdQuestionAnswer()
	require.NotEmpty(t, q)
	require.NotEmpty(t, a)
	img, err := vc.DrawCaptcha(q)
	require.NoError(t, err)
	require.True(t, strings.HasPrefix(img, "data:image/png;base64,"))
	fmt.Println(q, a)
}
