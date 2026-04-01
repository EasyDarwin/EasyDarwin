// Code generated : DO NOT EDIT.
// Copyright (c) 2022 Jean-Francois SMIGIELSKI
// Distributed under the MIT License

package device

import (
	"context"
	"github.com/juju/errors"
	"github.com/use-go/onvif"
	"github.com/use-go/onvif/sdk"
	"github.com/use-go/onvif/device"
)

// Call_SetSystemDateAndTime forwards the call to dev.CallMethod() then parses the payload of the reply as a SetSystemDateAndTimeResponse.
func Call_SetSystemDateAndTime(ctx context.Context, dev *onvif.Device, request device.SetSystemDateAndTime) (device.SetSystemDateAndTimeResponse, error) {
	type Envelope struct {
		Header struct{}
		Body   struct {
			SetSystemDateAndTimeResponse device.SetSystemDateAndTimeResponse
		}
	}
	var reply Envelope
	if httpReply, err := dev.CallMethod(request); err != nil {
		return reply.Body.SetSystemDateAndTimeResponse, errors.Annotate(err, "call")
	} else {
		err = sdk.ReadAndParse(ctx, httpReply, &reply, "SetSystemDateAndTime")
		return reply.Body.SetSystemDateAndTimeResponse, errors.Annotate(err, "reply")
	}
}
