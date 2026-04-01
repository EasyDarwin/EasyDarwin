package transaction

import (
	"fmt"
	"sync"
	"time"

	"github.com/discoviking/fsm"

	"github.com/ghettovoice/gosip/log"
	"github.com/ghettovoice/gosip/sip"
	"github.com/ghettovoice/gosip/timing"
)

type ServerTx interface {
	Tx
	Respond(res sip.Response) error
	Acks() <-chan sip.Request
	Cancels() <-chan sip.Request
}

type serverTx struct {
	commonTx
	lastAck      sip.Request
	lastCancel   sip.Request
	acks         chan sip.Request
	cancels      chan sip.Request
	timer_g      timing.Timer
	timer_g_time time.Duration
	timer_h      timing.Timer
	timer_i      timing.Timer
	timer_i_time time.Duration
	timer_j      timing.Timer
	timer_1xx    timing.Timer
	timer_l      timing.Timer
	reliable     bool

	mu        sync.RWMutex
	closeOnce sync.Once
}

func NewServerTx(origin sip.Request, tpl sip.Transport, logger log.Logger) (ServerTx, error) {
	key, err := MakeServerTxKey(origin)
	if err != nil {
		return nil, err
	}

	tx := new(serverTx)
	tx.key = key
	tx.tpl = tpl
	// about ~10 retransmits
	tx.acks = make(chan sip.Request, 64)
	tx.cancels = make(chan sip.Request, 64)
	tx.errs = make(chan error, 64)
	tx.done = make(chan bool)
	tx.log = logger.
		WithPrefix("transaction.ServerTx").
		WithFields(
			origin.Fields().WithFields(log.Fields{
				"transaction_ptr": fmt.Sprintf("%p", tx),
				"transaction_key": tx.key,
			}),
		)
	tx.origin = origin.WithFields(log.Fields{
		"transaction_ptr": fmt.Sprintf("%p", tx),
		"transaction_key": tx.key,
	}).(sip.Request)
	tx.reliable = tx.tpl.IsReliable(origin.Transport())

	return tx, nil
}

func (tx *serverTx) Init() error {
	tx.initFSM()

	tx.mu.Lock()

	if tx.reliable {
		tx.timer_i_time = 0
	} else {
		tx.timer_g_time = Timer_G
		tx.timer_i_time = Timer_I
	}

	tx.mu.Unlock()

	// RFC 3261 - 17.2.1
	if tx.Origin().IsInvite() {
		tx.Log().Tracef("set timer_1xx to %v", Timer_1xx)

		tx.mu.Lock()
		tx.timer_1xx = timing.AfterFunc(Timer_1xx, func() {
			select {
			case <-tx.done:
				return
			default:
			}

			tx.Log().Trace("timer_1xx fired")

			if err := tx.Respond(
				sip.NewResponseFromRequest(
					"",
					tx.Origin(),
					100,
					"Trying",
					"",
				),
			); err != nil {
				tx.Log().Errorf("send '100 Trying' response failed: %s", err)
			}
		})
		tx.mu.Unlock()
	}

	return nil
}

func (tx *serverTx) Receive(msg sip.Message) error {
	req, ok := msg.(sip.Request)
	if !ok {
		return &sip.UnexpectedMessageError{
			Err: fmt.Errorf("%s recevied unexpected %s", tx, msg),
			Msg: req.String(),
		}
	}

	tx.mu.Lock()
	if tx.timer_1xx != nil {
		tx.timer_1xx.Stop()
		tx.timer_1xx = nil
	}
	tx.mu.Unlock()

	var input = fsm.NO_INPUT
	switch {
	case req.Method() == tx.Origin().Method():
		input = server_input_request
	case req.IsAck(): // ACK for non-2xx response
		input = server_input_ack
		tx.mu.Lock()
		tx.lastAck = req
		tx.mu.Unlock()
	case req.IsCancel():
		input = server_input_cancel
		tx.mu.Lock()
		tx.lastCancel = req
		tx.mu.Unlock()
	default:
		return &sip.UnexpectedMessageError{
			Err: fmt.Errorf("invalid %s correlated to %s", msg, tx),
			Msg: req.String(),
		}
	}

	tx.fsmMu.RLock()
	defer tx.fsmMu.RUnlock()

	return tx.fsm.Spin(input)
}

func (tx *serverTx) Respond(res sip.Response) error {
	if res.IsCancel() {
		_ = tx.tpl.Send(res)
		return nil
	}

	tx.mu.Lock()
	tx.lastResp = res

	if tx.timer_1xx != nil {
		tx.timer_1xx.Stop()
		tx.timer_1xx = nil
	}
	tx.mu.Unlock()

	var input fsm.Input
	switch {
	case res.IsProvisional():
		input = server_input_user_1xx
	case res.IsSuccess():
		input = server_input_user_2xx
	default:
		input = server_input_user_300_plus
	}

	tx.fsmMu.RLock()
	defer tx.fsmMu.RUnlock()

	return tx.fsm.Spin(input)
}

func (tx *serverTx) Acks() <-chan sip.Request {
	return tx.acks
}

func (tx *serverTx) Cancels() <-chan sip.Request {
	return tx.cancels
}

func (tx *serverTx) Terminate() {
	select {
	case <-tx.done:
		return
	default:
	}

	tx.delete()
}

// FSM States
const (
	server_state_trying = iota
	server_state_proceeding
	server_state_completed
	server_state_confirmed
	server_state_accepted
	server_state_terminated
)

// FSM Inputs
const (
	server_input_request fsm.Input = iota
	server_input_ack
	server_input_cancel
	server_input_user_1xx
	server_input_user_2xx
	server_input_user_300_plus
	server_input_timer_g
	server_input_timer_h
	server_input_timer_i
	server_input_timer_j
	server_input_timer_l
	server_input_transport_err
	server_input_delete
)

// Choose the right FSM init function depending on request method.
func (tx *serverTx) initFSM() {
	if tx.Origin().IsInvite() {
		tx.initInviteFSM()
	} else {
		tx.initNonInviteFSM()
	}
}

func (tx *serverTx) initInviteFSM() {
	// Define States
	tx.Log().Debug("initialising INVITE transaction FSM")

	// Proceeding
	server_state_def_proceeding := fsm.State{
		Index: server_state_proceeding,
		Outcomes: map[fsm.Input]fsm.Outcome{
			server_input_request:       {server_state_proceeding, tx.act_respond},
			server_input_cancel:        {server_state_proceeding, tx.act_cancel},
			server_input_user_1xx:      {server_state_proceeding, tx.act_respond},
			server_input_user_2xx:      {server_state_accepted, tx.act_respond_accept},
			server_input_user_300_plus: {server_state_completed, tx.act_respond_complete},
			server_input_transport_err: {server_state_terminated, tx.act_trans_err},
		},
	}

	// Completed
	server_state_def_completed := fsm.State{
		Index: server_state_completed,
		Outcomes: map[fsm.Input]fsm.Outcome{
			server_input_request:       {server_state_completed, tx.act_respond},
			server_input_ack:           {server_state_confirmed, tx.act_confirm},
			server_input_cancel:        {server_state_completed, fsm.NO_ACTION},
			server_input_user_1xx:      {server_state_completed, fsm.NO_ACTION},
			server_input_user_2xx:      {server_state_completed, fsm.NO_ACTION},
			server_input_user_300_plus: {server_state_completed, fsm.NO_ACTION},
			server_input_timer_g:       {server_state_completed, tx.act_respond_complete},
			server_input_timer_h:       {server_state_terminated, tx.act_delete},
			server_input_transport_err: {server_state_terminated, tx.act_trans_err},
		},
	}

	// Confirmed
	server_state_def_confirmed := fsm.State{
		Index: server_state_confirmed,
		Outcomes: map[fsm.Input]fsm.Outcome{
			server_input_request:       {server_state_confirmed, fsm.NO_ACTION},
			server_input_ack:           {server_state_confirmed, fsm.NO_ACTION},
			server_input_cancel:        {server_state_confirmed, fsm.NO_ACTION},
			server_input_user_1xx:      {server_state_confirmed, fsm.NO_ACTION},
			server_input_user_2xx:      {server_state_confirmed, fsm.NO_ACTION},
			server_input_user_300_plus: {server_state_confirmed, fsm.NO_ACTION},
			server_input_timer_i:       {server_state_terminated, tx.act_delete},
			server_input_timer_g:       {server_state_confirmed, fsm.NO_ACTION},
			server_input_timer_h:       {server_state_confirmed, fsm.NO_ACTION},
		},
	}

	server_state_def_accepted := fsm.State{
		Index: server_state_accepted,
		Outcomes: map[fsm.Input]fsm.Outcome{
			server_input_request:       {server_state_accepted, fsm.NO_ACTION},
			server_input_ack:           {server_state_accepted, tx.act_passup_ack},
			server_input_cancel:        {server_state_accepted, fsm.NO_ACTION},
			server_input_user_1xx:      {server_state_accepted, fsm.NO_ACTION},
			server_input_user_2xx:      {server_state_accepted, tx.act_respond},
			server_input_user_300_plus: {server_state_accepted, fsm.NO_ACTION},
			server_input_transport_err: {server_state_terminated, tx.act_trans_err},
			server_input_timer_l:       {server_state_terminated, tx.act_delete},
		},
	}

	// Terminated
	server_state_def_terminated := fsm.State{
		Index: server_state_terminated,
		Outcomes: map[fsm.Input]fsm.Outcome{
			server_input_request:       {server_state_terminated, fsm.NO_ACTION},
			server_input_ack:           {server_state_terminated, fsm.NO_ACTION},
			server_input_cancel:        {server_state_terminated, fsm.NO_ACTION},
			server_input_user_1xx:      {server_state_terminated, fsm.NO_ACTION},
			server_input_user_2xx:      {server_state_terminated, fsm.NO_ACTION},
			server_input_user_300_plus: {server_state_terminated, fsm.NO_ACTION},
			server_input_delete:        {server_state_terminated, tx.act_delete},
			server_input_timer_i:       {server_state_terminated, fsm.NO_ACTION},
			server_input_timer_l:       {server_state_terminated, fsm.NO_ACTION},
		},
	}

	// Define FSM
	fsm_, err := fsm.Define(
		server_state_def_proceeding,
		server_state_def_completed,
		server_state_def_confirmed,
		server_state_def_accepted,
		server_state_def_terminated,
	)
	if err != nil {
		tx.Log().Errorf("define INVITE transaction FSM failed: %s", err)

		return
	}

	tx.fsmMu.Lock()
	tx.fsm = fsm_
	tx.fsmMu.Unlock()
}

func (tx *serverTx) initNonInviteFSM() {
	// Define States
	tx.Log().Debug("initialising non-INVITE transaction FSM")

	// Trying
	server_state_def_trying := fsm.State{
		Index: server_state_trying,
		Outcomes: map[fsm.Input]fsm.Outcome{
			server_input_request:       {server_state_trying, fsm.NO_ACTION},
			server_input_cancel:        {server_state_trying, fsm.NO_ACTION},
			server_input_user_1xx:      {server_state_proceeding, tx.act_respond},
			server_input_user_2xx:      {server_state_completed, tx.act_final},
			server_input_user_300_plus: {server_state_completed, tx.act_final},
			server_input_transport_err: {server_state_terminated, tx.act_trans_err},
		},
	}

	// Proceeding
	server_state_def_proceeding := fsm.State{
		Index: server_state_proceeding,
		Outcomes: map[fsm.Input]fsm.Outcome{
			server_input_request:       {server_state_proceeding, tx.act_respond},
			server_input_cancel:        {server_state_proceeding, fsm.NO_ACTION},
			server_input_user_1xx:      {server_state_proceeding, tx.act_respond},
			server_input_user_2xx:      {server_state_completed, tx.act_final},
			server_input_user_300_plus: {server_state_completed, tx.act_final},
			server_input_transport_err: {server_state_terminated, tx.act_trans_err},
		},
	}

	// Completed
	server_state_def_completed := fsm.State{
		Index: server_state_completed,
		Outcomes: map[fsm.Input]fsm.Outcome{
			server_input_request:       {server_state_completed, tx.act_respond},
			server_input_cancel:        {server_state_completed, fsm.NO_ACTION},
			server_input_user_1xx:      {server_state_completed, fsm.NO_ACTION},
			server_input_user_2xx:      {server_state_completed, fsm.NO_ACTION},
			server_input_user_300_plus: {server_state_completed, fsm.NO_ACTION},
			server_input_timer_j:       {server_state_terminated, tx.act_delete},
			server_input_transport_err: {server_state_terminated, tx.act_trans_err},
		},
	}

	// Terminated
	server_state_def_terminated := fsm.State{
		Index: server_state_terminated,
		Outcomes: map[fsm.Input]fsm.Outcome{
			server_input_request:       {server_state_terminated, fsm.NO_ACTION},
			server_input_cancel:        {server_state_terminated, fsm.NO_ACTION},
			server_input_user_1xx:      {server_state_terminated, fsm.NO_ACTION},
			server_input_user_2xx:      {server_state_terminated, fsm.NO_ACTION},
			server_input_user_300_plus: {server_state_terminated, fsm.NO_ACTION},
			server_input_timer_j:       {server_state_terminated, fsm.NO_ACTION},
			server_input_delete:        {server_state_terminated, tx.act_delete},
		},
	}

	// Define FSM
	fsm_, err := fsm.Define(
		server_state_def_trying,
		server_state_def_proceeding,
		server_state_def_completed,
		server_state_def_terminated,
	)
	if err != nil {
		tx.Log().Errorf("define non-INVITE FSM failed: %s", err)

		return
	}

	tx.fsmMu.Lock()
	tx.fsm = fsm_
	tx.fsmMu.Unlock()
}

func (tx *serverTx) transportErr() {
	// todo bloody patch
	defer func() { recover() }()

	var resStr string
	tx.mu.RLock()
	if tx.lastResp != nil {
		resStr = tx.lastResp.Short()
	}
	err := tx.lastErr
	tx.mu.RUnlock()

	err = &TxTransportError{
		fmt.Errorf("transaction failed to send %s: %w", resStr, err),
		tx.Key(),
		fmt.Sprintf("%p", tx),
	}

	select {
	case <-tx.done:
	case tx.errs <- err:
	}
}

func (tx *serverTx) timeoutErr() {
	// todo bloody patch
	defer func() { recover() }()

	err := &TxTimeoutError{
		fmt.Errorf("transaction timed out"),
		tx.Key(),
		fmt.Sprintf("%p", tx),
	}

	select {
	case <-tx.done:
	case tx.errs <- err:
	}
}

func (tx *serverTx) delete() {
	select {
	case <-tx.done:
		return
	default:
	}
	// todo bloody patch
	defer func() { recover() }()

	tx.closeOnce.Do(func() {
		tx.mu.Lock()

		close(tx.done)
		close(tx.acks)
		close(tx.cancels)
		close(tx.errs)

		tx.mu.Unlock()

		tx.Log().Debug("transaction done")
	})

	time.Sleep(time.Microsecond)

	tx.mu.Lock()
	if tx.timer_i != nil {
		tx.timer_i.Stop()
		tx.timer_i = nil
	}
	if tx.timer_g != nil {
		tx.timer_g.Stop()
		tx.timer_g = nil
	}
	if tx.timer_h != nil {
		tx.timer_h.Stop()
		tx.timer_h = nil
	}
	if tx.timer_j != nil {
		tx.timer_j.Stop()
		tx.timer_j = nil
	}
	if tx.timer_l != nil {
		tx.timer_l.Stop()
		tx.timer_l = nil
	}
	if tx.timer_1xx != nil {
		tx.timer_1xx.Stop()
		tx.timer_1xx = nil
	}
	tx.mu.Unlock()
}

// Define actions.
// Send response
func (tx *serverTx) act_respond() fsm.Input {
	tx.mu.RLock()
	lastResp := tx.lastResp
	tx.mu.RUnlock()

	if lastResp == nil {
		return fsm.NO_INPUT
	}

	tx.Log().Debug("act_respond")

	lastErr := tx.tpl.Send(lastResp)

	tx.mu.Lock()
	tx.lastErr = lastErr
	tx.mu.Unlock()

	if lastErr != nil {
		return server_input_transport_err
	}

	return fsm.NO_INPUT
}

func (tx *serverTx) act_respond_complete() fsm.Input {
	tx.mu.RLock()
	lastResp := tx.lastResp
	tx.mu.RUnlock()

	if lastResp == nil {
		return fsm.NO_INPUT
	}

	tx.Log().Debug("act_respond_complete")

	lastErr := tx.tpl.Send(lastResp)

	tx.mu.Lock()
	tx.lastErr = lastErr
	tx.mu.Unlock()

	if lastErr != nil {
		return server_input_transport_err
	}

	if !tx.reliable {
		tx.mu.Lock()
		if tx.timer_g == nil {
			tx.Log().Tracef("timer_g set to %v", tx.timer_g_time)

			tx.timer_g = timing.AfterFunc(tx.timer_g_time, func() {
				select {
				case <-tx.done:
					return
				default:
				}

				tx.Log().Trace("timer_g fired")

				tx.fsmMu.RLock()
				if err := tx.fsm.Spin(server_input_timer_g); err != nil {
					tx.Log().Errorf("spin FSM to server_input_timer_g failed: %s", err)
				}
				tx.fsmMu.RUnlock()
			})
		} else {
			tx.timer_g_time *= 2
			if tx.timer_g_time > T2 {
				tx.timer_g_time = T2
			}

			tx.Log().Tracef("timer_g reset to %v", tx.timer_g_time)

			tx.timer_g.Reset(tx.timer_g_time)
		}
		tx.mu.Unlock()
	}

	tx.mu.Lock()
	if tx.timer_h == nil {
		tx.Log().Tracef("timer_h set to %v", Timer_H)

		tx.timer_h = timing.AfterFunc(Timer_H, func() {
			select {
			case <-tx.done:
				return
			default:
			}

			tx.Log().Trace("timer_h fired")

			tx.fsmMu.RLock()
			if err := tx.fsm.Spin(server_input_timer_h); err != nil {
				tx.Log().Errorf("spin FSM to server_input_timer_h failed: %s", err)
			}
			tx.fsmMu.RUnlock()
		})
	}
	tx.mu.Unlock()

	return fsm.NO_INPUT
}

func (tx *serverTx) act_respond_accept() fsm.Input {
	tx.mu.RLock()
	lastResp := tx.lastResp
	tx.mu.RUnlock()

	if lastResp == nil {
		return fsm.NO_INPUT
	}

	tx.Log().Debug("act_respond_accept")

	lastErr := tx.tpl.Send(lastResp)

	tx.mu.Lock()
	tx.lastErr = lastErr
	tx.mu.Unlock()

	if lastErr != nil {
		return server_input_transport_err
	}

	tx.mu.Lock()
	tx.Log().Tracef("timer_l set to %v", Timer_L)

	tx.timer_l = timing.AfterFunc(Timer_L, func() {
		select {
		case <-tx.done:
			return
		default:
		}

		tx.Log().Trace("timer_l fired")

		tx.fsmMu.RLock()
		if err := tx.fsm.Spin(server_input_timer_l); err != nil {
			tx.Log().Errorf("spin FSM to server_input_timer_l failed: %s", err)
		}
		tx.fsmMu.RUnlock()
	})
	tx.mu.Unlock()

	return fsm.NO_INPUT
}

func (tx *serverTx) act_passup_ack() fsm.Input {
	tx.Log().Debug("act_passup_ack")

	tx.mu.RLock()
	ack := tx.lastAck
	tx.mu.RUnlock()

	if ack != nil {
		select {
		case <-tx.done:
		case tx.acks <- ack:
		}
	}

	return fsm.NO_INPUT
}

// Send final response
func (tx *serverTx) act_final() fsm.Input {
	tx.mu.RLock()
	lastResp := tx.lastResp
	tx.mu.RUnlock()

	if lastResp == nil {
		return fsm.NO_INPUT
	}

	tx.Log().Debug("act_final")

	lastErr := tx.tpl.Send(tx.lastResp)

	tx.mu.Lock()
	tx.lastErr = lastErr
	tx.mu.Unlock()

	if lastErr != nil {
		return server_input_transport_err
	}

	tx.mu.Lock()

	tx.Log().Tracef("timer_j set to %v", Timer_J)

	tx.timer_j = timing.AfterFunc(Timer_J, func() {
		select {
		case <-tx.done:
			return
		default:
		}

		tx.Log().Trace("timer_j fired")

		tx.fsmMu.RLock()
		if err := tx.fsm.Spin(server_input_timer_j); err != nil {
			tx.Log().Errorf("spin FSM to server_input_timer_j failed: %s", err)
		}
		tx.fsmMu.RUnlock()
	})

	tx.mu.Unlock()

	return fsm.NO_INPUT
}

// Inform user of transport error
func (tx *serverTx) act_trans_err() fsm.Input {
	tx.Log().Debug("act_trans_err")

	tx.transportErr()

	return server_input_delete
}

// Inform user of timeout error
func (tx *serverTx) act_timeout() fsm.Input {
	tx.Log().Debug("act_timeout")

	tx.timeoutErr()

	return server_input_delete
}

// Just delete the transaction.
func (tx *serverTx) act_delete() fsm.Input {
	tx.Log().Debug("act_delete")

	tx.delete()

	return fsm.NO_INPUT
}

// Send response and delete the transaction.
func (tx *serverTx) act_respond_delete() fsm.Input {
	tx.Log().Debug("act_respond_delete")

	tx.delete()

	tx.mu.RLock()
	lastErr := tx.tpl.Send(tx.lastResp)
	tx.mu.RUnlock()

	tx.mu.Lock()
	tx.lastErr = lastErr
	tx.mu.Unlock()

	if lastErr != nil {
		return server_input_transport_err
	}

	return fsm.NO_INPUT
}

func (tx *serverTx) act_confirm() fsm.Input {
	tx.Log().Debug("act_confirm")

	// todo bloody patch
	defer func() { recover() }()

	tx.mu.Lock()

	if tx.timer_g != nil {
		tx.timer_g.Stop()
		tx.timer_g = nil
	}

	if tx.timer_h != nil {
		tx.timer_h.Stop()
		tx.timer_h = nil
	}

	tx.Log().Tracef("timer_i set to %v", Timer_I)

	tx.timer_i = timing.AfterFunc(Timer_I, func() {
		select {
		case <-tx.done:
			return
		default:
		}

		tx.Log().Trace("timer_i fired")

		tx.fsmMu.RLock()
		if err := tx.fsm.Spin(server_input_timer_i); err != nil {
			tx.Log().Errorf("spin FSM to server_input_timer_i failed: %s", err)
		}
		tx.fsmMu.RUnlock()
	})

	tx.mu.Unlock()

	tx.mu.RLock()
	ack := tx.lastAck
	tx.mu.RUnlock()

	if ack != nil {
		select {
		case <-tx.done:
		case tx.acks <- ack:
		}
	}

	return fsm.NO_INPUT
}

func (tx *serverTx) act_cancel() fsm.Input {
	tx.Log().Debug("act_cancel")

	// todo bloody patch
	defer func() { recover() }()

	tx.mu.RLock()
	cancel := tx.lastCancel
	tx.mu.RUnlock()

	if cancel != nil {
		select {
		case <-tx.done:
		case tx.cancels <- cancel:
		}
	}

	return fsm.NO_INPUT
}
