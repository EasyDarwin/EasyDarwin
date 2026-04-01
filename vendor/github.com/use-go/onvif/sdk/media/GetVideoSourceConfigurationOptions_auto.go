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

// Call_GetVideoSourceConfigurationOptions forwards the call to dev.CallMethod() then parses the payload of the reply as a GetVideoSourceConfigurationOptionsResponse.
func Call_GetVideoSourceConfigurationOptions(ctx context.Context, dev *onvif.Device, request media.GetVideoSourceConfigurationOptions) (media.GetVideoSourceConfigurationOptionsResponse, error) {
	type Envelope struct {
		Header struct{}
		Body   struct {
			GetVideoSourceConfigurationOptionsResponse media.GetVideoSourceConfigurationOptionsResponse
		}
	}
	var reply Envelope
	if httpReply, err := dev.CallMethod(request); err != nil {
		return reply.Body.GetVideoSourceConfigurationOptionsResponse, errors.Annotate(err, "call")
	} else {
		err = sdk.ReadAndParse(ctx, httpReply, &reply, "GetVideoSourceConfigurationOptions")
		return reply.Body.GetVideoSourceConfigurationOptionsResponse, errors.Annotate(err, "reply")
	}
}
