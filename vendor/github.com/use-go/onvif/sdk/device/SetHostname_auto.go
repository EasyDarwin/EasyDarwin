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

// Call_SetHostname forwards the call to dev.CallMethod() then parses the payload of the reply as a SetHostnameResponse.
func Call_SetHostname(ctx context.Context, dev *onvif.Device, request device.SetHostname) (device.SetHostnameResponse, error) {
	type Envelope struct {
		Header struct{}
		Body   struct {
			SetHostnameResponse device.SetHostnameResponse
		}
	}
	var reply Envelope
	if httpReply, err := dev.CallMethod(request); err != nil {
		return reply.Body.SetHostnameResponse, errors.Annotate(err, "call")
	} else {
		err = sdk.ReadAndParse(ctx, httpReply, &reply, "SetHostname")
		return reply.Body.SetHostnameResponse, errors.Annotate(err, "reply")
	}
}
