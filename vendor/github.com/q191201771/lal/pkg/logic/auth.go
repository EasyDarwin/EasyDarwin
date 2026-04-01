// Copyright 2022, Chef.  All rights reserved.
// https://github.com/q191201771/lal
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

package logic

import (
	"github.com/q191201771/lal/pkg/base"
)

// TODO(chef): [refactor] 将simple_auth.go的内容合并过来，没必要弄两个文件 202209

type IAuthentication interface {
	OnPubStart(info base.PubStartInfo) error
	OnSubStart(info base.SubStartInfo) error
	OnHls(streamName, urlParam string) error
}
