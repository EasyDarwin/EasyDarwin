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

// Call_SetRelayOutputSettings forwards the call to dev.CallMethod() then parses the payload of the reply as a SetRelayOutputSettingsResponse.
func Call_SetRelayOutputSettings(ctx context.Context, dev *onvif.Device, request device.SetRelayOutputSettings) (device.SetRelayOutputSettingsResponse, error) {
	type Envelope struct {
		Header struct{}
		Body   struct {
			SetRelayOutputSettingsResponse device.SetRelayOutputSettingsResponse
		}
	}
	var reply Envelope
	if httpReply, err := dev.CallMethod(request); err != nil {
		return reply.Body.SetRelayOutputSettingsResponse, errors.Annotate(err, "call")
	} else {
		err = sdk.ReadAndParse(ctx, httpReply, &reply, "SetRelayOutputSettings")
		return reply.Body.SetRelayOutputSettingsResponse, errors.Annotate(err, "reply")
	}
}
