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

type ClientTx interface {
	Tx
	Responses() <-chan sip.Response
	Cancel() error

	OnAck(fn func(sip.Request))
	OnCancel(fn func(sip.Request))
}

type clientTx struct {
	commonTx
	responses    chan sip.Response
	timer_a_time time.Duration // Current duration of timer A.
	timer_a      timing.Timer
	timer_b      timing.Timer
	timer_d_time time.Duration // Current duration of timer D.
	timer_d      timing.Timer
	timer_m      timing.Timer
	reliable     bool

	mu        sync.RWMutex
	closeOnce sync.Once

	onAckFn, onCancFn func(sip.Request)
}

func NewClientTx(origin sip.Request, tpl sip.Transport, logger log.Logger) (ClientTx, error) {
	origin = prepareClientRequest(origin)
	key, err := MakeClientTxKey(origin)
	if err != nil {
		return nil, err
	}

	tx := new(clientTx)
	tx.key = key
	tx.tpl = tpl
	// buffer chan - about ~10 retransmit responses
	tx.responses = make(chan sip.Response, 64)
	tx.errs = make(chan error, 64)
	tx.done = make(chan bool)
	tx.log = logger.
		WithPrefix("transaction.ClientTx").
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

func prepareClientRequest(origin sip.Request) sip.Request {
	if viaHop, ok := origin.ViaHop(); ok {
		if viaHop.Params == nil {
			viaHop.Params = sip.NewParams()
		}
		if !viaHop.Params.Has("branch") {
			viaHop.Params.Add("branch", sip.String{Str: sip.GenerateBranch()})
		}
	} else {
		viaHop = &sip.ViaHop{
			ProtocolName:    "SIP",
			ProtocolVersion: "2.0",
			Params: sip.NewParams().
				Add("branch", sip.String{Str: sip.GenerateBranch()}),
		}

		origin.PrependHeader(sip.ViaHeader{viaHop})
	}

	return origin
}

func (tx *clientTx) Init() error {
	tx.initFSM()

	if err := tx.tpl.Send(tx.Origin()); err != nil {
		tx.mu.Lock()
		tx.lastErr = err
		tx.mu.Unlock()

		tx.fsmMu.RLock()
		if err := tx.fsm.Spin(client_input_transport_err); err != nil {
			tx.Log().Errorf("spin FSM to client_input_transport_err failed: %s", err)
		}
		tx.fsmMu.RUnlock()

		return err
	}

	if tx.reliable {
		tx.mu.Lock()
		tx.timer_d_time = 0
		tx.mu.Unlock()
	} else {
		// RFC 3261 - 17.1.1.2.
		// If an unreliable transport is being used, the client transaction MUST start timer A with a value of T1.
		// If a reliable transport is being used, the client transaction SHOULD NOT
		// start timer A (Timer A controls request retransmissions).
		// Timer A - retransmission
		tx.Log().Tracef("timer_a set to %v", Timer_A)

		tx.mu.Lock()
		tx.timer_a_time = Timer_A

		tx.timer_a = timing.AfterFunc(tx.timer_a_time, func() {
			select {
			case <-tx.done:
				return
			default:
			}

			tx.Log().Trace("timer_a fired")

			tx.fsmMu.RLock()
			if err := tx.fsm.Spin(client_input_timer_a); err != nil {
				tx.Log().Errorf("spin FSM to client_input_timer_a failed: %s", err)
			}
			tx.fsmMu.RUnlock()
		})
		// Timer D is set to 32 seconds for unreliable transports
		tx.timer_d_time = Timer_D
		tx.mu.Unlock()
	}

	// Timer B - timeout
	tx.Log().Tracef("timer_b set to %v", Timer_B)

	tx.mu.Lock()
	tx.timer_b = timing.AfterFunc(Timer_B, func() {
		select {
		case <-tx.done:
			return
		default:
		}

		tx.Log().Trace("timer_b fired")

		tx.fsmMu.RLock()
		if err := tx.fsm.Spin(client_input_timer_b); err != nil {
			tx.Log().Errorf("spin FSM to client_input_timer_b failed: %s", err)
		}
		tx.fsmMu.RUnlock()
	})
	tx.mu.Unlock()

	tx.mu.RLock()
	err := tx.lastErr
	tx.mu.RUnlock()

	return err
}

func (tx *clientTx) Receive(msg sip.Message) error {
	res, ok := msg.(sip.Response)
	if !ok {
		return &sip.UnexpectedMessageError{
			Err: fmt.Errorf("%s recevied unexpected %s", tx, msg.Short()),
			Msg: msg.String(),
		}
	}

	res = res.WithFields(log.Fields{
		"request_id": tx.origin.MessageID(),
	}).(sip.Response)

	var input fsm.Input
	if res.IsCancel() {
		input = client_input_canceled
	} else {
		tx.mu.Lock()
		tx.lastResp = res
		tx.mu.Unlock()

		switch {
		case res.IsProvisional():
			input = client_input_1xx
		case res.IsSuccess():
			input = client_input_2xx
		default:
			input = client_input_300_plus
		}
	}

	tx.fsmMu.RLock()
	defer tx.fsmMu.RUnlock()

	return tx.fsm.Spin(input)
}

func (tx *clientTx) Responses() <-chan sip.Response {
	return tx.responses
}

func (tx *clientTx) Cancel() error {
	tx.fsmMu.RLock()
	defer tx.fsmMu.RUnlock()

	return tx.fsm.Spin(client_input_cancel)
}

func (tx *clientTx) Terminate() {
	select {
	case <-tx.done:
		return
	default:
	}

	tx.delete()
}

func (tx *clientTx) cancel() {
	if !tx.Origin().IsInvite() {
		return
	}

	tx.mu.RLock()
	lastResp := tx.lastResp
	onCanc := tx.onCancFn
	tx.mu.RUnlock()

	cancelRequest := sip.NewCancelRequest("", tx.Origin(), log.Fields{
		"sent_at": time.Now(),
	})
	if onCanc != nil {
		onCanc(cancelRequest)
	}
	if err := tx.tpl.Send(cancelRequest); err != nil {
		var lastRespStr string
		if lastResp != nil {
			lastRespStr = lastResp.Short()
		}
		tx.Log().WithFields(map[string]interface{}{
			"invite_request":  tx.Origin().Short(),
			"invite_response": lastRespStr,
			"cancel_request":  cancelRequest.Short(),
		}).Errorf("send CANCEL request failed: %s", err)

		tx.mu.Lock()
		tx.lastErr = err
		tx.mu.Unlock()

		go func() {
			tx.fsmMu.RLock()
			if err := tx.fsm.Spin(client_input_transport_err); err != nil {
				tx.Log().Errorf("spin FSM to client_input_transport_err failed: %s", err)
			}
			tx.fsmMu.RUnlock()
		}()
	}
}

func (tx *clientTx) ack() {
	tx.mu.RLock()
	lastResp := tx.lastResp
	onAck := tx.onAckFn
	tx.mu.RUnlock()

	ack := sip.NewAckRequest("", tx.Origin(), lastResp, "", log.Fields{
		"sent_at": time.Now(),
	})
	if onAck != nil {
		onAck(ack)
	}
	err := tx.tpl.Send(ack)
	if err != nil {
		tx.Log().WithFields(log.Fields{
			"invite_request":  tx.Origin().Short(),
			"invite_response": lastResp.Short(),
			"ack_request":     ack.Short(),
		}).Errorf("send ACK request failed: %s", err)

		tx.mu.Lock()
		tx.lastErr = err
		tx.mu.Unlock()

		go func() {
			tx.fsmMu.RLock()
			if err := tx.fsm.Spin(client_input_transport_err); err != nil {
				tx.Log().Errorf("spin FSM to client_input_transport_err failed: %s", err)
			}
			tx.fsmMu.RUnlock()
		}()
	}
}

// FSM States
const (
	client_state_calling = iota
	client_state_proceeding
	client_state_completed
	client_state_accepted
	client_state_terminated
)

// FSM Inputs
const (
	client_input_1xx fsm.Input = iota
	client_input_2xx
	client_input_300_plus
	client_input_timer_a
	client_input_timer_b
	client_input_timer_d
	client_input_timer_m
	client_input_transport_err
	client_input_delete
	client_input_cancel
	client_input_canceled
)

// Initialises the correct kind of FSM based on request method.
func (tx *clientTx) initFSM() {
	if tx.Origin().IsInvite() {
		tx.initInviteFSM()
	} else {
		tx.initNonInviteFSM()
	}
}

func (tx *clientTx) initInviteFSM() {
	tx.Log().Debug("initialising INVITE transaction FSM")

	// Define States
	// Calling
	client_state_def_calling := fsm.State{
		Index: client_state_calling,
		Outcomes: map[fsm.Input]fsm.Outcome{
			client_input_1xx:           {client_state_proceeding, tx.act_invite_proceeding},
			client_input_2xx:           {client_state_accepted, tx.act_passup_accept},
			client_input_300_plus:      {client_state_completed, tx.act_invite_final},
			client_input_cancel:        {client_state_calling, tx.act_cancel},
			client_input_canceled:      {client_state_calling, tx.act_invite_canceled},
			client_input_timer_a:       {client_state_calling, tx.act_invite_resend},
			client_input_timer_b:       {client_state_terminated, tx.act_timeout},
			client_input_transport_err: {client_state_terminated, tx.act_trans_err},
		},
	}

	// Proceeding
	client_state_def_proceeding := fsm.State{
		Index: client_state_proceeding,
		Outcomes: map[fsm.Input]fsm.Outcome{
			client_input_1xx:           {client_state_proceeding, tx.act_passup},
			client_input_2xx:           {client_state_accepted, tx.act_passup_accept},
			client_input_300_plus:      {client_state_completed, tx.act_invite_final},
			client_input_cancel:        {client_state_proceeding, tx.act_cancel_timeout},
			client_input_canceled:      {client_state_proceeding, tx.act_invite_canceled},
			client_input_timer_a:       {client_state_proceeding, fsm.NO_ACTION},
			client_input_timer_b:       {client_state_terminated, tx.act_timeout},
			client_input_transport_err: {client_state_terminated, tx.act_trans_err},
		},
	}

	// Completed
	client_state_def_completed := fsm.State{
		Index: client_state_completed,
		Outcomes: map[fsm.Input]fsm.Outcome{
			client_input_1xx:           {client_state_completed, fsm.NO_ACTION},
			client_input_2xx:           {client_state_completed, fsm.NO_ACTION},
			client_input_300_plus:      {client_state_completed, tx.act_ack},
			client_input_cancel:        {client_state_completed, fsm.NO_ACTION},
			client_input_canceled:      {client_state_completed, fsm.NO_ACTION},
			client_input_transport_err: {client_state_terminated, tx.act_trans_err},
			client_input_timer_a:       {client_state_completed, fsm.NO_ACTION},
			client_input_timer_b:       {client_state_completed, fsm.NO_ACTION},
			client_input_timer_d:       {client_state_terminated, tx.act_delete},
		},
	}

	client_state_def_accepted := fsm.State{
		Index: client_state_accepted,
		Outcomes: map[fsm.Input]fsm.Outcome{
			client_input_1xx:      {client_state_accepted, fsm.NO_ACTION},
			client_input_2xx:      {client_state_accepted, tx.act_passup},
			client_input_300_plus: {client_state_accepted, fsm.NO_ACTION},
			client_input_cancel:   {client_state_accepted, fsm.NO_ACTION},
			client_input_canceled: {client_state_accepted, fsm.NO_ACTION},
			client_input_transport_err: {client_state_accepted, func() fsm.Input {
				tx.act_trans_err()
				return fsm.NO_INPUT
			}},
			client_input_timer_a: {client_state_accepted, fsm.NO_ACTION},
			client_input_timer_b: {client_state_accepted, fsm.NO_ACTION},
			client_input_timer_m: {client_state_terminated, tx.act_delete},
		},
	}

	// Terminated
	client_state_def_terminated := fsm.State{
		Index: client_state_terminated,
		Outcomes: map[fsm.Input]fsm.Outcome{
			client_input_1xx:           {client_state_terminated, fsm.NO_ACTION},
			client_input_2xx:           {client_state_terminated, fsm.NO_ACTION},
			client_input_300_plus:      {client_state_terminated, fsm.NO_ACTION},
			client_input_cancel:        {client_state_terminated, fsm.NO_ACTION},
			client_input_canceled:      {client_state_terminated, fsm.NO_ACTION},
			client_input_timer_a:       {client_state_terminated, fsm.NO_ACTION},
			client_input_timer_b:       {client_state_terminated, fsm.NO_ACTION},
			client_input_timer_d:       {client_state_terminated, fsm.NO_ACTION},
			client_input_timer_m:       {client_state_terminated, fsm.NO_ACTION},
			client_input_delete:        {client_state_terminated, tx.act_delete},
			client_input_transport_err: {client_state_terminated, fsm.NO_ACTION},
		},
	}

	fsm_, err := fsm.Define(
		client_state_def_calling,
		client_state_def_proceeding,
		client_state_def_completed,
		client_state_def_accepted,
		client_state_def_terminated,
	)

	if err != nil {
		tx.Log().Errorf("define INVITE transaction FSM failed: %s", err)

		return
	}

	tx.fsmMu.Lock()
	tx.fsm = fsm_
	tx.fsmMu.Unlock()
}

func (tx *clientTx) initNonInviteFSM() {
	tx.Log().Debug("initialising non-INVITE transaction FSM")

	// Define States
	// "Trying"
	client_state_def_calling := fsm.State{
		Index: client_state_calling,
		Outcomes: map[fsm.Input]fsm.Outcome{
			client_input_1xx:           {client_state_proceeding, tx.act_passup},
			client_input_2xx:           {client_state_completed, tx.act_non_invite_final},
			client_input_300_plus:      {client_state_completed, tx.act_non_invite_final},
			client_input_timer_a:       {client_state_calling, tx.act_non_invite_resend},
			client_input_timer_b:       {client_state_terminated, tx.act_timeout},
			client_input_transport_err: {client_state_terminated, tx.act_trans_err},
			client_input_cancel:        {client_state_calling, fsm.NO_ACTION},
		},
	}

	// Proceeding
	client_state_def_proceeding := fsm.State{
		Index: client_state_proceeding,
		Outcomes: map[fsm.Input]fsm.Outcome{
			client_input_1xx:           {client_state_proceeding, tx.act_passup},
			client_input_2xx:           {client_state_completed, tx.act_non_invite_final},
			client_input_300_plus:      {client_state_completed, tx.act_non_invite_final},
			client_input_timer_a:       {client_state_proceeding, tx.act_non_invite_resend},
			client_input_timer_b:       {client_state_terminated, tx.act_timeout},
			client_input_transport_err: {client_state_terminated, tx.act_trans_err},
			client_input_cancel:        {client_state_proceeding, fsm.NO_ACTION},
		},
	}

	// Completed
	client_state_def_completed := fsm.State{
		Index: client_state_completed,
		Outcomes: map[fsm.Input]fsm.Outcome{
			client_input_1xx:      {client_state_completed, fsm.NO_ACTION},
			client_input_2xx:      {client_state_completed, fsm.NO_ACTION},
			client_input_300_plus: {client_state_completed, fsm.NO_ACTION},
			client_input_timer_a:  {client_state_completed, fsm.NO_ACTION},
			client_input_timer_b:  {client_state_completed, fsm.NO_ACTION},
			client_input_timer_d:  {client_state_terminated, tx.act_delete},
			client_input_cancel:   {client_state_completed, fsm.NO_ACTION},
		},
	}

	// Terminated
	client_state_def_terminated := fsm.State{
		Index: client_state_terminated,
		Outcomes: map[fsm.Input]fsm.Outcome{
			client_input_1xx:      {client_state_terminated, fsm.NO_ACTION},
			client_input_2xx:      {client_state_terminated, fsm.NO_ACTION},
			client_input_300_plus: {client_state_terminated, fsm.NO_ACTION},
			client_input_timer_a:  {client_state_terminated, fsm.NO_ACTION},
			client_input_timer_b:  {client_state_terminated, fsm.NO_ACTION},
			client_input_timer_d:  {client_state_terminated, fsm.NO_ACTION},
			client_input_delete:   {client_state_terminated, tx.act_delete},
			client_input_cancel:   {client_state_terminated, fsm.NO_ACTION},
		},
	}

	fsm_, err := fsm.Define(
		client_state_def_calling,
		client_state_def_proceeding,
		client_state_def_completed,
		client_state_def_terminated,
	)

	if err != nil {
		tx.Log().Errorf("define non-INVITE transaction FSM failed: %s", err)

		return
	}

	tx.fsmMu.Lock()
	tx.fsm = fsm_
	tx.fsmMu.Unlock()
}

func (tx *clientTx) resend() {
	select {
	case <-tx.done:
		return
	default:
	}

	tx.Log().Debug("resend origin request")

	err := tx.tpl.Send(tx.Origin())

	tx.mu.Lock()
	tx.lastErr = err
	tx.mu.Unlock()

	if err != nil {
		go func() {
			tx.fsmMu.RLock()
			if err := tx.fsm.Spin(client_input_transport_err); err != nil {
				tx.Log().Errorf("spin FSM to client_input_transport_err failed: %s", err)
			}
			tx.fsmMu.RUnlock()
		}()
	}
}

func (tx *clientTx) passUp() {
	tx.mu.RLock()
	lastResp := tx.lastResp
	tx.mu.RUnlock()

	if lastResp != nil {
		select {
		case <-tx.done:
		case tx.responses <- lastResp:
		}
	}
}

func (tx *clientTx) transportErr() {
	// todo bloody patch
	defer func() { recover() }()

	tx.mu.RLock()
	err := tx.lastErr
	tx.mu.RUnlock()

	err = &TxTransportError{
		fmt.Errorf("transaction failed to send %s: %w", tx.origin.Short(), err),
		tx.Key(),
		fmt.Sprintf("%p", tx),
	}

	select {
	case <-tx.done:
	case tx.errs <- err:
	}
}

func (tx *clientTx) timeoutErr() {
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

func (tx *clientTx) delete() {
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
		close(tx.responses)
		close(tx.errs)

		tx.mu.Unlock()

		tx.Log().Debug("transaction done")
	})

	time.Sleep(time.Microsecond)

	tx.mu.Lock()
	if tx.timer_a != nil {
		tx.timer_a.Stop()
		tx.timer_a = nil
	}
	if tx.timer_b != nil {
		tx.timer_b.Stop()
		tx.timer_b = nil
	}
	if tx.timer_d != nil {
		tx.timer_d.Stop()
		tx.timer_d = nil
	}
	if tx.timer_m != nil {
		tx.timer_m.Stop()
		tx.timer_m = nil
	}
	tx.mu.Unlock()
}

func (tx *clientTx) OnAck(fn func(sip.Request)) {
	tx.mu.Lock()
	tx.onAckFn = fn
	tx.mu.Unlock()
}

func (tx *clientTx) OnCancel(fn func(sip.Request)) {
	tx.mu.Lock()
	tx.onCancFn = fn
	tx.mu.Unlock()
}

// Define actions
func (tx *clientTx) act_invite_resend() fsm.Input {
	tx.Log().Debug("act_invite_resend")

	tx.mu.Lock()

	tx.timer_a_time *= 2
	tx.timer_a.Reset(tx.timer_a_time)

	tx.mu.Unlock()

	tx.resend()

	return fsm.NO_INPUT
}

func (tx *clientTx) act_invite_canceled() fsm.Input {
	tx.Log().Debug("act_invite_canceled")

	// nothing to do here for now

	return fsm.NO_INPUT
}

func (tx *clientTx) act_non_invite_resend() fsm.Input {
	tx.Log().Debug("act_non_invite_resend")

	tx.mu.Lock()

	tx.timer_a_time *= 2
	// For non-INVITE, cap timer A at T2 seconds.
	if tx.timer_a_time > T2 {
		tx.timer_a_time = T2
	}
	tx.timer_a.Reset(tx.timer_a_time)

	tx.mu.Unlock()

	tx.resend()

	return fsm.NO_INPUT
}

func (tx *clientTx) act_passup() fsm.Input {
	tx.Log().Debug("act_passup")

	tx.passUp()

	tx.mu.Lock()

	if tx.timer_a != nil {
		tx.timer_a.Stop()
		tx.timer_a = nil
	}

	tx.mu.Unlock()

	return fsm.NO_INPUT
}

func (tx *clientTx) act_invite_proceeding() fsm.Input {
	tx.Log().Debug("act_invite_proceeding")

	tx.passUp()

	tx.mu.Lock()

	if tx.timer_a != nil {
		tx.timer_a.Stop()
		tx.timer_a = nil
	}
	if tx.timer_b != nil {
		tx.timer_b.Stop()
		tx.timer_b = nil
	}

	tx.mu.Unlock()

	return fsm.NO_INPUT
}

func (tx *clientTx) act_invite_final() fsm.Input {
	tx.Log().Debug("act_invite_final")

	tx.ack()
	tx.passUp()

	tx.mu.Lock()

	if tx.timer_a != nil {
		tx.timer_a.Stop()
		tx.timer_a = nil
	}
	if tx.timer_b != nil {
		tx.timer_b.Stop()
		tx.timer_b = nil
	}

	tx.Log().Tracef("timer_d set to %v", tx.timer_d_time)

	tx.timer_d = timing.AfterFunc(tx.timer_d_time, func() {
		select {
		case <-tx.done:
			return
		default:
		}

		tx.Log().Trace("timer_d fired")

		tx.fsmMu.RLock()
		if err := tx.fsm.Spin(client_input_timer_d); err != nil {
			tx.Log().Errorf("spin FSM to client_input_timer_d failed: %s", err)
		}
		tx.fsmMu.RUnlock()
	})

	tx.mu.Unlock()

	return fsm.NO_INPUT
}

func (tx *clientTx) act_non_invite_final() fsm.Input {
	tx.Log().Debug("act_non_invite_final")

	tx.passUp()

	tx.mu.Lock()

	if tx.timer_a != nil {
		tx.timer_a.Stop()
		tx.timer_a = nil
	}
	if tx.timer_b != nil {
		tx.timer_b.Stop()
		tx.timer_b = nil
	}

	tx.Log().Tracef("timer_d set to %v", tx.timer_d_time)

	tx.timer_d = timing.AfterFunc(tx.timer_d_time, func() {
		select {
		case <-tx.done:
			return
		default:
		}

		tx.Log().Trace("timer_d fired")

		tx.fsmMu.RLock()
		if err := tx.fsm.Spin(client_input_timer_d); err != nil {
			tx.Log().Errorf("spin FSM to client_input_timer_d failed: %s", err)
		}
		tx.fsmMu.RUnlock()
	})

	tx.mu.Unlock()

	return fsm.NO_INPUT
}

func (tx *clientTx) act_cancel() fsm.Input {
	tx.Log().Debug("act_cancel")

	tx.cancel()

	return fsm.NO_INPUT
}

func (tx *clientTx) act_cancel_timeout() fsm.Input {
	tx.Log().Debug("act_cancel")

	tx.cancel()

	tx.Log().Tracef("timer_b set to %v", Timer_B)

	tx.mu.Lock()
	if tx.timer_b != nil {
		tx.timer_b.Stop()
	}
	tx.timer_b = timing.AfterFunc(Timer_B, func() {
		select {
		case <-tx.done:
			return
		default:
		}

		tx.Log().Trace("timer_b fired")

		tx.fsmMu.RLock()
		if err := tx.fsm.Spin(client_input_timer_b); err != nil {
			tx.Log().Errorf("spin FSM to client_input_timer_b failed: %s", err)
		}
		tx.fsmMu.RUnlock()
	})
	tx.mu.Unlock()

	return fsm.NO_INPUT
}

func (tx *clientTx) act_ack() fsm.Input {
	tx.Log().Debug("act_ack")

	tx.ack()

	return fsm.NO_INPUT
}

func (tx *clientTx) act_trans_err() fsm.Input {
	tx.Log().Debug("act_trans_err")

	tx.transportErr()

	tx.mu.Lock()

	if tx.timer_a != nil {
		tx.timer_a.Stop()
		tx.timer_a = nil
	}

	tx.mu.Unlock()

	return client_input_delete
}

func (tx *clientTx) act_timeout() fsm.Input {
	tx.Log().Debug("act_timeout")

	tx.timeoutErr()

	tx.mu.Lock()

	if tx.timer_a != nil {
		tx.timer_a.Stop()
		tx.timer_a = nil
	}

	tx.mu.Unlock()

	return client_input_delete
}

func (tx *clientTx) act_passup_delete() fsm.Input {
	tx.Log().Debug("act_passup_delete")

	tx.passUp()

	tx.mu.Lock()

	if tx.timer_a != nil {
		tx.timer_a.Stop()
		tx.timer_a = nil
	}

	tx.mu.Unlock()

	return client_input_delete
}

func (tx *clientTx) act_passup_accept() fsm.Input {
	tx.Log().Debug("act_passup_accept")

	tx.passUp()

	tx.mu.Lock()

	if tx.timer_a != nil {
		tx.timer_a.Stop()
		tx.timer_a = nil
	}
	if tx.timer_b != nil {
		tx.timer_b.Stop()
		tx.timer_b = nil
	}

	tx.Log().Tracef("timer_m set to %v", Timer_M)

	tx.timer_m = timing.AfterFunc(Timer_M, func() {
		select {
		case <-tx.done:
			return
		default:
		}

		tx.Log().Trace("timer_m fired")

		tx.fsmMu.RLock()
		if err := tx.fsm.Spin(client_input_timer_m); err != nil {
			tx.Log().Errorf("spin FSM to client_input_timer_m failed: %s", err)
		}
		tx.fsmMu.RUnlock()
	})
	tx.mu.Unlock()

	return fsm.NO_INPUT
}

func (tx *clientTx) act_delete() fsm.Input {
	tx.Log().Debug("act_delete")

	tx.delete()

	return fsm.NO_INPUT
}
