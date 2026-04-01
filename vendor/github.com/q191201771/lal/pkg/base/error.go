// Copyright 2021, Chef.  All rights reserved.
// https://github.com/q191201771/lal
//
// Use of this source code is governed by a MIT-style license
// that can be found in the License file.
//
// Author: Chef (191201771@qq.com)

package base

import (
	"errors"
	"fmt"
)

// ----- 通用的 ---------------------------------------------------------------------------------------------------------

var (
	ErrShortBuffer  = errors.New("lal: buffer too short")
	ErrFileNotExist = errors.New("lal: file not exist")
)

// ----- pkg/aac -------------------------------------------------------------------------------------------------------

var ErrSamplingFrequencyIndex = errors.New("lal.aac: invalid sampling frequency index")

// ----- pkg/aac -------------------------------------------------------------------------------------------------------

var ErrAvc = errors.New("lal.avc: fxxk")

// ----- pkg/base ------------------------------------------------------------------------------------------------------

var (
	ErrAddrEmpty               = errors.New("lal.base: http server addr empty")
	ErrMultiRegisterForPattern = errors.New("lal.base: http server multiple registrations for pattern")

	ErrSessionNotStarted = errors.New("lal.base: session has not been started yet")

	ErrInvalidUrl = errors.New("lal.base: invalid url")
)

// ----- pkg/hevc ------------------------------------------------------------------------------------------------------

var ErrHevc = errors.New("lal.hevc: fxxk")

// ----- pkg/hls -------------------------------------------------------------------------------------------------------

var ErrHls = errors.New("lal.hls: fxxk")
var ErrHlsSessionNotFound = errors.New("lal.hls: hls session not found")

// ----- pkg/rtmp ------------------------------------------------------------------------------------------------------

var (
	ErrAmfInvalidType = errors.New("lal.rtmp: invalid amf0 type")
	ErrAmfTooShort    = errors.New("lal.rtmp: too short to unmarshal amf0 data")
	ErrAmfNotExist    = errors.New("lal.rtmp: not exist")

	ErrRtmpShortBuffer   = errors.New("lal.rtmp: buffer too short")
	ErrRtmpUnexpectedMsg = errors.New("lal.rtmp: unexpected msg")
)

func NewErrAmfInvalidType(b byte) error {
	return fmt.Errorf("%w. b=%d", ErrAmfInvalidType, b)
}

func NewErrRtmpShortBuffer(need, actual int, msg string) error {
	return fmt.Errorf("%w. need=%d, actual=%d, msg=%s", ErrRtmpShortBuffer, need, actual, msg)
}

// ----- pkg/rtprtcp ---------------------------------------------------------------------------------------------------

var ErrRtpRtcpShortBuffer = errors.New("lal.rtprtcp: buffer too short")

// ----- pkg/rtsp ------------------------------------------------------------------------------------------------------

var (
	ErrRtsp                     = errors.New("lal.rtsp: fxxk")
	ErrRtspClosedByObserver     = errors.New("lal.rtsp: close by observer")
	ErrRtspUnsupportedTransport = errors.New("lal.rtsp: unsupported Transport")
)

// ----- pkg/sdp -------------------------------------------------------------------------------------------------------

var ErrSdp = errors.New("lal.sdp: fxxk")

// ----- pkg/logic -----------------------------------------------------------------------------------------------------

var (
	ErrDupInStream      = errors.New("lal.logic: in stream already exist at group")
	ErrDisposedInStream = errors.New("lal.logic: in stream already disposed")

	ErrSimpleAuthParamNotFound = errors.New("lal.logic: simple auth failed since url param lal_secret not found")
	ErrSimpleAuthFailed        = errors.New("lal.logic: simple auth failed since url param lal_secret invalid")
)

// ----- pkg/gb28181 ---------------------------------------------------------------------------------------------------

var (
	ErrGb28181 = errors.New("lal.gb28181: fxxk")
)

// ---------------------------------------------------------------------------------------------------------------------
