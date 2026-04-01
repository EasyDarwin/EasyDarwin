package device

import (
	"github.com/use-go/onvif/xsd"
	"github.com/use-go/onvif/xsd/onvif"
)

type Service struct {
	Namespace xsd.AnyURI
	XAddr     xsd.AnyURI
	Capabilities
	Version onvif.OnvifVersion
}

type Capabilities struct {
	Any string
}

type DeviceServiceCapabilities struct {
	Network  NetworkCapabilities
	Security SecurityCapabilities
	System   SystemCapabilities
	Misc     MiscCapabilities
}

type NetworkCapabilities struct {
	IPFilter            xsd.Boolean `xml:"IPFilter,attr"`
	ZeroConfiguration   xsd.Boolean `xml:"ZeroConfiguration,attr"`
	IPVersion6          xsd.Boolean `xml:"IPVersion6,attr"`
	DynDNS              xsd.Boolean `xml:"DynDNS,attr"`
	Dot11Configuration  xsd.Boolean `xml:"Dot11Configuration,attr"`
	Dot1XConfigurations int         `xml:"Dot1XConfigurations,attr"`
	HostnameFromDHCP    xsd.Boolean `xml:"HostnameFromDHCP,attr"`
	NTP                 int         `xml:"NTP,attr"`
	DHCPv6              xsd.Boolean `xml:"DHCPv6,attr"`
}

type SecurityCapabilities struct {
	TLS1_0               xsd.Boolean    `xml:"TLS1_0,attr"`
	TLS1_1               xsd.Boolean    `xml:"TLS1_1,attr"`
	TLS1_2               xsd.Boolean    `xml:"TLS1_2,attr"`
	OnboardKeyGeneration xsd.Boolean    `xml:"OnboardKeyGeneration,attr"`
	AccessPolicyConfig   xsd.Boolean    `xml:"AccessPolicyConfig,attr"`
	DefaultAccessPolicy  xsd.Boolean    `xml:"DefaultAccessPolicy,attr"`
	Dot1X                xsd.Boolean    `xml:"Dot1X,attr"`
	RemoteUserHandling   xsd.Boolean    `xml:"RemoteUserHandling,attr"`
	X_509Token           xsd.Boolean    `xml:"X_509Token,attr"`
	SAMLToken            xsd.Boolean    `xml:"SAMLToken,attr"`
	KerberosToken        xsd.Boolean    `xml:"KerberosToken,attr"`
	UsernameToken        xsd.Boolean    `xml:"UsernameToken,attr"`
	HttpDigest           xsd.Boolean    `xml:"HttpDigest,attr"`
	RELToken             xsd.Boolean    `xml:"RELToken,attr"`
	SupportedEAPMethods  EAPMethodTypes `xml:"SupportedEAPMethods,attr"`
	MaxUsers             int            `xml:"MaxUsers,attr"`
	MaxUserNameLength    int            `xml:"MaxUserNameLength,attr"`
	MaxPasswordLength    int            `xml:"MaxPasswordLength,attr"`
}

type EAPMethodTypes struct {
	Types []int
}

type SystemCapabilities struct {
	DiscoveryResolve         xsd.Boolean          `xml:"DiscoveryResolve,attr"`
	DiscoveryBye             xsd.Boolean          `xml:"DiscoveryBye,attr"`
	RemoteDiscovery          xsd.Boolean          `xml:"RemoteDiscovery,attr"`
	SystemBackup             xsd.Boolean          `xml:"SystemBackup,attr"`
	SystemLogging            xsd.Boolean          `xml:"SystemLogging,attr"`
	FirmwareUpgrade          xsd.Boolean          `xml:"FirmwareUpgrade,attr"`
	HttpFirmwareUpgrade      xsd.Boolean          `xml:"HttpFirmwareUpgrade,attr"`
	HttpSystemBackup         xsd.Boolean          `xml:"HttpSystemBackup,attr"`
	HttpSystemLogging        xsd.Boolean          `xml:"HttpSystemLogging,attr"`
	HttpSupportInformation   xsd.Boolean          `xml:"HttpSupportInformation,attr"`
	StorageConfiguration     xsd.Boolean          `xml:"StorageConfiguration,attr"`
	MaxStorageConfigurations int                  `xml:"MaxStorageConfigurations,attr"`
	GeoLocationEntries       int                  `xml:"GeoLocationEntries,attr"`
	AutoGeo                  onvif.StringAttrList `xml:"AutoGeo,attr"`
}

type MiscCapabilities struct {
	AuxiliaryCommands onvif.StringAttrList `xml:"AuxiliaryCommands,attr"`
}

type StorageConfiguration struct {
	onvif.DeviceEntity
	Data StorageConfigurationData `xml:"tds:Data"`
}

type StorageConfigurationData struct {
	Type       xsd.String     `xml:"type,attr"`
	LocalPath  xsd.AnyURI     `xml:"tds:LocalPath"`
	StorageUri xsd.AnyURI     `xml:"tds:StorageUri"`
	User       UserCredential `xml:"tds:User"`
	Extension  xsd.AnyURI     `xml:"tds:Extension"`
}

type UserCredential struct {
	UserName  xsd.String  `xml:"tds:UserName"`
	Password  xsd.String  `xml:"tds:Password"`
	Extension xsd.AnyType `xml:"tds:Extension"`
}

//Device main types

type GetServices struct {
	XMLName           string      `xml:"tds:GetServices"`
	IncludeCapability xsd.Boolean `xml:"tds:IncludeCapability"`
}

type GetServicesResponse struct {
	Service Service
}

type GetServiceCapabilities struct {
	XMLName string `xml:"tds:GetServiceCapabilities"`
}

type GetServiceCapabilitiesResponse struct {
	Capabilities DeviceServiceCapabilities
}

type GetDeviceInformation struct {
	XMLName string `xml:"tds:GetDeviceInformation"`
}

type GetDeviceInformationResponse struct {
	Manufacturer    string
	Model           string
	FirmwareVersion string
	SerialNumber    string
	HardwareId      string
}

type SetSystemDateAndTime struct {
	XMLName         string                `xml:"tds:SetSystemDateAndTime"`
	DateTimeType    onvif.SetDateTimeType `xml:"tds:DateTimeType"`
	DaylightSavings xsd.Boolean           `xml:"tds:DaylightSavings"`
	TimeZone        onvif.TimeZone        `xml:"tds:TimeZone"`
	UTCDateTime     onvif.DateTime        `xml:"tds:UTCDateTime"`
}

type SetSystemDateAndTimeResponse struct {
}

type GetSystemDateAndTime struct {
	XMLName string `xml:"tds:GetSystemDateAndTime"`
}

type GetSystemDateAndTimeResponse struct {
	SystemDateAndTime onvif.SystemDateTime
}

type SetSystemFactoryDefault struct {
	XMLName        string                   `xml:"tds:SetSystemFactoryDefault"`
	FactoryDefault onvif.FactoryDefaultType `xml:"tds:FactoryDefault"`
}

type SetSystemFactoryDefaultResponse struct {
}

type UpgradeSystemFirmware struct {
	XMLName  string               `xml:"tds:UpgradeSystemFirmware"`
	Firmware onvif.AttachmentData `xml:"tds:Firmware"`
}

type UpgradeSystemFirmwareResponse struct {
	Message string
}

type SystemReboot struct {
	XMLName string `xml:"tds:SystemReboot"`
}

type SystemRebootResponse struct {
	Message string
}

//TODO: one or more repetitions
type RestoreSystem struct {
	XMLName     string           `xml:"tds:RestoreSystem"`
	BackupFiles onvif.BackupFile `xml:"tds:BackupFiles"`
}

type RestoreSystemResponse struct {
}

type GetSystemBackup struct {
	XMLName string `xml:"tds:GetSystemBackup"`
}

type GetSystemBackupResponse struct {
	BackupFiles onvif.BackupFile
}

type GetSystemLog struct {
	XMLName string              `xml:"tds:GetSystemLog"`
	LogType onvif.SystemLogType `xml:"tds:LogType"`
}

type GetSystemLogResponse struct {
	SystemLog onvif.SystemLog
}

type GetSystemSupportInformation struct {
	XMLName string `xml:"tds:GetSystemSupportInformation"`
}

type GetSystemSupportInformationResponse struct {
	SupportInformation onvif.SupportInformation
}

type GetScopes struct {
	XMLName string `xml:"tds:GetScopes"`
}

type GetScopesResponse struct {
	Scopes onvif.Scope
}

//TODO: one or more scopes
type SetScopes struct {
	XMLName string     `xml:"tds:SetScopes"`
	Scopes  xsd.AnyURI `xml:"tds:Scopes"`
}

type SetScopesResponse struct {
}

//TODO: list of scopes
type AddScopes struct {
	XMLName   string     `xml:"tds:AddScopes"`
	ScopeItem xsd.AnyURI `xml:"tds:ScopeItem"`
}

type AddScopesResponse struct {
}

//TODO: One or more repetitions
type RemoveScopes struct {
	XMLName   string     `xml:"tds:RemoveScopes"`
	ScopeItem xsd.AnyURI `xml:"onvif:ScopeItem"`
}

type RemoveScopesResponse struct {
	ScopeItem xsd.AnyURI
}

type GetDiscoveryMode struct {
	XMLName string `xml:"tds:GetDiscoveryMode"`
}

type GetDiscoveryModeResponse struct {
	DiscoveryMode onvif.DiscoveryMode
}

type SetDiscoveryMode struct {
	XMLName       string              `xml:"tds:SetDiscoveryMode"`
	DiscoveryMode onvif.DiscoveryMode `xml:"tds:DiscoveryMode"`
}

type SetDiscoveryModeResponse struct {
}

type GetRemoteDiscoveryMode struct {
	XMLName string `xml:"tds:GetRemoteDiscoveryMode"`
}

type GetRemoteDiscoveryModeResponse struct {
	RemoteDiscoveryMode onvif.DiscoveryMode
}

type SetRemoteDiscoveryMode struct {
	XMLName             string              `xml:"tds:SetRemoteDiscoveryMode"`
	RemoteDiscoveryMode onvif.DiscoveryMode `xml:"tds:RemoteDiscoveryMode"`
}

type SetRemoteDiscoveryModeResponse struct {
}

type GetDPAddresses struct {
	XMLName string `xml:"tds:GetDPAddresses"`
}

type GetDPAddressesResponse struct {
	DPAddress onvif.NetworkHost
}

type SetDPAddresses struct {
	XMLName   string            `xml:"tds:SetDPAddresses"`
	DPAddress onvif.NetworkHost `xml:"tds:DPAddress"`
}

type SetDPAddressesResponse struct {
}

type GetEndpointReference struct {
	XMLName string `xml:"tds:GetEndpointReference"`
}

type GetEndpointReferenceResponse struct {
	GUID string
}

type GetRemoteUser struct {
	XMLName string `xml:"tds:GetRemoteUser"`
}

type GetRemoteUserResponse struct {
	RemoteUser onvif.RemoteUser
}

type SetRemoteUser struct {
	XMLName    string           `xml:"tds:SetRemoteUser"`
	RemoteUser onvif.RemoteUser `xml:"tds:RemoteUser"`
}

type SetRemoteUserResponse struct {
}

type GetUsers struct {
	XMLName string `xml:"tds:GetUsers"`
}

type GetUsersResponse struct {
	User onvif.User
}

//TODO: List of users
type CreateUsers struct {
	XMLName string     `xml:"tds:CreateUsers"`
	User    onvif.User `xml:"tds:User,omitempty"`
}

type CreateUsersResponse struct {
}

//TODO: one or more Username
type DeleteUsers struct {
	XMLName  xsd.String `xml:"tds:DeleteUsers"`
	Username xsd.String `xml:"tds:Username"`
}

type DeleteUsersResponse struct {
}

type SetUser struct {
	XMLName string     `xml:"tds:SetUser"`
	User    onvif.User `xml:"tds:User"`
}

type SetUserResponse struct {
}

type GetWsdlUrl struct {
	XMLName string `xml:"tds:GetWsdlUrl"`
}

type GetWsdlUrlResponse struct {
	WsdlUrl xsd.AnyURI
}

type GetCapabilities struct {
	XMLName  string                   `xml:"tds:GetCapabilities"`
	Category onvif.CapabilityCategory `xml:"tds:Category"`
}

type GetCapabilitiesResponse struct {
	Capabilities onvif.Capabilities
}

type GetHostname struct {
	XMLName string `xml:"tds:GetHostname"`
}

type GetHostnameResponse struct {
	HostnameInformation onvif.HostnameInformation
}

type SetHostname struct {
	XMLName string    `xml:"tds:SetHostname"`
	Name    xsd.Token `xml:"tds:Name"`
}

type SetHostnameResponse struct {
}

type SetHostnameFromDHCP struct {
	XMLName  string      `xml:"tds:SetHostnameFromDHCP"`
	FromDHCP xsd.Boolean `xml:"tds:FromDHCP"`
}

type SetHostnameFromDHCPResponse struct {
	RebootNeeded xsd.Boolean
}

type GetDNS struct {
	XMLName string `xml:"tds:GetDNS"`
}

type GetDNSResponse struct {
	DNSInformation onvif.DNSInformation
}

type SetDNS struct {
	XMLName      string          `xml:"tds:SetDNS"`
	FromDHCP     xsd.Boolean     `xml:"tds:FromDHCP"`
	SearchDomain xsd.Token       `xml:"tds:SearchDomain"`
	DNSManual    onvif.IPAddress `xml:"tds:DNSManual"`
}

type SetDNSResponse struct {
}

type GetNTP struct {
	XMLName string `xml:"tds:GetNTP"`
}

type GetNTPResponse struct {
	NTPInformation onvif.NTPInformation
}

type SetNTP struct {
	XMLName   string            `xml:"tds:SetNTP"`
	FromDHCP  xsd.Boolean       `xml:"tds:FromDHCP"`
	NTPManual onvif.NetworkHost `xml:"tds:NTPManual"`
}

type SetNTPResponse struct {
}

type GetDynamicDNS struct {
	XMLName string `xml:"tds:GetDynamicDNS"`
}

type GetDynamicDNSResponse struct {
	DynamicDNSInformation onvif.DynamicDNSInformation
}

type SetDynamicDNS struct {
	XMLName string               `xml:"tds:SetDynamicDNS"`
	Type    onvif.DynamicDNSType `xml:"tds:Type"`
	Name    onvif.DNSName        `xml:"tds:Name"`
	TTL     xsd.Duration         `xml:"tds:TTL"`
}

type SetDynamicDNSResponse struct {
}

type GetNetworkInterfaces struct {
	XMLName string `xml:"tds:GetNetworkInterfaces"`
}

type GetNetworkInterfacesResponse struct {
	NetworkInterfaces onvif.NetworkInterface
}

type SetNetworkInterfaces struct {
	XMLName          string                                 `xml:"tds:SetNetworkInterfaces"`
	InterfaceToken   onvif.ReferenceToken                   `xml:"tds:InterfaceToken"`
	NetworkInterface onvif.NetworkInterfaceSetConfiguration `xml:"tds:NetworkInterface"`
}

type SetNetworkInterfacesResponse struct {
	RebootNeeded xsd.Boolean
}

type GetNetworkProtocols struct {
	XMLName string `xml:"tds:GetNetworkProtocols"`
}

type GetNetworkProtocolsResponse struct {
	NetworkProtocols onvif.NetworkProtocol
}

type SetNetworkProtocols struct {
	XMLName          string                `xml:"tds:SetNetworkProtocols"`
	NetworkProtocols onvif.NetworkProtocol `xml:"tds:NetworkProtocols"`
}

type SetNetworkProtocolsResponse struct {
}

type GetNetworkDefaultGateway struct {
	XMLName string `xml:"tds:GetNetworkDefaultGateway"`
}

type GetNetworkDefaultGatewayResponse struct {
	NetworkGateway onvif.NetworkGateway
}

type SetNetworkDefaultGateway struct {
	XMLName     string            `xml:"tds:SetNetworkDefaultGateway"`
	IPv4Address onvif.IPv4Address `xml:"tds:IPv4Address"`
	IPv6Address onvif.IPv6Address `xml:"tds:IPv6Address"`
}

type SetNetworkDefaultGatewayResponse struct {
}

type GetZeroConfiguration struct {
	XMLName string `xml:"tds:GetZeroConfiguration"`
}

type GetZeroConfigurationResponse struct {
	ZeroConfiguration onvif.NetworkZeroConfiguration
}

type SetZeroConfiguration struct {
	XMLName        string               `xml:"tds:SetZeroConfiguration"`
	InterfaceToken onvif.ReferenceToken `xml:"tds:InterfaceToken"`
	Enabled        xsd.Boolean          `xml:"tds:Enabled"`
}

type SetZeroConfigurationResponse struct {
}

type GetIPAddressFilter struct {
	XMLName string `xml:"tds:GetIPAddressFilter"`
}

type GetIPAddressFilterResponse struct {
	IPAddressFilter onvif.IPAddressFilter
}

type SetIPAddressFilter struct {
	XMLName         string                `xml:"tds:SetIPAddressFilter"`
	IPAddressFilter onvif.IPAddressFilter `xml:"tds:IPAddressFilter"`
}

type SetIPAddressFilterResponse struct {
}

//This operation adds an IP filter address to a device.
//If the device supports device access control based on
//IP filtering rules (denied or accepted ranges of IP addresses),
//the device shall support adding of IP filtering addresses through
//the AddIPAddressFilter command.
type AddIPAddressFilter struct {
	XMLName         string                `xml:"tds:AddIPAddressFilter"`
	IPAddressFilter onvif.IPAddressFilter `xml:"tds:IPAddressFilter"`
}

type AddIPAddressFilterResponse struct {
}

type RemoveIPAddressFilter struct {
	XMLName         string                `xml:"tds:RemoveIPAddressFilter"`
	IPAddressFilter onvif.IPAddressFilter `xml:"onvif:IPAddressFilter"`
}

type RemoveIPAddressFilterResponse struct {
}

type GetAccessPolicy struct {
	XMLName string `xml:"tds:GetAccessPolicy"`
}

type GetAccessPolicyResponse struct {
	PolicyFile onvif.BinaryData
}

type SetAccessPolicy struct {
	XMLName    string           `xml:"tds:SetAccessPolicy"`
	PolicyFile onvif.BinaryData `xml:"tds:PolicyFile"`
}

type SetAccessPolicyResponse struct {
}

type CreateCertificate struct {
	XMLName        string       `xml:"tds:CreateCertificate"`
	CertificateID  xsd.Token    `xml:"tds:CertificateID,omitempty"`
	Subject        string       `xml:"tds:Subject,omitempty"`
	ValidNotBefore xsd.DateTime `xml:"tds:ValidNotBefore,omitempty"`
	ValidNotAfter  xsd.DateTime `xml:"tds:ValidNotAfter,omitempty"`
}

type CreateCertificateResponse struct {
	NvtCertificate onvif.Certificate
}

type GetCertificates struct {
	XMLName string `xml:"tds:GetCertificates"`
}

type GetCertificatesResponse struct {
	NvtCertificate onvif.Certificate
}

type GetCertificatesStatus struct {
	XMLName string `xml:"tds:GetCertificatesStatus"`
}

type GetCertificatesStatusResponse struct {
	CertificateStatus onvif.CertificateStatus
}

type SetCertificatesStatus struct {
	XMLName           string                  `xml:"tds:SetCertificatesStatus"`
	CertificateStatus onvif.CertificateStatus `xml:"tds:CertificateStatus"`
}

type SetCertificatesStatusResponse struct {
}

//TODO: List of CertificateID
type DeleteCertificates struct {
	XMLName       string    `xml:"tds:DeleteCertificates"`
	CertificateID xsd.Token `xml:"tds:CertificateID"`
}

type DeleteCertificatesResponse struct {
}

//TODO: Откуда onvif:data = cid:21312413412
type GetPkcs10Request struct {
	XMLName       string           `xml:"tds:GetPkcs10Request"`
	CertificateID xsd.Token        `xml:"tds:CertificateID"`
	Subject       xsd.String       `xml:"tds:Subject"`
	Attributes    onvif.BinaryData `xml:"tds:Attributes"`
}

type GetPkcs10RequestResponse struct {
	Pkcs10Request onvif.BinaryData
}

//TODO: one or more NTVCertificate
type LoadCertificates struct {
	XMLName        string            `xml:"tds:LoadCertificates"`
	NVTCertificate onvif.Certificate `xml:"tds:NVTCertificate"`
}

type LoadCertificatesResponse struct {
}

type GetClientCertificateMode struct {
	XMLName string `xml:"tds:GetClientCertificateMode"`
}

type GetClientCertificateModeResponse struct {
	Enabled xsd.Boolean
}

type SetClientCertificateMode struct {
	XMLName string      `xml:"tds:SetClientCertificateMode"`
	Enabled xsd.Boolean `xml:"tds:Enabled"`
}

type SetClientCertificateModeResponse struct {
}

type GetRelayOutputs struct {
	XMLName string `xml:"tds:GetRelayOutputs"`
}

type GetRelayOutputsResponse struct {
	RelayOutputs onvif.RelayOutput
}

type SetRelayOutputSettings struct {
	XMLName          string                    `xml:"tds:SetRelayOutputSettings"`
	RelayOutputToken onvif.ReferenceToken      `xml:"tds:RelayOutputToken"`
	Properties       onvif.RelayOutputSettings `xml:"tds:Properties"`
}

type SetRelayOutputSettingsResponse struct {
}

type SetRelayOutputState struct {
	XMLName          string                  `xml:"tds:SetRelayOutputState"`
	RelayOutputToken onvif.ReferenceToken    `xml:"tds:RelayOutputToken"`
	LogicalState     onvif.RelayLogicalState `xml:"tds:LogicalState"`
}

type SetRelayOutputStateResponse struct {
}

type SendAuxiliaryCommand struct {
	XMLName          string              `xml:"tds:SendAuxiliaryCommand"`
	AuxiliaryCommand onvif.AuxiliaryData `xml:"tds:AuxiliaryCommand"`
}

type SendAuxiliaryCommandResponse struct {
	AuxiliaryCommandResponse onvif.AuxiliaryData
}

type GetCACertificates struct {
	XMLName string `xml:"tds:GetCACertificates"`
}

type GetCACertificatesResponse struct {
	CACertificate onvif.Certificate
}

//TODO: one or more CertificateWithPrivateKey
type LoadCertificateWithPrivateKey struct {
	XMLName                   string                          `xml:"tds:LoadCertificateWithPrivateKey"`
	CertificateWithPrivateKey onvif.CertificateWithPrivateKey `xml:"tds:CertificateWithPrivateKey"`
}

type LoadCertificateWithPrivateKeyResponse struct {
}

type GetCertificateInformation struct {
	XMLName       string    `xml:"tds:GetCertificateInformation"`
	CertificateID xsd.Token `xml:"tds:CertificateID"`
}

type GetCertificateInformationResponse struct {
	CertificateInformation onvif.CertificateInformation
}

type LoadCACertificates struct {
	XMLName       string            `xml:"tds:LoadCACertificates"`
	CACertificate onvif.Certificate `xml:"tds:CACertificate"`
}

type LoadCACertificatesResponse struct {
}

type CreateDot1XConfiguration struct {
	XMLName            string                   `xml:"tds:CreateDot1XConfiguration"`
	Dot1XConfiguration onvif.Dot1XConfiguration `xml:"tds:Dot1XConfiguration"`
}

type CreateDot1XConfigurationResponse struct {
}

type SetDot1XConfiguration struct {
	XMLName            string                   `xml:"tds:SetDot1XConfiguration"`
	Dot1XConfiguration onvif.Dot1XConfiguration `xml:"tds:Dot1XConfiguration"`
}

type SetDot1XConfigurationResponse struct {
}

type GetDot1XConfiguration struct {
	XMLName                 string               `xml:"tds:GetDot1XConfiguration"`
	Dot1XConfigurationToken onvif.ReferenceToken `xml:"tds:Dot1XConfigurationToken"`
}

type GetDot1XConfigurationResponse struct {
	Dot1XConfiguration onvif.Dot1XConfiguration
}

type GetDot1XConfigurations struct {
	XMLName string `xml:"tds:GetDot1XConfigurations"`
}

type GetDot1XConfigurationsResponse struct {
	Dot1XConfiguration onvif.Dot1XConfiguration
}

//TODO: Zero or more Dot1XConfigurationToken
type DeleteDot1XConfiguration struct {
	XMLName                 string               `xml:"tds:DeleteDot1XConfiguration"`
	Dot1XConfigurationToken onvif.ReferenceToken `xml:"tds:Dot1XConfigurationToken"`
}

type DeleteDot1XConfigurationResponse struct {
}

type GetDot11Capabilities struct {
	XMLName string `xml:"tds:GetDot11Capabilities"`
}

type GetDot11CapabilitiesResponse struct {
	Capabilities onvif.Dot11Capabilities
}

type GetDot11Status struct {
	XMLName        string               `xml:"tds:GetDot11Status"`
	InterfaceToken onvif.ReferenceToken `xml:"tds:InterfaceToken"`
}

type GetDot11StatusResponse struct {
	Status onvif.Dot11Status
}

type ScanAvailableDot11Networks struct {
	XMLName        string               `xml:"tds:ScanAvailableDot11Networks"`
	InterfaceToken onvif.ReferenceToken `xml:"tds:InterfaceToken"`
}

type ScanAvailableDot11NetworksResponse struct {
	Networks onvif.Dot11AvailableNetworks
}

type GetSystemUris struct {
	XMLName string `xml:"tds:GetSystemUris"`
}

type GetSystemUrisResponse struct {
	SystemLogUris   onvif.SystemLogUriList
	SupportInfoUri  xsd.AnyURI
	SystemBackupUri xsd.AnyURI
	Extension       xsd.AnyType
}

type StartFirmwareUpgrade struct {
	XMLName string `xml:"tds:StartFirmwareUpgrade"`
}

type StartFirmwareUpgradeResponse struct {
	UploadUri        xsd.AnyURI
	UploadDelay      xsd.Duration
	ExpectedDownTime xsd.Duration
}

type StartSystemRestore struct {
	XMLName string `xml:"tds:StartSystemRestore"`
}

type StartSystemRestoreResponse struct {
	UploadUri        xsd.AnyURI
	ExpectedDownTime xsd.Duration
}

type GetStorageConfigurations struct {
	XMLName string `xml:"tds:GetStorageConfigurations"`
}

type GetStorageConfigurationsResponse struct {
	StorageConfigurations StorageConfiguration
}

type CreateStorageConfiguration struct {
	XMLName              string `xml:"tds:CreateStorageConfiguration"`
	StorageConfiguration StorageConfigurationData
}

type CreateStorageConfigurationResponse struct {
	Token onvif.ReferenceToken
}

type GetStorageConfiguration struct {
	XMLName string               `xml:"tds:GetStorageConfiguration"`
	Token   onvif.ReferenceToken `xml:"tds:Token"`
}

type GetStorageConfigurationResponse struct {
	StorageConfiguration StorageConfiguration
}

type SetStorageConfiguration struct {
	XMLName              string               `xml:"tds:SetStorageConfiguration"`
	StorageConfiguration StorageConfiguration `xml:"tds:StorageConfiguration"`
}

type SetStorageConfigurationResponse struct {
}

type DeleteStorageConfiguration struct {
	XMLName string               `xml:"tds:DeleteStorageConfiguration"`
	Token   onvif.ReferenceToken `xml:"tds:Token"`
}

type DeleteStorageConfigurationResponse struct {
}

type GetGeoLocation struct {
	XMLName string `xml:"tds:GetGeoLocation"`
}

type GetGeoLocationResponse struct {
	Location onvif.LocationEntity
}

//TODO: one or more Location
type SetGeoLocation struct {
	XMLName  string               `xml:"tds:SetGeoLocation"`
	Location onvif.LocationEntity `xml:"tds:Location"`
}

type SetGeoLocationResponse struct {
}

type DeleteGeoLocation struct {
	XMLName  string               `xml:"tds:DeleteGeoLocation"`
	Location onvif.LocationEntity `xml:"tds:Location"`
}

type DeleteGeoLocationResponse struct {
}
