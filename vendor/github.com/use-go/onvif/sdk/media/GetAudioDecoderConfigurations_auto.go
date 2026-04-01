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

// Call_GetAudioDecoderConfigurations forwards the call to dev.CallMethod() then parses the payload of the reply as a GetAudioDecoderConfigurationsResponse.
func Call_GetAudioDecoderConfigurations(ctx context.Context, dev *onvif.Device, request media.GetAudioDecoderConfigurations) (media.GetAudioDecoderConfigurationsResponse, error) {
	type Envelope struct {
		Header struct{}
		Body   struct {
			GetAudioDecoderConfigurationsResponse media.GetAudioDecoderConfigurationsResponse
		}
	}
	var reply Envelope
	if httpReply, err := dev.CallMethod(request); err != nil {
		return reply.Body.GetAudioDecoderConfigurationsResponse, errors.Annotate(err, "call")
	} else {
		err = sdk.ReadAndParse(ctx, httpReply, &reply, "GetAudioDecoderConfigurations")
		return reply.Body.GetAudioDecoderConfigurationsResponse, errors.Annotate(err, "reply")
	}
}
