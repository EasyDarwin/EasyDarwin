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

// Call_GetGuaranteedNumberOfVideoEncoderInstances forwards the call to dev.CallMethod() then parses the payload of the reply as a GetGuaranteedNumberOfVideoEncoderInstancesResponse.
func Call_GetGuaranteedNumberOfVideoEncoderInstances(ctx context.Context, dev *onvif.Device, request media.GetGuaranteedNumberOfVideoEncoderInstances) (media.GetGuaranteedNumberOfVideoEncoderInstancesResponse, error) {
	type Envelope struct {
		Header struct{}
		Body   struct {
			GetGuaranteedNumberOfVideoEncoderInstancesResponse media.GetGuaranteedNumberOfVideoEncoderInstancesResponse
		}
	}
	var reply Envelope
	if httpReply, err := dev.CallMethod(request); err != nil {
		return reply.Body.GetGuaranteedNumberOfVideoEncoderInstancesResponse, errors.Annotate(err, "call")
	} else {
		err = sdk.ReadAndParse(ctx, httpReply, &reply, "GetGuaranteedNumberOfVideoEncoderInstances")
		return reply.Body.GetGuaranteedNumberOfVideoEncoderInstancesResponse, errors.Annotate(err, "reply")
	}
}
