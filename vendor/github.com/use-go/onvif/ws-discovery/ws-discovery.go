package wsdiscovery

import (
	"strings"

	"github.com/beevik/etree"
	"github.com/use-go/onvif/gosoap"
)

func buildProbeMessage(uuidV4 string, scopes, types []string, nmsp map[string]string) gosoap.SoapMessage {
	//Список namespace
	namespaces := make(map[string]string)
	namespaces["a"] = "http://schemas.xmlsoap.org/ws/2004/08/addressing"
	//namespaces["d"] = "http://schemas.xmlsoap.org/ws/2005/04/discovery"

	probeMessage := gosoap.NewEmptySOAP()

	probeMessage.AddRootNamespaces(namespaces)
	//if len(nmsp) != 0 {
	//	probeMessage.AddRootNamespaces(nmsp)
	//}

	//fmt.Println(probeMessage.String())

	//Содержимое Head
	var headerContent []*etree.Element

	action := etree.NewElement("a:Action")
	action.SetText("http://schemas.xmlsoap.org/ws/2005/04/discovery/Probe")
	action.CreateAttr("mustUnderstand", "1")

	msgID := etree.NewElement("a:MessageID")
	msgID.SetText("uuid:" + uuidV4)

	replyTo := etree.NewElement("a:ReplyTo")
	replyTo.CreateElement("a:Address").SetText("http://schemas.xmlsoap.org/ws/2004/08/addressing/role/anonymous")

	to := etree.NewElement("a:To")
	to.SetText("urn:schemas-xmlsoap-org:ws:2005:04:discovery")
	to.CreateAttr("mustUnderstand", "1")

	headerContent = append(headerContent, action, msgID, replyTo, to)
	probeMessage.AddHeaderContents(headerContent)

	//Содержимое Body
	probe := etree.NewElement("Probe")
	probe.CreateAttr("xmlns", "http://schemas.xmlsoap.org/ws/2005/04/discovery")

	if len(types) != 0 {
		typesTag := etree.NewElement("d:Types")
		if len(nmsp) != 0 {
			for key, value := range nmsp {
				typesTag.CreateAttr("xmlns:"+key, value)
			}
		}
		typesTag.CreateAttr("xmlns:d", "http://schemas.xmlsoap.org/ws/2005/04/discovery")
		//typesTag.CreateAttr("xmlns:dp0", "http://www.onvif.org/ver10/network/wsdl")
		var typesString string
		for _, j := range types {
			typesString += j
			typesString += " "
		}

		typesTag.SetText(strings.TrimSpace(typesString))

		probe.AddChild(typesTag)
	}

	if len(scopes) != 0 {
		scopesTag := etree.NewElement("d:Scopes")
		var scopesString string
		for _, j := range scopes {
			scopesString += j
			scopesString += " "
		}
		scopesTag.SetText(strings.TrimSpace(scopesString))

		probe.AddChild(scopesTag)
	}

	probeMessage.AddBodyContent(probe)

	return probeMessage
}
