package hik

import "encoding/xml"

// ProbeMatch
// <?xml version="1.0" encoding="UTF-8"?>
// <ProbeMatch><Uuid>F93CF8DC-DF53-424B-98A7-9FC0536E1083</Uuid>
// <Types>inquiry</Types>
// <DeviceType>142378</DeviceType>
// <DeviceDescription>DS-2CD2320D-I</DeviceDescription>
// <DeviceSN>DS-2CD2320D-I20180919AACHC52932642</DeviceSN>
// <CommandPort>8000</CommandPort>
// <HttpPort>80</HttpPort>
// <MAC>44-47-cc-37-ce-81</MAC>
// <IPv4Address>192.168.1.112</IPv4Address>
// <IPv4SubnetMask>255.255.255.0</IPv4SubnetMask>
// <IPv4Gateway>192.168.1.1</IPv4Gateway>
// <IPv6Address>240e:361:c0a:2a00:4647:ccff:fe37:ce81</IPv6Address>
// <IPv6Gateway>::</IPv6Gateway>
// <IPv6MaskLen>64</IPv6MaskLen>
// <DHCP>true</DHCP>
// <AnalogChannelNum>0</AnalogChannelNum>
// <DigitalChannelNum>1</DigitalChannelNum>
// <SoftwareVersion>V5.5.6build 180326</SoftwareVersion>
// <DSPVersion>V7.3 build 180326</DSPVersion>
// <BootTime>2024-09-18 09:21:28</BootTime>
// <Encrypt>true</Encrypt>
// <ResetAbility>false</ResetAbility>
// <DiskNumber>0</DiskNumber>
// <Activated>true</Activated>
// <PasswordResetAbility>true</PasswordResetAbility>
// <PasswordResetModeSecond>true</PasswordResetModeSecond>
// <SupportSecurityQuestion>true</SupportSecurityQuestion>
// <SupportHCPlatform>true</SupportHCPlatform>
// <HCPlatformEnable>flase</HCPlatformEnable>
// <IsModifyVerificationCode>true</IsModifyVerificationCode>
// <Salt>fc5237457341362c0d8810a4bfba0b85ed13421d1d553c62544a7df15948947a</Salt>
// <DeviceLock>true</DeviceLock>
// </ProbeMatch>
type ProbeMatch struct {
	XMLName                  xml.Name `xml:"ProbeMatch"`
	Text                     string   `xml:",chardata"`
	UUID                     string   `xml:"Uuid"`
	Types                    string   `xml:"Types"`
	DeviceType               string   `xml:"DeviceType"`
	DeviceDescription        string   `xml:"DeviceDescription"`
	DeviceSN                 string   `xml:"DeviceSN"`
	CommandPort              string   `xml:"CommandPort"`
	HTTPPort                 string   `xml:"HttpPort"`
	MAC                      string   `xml:"MAC"`
	IPv4Address              string   `xml:"IPv4Address"`
	IPv4SubnetMask           string   `xml:"IPv4SubnetMask"`
	IPv4Gateway              string   `xml:"IPv4Gateway"`
	IPv6Address              string   `xml:"IPv6Address"`
	IPv6Gateway              string   `xml:"IPv6Gateway"`
	IPv6MaskLen              string   `xml:"IPv6MaskLen"`
	DHCP                     string   `xml:"DHCP"`
	AnalogChannelNum         string   `xml:"AnalogChannelNum"`
	DigitalChannelNum        string   `xml:"DigitalChannelNum"`
	SoftwareVersion          string   `xml:"SoftwareVersion"`
	DSPVersion               string   `xml:"DSPVersion"`
	BootTime                 string   `xml:"BootTime"`
	Encrypt                  string   `xml:"Encrypt"`
	ResetAbility             string   `xml:"ResetAbility"`
	DiskNumber               string   `xml:"DiskNumber"`
	Activated                string   `xml:"Activated"`
	PasswordResetAbility     string   `xml:"PasswordResetAbility"`
	PasswordResetModeSecond  string   `xml:"PasswordResetModeSecond"`
	SupportSecurityQuestion  string   `xml:"SupportSecurityQuestion"`
	SupportHCPlatform        string   `xml:"SupportHCPlatform"`
	HCPlatformEnable         string   `xml:"HCPlatformEnable"`
	IsModifyVerificationCode string   `xml:"IsModifyVerificationCode"`
	Salt                     string   `xml:"Salt"`
	DeviceLock               string   `xml:"DeviceLock"`
}
