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

// Call_AddVideoAnalyticsConfiguration forwards the call to dev.CallMethod() then parses the payload of the reply as a AddVideoAnalyticsConfigurationResponse.
func Call_AddVideoAnalyticsConfiguration(ctx context.Context, dev *onvif.Device, request media.AddVideoAnalyticsConfiguration) (media.AddVideoAnalyticsConfigurationResponse, error) {
	type Envelope struct {
		Header struct{}
		Body   struct {
			AddVideoAnalyticsConfigurationResponse media.AddVideoAnalyticsConfigurationResponse
		}
	}
	var reply Envelope
	if httpReply, err := dev.CallMethod(request); err != nil {
		return reply.Body.AddVideoAnalyticsConfigurationResponse, errors.Annotate(err, "call")
	} else {
		err = sdk.ReadAndParse(ctx, httpReply, &reply, "AddVideoAnalyticsConfiguration")
		return reply.Body.AddVideoAnalyticsConfigurationResponse, errors.Annotate(err, "reply")
	}
}
