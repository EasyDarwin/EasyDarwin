// transaction package implements SIP Transaction Layer
package transaction

import (
	"fmt"
	"strings"
	"time"

	"github.com/ghettovoice/gosip/log"
	"github.com/ghettovoice/gosip/sip"
)

const (
	T1        = 500 * time.Millisecond
	T2        = 4 * time.Second
	T4        = 5 * time.Second
	Timer_A   = T1
	Timer_B   = 64 * T1
	Timer_D   = 32 * time.Second
	Timer_E   = T1
	Timer_F   = 64 * T1
	Timer_G   = T1
	Timer_H   = 64 * T1
	Timer_I   = T4
	Timer_J   = 64 * T1
	Timer_K   = T4
	Timer_1xx = 200 * time.Millisecond
	Timer_L   = 64 * T1
	Timer_M   = 64 * T1
)

type TxError interface {
	error
	Key() TxKey
	Terminated() bool
	Timeout() bool
	Transport() bool
}

type TxTerminatedError struct {
	Err   error
	TxKey TxKey
	TxPtr string
}

func (err *TxTerminatedError) Unwrap() error    { return err.Err }
func (err *TxTerminatedError) Terminated() bool { return true }
func (err *TxTerminatedError) Timeout() bool    { return false }
func (err *TxTerminatedError) Transport() bool  { return false }
func (err *TxTerminatedError) Key() TxKey       { return err.TxKey }
func (err *TxTerminatedError) Error() string {
	if err == nil {
		return "<nil>"
	}

	fields := log.Fields{
		"transaction_key": "???",
		"transaction_ptr": "???",
	}

	if err.TxKey != "" {
		fields["transaction_key"] = err.TxKey
	}
	if err.TxPtr != "" {
		fields["transaction_ptr"] = err.TxPtr
	}

	return fmt.Sprintf("transaction.TxTerminatedError<%s>: %s", fields, err.Err)
}

type TxTimeoutError struct {
	Err   error
	TxKey TxKey
	TxPtr string
}

func (err *TxTimeoutError) Unwrap() error    { return err.Err }
func (err *TxTimeoutError) Terminated() bool { return false }
func (err *TxTimeoutError) Timeout() bool    { return true }
func (err *TxTimeoutError) Transport() bool  { return false }
func (err *TxTimeoutError) Key() TxKey       { return err.TxKey }
func (err *TxTimeoutError) Error() string {
	if err == nil {
		return "<nil>"
	}

	fields := log.Fields{
		"transaction_key": "???",
		"transaction_ptr": "???",
	}

	if err.TxKey != "" {
		fields["transaction_key"] = err.TxKey
	}
	if err.TxPtr != "" {
		fields["transaction_ptr"] = err.TxPtr
	}

	return fmt.Sprintf("transaction.TxTimeoutError<%s>: %s", fields, err.Err)
}

type TxTransportError struct {
	Err   error
	TxKey TxKey
	TxPtr string
}

func (err *TxTransportError) Unwrap() error    { return err.Err }
func (err *TxTransportError) Terminated() bool { return false }
func (err *TxTransportError) Timeout() bool    { return false }
func (err *TxTransportError) Transport() bool  { return true }
func (err *TxTransportError) Key() TxKey       { return err.TxKey }
func (err *TxTransportError) Error() string {
	if err == nil {
		return "<nil>"
	}

	fields := log.Fields{
		"transaction_key": "???",
		"transaction_ptr": "???",
	}

	if err.TxKey != "" {
		fields["transaction_key"] = err.TxKey
	}
	if err.TxPtr != "" {
		fields["transaction_ptr"] = err.TxPtr
	}

	return fmt.Sprintf("transaction.TxTransportError<%s>: %s", fields, err.Err)
}

// MakeServerTxKey creates server commonTx key for matching retransmitting requests - RFC 3261 17.2.3.
func MakeServerTxKey(msg sip.Message) (TxKey, error) {
	var sep = "__"

	firstViaHop, ok := msg.ViaHop()
	if !ok {
		return "", fmt.Errorf("'Via' header not found or empty in message '%s'", msg.Short())
	}

	cseq, ok := msg.CSeq()
	if !ok {
		return "", fmt.Errorf("'CSeq' header not found in message '%s'", msg.Short())
	}
	method := cseq.MethodName
	if method == sip.ACK || method == sip.CANCEL {
		method = sip.INVITE
	}

	var isRFC3261 bool
	branch, ok := firstViaHop.Params.Get("branch")
	if ok && branch.String() != "" &&
		strings.HasPrefix(branch.String(), sip.RFC3261BranchMagicCookie) &&
		strings.TrimPrefix(branch.String(), sip.RFC3261BranchMagicCookie) != "" {

		isRFC3261 = true
	} else {
		isRFC3261 = false
	}

	// RFC 3261 compliant
	if isRFC3261 {
		var port sip.Port

		if firstViaHop.Port == nil {
			port = sip.DefaultPort(firstViaHop.Transport)
		} else {
			port = *firstViaHop.Port
		}

		return TxKey(strings.Join([]string{
			branch.String(),  // branch
			firstViaHop.Host, // sent-by Host
			fmt.Sprint(port), // sent-by Port
			string(method),   // request Method
		}, sep)), nil
	}
	// RFC 2543 compliant
	from, ok := msg.From()
	if !ok {
		return "", fmt.Errorf("'From' header not found in message '%s'", msg.Short())
	}
	fromTag, ok := from.Params.Get("tag")
	if !ok {
		return "", fmt.Errorf("'tag' param not found in 'From' header of message '%s'", msg.Short())
	}
	callId, ok := msg.CallID()
	if !ok {
		return "", fmt.Errorf("'Call-ID' header not found in message '%s'", msg.Short())
	}

	return TxKey(strings.Join([]string{
		// TODO: how to match core.Response in Send method to server tx? currently disabled
		// msg.Recipient().String(), // request-uri
		fromTag.String(),       // from tag
		callId.String(),        // Call-ID
		string(method),         // cseq method
		fmt.Sprint(cseq.SeqNo), // cseq num
		firstViaHop.String(),   // top Via
	}, sep)), nil
}

// MakeClientTxKey creates client commonTx key for matching responses - RFC 3261 17.1.3.
func MakeClientTxKey(msg sip.Message) (TxKey, error) {
	var sep = "__"

	cseq, ok := msg.CSeq()
	if !ok {
		return "", fmt.Errorf("'CSeq' header not found in message '%s'", msg.Short())
	}
	method := cseq.MethodName
	if method == sip.ACK || method == sip.CANCEL {
		method = sip.INVITE
	}

	firstViaHop, ok := msg.ViaHop()
	if !ok {
		return "", fmt.Errorf("'Via' header not found or empty in message '%s'", msg.Short())
	}

	branch, ok := firstViaHop.Params.Get("branch")
	if !ok || len(branch.String()) == 0 ||
		!strings.HasPrefix(branch.String(), sip.RFC3261BranchMagicCookie) ||
		len(strings.TrimPrefix(branch.String(), sip.RFC3261BranchMagicCookie)) == 0 {
		return "", fmt.Errorf("'branch' not found or empty in 'Via' header of message '%s'", msg.Short())
	}

	return TxKey(strings.Join([]string{
		branch.String(),
		string(method),
	}, sep)), nil
}
