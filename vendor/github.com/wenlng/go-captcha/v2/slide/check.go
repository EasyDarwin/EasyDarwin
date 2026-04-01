/**
 * @Author Awen
 * @Date 2024/06/01
 * @Email wengaolng@gmail.com
 **/

package slide

// CheckPoint is to the position of the detection point
func CheckPoint(sx, sy, dx, dy, padding int64) bool {
	newX := padding * 2
	newY := padding * 2
	newDx := dx - padding
	newDy := dy - padding

	return sx >= newDx &&
		sx <= newDx+newX &&
		sy >= newDy &&
		sy <= newDy+newY
}
