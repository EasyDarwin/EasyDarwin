package fn

import (
	"fmt"
	"slices"
	"testing"
)

func TestReverse(t *testing.T) {
	t.Run("string", func(t *testing.T) {
		s := []string{"a", "b", "c"}
		ns := Reverse(s)
		if slices.Compare(s, ns) == 0 {
			t.Fatal("not reverse")
		}
		if slices.Compare([]string{"c", "b", "a"}, ns) != 0 {
			t.Fatal("not reverse")
		}
	})
}

func TestNotRepeat(t *testing.T) {
	a := []int{1, 2, 3, 3, 4, 5, 3, 4}
	b := DeduplicationFunc(a, func(c int) string {
		return fmt.Sprint(c)
	})
	fmt.Println(b)
}
