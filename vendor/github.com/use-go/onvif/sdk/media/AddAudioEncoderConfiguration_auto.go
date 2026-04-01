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

// Call_AddAudioEncoderConfiguration forwards the call to dev.CallMethod() then parses the payload of the reply as a AddAudioEncoderConfigurationResponse.
func Call_AddAudioEncoderConfiguration(ctx context.Context, dev *onvif.Device, request media.AddAudioEncoderConfiguration) (media.AddAudioEncoderConfigurationResponse, error) {
	type Envelope struct {
		Header struct{}
		Body   struct {
			AddAudioEncoderConfigurationResponse media.AddAudioEncoderConfigurationResponse
		}
	}
	var reply Envelope
	if httpReply, err := dev.CallMethod(request); err != nil {
		return reply.Body.AddAudioEncoderConfigurationResponse, errors.Annotate(err, "call")
	} else {
		err = sdk.ReadAndParse(ctx, httpReply, &reply, "AddAudioEncoderConfiguration")
		return reply.Body.AddAudioEncoderConfigurationResponse, errors.Annotate(err, "reply")
	}
}
