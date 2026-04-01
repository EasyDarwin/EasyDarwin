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

// Call_StartMulticastStreaming forwards the call to dev.CallMethod() then parses the payload of the reply as a StartMulticastStreamingResponse.
func Call_StartMulticastStreaming(ctx context.Context, dev *onvif.Device, request media.StartMulticastStreaming) (media.StartMulticastStreamingResponse, error) {
	type Envelope struct {
		Header struct{}
		Body   struct {
			StartMulticastStreamingResponse media.StartMulticastStreamingResponse
		}
	}
	var reply Envelope
	if httpReply, err := dev.CallMethod(request); err != nil {
		return reply.Body.StartMulticastStreamingResponse, errors.Annotate(err, "call")
	} else {
		err = sdk.ReadAndParse(ctx, httpReply, &reply, "StartMulticastStreaming")
		return reply.Body.StartMulticastStreamingResponse, errors.Annotate(err, "reply")
	}
}
