// Copyright 2021, Chef.  All rights reserved.
// https://github.com/q191201771/lal
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

package httpflv

import "io"

func ReadAllTagsFromFlvFile(filename string) ([]Tag, error) {
	var tags []Tag

	var ffr FlvFileReader
	defer ffr.Dispose()
	err := ffr.Open(filename)
	if err != nil {
		return nil, err
	}

	for {
		tag, err := ffr.ReadTag()
		if err != nil {
			if err == io.EOF {
				return tags, nil
			} else {
				return tags, err
			}
		}
		tags = append(tags, tag)
	}
	// never reach here
}
