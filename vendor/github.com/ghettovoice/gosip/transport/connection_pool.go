package transport

import (
	"bytes"
	"errors"
	"fmt"
	"net"
	"sync"
	"time"

	"github.com/ghettovoice/gosip/log"
	"github.com/ghettovoice/gosip/sip"
	"github.com/ghettovoice/gosip/sip/parser"
	"github.com/ghettovoice/gosip/timing"
)

type ConnectionKey string

func (key ConnectionKey) String() string {
	return string(key)
}

// ConnectionPool used for active connection management.
type ConnectionPool interface {
	Done() <-chan struct{}
	String() string
	Put(connection Connection, ttl time.Duration) error
	Get(key ConnectionKey) (Connection, error)
	All() []Connection
	Drop(key ConnectionKey) error
	DropAll() error
	Length() int
}

// ConnectionHandler serves associated connection, i.e. parses
// incoming data, manages expiry time & etc.
type ConnectionHandler interface {
	Cancel()
	Done() <-chan struct{}
	String() string
	Key() ConnectionKey
	Connection() Connection
	// Expiry returns connection expiry time.
	Expiry() time.Time
	Expired() bool
	// Update updates connection expiry time.
	// TODO put later to allow runtime update
	// Update(conn Connection, ttl time.Duration)
	// Manage runs connection serving.
	Serve()
}

type connectionPool struct {
	store     map[ConnectionKey]ConnectionHandler
	msgMapper sip.MessageMapper

	output chan<- sip.Message
	errs   chan<- error
	cancel <-chan struct{}

	done     chan struct{}
	hmess    chan sip.Message
	herrs    chan error
	srvhDone chan struct{}

	hwg sync.WaitGroup
	mu  sync.RWMutex

	log log.Logger
}

func NewConnectionPool(
	output chan<- sip.Message,
	errs chan<- error,
	cancel <-chan struct{},
	msgMapper sip.MessageMapper,
	logger log.Logger,
) ConnectionPool {
	pool := &connectionPool{
		store:     make(map[ConnectionKey]ConnectionHandler),
		msgMapper: msgMapper,

		output: output,
		errs:   errs,
		cancel: cancel,

		done:     make(chan struct{}),
		hmess:    make(chan sip.Message),
		herrs:    make(chan error),
		srvhDone: make(chan struct{}),
	}

	pool.log = logger.
		WithPrefix("transport.ConnectionPool").
		WithFields(log.Fields{
			"connection_pool_ptr": fmt.Sprintf("%p", pool),
		})

	go func() {
		<-pool.cancel
		pool.dispose()
	}()
	go pool.serveHandlers()

	return pool
}

func (pool *connectionPool) String() string {
	if pool == nil {
		return "<nil>"
	}

	return fmt.Sprintf("transport.ConnectionPool<%s>", pool.Log().Fields())
}

func (pool *connectionPool) Log() log.Logger {
	return pool.log
}

func (pool *connectionPool) Done() <-chan struct{} {
	return pool.done
}

// Put adds new connection to pool or updates TTL of existing connection
// TTL - 0 - unlimited; 1 - ... - time to live in pool
func (pool *connectionPool) Put(connection Connection, ttl time.Duration) error {
	select {
	case <-pool.cancel:
		return &PoolError{
			fmt.Errorf("connection pool closed"),
			"get connection",
			pool.String(),
		}
	default:
	}

	key := connection.Key()
	if key == "" {
		return &PoolError{
			fmt.Errorf("empty connection key"),
			"put connection",
			pool.String(),
		}
	}

	pool.mu.Lock()
	defer pool.mu.Unlock()

	return pool.put(key, connection, ttl)
}

func (pool *connectionPool) Get(key ConnectionKey) (Connection, error) {
	pool.mu.RLock()
	defer pool.mu.RUnlock()

	return pool.getConnection(key)
}

func (pool *connectionPool) Drop(key ConnectionKey) error {
	pool.mu.Lock()
	defer pool.mu.Unlock()

	return pool.drop(key)
}

func (pool *connectionPool) DropAll() error {
	pool.mu.Lock()
	for key := range pool.store {
		if err := pool.drop(key); err != nil {
			pool.Log().Errorf("drop connection %s failed: %s", key, err)
		}
	}
	pool.mu.Unlock()

	return nil
}

func (pool *connectionPool) All() []Connection {
	pool.mu.RLock()
	conns := make([]Connection, 0)
	for _, handler := range pool.store {
		conns = append(conns, handler.Connection())
	}
	pool.mu.RUnlock()

	return conns
}

func (pool *connectionPool) Length() int {
	pool.mu.RLock()
	defer pool.mu.RUnlock()

	return len(pool.store)
}

func (pool *connectionPool) dispose() {
	// clean pool
	pool.DropAll()
	pool.hwg.Wait()

	// stop serveHandlers goroutine
	close(pool.hmess)
	close(pool.herrs)

	<-pool.srvhDone
	close(pool.done)
}

func (pool *connectionPool) serveHandlers() {
	pool.Log().Debug("begin serve connection handlers")
	defer pool.Log().Debug("stop serve connection handlers")

	defer close(pool.srvhDone)

	for {
		logger := pool.Log()

		select {
		case msg, ok := <-pool.hmess:
			// cancel signal, serveStore exists
			if !ok {
				return
			}
			if msg == nil {
				continue
			}

			logger = logger.WithFields(msg.Fields())
			logger.Trace("passing up SIP message")

			select {
			case <-pool.cancel:
				return
			case pool.output <- msg:
				logger.Trace("SIP message passed up")

				continue
			}
		case err, ok := <-pool.herrs:
			// cancel signal, serveStore exists
			if !ok {
				return
			}
			if err == nil {
				continue
			}

			// on ConnectionHandleError we should drop handler in some cases
			// all other possible errors ignored because in pool.herrs should be only ConnectionHandlerErrors
			// so ConnectionPool passes up only Network (when connection falls) and MalformedMessage errors
			var herr *ConnectionHandlerError
			if !errors.As(err, &herr) {
				// all other possible errors
				logger.Tracef("ignore non connection error: %s", err)

				continue
			}

			pool.mu.RLock()
			handler, gerr := pool.get(herr.Key)
			pool.mu.RUnlock()
			if gerr != nil {
				// ignore, handler already dropped out
				logger.Tracef("ignore error from already dropped out connection %s: %s", herr.Key, gerr)

				continue
			}

			logger = logger.WithFields(log.Fields{
				"connection_handler": handler.String(),
			})

			if herr.Expired() {
				// handler expired, drop it from pool and continue without emitting error
				if handler.Expired() {
					// connection expired
					logger.Debug("connection expired, drop it and go further")

					if err := pool.Drop(handler.Key()); err != nil {
						logger.Error(err)
					}
				} else {
					// Due to a race condition, the socket has been updated since this expiry happened.
					// Ignore the expiry since we already have a new socket for this address.
					logger.Trace("ignore spurious connection expiry")
				}

				continue
			} else if herr.EOF() {
				select {
				case <-pool.cancel:
					return
				default:
				}

				// remote endpoint closed
				logger.Debugf("connection EOF: %s; drop it and go further", herr)

				if err := pool.Drop(handler.Key()); err != nil {
					logger.Error(err)
				}

				var connErr *ConnectionError
				if errors.As(herr.Err, &connErr) {
					pool.errs <- herr.Err
				}

				continue
			} else if herr.Network() {
				// connection broken or closed
				logger.Debugf("connection network error: %s; drop it and pass the error up", herr)

				if err := pool.Drop(handler.Key()); err != nil {
					logger.Error(err)
				}
			} else {
				// syntax errors, malformed message errors and other
				logger.Tracef("connection error: %s; pass the error up", herr)
			}
			// send initial error
			select {
			case <-pool.cancel:
				return
			case pool.errs <- herr.Err:
				logger.Trace("error passed up")

				continue
			}
		}
	}
}

func (pool *connectionPool) put(key ConnectionKey, conn Connection, ttl time.Duration) error {
	if _, err := pool.get(key); err == nil {
		return &PoolError{
			fmt.Errorf("key %s already exists in the pool", key),
			"put connection",
			pool.String(),
		}
	}

	// wrap to handler
	handler := NewConnectionHandler(
		conn,
		ttl,
		pool.hmess,
		pool.herrs,
		pool.msgMapper,
		pool.Log(),
	)

	logger := log.AddFieldsFrom(pool.Log(), handler)
	logger.Tracef("put connection to the pool with TTL = %s", ttl)

	pool.store[handler.Key()] = handler

	// start serving
	pool.hwg.Add(1)
	go handler.Serve()
	go func() {
		<-handler.Done()
		pool.hwg.Done()
	}()

	return nil
}

func (pool *connectionPool) drop(key ConnectionKey) error {
	// check existence in pool
	handler, err := pool.get(key)
	if err != nil {
		return err
	}

	handler.Cancel()

	logger := log.AddFieldsFrom(pool.Log(), handler)
	logger.Trace("drop connection from the pool")

	// modify store
	delete(pool.store, key)

	return nil
}

func (pool *connectionPool) get(key ConnectionKey) (ConnectionHandler, error) {
	if handler, ok := pool.store[key]; ok {
		return handler, nil
	}

	return nil, &PoolError{
		fmt.Errorf("connection %s not found in the pool", key),
		"get connection",
		pool.String(),
	}
}

func (pool *connectionPool) getConnection(key ConnectionKey) (Connection, error) {
	var conn Connection
	handler, err := pool.get(key)
	if err == nil {
		conn = handler.Connection()
	}
	return conn, err
}

// connectionHandler actually serves associated connection
type connectionHandler struct {
	connection Connection
	msgMapper  sip.MessageMapper

	timer  timing.Timer
	ttl    time.Duration
	expiry time.Time

	output     chan<- sip.Message
	errs       chan<- error
	cancelOnce sync.Once
	canceled   chan struct{}
	done       chan struct{}

	log log.Logger
}

func NewConnectionHandler(
	conn Connection,
	ttl time.Duration,
	output chan<- sip.Message,
	errs chan<- error,
	msgMapper sip.MessageMapper,
	logger log.Logger,
) ConnectionHandler {
	handler := &connectionHandler{
		connection: conn,
		msgMapper:  msgMapper,

		output:   output,
		errs:     errs,
		canceled: make(chan struct{}),
		done:     make(chan struct{}),

		ttl: ttl,
	}

	handler.log = logger.
		WithPrefix("transport.ConnectionHandler").
		WithFields(log.Fields{
			"connection_handler_ptr": fmt.Sprintf("%p", handler),
			"connection_ptr":         fmt.Sprintf("%p", conn),
			"connection_key":         conn.Key(),
			"connection_network":     conn.Network(),
		})

	// handler.Update(ttl)
	if ttl > 0 {
		handler.expiry = time.Now().Add(ttl)
		handler.timer = timing.NewTimer(ttl)
	} else {
		handler.expiry = time.Time{}
		handler.timer = timing.NewTimer(0)
		if !handler.timer.Stop() {
			<-handler.timer.C()
		}
	}

	if handler.msgMapper == nil {
		handler.msgMapper = func(msg sip.Message) sip.Message {
			return msg
		}
	}

	return handler
}

func (handler *connectionHandler) String() string {
	if handler == nil {
		return "<nil>"
	}

	return fmt.Sprintf("transport.ConnectionHandler<%s>", handler.Log().Fields())
}

func (handler *connectionHandler) Log() log.Logger {
	return handler.log
}

func (handler *connectionHandler) Key() ConnectionKey {
	return handler.connection.Key()
}

func (handler *connectionHandler) Connection() Connection {
	return handler.connection
}

func (handler *connectionHandler) Expiry() time.Time {
	return handler.expiry
}

func (handler *connectionHandler) Expired() bool {
	return !handler.Expiry().IsZero() && handler.Expiry().Before(time.Now())
}

// resets the timeout timer.
// func (handler *connectionHandler) Update(ttl time.Duration) {
// 	if ttl > 0 {
// 		expiryTime := timing.Now().Put(ttl)
// 		handler.Log().Debugf("set %s expiry time to %s", handler, expiryTime)
// 		handler.expiry = expiryTime
//
// 		if handler.timer == nil {
// 			handler.timer = timing.NewTimer(ttl)
// 		} else {
// 			handler.timer.Reset(ttl)
// 		}
// 	} else {
// 		handler.Log().Debugf("set %s unlimited expiry time")
// 		handler.expiry = time.Time{}
//
// 		if handler.timer == nil {
// 			handler.timer = timing.NewTimer(0)
// 		}
// 		handler.timer.Stop()
// 	}
// }

// Serve is connection serving loop.
// Waits for the connection to expire, and notifies the pool when it does.
func (handler *connectionHandler) Serve() {
	defer close(handler.done)

	handler.Log().Debug("begin serve connection")
	defer handler.Log().Debug("stop serve connection")

	// start connection serving goroutines
	handler.readConnection()
}

func (handler *connectionHandler) readStream() {
	msgs := make(chan sip.Message)
	errs := make(chan error)
	strPrs := parser.NewParser(msgs, errs, true, handler.Log())
	raddr := handler.Connection().RemoteAddr().String()
	go func() {
		defer func() {
			_ = handler.Connection().Close()
			strPrs.Stop()
			close(msgs)
			close(errs)
		}()

		handler.Log().Debug("begin read connection")
		defer handler.Log().Debug("stop read connection")

		buf := make([]byte, bufferSize)
		var (
			num int
			err error
		)

		for {
			// wait for data
			num, err = handler.Connection().Read(buf)
			if err != nil {
				// broken or closed connection
				// so send error and exit
				handler.handleError(err, raddr)

				return
			}
			data := buf[:num]
			if _, err := strPrs.Write(data); err != nil {
				handler.handleError(err, raddr)
			}
		}
	}()
	handler.pipeOutputs(raddr, msgs, errs)
}

func (handler *connectionHandler) readPacket() {
	handler.Log().Debug("begin read connection")
	defer handler.Log().Debug("stop read connection")

	buf := make([]byte, bufferSize)
	pktPrs := parser.NewPacketParser(handler.Log())
	var (
		num   int
		err   error
		raddr net.Addr
	)
	for {
		num, raddr, err = handler.Connection().ReadFrom(buf)
		if err != nil {
			handler.handleError(err, "")
			return
		}
		if len(bytes.Trim(buf[:num], "\x00")) == 0 {
			continue
		}

		if msg, err := pktPrs.ParseMessage(buf[:num]); err == nil {
			handler.handleMessage(msg, raddr.String())
		} else {
			handler.handleError(err, raddr.String())
		}
	}
}

func (handler *connectionHandler) readConnection() {
	if handler.Connection().Streamed() {
		handler.readStream()
	} else {
		handler.readPacket()
	}
}

func (handler *connectionHandler) pipeOutputs(raddr string, msgs <-chan sip.Message, errs <-chan error) {
	handler.Log().Debug("begin pipe outputs")
	defer handler.Log().Debug("stop pipe outputs")

	for {
		select {
		case <-handler.timer.C():
			if handler.Expiry().IsZero() {
				// handler expiryTime is zero only when TTL = 0 (unlimited handler)
				// so we must not get here with zero expiryTime
				handler.Log().Panic("fires expiry timer with ZERO expiryTime")
			}

			// pass up to the pool
			handler.handleError(ExpireError("connection expired"), raddr)
		case msg, ok := <-msgs:
			if !ok {
				return
			}
			handler.handleMessage(msg, raddr)
		case err, ok := <-errs:
			if !ok {
				return
			}
			handler.handleError(err, raddr)
		}
	}
}

func (handler *connectionHandler) handleMessage(msg sip.Message, raddr string) {
	msg.SetDestination(handler.Connection().LocalAddr().String())
	rhost, rport, _ := net.SplitHostPort(raddr)

	switch msg := msg.(type) {
	case sip.Request:
		// RFC 3261 - 18.2.1
		viaHop, ok := msg.ViaHop()
		if !ok {
			handler.Log().Warn("ignore message without 'Via' header")

			return
		}

		if rhost != "" && rhost != viaHop.Host {
			viaHop.Params.Add("received", sip.String{Str: rhost})
		}

		// rfc3581
		if viaHop.Params.Has("rport") {
			viaHop.Params.Add("rport", sip.String{Str: rport})
		}

		if !handler.Connection().Streamed() {
			if !viaHop.Params.Has("rport") {
				var port sip.Port
				if viaHop.Port != nil {
					port = *viaHop.Port
				} else {
					port = sip.DefaultPort(handler.Connection().Network())
				}
				raddr = fmt.Sprintf("%s:%d", rhost, port)
			}
		}

		msg.SetTransport(handler.connection.Network())
		msg.SetSource(raddr)
	case sip.Response:
		// Set Remote Address as response source
		msg.SetTransport(handler.connection.Network())
		msg.SetSource(raddr)
	}

	msg = handler.msgMapper(msg.WithFields(log.Fields{
		"connection_key": handler.Connection().Key(),
		"received_at":    time.Now(),
	}))

	// pass up
	select {
	case <-handler.canceled:
		return
	case handler.output <- msg:
	}

	if !handler.Expiry().IsZero() {
		handler.expiry = time.Now().Add(handler.ttl)
		handler.timer.Reset(handler.ttl)
	}
}

func (handler *connectionHandler) handleError(err error, raddr string) {
	if isSyntaxError(err) {
		handler.Log().Tracef("ignore error: %s", err)
		return
	}

	err = &ConnectionHandlerError{
		err,
		handler.Key(),
		fmt.Sprintf("%p", handler),
		handler.Connection().Network(),
		fmt.Sprintf("%v", handler.Connection().LocalAddr()),
		raddr,
	}

	select {
	case <-handler.canceled:
	case handler.errs <- err:
	}
}

func isSyntaxError(err error) bool {
	var perr parser.Error
	if errors.As(err, &perr) && perr.Syntax() {
		return true
	}

	var merr sip.MessageError
	if errors.As(err, &merr) && merr.Broken() {
		return true
	}

	return false
}

// Cancel simply calls runtime provided cancel function.
func (handler *connectionHandler) Cancel() {
	handler.cancelOnce.Do(func() {
		close(handler.canceled)
		handler.Connection().Close()

		handler.Log().Debug("connection handler canceled")
	})
}

func (handler *connectionHandler) Done() <-chan struct{} {
	return handler.done
}
