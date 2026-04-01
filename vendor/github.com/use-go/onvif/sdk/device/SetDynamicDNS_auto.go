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

// Call_SetDynamicDNS forwards the call to dev.CallMethod() then parses the payload of the reply as a SetDynamicDNSResponse.
func Call_SetDynamicDNS(ctx context.Context, dev *onvif.Device, request device.SetDynamicDNS) (device.SetDynamicDNSResponse, error) {
	type Envelope struct {
		Header struct{}
		Body   struct {
			SetDynamicDNSResponse device.SetDynamicDNSResponse
		}
	}
	var reply Envelope
	if httpReply, err := dev.CallMethod(request); err != nil {
		return reply.Body.SetDynamicDNSResponse, errors.Annotate(err, "call")
	} else {
		err = sdk.ReadAndParse(ctx, httpReply, &reply, "SetDynamicDNS")
		return reply.Body.SetDynamicDNSResponse, errors.Annotate(err, "reply")
	}
}
