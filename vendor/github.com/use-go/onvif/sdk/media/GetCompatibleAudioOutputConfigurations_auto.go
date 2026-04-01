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

// Call_GetCompatibleAudioOutputConfigurations forwards the call to dev.CallMethod() then parses the payload of the reply as a GetCompatibleAudioOutputConfigurationsResponse.
func Call_GetCompatibleAudioOutputConfigurations(ctx context.Context, dev *onvif.Device, request media.GetCompatibleAudioOutputConfigurations) (media.GetCompatibleAudioOutputConfigurationsResponse, error) {
	type Envelope struct {
		Header struct{}
		Body   struct {
			GetCompatibleAudioOutputConfigurationsResponse media.GetCompatibleAudioOutputConfigurationsResponse
		}
	}
	var reply Envelope
	if httpReply, err := dev.CallMethod(request); err != nil {
		return reply.Body.GetCompatibleAudioOutputConfigurationsResponse, errors.Annotate(err, "call")
	} else {
		err = sdk.ReadAndParse(ctx, httpReply, &reply, "GetCompatibleAudioOutputConfigurations")
		return reply.Body.GetCompatibleAudioOutputConfigurationsResponse, errors.Annotate(err, "reply")
	}
}
