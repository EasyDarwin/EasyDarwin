package media

import (
	"github.com/use-go/onvif/xsd"
	"github.com/use-go/onvif/xsd/onvif"
)

type Capabilities struct {
	SnapshotUri           bool `xml:"SnapshotUri,attr"`
	Rotation              bool `xml:"Rotation,attr"`
	VideoSourceMode       bool `xml:"VideoSourceMode,attr"`
	OSD                   bool `xml:"OSD,attr"`
	TemporaryOSDText      bool `xml:"TemporaryOSDText,attr"`
	EXICompression        bool `xml:"EXICompression,attr"`
	ProfileCapabilities   ProfileCapabilities
	StreamingCapabilities StreamingCapabilities
}

type ProfileCapabilities struct {
	MaximumNumberOfProfiles int `xml:"MaximumNumberOfProfiles,attr"`
}

type StreamingCapabilities struct {
	RTPMulticast        bool `xml:"RTPMulticast,attr"`
	RTP_TCP             bool `xml:"RTP_TCP,attr"`
	RTP_RTSP_TCP        bool `xml:"RTP_RTSP_TCP,attr"`
	NonAggregateControl bool `xml:"NonAggregateControl,attr"`
	NoRTSPStreaming     bool `xml:"NoRTSPStreaming,attr"`
}

//Media main types

type GetServiceCapabilities struct {
	XMLName string `xml:"trt:GetServiceCapabilities"`
}

type GetServiceCapabilitiesResponse struct {
	Capabilities Capabilities
}

type GetVideoSources struct {
	XMLName string `xml:"trt:GetVideoSources"`
}

type GetVideoSourcesResponse struct {
	VideoSources onvif.VideoSource
}

type GetAudioSources struct {
	XMLName string `xml:"trt:GetAudioSources"`
}

type GetAudioSourcesResponse struct {
	AudioSources onvif.AudioSource
}

type GetAudioOutputs struct {
	XMLName string `xml:"trt:GetAudioOutputs"`
}

type GetAudioOutputsResponse struct {
	AudioOutputs onvif.AudioOutput
}

type CreateProfile struct {
	XMLName string               `xml:"trt:CreateProfile"`
	Name    onvif.Name           `xml:"trt:Name"`
	Token   onvif.ReferenceToken `xml:"trt:Token"`
}

type CreateProfileResponse struct {
	Profile onvif.Profile
}

type GetProfile struct {
	XMLName      string               `xml:"trt:GetProfile"`
	ProfileToken onvif.ReferenceToken `xml:"trt:ProfileToken"`
}

type GetProfileResponse struct {
	Profile onvif.Profile
}

type GetProfiles struct {
	XMLName string `xml:"trt:GetProfiles"`
}

type GetProfilesResponse struct {
	Profiles []onvif.Profile
}

type AddVideoEncoderConfiguration struct {
	XMLName            string               `xml:"trt:AddVideoEncoderConfiguration"`
	ProfileToken       onvif.ReferenceToken `xml:"trt:ProfileToken"`
	ConfigurationToken onvif.ReferenceToken `xml:"trt:ConfigurationToken"`
}

type AddVideoEncoderConfigurationResponse struct {
}

type RemoveVideoEncoderConfiguration struct {
	XMLName      string               `xml:"trt:RemoveVideoEncoderConfiguration"`
	ProfileToken onvif.ReferenceToken `xml:"trt:ProfileToken"`
}

type RemoveVideoEncoderConfigurationResponse struct {
}

type AddVideoSourceConfiguration struct {
	XMLName            string               `xml:"trt:AddVideoSourceConfiguration"`
	ProfileToken       onvif.ReferenceToken `xml:"trt:ProfileToken"`
	ConfigurationToken onvif.ReferenceToken `xml:"trt:ConfigurationToken"`
}

type AddVideoSourceConfigurationResponse struct {
}

type RemoveVideoSourceConfiguration struct {
	XMLName      string               `xml:"trt:RemoveVideoSourceConfiguration"`
	ProfileToken onvif.ReferenceToken `xml:"trt:ProfileToken"`
}

type RemoveVideoSourceConfigurationResponse struct {
}

type AddAudioEncoderConfiguration struct {
	XMLName            string               `xml:"trt:AddAudioEncoderConfiguration"`
	ProfileToken       onvif.ReferenceToken `xml:"trt:ProfileToken"`
	ConfigurationToken onvif.ReferenceToken `xml:"trt:ConfigurationToken"`
}

type AddAudioEncoderConfigurationResponse struct {
}

type RemoveAudioEncoderConfiguration struct {
	XMLName      string               `xml:"trt:RemoveAudioEncoderConfiguration"`
	ProfileToken onvif.ReferenceToken `xml:"trt:ProfileToken"`
}

type RemoveAudioEncoderConfigurationResponse struct {
}

type AddAudioSourceConfiguration struct {
	XMLName            string               `xml:"trt:AddAudioSourceConfiguration"`
	ProfileToken       onvif.ReferenceToken `xml:"trt:ProfileToken"`
	ConfigurationToken onvif.ReferenceToken `xml:"trt:ConfigurationToken"`
}

type AddAudioSourceConfigurationResponse struct {
}

type RemoveAudioSourceConfiguration struct {
	XMLName      string               `xml:"trt:RemoveAudioSourceConfiguration"`
	ProfileToken onvif.ReferenceToken `xml:"trt:ProfileToken"`
}

type RemoveAudioSourceConfigurationResponse struct {
}

type AddPTZConfiguration struct {
	XMLName            string               `xml:"trt:AddPTZConfiguration"`
	ProfileToken       onvif.ReferenceToken `xml:"trt:ProfileToken"`
	ConfigurationToken onvif.ReferenceToken `xml:"trt:ConfigurationToken"`
}

type AddPTZConfigurationResponse struct {
}

type RemovePTZConfiguration struct {
	XMLName      string               `xml:"trt:RemovePTZConfiguration"`
	ProfileToken onvif.ReferenceToken `xml:"trt:ProfileToken"`
}

type RemovePTZConfigurationResponse struct {
}

type AddVideoAnalyticsConfiguration struct {
	XMLName            string               `xml:"trt:AddVideoAnalyticsConfiguration"`
	ProfileToken       onvif.ReferenceToken `xml:"trt:ProfileToken"`
	ConfigurationToken onvif.ReferenceToken `xml:"trt:ConfigurationToken"`
}

type AddVideoAnalyticsConfigurationResponse struct {
}

type RemoveVideoAnalyticsConfiguration struct {
	XMLName      string               `xml:"trt:RemoveVideoAnalyticsConfiguration"`
	ProfileToken onvif.ReferenceToken `xml:"trt:ProfileToken"`
}

type RemoveVideoAnalyticsConfigurationResponse struct {
}

type AddMetadataConfiguration struct {
	XMLName            string               `xml:"trt:AddMetadataConfiguration"`
	ProfileToken       onvif.ReferenceToken `xml:"trt:ProfileToken"`
	ConfigurationToken onvif.ReferenceToken `xml:"trt:ConfigurationToken"`
}

type AddMetadataConfigurationResponse struct {
}

type RemoveMetadataConfiguration struct {
	XMLName      string               `xml:"trt:RemoveMetadataConfiguration"`
	ProfileToken onvif.ReferenceToken `xml:"trt:ProfileToken"`
}

type RemoveMetadataConfigurationResponse struct {
}

type AddAudioOutputConfiguration struct {
	XMLName            string               `xml:"trt:AddAudioOutputConfiguration"`
	ProfileToken       onvif.ReferenceToken `xml:"trt:ProfileToken"`
	ConfigurationToken onvif.ReferenceToken `xml:"trt:ConfigurationToken"`
}

type AddAudioOutputConfigurationResponse struct {
}

type RemoveAudioOutputConfiguration struct {
	XMLName      string               `xml:"trt:RemoveAudioOutputConfiguration"`
	ProfileToken onvif.ReferenceToken `xml:"trt:ProfileToken"`
}

type RemoveAudioOutputConfigurationResponse struct {
}

type AddAudioDecoderConfiguration struct {
	XMLName            string               `xml:"trt:AddAudioDecoderConfiguration"`
	ProfileToken       onvif.ReferenceToken `xml:"trt:ProfileToken"`
	ConfigurationToken onvif.ReferenceToken `xml:"trt:ConfigurationToken"`
}

type AddAudioDecoderConfigurationResponse struct {
}

type RemoveAudioDecoderConfiguration struct {
	XMLName      string               `xml:"trt:RemoveAudioDecoderConfiguration"`
	ProfileToken onvif.ReferenceToken `xml:"trt:ProfileToken"`
}

type RemoveAudioDecoderConfigurationResponse struct {
}

type DeleteProfile struct {
	XMLName      string               `xml:"trt:DeleteProfile"`
	ProfileToken onvif.ReferenceToken `xml:"trt:ProfileToken"`
}

type DeleteProfileResponse struct {
}

type GetVideoSourceConfigurations struct {
	XMLName string `xml:"trt:GetVideoSourceConfigurations"`
}

type GetVideoSourceConfigurationsResponse struct {
	Configurations onvif.VideoSourceConfiguration
}

type GetVideoEncoderConfigurations struct {
	XMLName string `xml:"trt:GetVideoEncoderConfigurations"`
}

type GetVideoEncoderConfigurationsResponse struct {
	Configurations onvif.VideoEncoderConfiguration
}

type GetAudioSourceConfigurations struct {
	XMLName string `xml:"trt:GetAudioSourceConfigurations"`
}

type GetAudioSourceConfigurationsResponse struct {
	Configurations onvif.AudioSourceConfiguration
}

type GetAudioEncoderConfigurations struct {
	XMLName string `xml:"trt:GetAudioEncoderConfigurations"`
}

type GetAudioEncoderConfigurationsResponse struct {
	Configurations onvif.AudioEncoderConfiguration
}

type GetVideoAnalyticsConfigurations struct {
	XMLName string `xml:"trt:GetVideoAnalyticsConfigurations"`
}

type GetVideoAnalyticsConfigurationsResponse struct {
	Configurations onvif.VideoAnalyticsConfiguration
}

type GetMetadataConfigurations struct {
	XMLName string `xml:"trt:GetMetadataConfigurations"`
}

type GetMetadataConfigurationsResponse struct {
	Configurations onvif.MetadataConfiguration
}

type GetAudioOutputConfigurations struct {
	XMLName string `xml:"trt:GetAudioOutputConfigurations"`
}

type GetAudioOutputConfigurationsResponse struct {
	Configurations onvif.AudioOutputConfiguration
}

type GetAudioDecoderConfigurations struct {
	XMLName string `xml:"trt:GetAudioDecoderConfigurations"`
}

type GetAudioDecoderConfigurationsResponse struct {
	Configurations onvif.AudioDecoderConfiguration
}

type GetVideoSourceConfiguration struct {
	XMLName            string               `xml:"trt:GetVideoSourceConfiguration"`
	ConfigurationToken onvif.ReferenceToken `xml:"trt:ConfigurationToken"`
}

type GetVideoSourceConfigurationResponse struct {
	Configuration onvif.VideoSourceConfiguration
}

type GetVideoEncoderConfiguration struct {
	XMLName            string               `xml:"trt:GetVideoEncoderConfiguration"`
	ConfigurationToken onvif.ReferenceToken `xml:"trt:ConfigurationToken"`
}

type GetVideoEncoderConfigurationResponse struct {
	Configuration onvif.VideoEncoderConfiguration
}

type GetAudioSourceConfiguration struct {
	XMLName            string               `xml:"trt:GetAudioSourceConfiguration"`
	ConfigurationToken onvif.ReferenceToken `xml:"trt:ConfigurationToken"`
}

type GetAudioSourceConfigurationResponse struct {
	Configuration onvif.AudioSourceConfiguration
}

type GetAudioEncoderConfiguration struct {
	XMLName            string               `xml:"trt:GetAudioEncoderConfiguration"`
	ConfigurationToken onvif.ReferenceToken `xml:"trt:ConfigurationToken"`
}

type GetAudioEncoderConfigurationResponse struct {
	Configuration onvif.AudioEncoderConfiguration
}

type GetVideoAnalyticsConfiguration struct {
	XMLName            string               `xml:"trt:GetVideoAnalyticsConfiguration"`
	ConfigurationToken onvif.ReferenceToken `xml:"trt:ConfigurationToken"`
}

type GetVideoAnalyticsConfigurationResponse struct {
	Configuration onvif.VideoAnalyticsConfiguration
}

type GetMetadataConfiguration struct {
	XMLName            string               `xml:"trt:GetMetadataConfiguration"`
	ConfigurationToken onvif.ReferenceToken `xml:"trt:ConfigurationToken"`
}

type GetMetadataConfigurationResponse struct {
	Configuration onvif.MetadataConfiguration
}

type GetAudioOutputConfiguration struct {
	XMLName            string               `xml:"trt:GetAudioOutputConfiguration"`
	ConfigurationToken onvif.ReferenceToken `xml:"trt:ConfigurationToken"`
}

type GetAudioOutputConfigurationResponse struct {
	Configuration onvif.AudioOutputConfiguration
}

type GetAudioDecoderConfiguration struct {
	XMLName            string               `xml:"trt:GetAudioDecoderConfiguration"`
	ConfigurationToken onvif.ReferenceToken `xml:"trt:ConfigurationToken"`
}

type GetAudioDecoderConfigurationResponse struct {
	Configuration onvif.AudioDecoderConfiguration
}

type GetCompatibleVideoEncoderConfigurations struct {
	XMLName      string               `xml:"trt:GetCompatibleVideoEncoderConfigurations"`
	ProfileToken onvif.ReferenceToken `xml:"trt:ProfileToken"`
}

type GetCompatibleVideoEncoderConfigurationsResponse struct {
	Configurations onvif.VideoEncoderConfiguration
}

type GetCompatibleVideoSourceConfigurations struct {
	XMLName      string               `xml:"trt:GetCompatibleVideoSourceConfigurations"`
	ProfileToken onvif.ReferenceToken `xml:"trt:ProfileToken"`
}

type GetCompatibleVideoSourceConfigurationsResponse struct {
	Configurations onvif.VideoSourceConfiguration
}

type GetCompatibleAudioEncoderConfigurations struct {
	XMLName      string               `xml:"trt:GetCompatibleAudioEncoderConfigurations"`
	ProfileToken onvif.ReferenceToken `xml:"trt:ProfileToken"`
}

type GetCompatibleAudioEncoderConfigurationsResponse struct {
	Configurations onvif.AudioEncoderConfiguration
}

type GetCompatibleAudioSourceConfigurations struct {
	XMLName      string               `xml:"trt:GetCompatibleAudioSourceConfigurations"`
	ProfileToken onvif.ReferenceToken `xml:"trt:ProfileToken"`
}

type GetCompatibleAudioSourceConfigurationsResponse struct {
	Configurations onvif.AudioSourceConfiguration
}

type GetCompatibleVideoAnalyticsConfigurations struct {
	XMLName      string               `xml:"trt:GetCompatibleVideoAnalyticsConfigurations"`
	ProfileToken onvif.ReferenceToken `xml:"trt:ProfileToken"`
}

type GetCompatibleVideoAnalyticsConfigurationsResponse struct {
	Configurations onvif.VideoAnalyticsConfiguration
}

type GetCompatibleMetadataConfigurations struct {
	XMLName      string               `xml:"trt:GetCompatibleMetadataConfigurations"`
	ProfileToken onvif.ReferenceToken `xml:"trt:ProfileToken"`
}

type GetCompatibleMetadataConfigurationsResponse struct {
	Configurations onvif.MetadataConfiguration
}

type GetCompatibleAudioOutputConfigurations struct {
	XMLName      string               `xml:"trt:GetCompatibleAudioOutputConfigurations"`
	ProfileToken onvif.ReferenceToken `xml:"trt:ProfileToken"`
}

type GetCompatibleAudioOutputConfigurationsResponse struct {
	Configurations onvif.AudioOutputConfiguration
}

type GetCompatibleAudioDecoderConfigurations struct {
	XMLName      string               `xml:"trt:GetCompatibleAudioDecoderConfigurations"`
	ProfileToken onvif.ReferenceToken `xml:"trt:ProfileToken"`
}

type GetCompatibleAudioDecoderConfigurationsResponse struct {
	Configurations onvif.AudioDecoderConfiguration
}

type SetVideoSourceConfiguration struct {
	XMLName          string                         `xml:"trt:SetVideoSourceConfiguration"`
	Configuration    onvif.VideoSourceConfiguration `xml:"trt:Configuration"`
	ForcePersistence xsd.Boolean                    `xml:"trt:ForcePersistence"`
}

type SetVideoSourceConfigurationResponse struct {
}

type SetVideoEncoderConfiguration struct {
	XMLName          string                          `xml:"trt:SetVideoEncoderConfiguration"`
	Configuration    onvif.VideoEncoderConfiguration `xml:"trt:Configuration"`
	ForcePersistence xsd.Boolean                     `xml:"trt:ForcePersistence"`
}

type SetVideoEncoderConfigurationResponse struct {
}

type SetAudioSourceConfiguration struct {
	XMLName          string                         `xml:"trt:SetAudioSourceConfiguration"`
	Configuration    onvif.AudioSourceConfiguration `xml:"trt:Configuration"`
	ForcePersistence xsd.Boolean                    `xml:"trt:ForcePersistence"`
}

type SetAudioSourceConfigurationResponse struct {
}

type SetAudioEncoderConfiguration struct {
	XMLName          string                          `xml:"trt:SetAudioEncoderConfiguration"`
	Configuration    onvif.AudioEncoderConfiguration `xml:"trt:Configuration"`
	ForcePersistence xsd.Boolean                     `xml:"trt:ForcePersistence"`
}

type SetAudioEncoderConfigurationResponse struct {
}

type SetVideoAnalyticsConfiguration struct {
	XMLName          string                            `xml:"trt:SetVideoAnalyticsConfiguration"`
	Configuration    onvif.VideoAnalyticsConfiguration `xml:"trt:Configuration"`
	ForcePersistence bool                              `xml:"trt:ForcePersistence"`
}

type SetVideoAnalyticsConfigurationResponse struct {
}

type SetMetadataConfiguration struct {
	XMLName          string                      `xml:"trt:GetDeviceInformation"`
	Configuration    onvif.MetadataConfiguration `xml:"trt:Configuration"`
	ForcePersistence xsd.Boolean                 `xml:"trt:ForcePersistence"`
}

type SetMetadataConfigurationResponse struct {
}

type SetAudioOutputConfiguration struct {
	XMLName          string                         `xml:"trt:SetAudioOutputConfiguration"`
	Configuration    onvif.AudioOutputConfiguration `xml:"trt:Configuration"`
	ForcePersistence bool                           `xml:"trt:ForcePersistence"`
}

type SetAudioOutputConfigurationResponse struct {
}

type SetAudioDecoderConfiguration struct {
	XMLName          string                          `xml:"trt:SetAudioDecoderConfiguration"`
	Configuration    onvif.AudioDecoderConfiguration `xml:"trt:Configuration"`
	ForcePersistence xsd.Boolean                     `xml:"trt:ForcePersistence"`
}

type SetAudioDecoderConfigurationResponse struct {
}

type GetVideoSourceConfigurationOptions struct {
	XMLName            string               `xml:"trt:GetVideoSourceConfigurationOptions"`
	ProfileToken       onvif.ReferenceToken `xml:"trt:ProfileToken"`
	ConfigurationToken onvif.ReferenceToken `xml:"trt:ConfigurationToken"`
}

type GetVideoSourceConfigurationOptionsResponse struct {
	Options onvif.VideoSourceConfigurationOptions
}

type GetVideoEncoderConfigurationOptions struct {
	XMLName            string               `xml:"trt:GetVideoEncoderConfigurationOptions"`
	ProfileToken       onvif.ReferenceToken `xml:"trt:ProfileToken"`
	ConfigurationToken onvif.ReferenceToken `xml:"trt:ConfigurationToken"`
}

type GetVideoEncoderConfigurationOptionsResponse struct {
	Options onvif.VideoEncoderConfigurationOptions
}

type GetAudioSourceConfigurationOptions struct {
	XMLName            string               `xml:"trt:GetAudioSourceConfigurationOptions"`
	ProfileToken       onvif.ReferenceToken `xml:"trt:ProfileToken"`
	ConfigurationToken onvif.ReferenceToken `xml:"trt:ConfigurationToken"`
}

type GetAudioSourceConfigurationOptionsResponse struct {
	Options onvif.AudioSourceConfigurationOptions
}

type GetAudioEncoderConfigurationOptions struct {
	XMLName            string               `xml:"trt:GetAudioEncoderConfigurationOptions"`
	ProfileToken       onvif.ReferenceToken `xml:"trt:ProfileToken"`
	ConfigurationToken onvif.ReferenceToken `xml:"trt:ConfigurationToken"`
}

type GetAudioEncoderConfigurationOptionsResponse struct {
	Options onvif.AudioEncoderConfigurationOptions
}

type GetMetadataConfigurationOptions struct {
	XMLName            string               `xml:"trt:GetMetadataConfigurationOptions"`
	ProfileToken       onvif.ReferenceToken `xml:"trt:ProfileToken"`
	ConfigurationToken onvif.ReferenceToken `xml:"trt:ConfigurationToken"`
}

type GetMetadataConfigurationOptionsResponse struct {
	Options onvif.MetadataConfigurationOptions
}

type GetAudioOutputConfigurationOptions struct {
	XMLName            string               `xml:"trt:GetAudioOutputConfigurationOptions"`
	ProfileToken       onvif.ReferenceToken `xml:"trt:ProfileToken"`
	ConfigurationToken onvif.ReferenceToken `xml:"trt:ConfigurationToken"`
}

type GetAudioOutputConfigurationOptionsResponse struct {
	Options onvif.AudioOutputConfigurationOptions
}

type GetAudioDecoderConfigurationOptions struct {
	XMLName            string               `xml:"trt:GetAudioDecoderConfigurationOptions"`
	ProfileToken       onvif.ReferenceToken `xml:"trt:ProfileToken"`
	ConfigurationToken onvif.ReferenceToken `xml:"trt:ConfigurationToken"`
}

type GetAudioDecoderConfigurationOptionsResponse struct {
	Options onvif.AudioDecoderConfigurationOptions
}

type GetGuaranteedNumberOfVideoEncoderInstances struct {
	XMLName            string               `xml:"trt:GetGuaranteedNumberOfVideoEncoderInstances"`
	ConfigurationToken onvif.ReferenceToken `xml:"trt:ConfigurationToken"`
}

type GetGuaranteedNumberOfVideoEncoderInstancesResponse struct {
	TotalNumber int
	JPEG        int
	H264        int
	MPEG4       int
}

type GetStreamUri struct {
	XMLName      string               `xml:"trt:GetStreamUri"`
	StreamSetup  onvif.StreamSetup    `xml:"trt:StreamSetup"`
	ProfileToken onvif.ReferenceToken `xml:"trt:ProfileToken"`
}

type GetStreamUriResponse struct {
	MediaUri onvif.MediaUri
}

type StartMulticastStreaming struct {
	XMLName      string               `xml:"trt:StartMulticastStreaming"`
	ProfileToken onvif.ReferenceToken `xml:"trt:ProfileToken"`
}

type StartMulticastStreamingResponse struct {
}

type StopMulticastStreaming struct {
	XMLName      string               `xml:"trt:StopMulticastStreaming"`
	ProfileToken onvif.ReferenceToken `xml:"trt:ProfileToken"`
}

type StopMulticastStreamingResponse struct {
}

type SetSynchronizationPoint struct {
	XMLName      string               `xml:"trt:SetSynchronizationPoint"`
	ProfileToken onvif.ReferenceToken `xml:"trt:ProfileToken"`
}

type SetSynchronizationPointResponse struct {
}

type GetSnapshotUri struct {
	XMLName      string               `xml:"trt:GetSnapshotUri"`
	ProfileToken onvif.ReferenceToken `xml:"trt:ProfileToken"`
}

type GetSnapshotUriResponse struct {
	MediaUri onvif.MediaUri
}

type GetVideoSourceModes struct {
	XMLName          string               `xml:"trt:GetVideoSourceModes"`
	VideoSourceToken onvif.ReferenceToken `xml:"trt:VideoSourceToken"`
}

type GetVideoSourceModesResponse struct {
	VideoSourceModes onvif.VideoSourceMode
}

type SetVideoSourceMode struct {
	XMLName              string               `xml:"trt:SetVideoSourceMode"`
	VideoSourceToken     onvif.ReferenceToken `xml:"trt:VideoSourceToken"`
	VideoSourceModeToken onvif.ReferenceToken `xml:"trt:VideoSourceModeToken"`
}

type SetVideoSourceModeResponse struct {
	Reboot bool
}

type GetOSDs struct {
	XMLName            string               `xml:"trt:GetOSDs"`
	ConfigurationToken onvif.ReferenceToken `xml:"trt:ConfigurationToken"`
}

type GetOSDsResponse struct {
	OSDs onvif.OSDConfiguration
}

type GetOSD struct {
	XMLName  string               `xml:"trt:GetOSD"`
	OSDToken onvif.ReferenceToken `xml:"trt:OSDToken"`
}

type GetOSDResponse struct {
	OSD onvif.OSDConfiguration
}

type GetOSDOptions struct {
	XMLName            string               `xml:"trt:GetOSDOptions"`
	ConfigurationToken onvif.ReferenceToken `xml:"trt:ConfigurationToken"`
}

type GetOSDOptionsResponse struct {
	OSDOptions onvif.OSDConfigurationOptions
}

type SetOSD struct {
	XMLName string                 `xml:"trt:SetOSD"`
	OSD     onvif.OSDConfiguration `xml:"trt:OSD"`
}

type SetOSDResponse struct {
}

type CreateOSD struct {
	XMLName string                 `xml:"trt:CreateOSD"`
	OSD     onvif.OSDConfiguration `xml:"trt:OSD"`
}

type CreateOSDResponse struct {
	OSDToken onvif.ReferenceToken
}

type DeleteOSD struct {
	XMLName  string               `xml:"trt:DeleteOSD"`
	OSDToken onvif.ReferenceToken `xml:"trt:OSDToken"`
}

type DeleteOSDResponse struct {
}
