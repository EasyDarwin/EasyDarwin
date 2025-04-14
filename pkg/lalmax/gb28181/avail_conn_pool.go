// Copyright 2020, Chef.  All rights reserved.
// https://github.com/q191201771/naza
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)
//根据naza修改，新增tcp

package gb28181

import (
	"errors"
	"net"
	"sync"
)

var ErrNazaNet = errors.New("gb28181: fxxk")

type OnListenWithPort func(port uint16) (net.Listener, error)

// 从指定的端口范围内，寻找可绑定监听的端口，绑定监听并返回
type AvailConnPool struct {
	minPort uint16
	maxPort uint16

	m                sync.Mutex
	lastPort         uint16
	onListenWithPort OnListenWithPort
}

func NewAvailConnPool(minPort uint16, maxPort uint16) *AvailConnPool {
	return &AvailConnPool{
		minPort:  minPort,
		maxPort:  maxPort,
		lastPort: minPort,
	}
}
func (a *AvailConnPool) WithListenWithPort(listenWithPort OnListenWithPort) {
	a.onListenWithPort = listenWithPort
}
func (a *AvailConnPool) Acquire() (net.Listener, uint16, error) {
	a.m.Lock()
	defer a.m.Unlock()

	loopFirstFlag := true
	p := a.lastPort
	for {
		// 找了一轮也没有可用的，返回错误
		if !loopFirstFlag && p == a.lastPort {
			return nil, 0, ErrNazaNet
		}
		loopFirstFlag = false
		if a.onListenWithPort == nil {
			return nil, 0, ErrNazaNet
		}
		listener, err := a.onListenWithPort(p)

		// 绑定失败，尝试下一个端口
		if err != nil {
			p = a.nextPort(p)
			continue
		}

		// 绑定成功，更新last，返回结果
		a.lastPort = a.nextPort(p)
		return listener, p, nil
	}
}

// 通过Acquire获取到可用net.UDPConn对象后，将对象关闭，只返回可用的端口
func (a *AvailConnPool) Peek() (uint16, error) {
	conn, port, err := a.Acquire()
	if err == nil {
		err = conn.Close()
	}
	return port, err
}
func (a *AvailConnPool) ListenWithPort(port uint16) (net.Listener, error) {
	if a.onListenWithPort == nil {
		return nil, ErrNazaNet
	}
	return a.onListenWithPort(port)
}
func (a *AvailConnPool) nextPort(p uint16) uint16 {
	if p == a.maxPort {
		return a.minPort
	}

	return p + 1
}
