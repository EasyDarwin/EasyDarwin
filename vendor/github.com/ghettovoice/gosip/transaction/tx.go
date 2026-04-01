package transaction

import (
	"fmt"
	"sync"

	"github.com/discoviking/fsm"

	"github.com/ghettovoice/gosip/log"
	"github.com/ghettovoice/gosip/sip"
)

type TxKey = sip.TransactionKey

// Tx is an common SIP transaction
type Tx interface {
	Init() error
	Key() TxKey
	Origin() sip.Request
	// Receive receives message from transport layer.
	Receive(msg sip.Message) error
	String() string
	Transport() sip.Transport
	Terminate()
	Errors() <-chan error
	Done() <-chan bool
}

type commonTx struct {
	key      TxKey
	fsm      *fsm.FSM
	fsmMu    sync.RWMutex
	origin   sip.Request
	tpl      sip.Transport
	lastResp sip.Response

	errs    chan error
	lastErr error
	done    chan bool

	log log.Logger
}

func (tx *commonTx) String() string {
	if tx == nil {
		return "<nil>"
	}

	fields := tx.Log().Fields().WithFields(log.Fields{
		"key": tx.key,
	})

	return fmt.Sprintf("%s<%s>", tx.Log().Prefix(), fields)
}

func (tx *commonTx) Log() log.Logger {
	return tx.log
}

func (tx *commonTx) Origin() sip.Request {
	return tx.origin
}

func (tx *commonTx) Key() TxKey {
	return tx.key
}

func (tx *commonTx) Transport() sip.Transport {
	return tx.tpl
}

func (tx *commonTx) Errors() <-chan error {
	return tx.errs
}

func (tx *commonTx) Done() <-chan bool {
	return tx.done
}
