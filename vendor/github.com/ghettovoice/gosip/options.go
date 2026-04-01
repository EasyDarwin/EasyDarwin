package gosip

import "github.com/ghettovoice/gosip/sip"

type RequestWithContextOption interface {
	ApplyRequestWithContext(options *RequestWithContextOptions)
}

type RequestWithContextOptions struct {
	ResponseHandler func(res sip.Response, request sip.Request)
	Authorizer      sip.Authorizer
	ClientTransactionCallbacks
}

type ClientTransactionCallbacks struct {
	OnAck, OnCancel func(sip.Request)
}

type withResponseHandler struct {
	handler func(res sip.Response, request sip.Request)
}

func (o withResponseHandler) ApplyRequestWithContext(options *RequestWithContextOptions) {
	options.ResponseHandler = o.handler
}

func WithResponseHandler(handler func(res sip.Response, request sip.Request)) RequestWithContextOption {
	return withResponseHandler{handler}
}

type withAuthorizer struct {
	authorizer sip.Authorizer
}

func (o withAuthorizer) ApplyRequestWithContext(options *RequestWithContextOptions) {
	options.Authorizer = o.authorizer
}

func WithAuthorizer(authorizer sip.Authorizer) RequestWithContextOption {
	return withAuthorizer{authorizer}
}

type withClientTxCallbacks struct {
	onAckFn, onCancFn func(sip.Request)
}

func WithClientTransactionCallbacks(callbacks ClientTransactionCallbacks) RequestWithContextOption {
	return withClientTxCallbacks{callbacks.OnAck, callbacks.OnCancel}
}

func (o withClientTxCallbacks) ApplyRequestWithContext(options *RequestWithContextOptions) {
	options.OnAck = o.onAckFn
	options.OnCancel = o.onCancFn
}
