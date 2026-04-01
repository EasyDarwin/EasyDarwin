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

// Call_SetNetworkProtocols forwards the call to dev.CallMethod() then parses the payload of the reply as a SetNetworkProtocolsResponse.
func Call_SetNetworkProtocols(ctx context.Context, dev *onvif.Device, request device.SetNetworkProtocols) (device.SetNetworkProtocolsResponse, error) {
	type Envelope struct {
		Header struct{}
		Body   struct {
			SetNetworkProtocolsResponse device.SetNetworkProtocolsResponse
		}
	}
	var reply Envelope
	if httpReply, err := dev.CallMethod(request); err != nil {
		return reply.Body.SetNetworkProtocolsResponse, errors.Annotate(err, "call")
	} else {
		err = sdk.ReadAndParse(ctx, httpReply, &reply, "SetNetworkProtocols")
		return reply.Body.SetNetworkProtocolsResponse, errors.Annotate(err, "reply")
	}
}
