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

// Call_GetCompatibleAudioEncoderConfigurations forwards the call to dev.CallMethod() then parses the payload of the reply as a GetCompatibleAudioEncoderConfigurationsResponse.
func Call_GetCompatibleAudioEncoderConfigurations(ctx context.Context, dev *onvif.Device, request media.GetCompatibleAudioEncoderConfigurations) (media.GetCompatibleAudioEncoderConfigurationsResponse, error) {
	type Envelope struct {
		Header struct{}
		Body   struct {
			GetCompatibleAudioEncoderConfigurationsResponse media.GetCompatibleAudioEncoderConfigurationsResponse
		}
	}
	var reply Envelope
	if httpReply, err := dev.CallMethod(request); err != nil {
		return reply.Body.GetCompatibleAudioEncoderConfigurationsResponse, errors.Annotate(err, "call")
	} else {
		err = sdk.ReadAndParse(ctx, httpReply, &reply, "GetCompatibleAudioEncoderConfigurations")
		return reply.Body.GetCompatibleAudioEncoderConfigurationsResponse, errors.Annotate(err, "reply")
	}
}
