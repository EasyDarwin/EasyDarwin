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

// Call_RemoveAudioSourceConfiguration forwards the call to dev.CallMethod() then parses the payload of the reply as a RemoveAudioSourceConfigurationResponse.
func Call_RemoveAudioSourceConfiguration(ctx context.Context, dev *onvif.Device, request media.RemoveAudioSourceConfiguration) (media.RemoveAudioSourceConfigurationResponse, error) {
	type Envelope struct {
		Header struct{}
		Body   struct {
			RemoveAudioSourceConfigurationResponse media.RemoveAudioSourceConfigurationResponse
		}
	}
	var reply Envelope
	if httpReply, err := dev.CallMethod(request); err != nil {
		return reply.Body.RemoveAudioSourceConfigurationResponse, errors.Annotate(err, "call")
	} else {
		err = sdk.ReadAndParse(ctx, httpReply, &reply, "RemoveAudioSourceConfiguration")
		return reply.Body.RemoveAudioSourceConfigurationResponse, errors.Annotate(err, "reply")
	}
}
