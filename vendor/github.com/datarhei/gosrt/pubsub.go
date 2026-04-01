package srt

import (
	"context"
	"fmt"
	"io"
	"sync"

	"github.com/datarhei/gosrt/packet"
)

// PubSub is a publish/subscriber service for SRT connections.
type PubSub interface {
	// Publish accepts a SRT connection where it reads from. It blocks
	// until the connection closes. The returned error indicates why it
	// stopped. There can be only one publisher.
	Publish(c Conn) error

	// Subscribe accepts a SRT connection where it writes the data from
	// the publisher to. It blocks until an error happens. If the publisher
	// disconnects, io.EOF is returned. There can be an arbitrary number
	// of subscribers.
	Subscribe(c Conn) error
}

// pubSub is an implementation of the PubSub interface
type pubSub struct {
	incoming      chan packet.Packet
	ctx           context.Context
	cancel        context.CancelFunc
	publish       bool
	publishLock   sync.Mutex
	listeners     map[uint32]chan packet.Packet
	listenersLock sync.Mutex
	logger        Logger
}

// PubSubConfig is for configuring a new PubSub
type PubSubConfig struct {
	Logger Logger // Optional logger
}

// NewPubSub returns a PubSub. After the publishing connection closed
// this PubSub can't be used anymore.
func NewPubSub(config PubSubConfig) PubSub {
	pb := &pubSub{
		incoming:  make(chan packet.Packet, 1024),
		listeners: make(map[uint32]chan packet.Packet),
		logger:    config.Logger,
	}

	pb.ctx, pb.cancel = context.WithCancel(context.Background())

	if pb.logger == nil {
		pb.logger = NewLogger(nil)
	}

	go pb.broadcast()

	return pb
}

func (pb *pubSub) broadcast() {
	defer func() {
		pb.logger.Print("pubsub:close", 0, 1, func() string { return "exiting broadcast loop" })
	}()

	pb.logger.Print("pubsub:new", 0, 1, func() string { return "starting broadcast loop" })

	for {
		select {
		case <-pb.ctx.Done():
			return
		case p := <-pb.incoming:
			pb.listenersLock.Lock()
			for socketId, c := range pb.listeners {
				pp := p.Clone()

				select {
				case c <- pp:
				default:
					pb.logger.Print("pubsub:error", socketId, 1, func() string { return "broadcast target queue is full" })
				}
			}
			pb.listenersLock.Unlock()

			// We don't need this packet anymore
			p.Decommission()
		}
	}
}

func (pb *pubSub) Publish(c Conn) error {
	pb.publishLock.Lock()
	defer pb.publishLock.Unlock()

	if pb.publish {
		err := fmt.Errorf("only one publisher is allowed")
		pb.logger.Print("pubsub:error", 0, 1, func() string { return err.Error() })
		return err
	}

	var p packet.Packet
	var err error

	socketId := c.SocketId()

	pb.logger.Print("pubsub:publish", socketId, 1, func() string { return "new publisher" })

	pb.publish = true

	for {
		p, err = c.ReadPacket()
		if err != nil {
			pb.logger.Print("pubsub:error", socketId, 1, func() string { return err.Error() })
			break
		}

		select {
		case pb.incoming <- p:
		default:
			pb.logger.Print("pubsub:error", socketId, 1, func() string { return "incoming queue is full" })
		}
	}

	pb.cancel()

	return err
}

func (pb *pubSub) Subscribe(c Conn) error {
	l := make(chan packet.Packet, 1024)
	socketId := c.SocketId()

	pb.logger.Print("pubsub:subscribe", socketId, 1, func() string { return "new subscriber" })

	pb.listenersLock.Lock()
	pb.listeners[socketId] = l
	pb.listenersLock.Unlock()

	defer func() {
		pb.listenersLock.Lock()
		delete(pb.listeners, socketId)
		pb.listenersLock.Unlock()
	}()

	for {
		select {
		case <-pb.ctx.Done():
			return io.EOF
		case p := <-l:
			err := c.WritePacket(p)
			p.Decommission()
			if err != nil {
				pb.logger.Print("pubsub:error", socketId, 1, func() string { return err.Error() })
				return err
			}
		}
	}
}
