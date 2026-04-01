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

// Call_DeleteUsers forwards the call to dev.CallMethod() then parses the payload of the reply as a DeleteUsersResponse.
func Call_DeleteUsers(ctx context.Context, dev *onvif.Device, request device.DeleteUsers) (device.DeleteUsersResponse, error) {
	type Envelope struct {
		Header struct{}
		Body   struct {
			DeleteUsersResponse device.DeleteUsersResponse
		}
	}
	var reply Envelope
	if httpReply, err := dev.CallMethod(request); err != nil {
		return reply.Body.DeleteUsersResponse, errors.Annotate(err, "call")
	} else {
		err = sdk.ReadAndParse(ctx, httpReply, &reply, "DeleteUsers")
		return reply.Body.DeleteUsersResponse, errors.Annotate(err, "reply")
	}
}
