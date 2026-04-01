// Copyright 2020, Chef.  All rights reserved.
// https://github.com/q191201771/naza
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

package nazanet

import (
	"net"
	"sync"
)

// 从指定的UDP端口范围内，寻找可绑定监听的端口，绑定监听并返回
// Pool只提供Acquire获取接口，不提供释放接口，连接资源是标准*net.UDPConn对象，需要释放时，外部直接Close即可
//
type AvailUdpConnPool struct {
	minPort uint16
	maxPort uint16

	m        sync.Mutex
	lastPort uint16
}

func NewAvailUdpConnPool(minPort uint16, maxPort uint16) *AvailUdpConnPool {
	return &AvailUdpConnPool{
		minPort:  minPort,
		maxPort:  maxPort,
		lastPort: minPort,
	}
}

func (a *AvailUdpConnPool) Acquire() (*net.UDPConn, uint16, error) {
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

		conn, err := listenUdpWithPort(p)

		// 绑定失败，尝试下一个端口
		if err != nil {
			p = a.nextPort(p)
			continue
		}

		// 绑定成功，更新last，返回结果
		a.lastPort = a.nextPort(p)
		return conn, p, nil
	}
}

// 有的业务场景，需要返回两个可用的端口，并且必须是连续的
// @return 前面的是端口小的，后面的是端口+1的
//
func (a *AvailUdpConnPool) Acquire2() (*net.UDPConn, uint16, *net.UDPConn, uint16, error) {
	a.m.Lock()
	defer a.m.Unlock()

	loopFirstFlag := true
	p := a.lastPort
	for {
		// 找了一轮也没有可用的，返回错误
		if !loopFirstFlag && p == a.lastPort {
			return nil, 0, nil, 0, ErrNazaNet
		}
		loopFirstFlag = false

		// 因为第一个端口如果为最大值，那么和第二个端口肯定不是线性连续了
		if p == a.maxPort {
			p = a.minPort
			continue
		}

		conn, err := listenUdpWithPort(p)

		// 第一个就绑定失败，尝试下一个端口
		if err != nil {
			p = a.nextPort(p)
			continue
		}

		// 绑定成功，因为我们需要两个，所以我们还要找第二个

		// 因为前面已经有判断最大值了，所以直接+1
		conn2, err := listenUdpWithPort(p + 1)

		// 第二个失败了，关闭第一个，然后从第二个的下一个重新尝试
		if err != nil {
			_ = conn.Close()
			p = a.nextPort(p + 1)
			continue
		}

		// 绑定成功，更新last，返回结果
		a.lastPort = a.nextPort(p + 1)
		return conn, p, conn2, p + 1, nil
	}
}

// 通过Acquire获取到可用net.UDPConn对象后，将对象关闭，只返回可用的端口
func (a *AvailUdpConnPool) Peek() (uint16, error) {
	conn, port, err := a.Acquire()
	if err == nil {
		err = conn.Close()
	}
	return port, err
}

func (a *AvailUdpConnPool) nextPort(p uint16) uint16 {
	if p == a.maxPort {
		return a.minPort
	}

	return p + 1
}
