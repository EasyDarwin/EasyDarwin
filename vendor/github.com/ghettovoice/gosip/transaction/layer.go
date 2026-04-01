package transaction

import (
	"fmt"
	"sync"

	"github.com/ghettovoice/gosip/log"
	"github.com/ghettovoice/gosip/sip"
)

// Layer serves client and server transactions.
type Layer interface {
	Cancel()
	Done() <-chan struct{}
	String() string
	Request(req sip.Request) (sip.ClientTransaction, error)
	Respond(res sip.Response) (sip.ServerTransaction, error)
	Transport() sip.Transport
	// Requests returns channel with new incoming server transactions.
	Requests() <-chan sip.ServerTransaction
	// ACKs on 2xx
	Acks() <-chan sip.Request
	// Responses returns channel with not matched responses.
	Responses() <-chan sip.Response
	Errors() <-chan error
}

type layer struct {
	tpl          sip.Transport
	requests     chan sip.ServerTransaction
	acks         chan sip.Request
	responses    chan sip.Response
	transactions *transactionStore

	errs     chan error
	done     chan struct{}
	canceled chan struct{}

	txWg       sync.WaitGroup
	serveTxCh  chan Tx
	cancelOnce sync.Once

	log log.Logger
}

func NewLayer(tpl sip.Transport, logger log.Logger) Layer {
	txl := &layer{
		tpl:          tpl,
		transactions: newTransactionStore(),

		requests:  make(chan sip.ServerTransaction),
		acks:      make(chan sip.Request),
		responses: make(chan sip.Response),

		errs:      make(chan error),
		done:      make(chan struct{}),
		canceled:  make(chan struct{}),
		serveTxCh: make(chan Tx),
	}
	txl.log = logger.
		WithPrefix("transaction.Layer").
		WithFields(log.Fields{
			"transaction_layer_ptr": fmt.Sprintf("%p", txl),
		})

	go txl.listenMessages()

	return txl
}

func (txl *layer) String() string {
	if txl == nil {
		return "<nil>"
	}

	return fmt.Sprintf("transaction.Layer<%s>", txl.Log().Fields())
}

func (txl *layer) Log() log.Logger {
	return txl.log
}

func (txl *layer) Cancel() {
	select {
	case <-txl.canceled:
		return
	default:
	}

	txl.cancelOnce.Do(func() {
		close(txl.canceled)

		txl.Log().Debug("transaction layer canceled")
	})
}

func (txl *layer) Done() <-chan struct{} {
	return txl.done
}

func (txl *layer) Requests() <-chan sip.ServerTransaction {
	return txl.requests
}

func (txl *layer) Acks() <-chan sip.Request {
	return txl.acks
}

func (txl *layer) Responses() <-chan sip.Response {
	return txl.responses
}

func (txl *layer) Errors() <-chan error {
	return txl.errs
}

func (txl *layer) Transport() sip.Transport {
	return txl.tpl
}

func (txl *layer) Request(req sip.Request) (sip.ClientTransaction, error) {
	select {
	case <-txl.canceled:
		return nil, fmt.Errorf("transaction layer is canceled")
	default:
	}

	if req.IsAck() {
		return nil, fmt.Errorf("ACK request must be sent directly through transport")
	}

	tx, err := NewClientTx(req, txl.tpl, txl.Log())
	if err != nil {
		return nil, err
	}

	logger := log.AddFieldsFrom(txl.Log(), req, tx)
	logger.Debug("client transaction created")

	if err := tx.Init(); err != nil {
		return nil, err
	}

	txl.transactions.put(tx.Key(), tx)

	select {
	case <-txl.canceled:
		return tx, fmt.Errorf("transaction layer is canceled")
	case txl.serveTxCh <- tx:
	}

	return tx, nil
}

func (txl *layer) Respond(res sip.Response) (sip.ServerTransaction, error) {
	select {
	case <-txl.canceled:
		return nil, fmt.Errorf("transaction layer is canceled")
	default:
	}

	tx, err := txl.getServerTx(res)
	if err != nil {
		return nil, err
	}

	err = tx.Respond(res)
	if err != nil {
		return nil, err
	}

	return tx, nil
}

func (txl *layer) listenMessages() {
	defer func() {
		txl.txWg.Wait()

		close(txl.requests)
		close(txl.responses)
		close(txl.acks)
		close(txl.errs)
		close(txl.done)
	}()

	txl.Log().Debug("start listen messages")
	defer txl.Log().Debug("stop listen messages")

	for {
		select {
		case <-txl.canceled:
			return
		case tx := <-txl.serveTxCh:
			txl.txWg.Add(1)
			go txl.serveTransaction(tx)
		case msg, ok := <-txl.tpl.Messages():
			if !ok {
				continue
			}

			txl.handleMessage(msg)
		}
	}
}

func (txl *layer) serveTransaction(tx Tx) {
	logger := log.AddFieldsFrom(txl.Log(), tx)

	defer func() {
		txl.transactions.drop(tx.Key())

		logger.Debug("transaction deleted")

		txl.txWg.Done()
	}()

	logger.Debug("start serve transaction")
	defer logger.Debug("stop serve transaction")

	for {
		select {
		case <-txl.canceled:
			tx.Terminate()
			return
		case <-tx.Done():
			return
		}
	}
}

func (txl *layer) handleMessage(msg sip.Message) {
	select {
	case <-txl.canceled:
		return
	default:
	}

	logger := txl.Log().WithFields(msg.Fields())
	logger.Debugf("handling SIP message")

	switch msg := msg.(type) {
	case sip.Request:
		txl.handleRequest(msg, logger)
	case sip.Response:
		txl.handleResponse(msg, logger)
	default:
		logger.Error("unsupported message, skip it")
		// todo pass up error?
	}
}

func (txl *layer) handleRequest(req sip.Request, logger log.Logger) {
	select {
	case <-txl.canceled:
		return
	default:
	}

	// try to match to existent tx: request retransmission, or ACKs on non-2xx, or CANCEL
	tx, err := txl.getServerTx(req)
	if err == nil {
		logger = log.AddFieldsFrom(logger, tx)

		if err := tx.Receive(req); err != nil {
			logger.Error(err)
		}

		return
	}
	// ACK on 2xx
	if req.IsAck() {
		select {
		case <-txl.canceled:
		case txl.acks <- req:
		}
		return
	}
	if req.IsCancel() {
		// transaction for CANCEL already completed and terminated
		res := sip.NewResponseFromRequest("", req, 481, "Transaction Does Not Exist", "")
		if err := txl.tpl.Send(res); err != nil {
			logger.Error(fmt.Errorf("respond '481 Transaction Does Not Exist' on non-matched CANCEL request: %w", err))
		}
		return
	}

	tx, err = NewServerTx(req, txl.tpl, txl.Log())
	if err != nil {
		logger.Error(err)

		return
	}

	logger = log.AddFieldsFrom(logger, tx)
	logger.Debug("new server transaction created")

	if err := tx.Init(); err != nil {
		logger.Error(err)

		return
	}

	// put tx to store, to match retransmitting requests later
	txl.transactions.put(tx.Key(), tx)

	txl.txWg.Add(1)
	go txl.serveTransaction(tx)

	// pass up request
	logger.Trace("passing up SIP request...")

	select {
	case <-txl.canceled:
		return
	case txl.requests <- tx:
		logger.Trace("SIP request passed up")
	}
}

func (txl *layer) handleResponse(res sip.Response, logger log.Logger) {
	select {
	case <-txl.canceled:
		return
	default:
	}

	tx, err := txl.getClientTx(res)
	if err != nil {
		logger.Tracef("passing up non-matched SIP response: %s", err)

		// RFC 3261 - 17.1.1.2.
		// Not matched responses should be passed directly to the UA
		select {
		case <-txl.canceled:
		case txl.responses <- res:
			logger.Trace("non-matched SIP response passed up")
		}

		return
	}

	logger = log.AddFieldsFrom(logger, tx)

	if err := tx.Receive(res); err != nil {
		logger.Error(err)

		return
	}
}

// RFC 17.1.3.
func (txl *layer) getClientTx(msg sip.Message) (ClientTx, error) {
	logger := txl.Log().WithFields(msg.Fields())

	logger.Trace("searching client transaction")

	key, err := MakeClientTxKey(msg)
	if err != nil {
		return nil, fmt.Errorf("%s failed to match message '%s' to client transaction: %w", txl, msg.Short(), err)
	}

	tx, ok := txl.transactions.get(key)
	if !ok {
		return nil, fmt.Errorf(
			"%s failed to match message '%s' to client transaction: transaction with key '%s' not found",
			txl,
			msg.Short(),
			key,
		)
	}

	logger = log.AddFieldsFrom(logger, tx)

	switch tx := tx.(type) {
	case ClientTx:
		logger.Trace("client transaction found")

		return tx, nil
	default:
		return nil, fmt.Errorf(
			"%s failed to match message '%s' to client transaction: found %s is not a client transaction",
			txl,
			msg.Short(),
			tx,
		)
	}
}

// RFC 17.2.3.
func (txl *layer) getServerTx(msg sip.Message) (ServerTx, error) {
	logger := txl.Log().WithFields(msg.Fields())

	logger.Trace("searching server transaction")

	key, err := MakeServerTxKey(msg)
	if err != nil {
		return nil, fmt.Errorf("%s failed to match message '%s' to server transaction: %w", txl, msg.Short(), err)
	}

	tx, ok := txl.transactions.get(key)
	if !ok {
		return nil, fmt.Errorf(
			"%s failed to match message '%s' to server transaction: transaction with key '%s' not found",
			txl,
			msg.Short(),
			key,
		)
	}

	logger = log.AddFieldsFrom(logger)

	switch tx := tx.(type) {
	case ServerTx:
		logger.Trace("server transaction found")

		return tx, nil
	default:
		return nil, fmt.Errorf(
			"%s failed to match message '%s' to server transaction: found %s is not server transaction",
			txl,
			msg.Short(),
			tx,
		)
	}
}

type transactionStore struct {
	transactions map[TxKey]Tx

	mu sync.RWMutex
}

func newTransactionStore() *transactionStore {
	return &transactionStore{
		transactions: make(map[TxKey]Tx),
	}
}

func (store *transactionStore) put(key TxKey, tx Tx) {
	store.mu.Lock()
	defer store.mu.Unlock()
	store.transactions[key] = tx
}

func (store *transactionStore) get(key TxKey) (Tx, bool) {
	store.mu.RLock()
	defer store.mu.RUnlock()
	tx, ok := store.transactions[key]
	return tx, ok
}

func (store *transactionStore) drop(key TxKey) bool {
	if _, ok := store.get(key); !ok {
		return false
	}
	store.mu.Lock()
	defer store.mu.Unlock()
	delete(store.transactions, key)
	return true
}

func (store *transactionStore) all() []Tx {
	all := make([]Tx, 0)
	store.mu.RLock()
	defer store.mu.RUnlock()
	for _, tx := range store.transactions {
		all = append(all, tx)
	}

	return all
}
