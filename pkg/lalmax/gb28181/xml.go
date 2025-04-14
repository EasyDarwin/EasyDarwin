package gb28181

import (
	"encoding/xml"
	"fmt"
)

var (
	// CatalogXML 获取设备列表xml样式
	CatalogXML = `<?xml version="1.0"?><Query>
<CmdType>Catalog</CmdType>
<SN>%d</SN>
<DeviceID>%s</DeviceID>
</Query>
`
	// RecordInfoXML 获取录像文件列表xml样式
	RecordInfoXML = `<?xml version="1.0"?>
<Query>
<CmdType>RecordInfo</CmdType>
<SN>%d</SN>
<DeviceID>%s</DeviceID>
<StartTime>%s</StartTime>
<EndTime>%s</EndTime>
<Secrecy>0</Secrecy>
<Type>all</Type>
</Query>
`
	// DeviceInfoXML 查询设备详情xml样式
	DeviceInfoXML = `<?xml version="1.0"?>
<Query>
<CmdType>DeviceInfo</CmdType>
<SN>%d</SN>
<DeviceID>%s</DeviceID>
</Query>
`
	// DevicePositionXML 订阅设备位置
	DevicePositionXML = `<?xml version="1.0"?>
<Query>
<CmdType>MobilePosition</CmdType>
<SN>%d</SN>
<DeviceID>%s</DeviceID>
<Interval>%d</Interval>
</Query>`
)

func BuildCatalogXML(sn int, id string) string {
	return fmt.Sprintf(CatalogXML, sn, id)
}

// AlarmResponseXML alarm response xml样式
var (
	AlarmResponseXML = `<?xml version="1.0"?>
<Response>
<CmdType>Alarm</CmdType>
<SN>17430</SN>
<DeviceID>%s</DeviceID>
</Response>
`
)

// BuildRecordInfoXML 获取录像文件列表指令
func BuildAlarmResponseXML(id string) string {
	return fmt.Sprintf(AlarmResponseXML, id)
}

func BuildDeviceInfoXML(sn int, id string) string {
	return fmt.Sprintf(DeviceInfoXML, sn, id)
}

func XmlEncode(v interface{}) (string, error) {
	xmlData, err := xml.MarshalIndent(v, "", " ")
	if err != nil {
		return "", err
	}
	xml := string(xmlData)
	xml = `<?xml version="1.0" ?>` + "\n" + xml + "\n"
	return xml, err
}
