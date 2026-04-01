// Copyright 2014 Canonical Ltd.
// Licensed under the LGPLv3, see LICENCE file for details.

package errors

import (
	stderror "errors"
	"fmt"
	"strings"
)

// a ConstError is a prototype for a certain type of error
type ConstError string

// ConstError implements error
func (e ConstError) Error() string {
	return string(e)
}

// Different types of errors
const (
	// Timeout represents an error on timeout.
	Timeout = ConstError("timeout")
	// NotFound represents an error when something has not been found.
	NotFound = ConstError("not found")
	// UserNotFound represents an error when a non-existent user is looked up.
	UserNotFound = ConstError("user not found")
	// Unauthorized represents an error when an operation is unauthorized.
	Unauthorized = ConstError("unauthorized")
	// NotImplemented represents an error when something is not
	// implemented.
	NotImplemented = ConstError("not implemented")
	// AlreadyExists represents and error when something already exists.
	AlreadyExists = ConstError("already exists")
	// NotSupported represents an error when something is not supported.
	NotSupported = ConstError("not supported")
	// NotValid represents an error when something is not valid.
	NotValid = ConstError("not valid")
	// NotProvisioned represents an error when something is not yet provisioned.
	NotProvisioned = ConstError("not provisioned")
	// NotAssigned represents an error when something is not yet assigned to
	// something else.
	NotAssigned = ConstError("not assigned")
	// BadRequest represents an error when a request has bad parameters.
	BadRequest = ConstError("bad request")
	// MethodNotAllowed represents an error when an HTTP request
	// is made with an inappropriate method.
	MethodNotAllowed = ConstError("method not allowed")
	// Forbidden represents an error when a request cannot be completed because of
	// missing privileges.
	Forbidden = ConstError("forbidden")
	// QuotaLimitExceeded is emitted when an action failed due to a quota limit check.
	QuotaLimitExceeded = ConstError("quota limit exceeded")
	// NotYetAvailable is the error returned when a resource is not yet available
	// but it might be in the future.
	NotYetAvailable = ConstError("not yet available")
)

// constSuppressor is a small type wrapper for ConstError to surpress the error
// value from returning an error value. This allows us to maintain backwards
// compatibility.
type constSuppressor ConstError

func (c constSuppressor) Error() string { return "" }

func (c constSuppressor) Unwrap() error { return ConstError(c) }

// errWithType is an Err bundled with its error type (a ConstError)
type errWithType struct {
	error
	errType ConstError
}

// Is compares `target` with e's error type
func (e *errWithType) Is(target error) bool {
	if &e.errType == nil {
		return false
	}
	return target == e.errType
}

// Unwrap an errWithType gives the underlying Err
func (e *errWithType) Unwrap() error {
	return e.error
}

func wrapErrorWithMsg(err error, msg string) error {
	if err == nil {
		return stderror.New(msg)
	}
	if msg == "" {
		return err
	}
	return fmt.Errorf("%s: %w", msg, err)
}

func makeWrappedConstError(err error, format string, args ...interface{}) error {
	separator := " "
	if err.Error() == "" {
		separator = ""
	}
	return fmt.Errorf(strings.Join([]string{format, "%w"}, separator), append(args, err)...)
}

// WithType is responsible for annotating an already existing error so that it
// also satisfies that of a ConstError. The resultant error returned should
// satisfy Is(err, errType). If err is nil then a nil error will also be returned.
//
// Now with Go's Is, As and Unwrap support it no longer makes sense to Wrap()
// 2 errors as both of those errors could be chains of errors in their own right.
// WithType aims to solve some of the usefulness of Wrap with the ability to
// make a pre-existing error also satisfy a ConstError type.
func WithType(err error, errType ConstError) error {
	if err == nil {
		return nil
	}
	return &errWithType{
		error:   err,
		errType: errType,
	}
}

// Timeoutf returns an error which satisfies Is(err, Timeout) and the Locationer
// interface.
func Timeoutf(format string, args ...interface{}) error {
	return newLocationError(
		makeWrappedConstError(Timeout, format, args...),
		1,
	)
}

// NewTimeout returns an error which wraps err and satisfies Is(err, Timeout)
// and the Locationer interface.
func NewTimeout(err error, msg string) error {
	return &errWithType{
		error:   newLocationError(wrapErrorWithMsg(err, msg), 1),
		errType: Timeout,
	}
}

// Deprecated: IsTimeout reports whether err is a  Timeout error. Use
// Is(err, Timeout).
func IsTimeout(err error) bool {
	return Is(err, Timeout)
}

// NotFoundf returns an error which satisfies Is(err, NotFound) and the
// Locationer interface.
func NotFoundf(format string, args ...interface{}) error {
	return newLocationError(
		makeWrappedConstError(NotFound, format, args...),
		1,
	)
}

// NewNotFound returns an error which wraps err and satisfies Is(err, NotFound)
// and the Locationer interface.
func NewNotFound(err error, msg string) error {
	return &errWithType{
		error:   newLocationError(wrapErrorWithMsg(err, msg), 1),
		errType: NotFound,
	}
}

// Deprecated: IsNotFound reports whether err is a NotFound error. Use
// Is(err, NotFound).
func IsNotFound(err error) bool {
	return Is(err, NotFound)
}

// UserNotFoundf returns an error which satisfies Is(err, UserNotFound) and the
// Locationer interface.
func UserNotFoundf(format string, args ...interface{}) error {
	return newLocationError(
		makeWrappedConstError(UserNotFound, format, args...),
		1,
	)
}

// NewUserNotFound returns an error which wraps err and satisfies
// Is(err, UserNotFound) and the Locationer interface.
func NewUserNotFound(err error, msg string) error {
	return &errWithType{
		error:   newLocationError(wrapErrorWithMsg(err, msg), 1),
		errType: UserNotFound,
	}
}

// Deprecated: IsUserNotFound reports whether err is a UserNotFound error. Use
// Is(err, UserNotFound).
func IsUserNotFound(err error) bool {
	return Is(err, UserNotFound)
}

// Unauthorizedf returns an error that satisfies Is(err, Unauthorized) and
// the Locationer interface.
func Unauthorizedf(format string, args ...interface{}) error {
	return newLocationError(
		makeWrappedConstError(constSuppressor(Unauthorized), format, args...),
		1,
	)
}

// NewUnauthorized returns an error which wraps err and satisfies
// Is(err, Unathorized) and the Locationer interface.
func NewUnauthorized(err error, msg string) error {
	return &errWithType{
		error:   newLocationError(wrapErrorWithMsg(err, msg), 1),
		errType: Unauthorized,
	}
}

// Deprecated: IsUnauthorized reports whether err is a Unauthorized error. Use
// Is(err, Unauthorized).
func IsUnauthorized(err error) bool {
	return Is(err, Unauthorized)
}

// NotImplementedf returns an error which satisfies Is(err, NotImplemented) and
// the Locationer interface.
func NotImplementedf(format string, args ...interface{}) error {
	return newLocationError(
		makeWrappedConstError(NotImplemented, format, args...),
		1,
	)
}

// NewNotImplemented returns an error which wraps err and satisfies
// Is(err, NotImplemented) and the Locationer interface.
func NewNotImplemented(err error, msg string) error {
	return &errWithType{
		error:   newLocationError(wrapErrorWithMsg(err, msg), 1),
		errType: NotImplemented,
	}
}

// Deprecated: IsNotImplemented reports whether err is a NotImplemented error.
// Use Is(err, NotImplemented).
func IsNotImplemented(err error) bool {
	return Is(err, NotImplemented)
}

// AlreadyExistsf returns an error which satisfies Is(err, AlreadyExists) and
// the Locationer interface.
func AlreadyExistsf(format string, args ...interface{}) error {
	return newLocationError(
		makeWrappedConstError(AlreadyExists, format, args...),
		1,
	)
}

// NewAlreadyExists returns an error which wraps err and satisfies
// Is(err, AlreadyExists) and the Locationer interface.
func NewAlreadyExists(err error, msg string) error {
	return &errWithType{
		error:   newLocationError(wrapErrorWithMsg(err, msg), 1),
		errType: AlreadyExists,
	}
}

// Deprecated: IsAlreadyExists reports whether the err is a AlreadyExists
// error. Use Is(err, AlreadyExists).
func IsAlreadyExists(err error) bool {
	return Is(err, AlreadyExists)
}

// NotSupportedf returns an error which satisfies Is(err, NotSupported) and the
// Locationer interface.
func NotSupportedf(format string, args ...interface{}) error {
	return newLocationError(
		makeWrappedConstError(NotSupported, format, args...),
		1,
	)
}

// NewNotSupported returns an error which satisfies Is(err, NotSupported) and
// the Locationer interface.
func NewNotSupported(err error, msg string) error {
	return &errWithType{
		error:   newLocationError(wrapErrorWithMsg(err, msg), 1),
		errType: NotSupported,
	}
}

// Deprecated: IsNotSupported reports whether err is a NotSupported error. Use
// Is(err, NotSupported).
func IsNotSupported(err error) bool {
	return Is(err, NotSupported)
}

// NotValidf returns an error which satisfies Is(err, NotValid) and the
// Locationer interface.
func NotValidf(format string, args ...interface{}) error {
	return newLocationError(
		makeWrappedConstError(NotValid, format, args...),
		1,
	)
}

// NewNotValid returns an error which wraps err and satisfies Is(err, NotValid)
// and the Locationer interface.
func NewNotValid(err error, msg string) error {
	return &errWithType{
		error:   newLocationError(wrapErrorWithMsg(err, msg), 1),
		errType: NotValid,
	}
}

// Deprecated: IsNotValid reports whether err is a NotValid error. Use
// Is(err, NotValid).
func IsNotValid(err error) bool {
	return Is(err, NotValid)
}

// NotProvisionedf returns an error which satisfies Is(err, NotProvisioned) and
// the Locationer interface.
func NotProvisionedf(format string, args ...interface{}) error {
	return newLocationError(
		makeWrappedConstError(NotProvisioned, format, args...),
		1,
	)
}

// NewNotProvisioned returns an error which wraps err and satisfies
// Is(err, NotProvisioned) and the Locationer interface.
func NewNotProvisioned(err error, msg string) error {
	return &errWithType{
		error:   newLocationError(wrapErrorWithMsg(err, msg), 1),
		errType: NotProvisioned,
	}
}

// Deprecated: IsNotProvisioned reports whether err is a NotProvisioned error.
// Use Is(err, NotProvisioned).
func IsNotProvisioned(err error) bool {
	return Is(err, NotProvisioned)
}

// NotAssignedf returns an error which satisfies Is(err, NotAssigned) and the
// Locationer interface.
func NotAssignedf(format string, args ...interface{}) error {
	return newLocationError(
		makeWrappedConstError(NotAssigned, format, args...),
		1,
	)
}

// NewNotAssigned returns an error which wraps err and satisfies
// Is(err, NotAssigned) and the Locationer interface.
func NewNotAssigned(err error, msg string) error {
	return &errWithType{
		error:   newLocationError(wrapErrorWithMsg(err, msg), 1),
		errType: NotAssigned,
	}
}

// Deprecated: IsNotAssigned reports whether err is a NotAssigned error.
// Use Is(err, NotAssigned)
func IsNotAssigned(err error) bool {
	return Is(err, NotAssigned)
}

// BadRequestf returns an error which satisfies Is(err, BadRequest) and the
// Locationer interface.
func BadRequestf(format string, args ...interface{}) error {
	return newLocationError(
		makeWrappedConstError(constSuppressor(BadRequest), format, args...),
		1,
	)
}

// NewBadRequest returns an error which wraps err and satisfies
// Is(err, BadRequest) and the Locationer interface.
func NewBadRequest(err error, msg string) error {
	return &errWithType{
		error:   newLocationError(wrapErrorWithMsg(err, msg), 1),
		errType: BadRequest,
	}
}

// Deprecated: IsBadRequest reports whether err is a BadRequest error.
// Use Is(err, BadRequest)
func IsBadRequest(err error) bool {
	return Is(err, BadRequest)
}

// MethodNotAllowedf returns an error which satisfies Is(err, MethodNotAllowed)
// and the Locationer interface.
func MethodNotAllowedf(format string, args ...interface{}) error {
	return newLocationError(
		makeWrappedConstError(constSuppressor(MethodNotAllowed), format, args...),
		1,
	)
}

// NewMethodNotAllowed returns an error which wraps err and satisfies
// Is(err, MethodNotAllowed) and the Locationer interface.
func NewMethodNotAllowed(err error, msg string) error {
	return &errWithType{
		error:   newLocationError(wrapErrorWithMsg(err, msg), 1),
		errType: MethodNotAllowed,
	}
}

// Deprecated: IsMethodNotAllowed reports whether err is a MethodNotAllowed
// error. Use Is(err, MethodNotAllowed)
func IsMethodNotAllowed(err error) bool {
	return Is(err, MethodNotAllowed)
}

// Forbiddenf returns an error which satistifes Is(err, Forbidden) and the
// Locationer interface.
func Forbiddenf(format string, args ...interface{}) error {
	return newLocationError(
		makeWrappedConstError(constSuppressor(Forbidden), format, args...),
		1,
	)
}

// NewForbidden returns an error which wraps err and satisfies
// Is(err, Forbidden) and the Locationer interface.
func NewForbidden(err error, msg string) error {
	return &errWithType{
		error:   newLocationError(wrapErrorWithMsg(err, msg), 1),
		errType: Forbidden,
	}
}

// Deprecated: IsForbidden reports whether err is a Forbidden error. Use
// Is(err, Forbidden).
func IsForbidden(err error) bool {
	return Is(err, Forbidden)
}

// QuotaLimitExceededf returns an error which satisfies
// Is(err, QuotaLimitExceeded) and the Locationer interface.
func QuotaLimitExceededf(format string, args ...interface{}) error {
	return newLocationError(
		makeWrappedConstError(constSuppressor(QuotaLimitExceeded), format, args...),
		1,
	)
}

// NewQuotaLimitExceeded returns an error which wraps err and satisfies
// Is(err, QuotaLimitExceeded) and the Locationer interface.
func NewQuotaLimitExceeded(err error, msg string) error {
	return &errWithType{
		error:   newLocationError(wrapErrorWithMsg(err, msg), 1),
		errType: QuotaLimitExceeded,
	}
}

// Deprecated: IsQuotaLimitExceeded reports whether err is a QuoteLimitExceeded
// err. Use Is(err, QuotaLimitExceeded).
func IsQuotaLimitExceeded(err error) bool {
	return Is(err, QuotaLimitExceeded)
}

// NotYetAvailablef returns an error which satisfies Is(err, NotYetAvailable)
// and the Locationer interface.
func NotYetAvailablef(format string, args ...interface{}) error {
	return newLocationError(
		makeWrappedConstError(constSuppressor(NotYetAvailable), format, args...),
		1,
	)
}

// NewNotYetAvailable returns an error which wraps err and satisfies
// Is(err, NotYetAvailable) and the Locationer interface.
func NewNotYetAvailable(err error, msg string) error {
	return &errWithType{
		error:   newLocationError(wrapErrorWithMsg(err, msg), 1),
		errType: NotYetAvailable,
	}
}

// Deprecated: IsNotYetAvailable reports whether err is a NotYetAvailable err.
// Use Is(err, NotYetAvailable)
func IsNotYetAvailable(err error) bool {
	return Is(err, NotYetAvailable)
}
