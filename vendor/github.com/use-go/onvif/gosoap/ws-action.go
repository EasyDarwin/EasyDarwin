package gosoap

import (
	"encoding/xml"
)

//Xlmns XML Scheam
var actionHeaders = map[string]string{
	"wsnt:Subscribe":     "http://docs.oasis-open.org/wsn/bw-2/NotificationProducer/SubscribeRequest",
	"ResumeSubscription": "http://docs.oasis-open.org/wsn/bw-2/PausableSubscriptionManager/ResumeSubscriptionRequest",
	"PauseSubscription":  "http://docs.oasis-open.org/wsn/bw-2/PausableSubscriptionManager/PauseSubscriptionRequest",
	"Unsubscribe":        "http://docs.oasis-open.org/wsn/bw-2/PausableSubscriptionManager/UnsubscribeRequest",
	"RenewRequest":       "http://docs.oasis-open.org/wsn/bw-2/PausableSubscriptionManager/RenewRequest",
}

/*************************
	Action Type in Header
*************************/

//Action type
type Action struct {
	//XMLName xml.Name  `xml:"wsse:Security"`
	XMLName   xml.Name `xml:"wsa:Action"`
	Operation string   `xml:",chardata"`
}

/*
   <wsa:Action>
     http://docs.oasis-open.org/wsn/bw-2/NotificationProducer/SubscribeRequest
   </wsa:Action>
*/

//NewAction get a new Action Section
func NewAction(key, value string) Action {

	/** Generating Nonce sequence **/
	auth := Action{

	//	Created: time.Now().UTC().Format(time.RFC3339Nano),
	}

	return auth
}
