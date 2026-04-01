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

// Call_GetAudioEncoderConfigurations forwards the call to dev.CallMethod() then parses the payload of the reply as a GetAudioEncoderConfigurationsResponse.
func Call_GetAudioEncoderConfigurations(ctx context.Context, dev *onvif.Device, request media.GetAudioEncoderConfigurations) (media.GetAudioEncoderConfigurationsResponse, error) {
	type Envelope struct {
		Header struct{}
		Body   struct {
			GetAudioEncoderConfigurationsResponse media.GetAudioEncoderConfigurationsResponse
		}
	}
	var reply Envelope
	if httpReply, err := dev.CallMethod(request); err != nil {
		return reply.Body.GetAudioEncoderConfigurationsResponse, errors.Annotate(err, "call")
	} else {
		err = sdk.ReadAndParse(ctx, httpReply, &reply, "GetAudioEncoderConfigurations")
		return reply.Body.GetAudioEncoderConfigurationsResponse, errors.Annotate(err, "reply")
	}
}
