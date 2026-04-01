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

// Call_GetMetadataConfigurations forwards the call to dev.CallMethod() then parses the payload of the reply as a GetMetadataConfigurationsResponse.
func Call_GetMetadataConfigurations(ctx context.Context, dev *onvif.Device, request media.GetMetadataConfigurations) (media.GetMetadataConfigurationsResponse, error) {
	type Envelope struct {
		Header struct{}
		Body   struct {
			GetMetadataConfigurationsResponse media.GetMetadataConfigurationsResponse
		}
	}
	var reply Envelope
	if httpReply, err := dev.CallMethod(request); err != nil {
		return reply.Body.GetMetadataConfigurationsResponse, errors.Annotate(err, "call")
	} else {
		err = sdk.ReadAndParse(ctx, httpReply, &reply, "GetMetadataConfigurations")
		return reply.Body.GetMetadataConfigurationsResponse, errors.Annotate(err, "reply")
	}
}
