package primitives

import (
	"fmt"
	"strings"
)

// AttributesUnmarshal decodes attributes.
func AttributesUnmarshal(v string) (map[string]string, error) {
	ret := make(map[string]string)

	for {
		if len(v) == 0 {
			break
		}

		// read key
		i := strings.IndexByte(v, '=')
		if i < 0 {
			return nil, fmt.Errorf("key not found")
		}
		var key string
		key, v = v[:i], v[i+1:]
		key = strings.TrimLeft(key, " ")

		// read value
		var val string
		if len(v) != 0 && v[0] == '"' {
			v = v[1:]
			i = strings.IndexByte(v, '"')
			if i < 0 {
				return nil, fmt.Errorf("value end delimiter not found")
			}
			val, v = v[:i], v[i+1:]
			ret[key] = val

			if len(v) != 0 {
				if v[0] != ',' {
					return nil, fmt.Errorf("delimiter not found")
				}
				v = v[1:]
			}
		} else {
			i = strings.IndexByte(v, ',')
			if i >= 0 {
				val, v = v[:i], v[i+1:]
				ret[key] = val
			} else {
				val = v
				ret[key] = val
				break
			}
		}
	}

	return ret, nil
}
