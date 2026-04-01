// Code generated : DO NOT EDIT.
// Copyright (c) 2022 Jean-Francois SMIGIELSKI
// Distributed under the MIT License

package media

import (
	"context"
	"github.com/juju/errors"
	"github.com/use-go/onvif"
	"github.com/use-go/onvif/sdk"
	"github.com/use-go/onvif/media"
)

// Call_GetVideoSourceConfiguration forwards the call to dev.CallMethod() then parses the payload of the reply as a GetVideoSourceConfigurationResponse.
func Call_GetVideoSourceConfiguration(ctx context.Context, dev *onvif.Device, request media.GetVideoSourceConfiguration) (media.GetVideoSourceConfigurationResponse, error) {
	type Envelope struct {
		Header struct{}
		Body   struct {
			GetVideoSourceConfigurationResponse media.GetVideoSourceConfigurationResponse
		}
	}
	var reply Envelope
	if httpReply, err := dev.CallMethod(request); err != nil {
		return reply.Body.GetVideoSourceConfigurationResponse, errors.Annotate(err, "call")
	} else {
		err = sdk.ReadAndParse(ctx, httpReply, &reply, "GetVideoSourceConfiguration")
		return reply.Body.GetVideoSourceConfigurationResponse, errors.Annotate(err, "reply")
	}
}
