package onvif

import (
	"github.com/use-go/onvif/xsd"
)

// BUG(r): Enum types implemented as simple string

//TODO: enumerations
//TODO: type <typeName> struct {Any string} convert to type <typeName> AnyType
//TODO: process restrictions

//todo посмотреть все Extensions (Any string)
//todo что делать с xs:any = Any
//todo IntList и ему подобные. Проверить нужен ли слайс. Изменить на slice
//todo посмотреть можно ли заменить StreamType и ему подобные типы на вмтроенные типы
//todo оттестировать тип VideoSourceMode из-за Description-а

//todo в документации описать, что Capabilities повторяеся у каждого сервиса, поэтому у каждого свой Capabilities (MediaCapabilities)
//todo AuxiliaryData и другие simpleTypes, как реализовать рестрикшн
//todo Name и ему подобные необходимо изучить на наличие "List of..." ошибок

//todo Add in buit in

type ContentType string // minLength value="3"
type DNSName xsd.Token

type DeviceEntity struct {
	Token ReferenceToken `xml:"token,attr"`
}

type ReferenceToken xsd.String

type Name xsd.String

type IntRectangle struct {
	X      int `xml:"x,attr"`
	Y      int `xml:"y,attr"`
	Width  int `xml:"width,attr"`
	Height int `xml:"height,attr"`
}

type IntRectangleRange struct {
	XRange      IntRange
	YRange      IntRange
	WidthRange  IntRange
	HeightRange IntRange
}

type IntRange struct {
	Min int
	Max int
}

type FloatRange struct {
	Min float64 `xml:"Min"`
	Max float64 `xml:"Max"`
}

type OSDConfiguration struct {
	DeviceEntity                  `xml:"token,attr"`
	VideoSourceConfigurationToken OSDReference              `xml:"onvif:VideoSourceConfigurationToken"`
	Type                          OSDType                   `xml:"onvif:Type"`
	Position                      OSDPosConfiguration       `xml:"onvif:Position"`
	TextString                    OSDTextConfiguration      `xml:"onvif:TextString"`
	Image                         OSDImgConfiguration       `xml:"onvif:Image"`
	Extension                     OSDConfigurationExtension `xml:"onvif:Extension"`
}

type OSDType xsd.String

type OSDPosConfiguration struct {
	Type      string                       `xml:"onvif:Type"`
	Pos       Vector                       `xml:"onvif:Pos"`
	Extension OSDPosConfigurationExtension `xml:"onvif:Extension"`
}

type Vector struct {
	X float64 `xml:"x,attr"`
	Y float64 `xml:"y,attr"`
}

type OSDPosConfigurationExtension xsd.AnyType

type OSDReference ReferenceToken

type OSDTextConfiguration struct {
	IsPersistentText xsd.Boolean `xml:"IsPersistentText,attr"`

	Type            xsd.String                    `xml:"onvif:Type"`
	DateFormat      xsd.String                    `xml:"onvif:DateFormat"`
	TimeFormat      xsd.String                    `xml:"onvif:TimeFormat"`
	FontSize        xsd.Int                       `xml:"onvif:FontSize"`
	FontColor       OSDColor                      `xml:"onvif:FontColor"`
	BackgroundColor OSDColor                      `xml:"onvif:BackgroundColor"`
	PlainText       xsd.String                    `xml:"onvif:PlainText"`
	Extension       OSDTextConfigurationExtension `xml:"onvif:Extension"`
}

type OSDColor struct {
	Transparent int `xml:"Transparent,attr"`

	Color Color `xml:"onvif:Color"`
}

type Color struct {
	X          float64    `xml:"X,attr"`
	Y          float64    `xml:"Y,attr"`
	Z          float64    `xml:"Z,attr"`
	Colorspace xsd.AnyURI `xml:"Colorspace,attr"`
}

type OSDTextConfigurationExtension xsd.AnyType

type OSDImgConfiguration struct {
	ImgPath   xsd.AnyURI                   `xml:"onvif:ImgPath"`
	Extension OSDImgConfigurationExtension `xml:"onvif:Extension"`
}

type OSDImgConfigurationExtension xsd.AnyType

type OSDConfigurationExtension xsd.AnyType

type VideoSource struct {
	DeviceEntity
	Framerate  float64
	Resolution VideoResolution
	Imaging    ImagingSettings
	Extension  VideoSourceExtension
}

type VideoResolution struct {
	Width  xsd.Int `xml:"onvif:Width"`
	Height xsd.Int `xml:"onvif:Height"`
}

type ImagingSettings struct {
	BacklightCompensation BacklightCompensation
	Brightness            float64
	ColorSaturation       float64
	Contrast              float64
	Exposure              Exposure
	Focus                 FocusConfiguration
	IrCutFilter           IrCutFilterMode
	Sharpness             float64
	WideDynamicRange      WideDynamicRange
	WhiteBalance          WhiteBalance
	Extension             ImagingSettingsExtension
}

type BacklightCompensation struct {
	Mode  BacklightCompensationMode
	Level float64
}

type BacklightCompensationMode xsd.String

type Exposure struct {
	Mode            ExposureMode
	Priority        ExposurePriority
	Window          Rectangle
	MinExposureTime float64
	MaxExposureTime float64
	MinGain         float64
	MaxGain         float64
	MinIris         float64
	MaxIris         float64
	ExposureTime    float64
	Gain            float64
	Iris            float64
}

type ExposureMode xsd.String

type ExposurePriority xsd.String

type Rectangle struct {
	Bottom float64 `xml:"bottom,attr"`
	Top    float64 `xml:"top,attr"`
	Right  float64 `xml:"right,attr"`
	Left   float64 `xml:"left,attr"`
}

type FocusConfiguration struct {
	AutoFocusMode AutoFocusMode
	DefaultSpeed  float64
	NearLimit     float64
	FarLimit      float64
}

type AutoFocusMode xsd.String

type IrCutFilterMode xsd.String

type WideDynamicRange struct {
	Mode  WideDynamicMode `xml:"onvif:Mode"`
	Level float64         `xml:"onvif:Level"`
}

type WideDynamicMode xsd.String

type WhiteBalance struct {
	Mode   WhiteBalanceMode
	CrGain float64
	CbGain float64
}

type WhiteBalanceMode xsd.String

type ImagingSettingsExtension xsd.AnyType

type VideoSourceExtension struct {
	Imaging   ImagingSettings20
	Extension VideoSourceExtension2
}

type ImagingSettings20 struct {
	BacklightCompensation BacklightCompensation20    `xml:"onvif:BacklightCompensation"`
	Brightness            float64                    `xml:"onvif:Brightness"`
	ColorSaturation       float64                    `xml:"onvif:ColorSaturation"`
	Contrast              float64                    `xml:"onvif:Contrast"`
	Exposure              Exposure20                 `xml:"onvif:Exposure"`
	Focus                 FocusConfiguration20       `xml:"onvif:Focus"`
	IrCutFilter           IrCutFilterMode            `xml:"onvif:IrCutFilter"`
	Sharpness             float64                    `xml:"onvif:Sharpness"`
	WideDynamicRange      WideDynamicRange20         `xml:"onvif:WideDynamicRange"`
	WhiteBalance          WhiteBalance20             `xml:"onvif:WhiteBalance"`
	Extension             ImagingSettingsExtension20 `xml:"onvif:Extension"`
}

type BacklightCompensation20 struct {
	Mode  BacklightCompensationMode `xml:"onvif:Mode"`
	Level float64                   `xml:"onvif:Level"`
}

type Exposure20 struct {
	Mode            ExposureMode     `xml:"onvif:Mode"`
	Priority        ExposurePriority `xml:"onvif:Priority"`
	Window          Rectangle        `xml:"onvif:Window"`
	MinExposureTime float64          `xml:"onvif:MinExposureTime"`
	MaxExposureTime float64          `xml:"onvif:MaxExposureTime"`
	MinGain         float64          `xml:"onvif:MinGain"`
	MaxGain         float64          `xml:"onvif:MaxGain"`
	MinIris         float64          `xml:"onvif:MinIris"`
	MaxIris         float64          `xml:"onvif:MaxIris"`
	ExposureTime    float64          `xml:"onvif:ExposureTime"`
	Gain            float64          `xml:"onvif:Gain"`
	Iris            float64          `xml:"onvif:Iris"`
}

type FocusConfiguration20 struct {
	AutoFocusMode AutoFocusMode                 `xml:"onvif:AutoFocusMode"`
	DefaultSpeed  float64                       `xml:"onvif:DefaultSpeed"`
	NearLimit     float64                       `xml:"onvif:NearLimit"`
	FarLimit      float64                       `xml:"onvif:FarLimit"`
	Extension     FocusConfiguration20Extension `xml:"onvif:Extension"`
}

type FocusConfiguration20Extension xsd.AnyType

type WideDynamicRange20 struct {
	Mode  WideDynamicMode `xml:"onvif:Mode"`
	Level float64         `xml:"onvif:Level"`
}

type WhiteBalance20 struct {
	Mode      WhiteBalanceMode        `xml:"onvif:Mode"`
	CrGain    float64                 `xml:"onvif:CrGain"`
	CbGain    float64                 `xml:"onvif:CbGain"`
	Extension WhiteBalance20Extension `xml:"onvif:Extension"`
}

type WhiteBalance20Extension xsd.AnyType

type ImagingSettingsExtension20 struct {
	ImageStabilization ImageStabilization          `xml:"onvif:ImageStabilization"`
	Extension          ImagingSettingsExtension202 `xml:"onvif:Extension"`
}

type ImageStabilization struct {
	Mode      ImageStabilizationMode      `xml:"onvif:Mode"`
	Level     float64                     `xml:"onvif:Level"`
	Extension ImageStabilizationExtension `xml:"onvif:Extension"`
}

type ImageStabilizationMode xsd.String

type ImageStabilizationExtension xsd.AnyType

type ImagingSettingsExtension202 struct {
	IrCutFilterAutoAdjustment IrCutFilterAutoAdjustment   `xml:"onvif:IrCutFilterAutoAdjustment"`
	Extension                 ImagingSettingsExtension203 `xml:"onvif:Extension"`
}

type IrCutFilterAutoAdjustment struct {
	BoundaryType   string                             `xml:"onvif:BoundaryType"`
	BoundaryOffset float64                            `xml:"onvif:BoundaryOffset"`
	ResponseTime   xsd.Duration                       `xml:"onvif:ResponseTime"`
	Extension      IrCutFilterAutoAdjustmentExtension `xml:"onvif:Extension"`
}

type IrCutFilterAutoAdjustmentExtension xsd.AnyType

type ImagingSettingsExtension203 struct {
	ToneCompensation ToneCompensation            `xml:"onvif:ToneCompensation"`
	Defogging        Defogging                   `xml:"onvif:Defogging"`
	NoiseReduction   NoiseReduction              `xml:"onvif:NoiseReduction"`
	Extension        ImagingSettingsExtension204 `xml:"onvif:Extension"`
}

type ToneCompensation struct {
	Mode      string                    `xml:"onvif:Mode"`
	Level     float64                   `xml:"onvif:Level"`
	Extension ToneCompensationExtension `xml:"onvif:Extension"`
}

type ToneCompensationExtension xsd.AnyType

type Defogging struct {
	Mode      string
	Level     float64
	Extension DefoggingExtension
}

type DefoggingExtension xsd.AnyType

type NoiseReduction struct {
	Level float64 `xml:"onvif:Level"`
}

type ImagingSettingsExtension204 xsd.AnyType

type VideoSourceExtension2 xsd.AnyType

type AudioSource struct {
	DeviceEntity
	Channels int
}

type AudioOutput struct {
	DeviceEntity
}

type Profile struct {
	Token                       ReferenceToken `xml:"token,attr"`
	Fixed                       bool           `xml:"fixed,attr"`
	Name                        Name
	VideoSourceConfiguration    VideoSourceConfiguration
	AudioSourceConfiguration    AudioSourceConfiguration
	VideoEncoderConfiguration   VideoEncoderConfiguration
	AudioEncoderConfiguration   AudioEncoderConfiguration
	VideoAnalyticsConfiguration VideoAnalyticsConfiguration
	PTZConfiguration            PTZConfiguration
	MetadataConfiguration       MetadataConfiguration
	Extension                   ProfileExtension
}

type VideoSourceConfiguration struct {
	ConfigurationEntity
	ViewMode    string                            `xml:"ViewMode,attr"`
	SourceToken ReferenceToken                    `xml:"onvif:SourceToken"`
	Bounds      IntRectangle                      `xml:"onvif:Bounds"`
	Extension   VideoSourceConfigurationExtension `xml:"onvif:Extension"`
}

type ConfigurationEntity struct {
	Token    ReferenceToken `xml:"token,attr"`
	Name     Name           `xml:"onvif:Name"`
	UseCount int            `xml:"onvif:UseCount"`
}

type VideoSourceConfigurationExtension struct {
	Rotate    Rotate                             `xml:"onvif:Rotate"`
	Extension VideoSourceConfigurationExtension2 `xml:"onvif:Extension"`
}

type Rotate struct {
	Mode      RotateMode      `xml:"onvif:Mode"`
	Degree    xsd.Int         `xml:"onvif:Degree"`
	Extension RotateExtension `xml:"onvif:Extension"`
}

type RotateMode xsd.String

type RotateExtension xsd.AnyType

type VideoSourceConfigurationExtension2 struct {
	LensDescription  LensDescription  `xml:"onvif:LensDescription"`
	SceneOrientation SceneOrientation `xml:"onvif:SceneOrientation"`
}

type LensDescription struct {
	FocalLength float64        `xml:"FocalLength,attr"`
	Offset      LensOffset     `xml:"onvif:Offset"`
	Projection  LensProjection `xml:"onvif:Projection"`
	XFactor     float64        `xml:"onvif:XFactor"`
}

type LensOffset struct {
	X float64 `xml:"x,attr"`
	Y float64 `xml:"y,attr"`
}

type LensProjection struct {
	Angle         float64 `xml:"onvif:Angle"`
	Radius        float64 `xml:"onvif:Radius"`
	Transmittance float64 `xml:"onvif:Transmittance"`
}

type SceneOrientation struct {
	Mode        SceneOrientationMode `xml:"onvif:Mode"`
	Orientation xsd.String           `xml:"onvif:Orientation"`
}

type SceneOrientationMode xsd.String

type AudioSourceConfiguration struct {
	ConfigurationEntity
	SourceToken ReferenceToken `xml:"onvif:SourceToken"`
}

type VideoEncoderConfiguration struct {
	ConfigurationEntity
	Encoding       VideoEncoding          `xml:"onvif:Encoding"`
	Resolution     VideoResolution        `xml:"onvif:Resolution"`
	Quality        float64                `xml:"onvif:Quality"`
	RateControl    VideoRateControl       `xml:"onvif:RateControl"`
	MPEG4          Mpeg4Configuration     `xml:"onvif:MPEG4"`
	H264           H264Configuration      `xml:"onvif:H264"`
	Multicast      MulticastConfiguration `xml:"onvif:Multicast"`
	SessionTimeout xsd.Duration           `xml:"onvif:SessionTimeout"`
}

type VideoEncoding xsd.String

type VideoRateControl struct {
	FrameRateLimit   xsd.Int `xml:"onvif:FrameRateLimit"`
	EncodingInterval xsd.Int `xml:"onvif:EncodingInterval"`
	BitrateLimit     xsd.Int `xml:"onvif:BitrateLimit"`
}

type Mpeg4Configuration struct {
	GovLength    xsd.Int      `xml:"onvif:GovLength"`
	Mpeg4Profile Mpeg4Profile `xml:"onvif:Mpeg4Profile"`
}

type Mpeg4Profile xsd.String

type H264Configuration struct {
	GovLength   xsd.Int     `xml:"onvif:GovLength"`
	H264Profile H264Profile `xml:"onvif:H264Profile"`
}

type H264Profile xsd.String

type MulticastConfiguration struct {
	Address   IPAddress   `xml:"onvif:Address"`
	Port      int         `xml:"onvif:Port"`
	TTL       int         `xml:"onvif:TTL"`
	AutoStart xsd.Boolean `xml:"onvif:AutoStart"`
}

type IPAddress struct {
	Type        IPType      `xml:"onvif:Type"`
	IPv4Address IPv4Address `xml:"onvif:IPv4Address"`
	IPv6Address IPv6Address `xml:"onvif:IPv6Address"`
}

type IPType xsd.String

//IPv4 address
type IPv4Address xsd.Token

//IPv6 address
type IPv6Address xsd.Token

type AudioEncoderConfiguration struct {
	ConfigurationEntity
	Encoding       AudioEncoding          `xml:"onvif:Encoding"`
	Bitrate        int                    `xml:"onvif:Bitrate"`
	SampleRate     int                    `xml:"onvif:SampleRate"`
	Multicast      MulticastConfiguration `xml:"onvif:Multicast"`
	SessionTimeout xsd.Duration           `xml:"onvif:SessionTimeout"`
}

type AudioEncoding xsd.String

type VideoAnalyticsConfiguration struct {
	ConfigurationEntity
	AnalyticsEngineConfiguration AnalyticsEngineConfiguration `xml:"onvif:AnalyticsEngineConfiguration"`
	RuleEngineConfiguration      RuleEngineConfiguration      `xml:"onvif:RuleEngineConfiguration"`
}

type AnalyticsEngineConfiguration struct {
	AnalyticsModule Config                                `xml:"onvif:AnalyticsModule"`
	Extension       AnalyticsEngineConfigurationExtension `xml:"onvif:Extension"`
}

type Config struct {
	Name       string    `xml:"Name,attr"`
	Type       xsd.QName `xml:"Type,attr"`
	Parameters ItemList  `xml:"onvif:Parameters"`
}

type ItemList struct {
	SimpleItem  SimpleItem        `xml:"onvif:SimpleItem"`
	ElementItem ElementItem       `xml:"onvif:ElementItem"`
	Extension   ItemListExtension `xml:"onvif:Extension"`
}

type SimpleItem struct {
	Name  string            `xml:"onvif:Name,attr"`
	Value xsd.AnySimpleType `xml:"onvif:Value,attr"`
}

type ElementItem struct {
	Name string `xml:"Name,attr"`
}

type ItemListExtension xsd.AnyType

type AnalyticsEngineConfigurationExtension xsd.AnyType

type RuleEngineConfiguration struct {
	Rule      Config                           `xml:"onvif:Rule"`
	Extension RuleEngineConfigurationExtension `xml:"onvif:Extension"`
}

type RuleEngineConfigurationExtension xsd.AnyType

type PTZConfiguration struct {
	ConfigurationEntity
	MoveRamp                               int                       `xml:"MoveRamp,attr"`
	PresetRamp                             int                       `xml:"PresetRamp,attr"`
	PresetTourRamp                         int                       `xml:"PresetTourRamp,attr"`
	NodeToken                              ReferenceToken            `xml:"NodeToken"`
	DefaultAbsolutePantTiltPositionSpace   xsd.AnyURI                `xml:"DefaultAbsolutePantTiltPositionSpace"`
	DefaultAbsoluteZoomPositionSpace       xsd.AnyURI                `xml:"DefaultAbsoluteZoomPositionSpace"`
	DefaultRelativePanTiltTranslationSpace xsd.AnyURI                `xml:"DefaultRelativePanTiltTranslationSpace"`
	DefaultRelativeZoomTranslationSpace    xsd.AnyURI                `xml:"DefaultRelativeZoomTranslationSpace"`
	DefaultContinuousPanTiltVelocitySpace  xsd.AnyURI                `xml:"DefaultContinuousPanTiltVelocitySpace"`
	DefaultContinuousZoomVelocitySpace     xsd.AnyURI                `xml:"DefaultContinuousZoomVelocitySpace"`
	DefaultPTZSpeed                        PTZSpeed                  `xml:"DefaultPTZSpeed"`
	DefaultPTZTimeout                      xsd.Duration              `xml:"DefaultPTZTimeout"`
	PanTiltLimits                          PanTiltLimits             `xml:"PanTiltLimits"`
	ZoomLimits                             ZoomLimits                `xml:"ZoomLimits"`
	Extension                              PTZConfigurationExtension `xml:"Extension"`
}

type PTZSpeed struct {
	PanTilt Vector2D `xml:"onvif:PanTilt"`
	Zoom    Vector1D `xml:"onvif:Zoom"`
}

type Vector2D struct {
	X     float64    `xml:"x,attr"`
	Y     float64    `xml:"y,attr"`
	Space xsd.AnyURI `xml:"space,attr"`
}

type Vector1D struct {
	X     float64    `xml:"x,attr"`
	Space xsd.AnyURI `xml:"space,attr"`
}

type PanTiltLimits struct {
	Range Space2DDescription `xml:"Range"`
}

type Space2DDescription struct {
	URI    xsd.AnyURI `xml:"URI"`
	XRange FloatRange `xml:"XRange"`
	YRange FloatRange `xml:"YRange"`
}

type ZoomLimits struct {
	Range Space1DDescription `xml:"Range"`
}

type Space1DDescription struct {
	URI    xsd.AnyURI `xml:"URI"`
	XRange FloatRange `xml:"XRange"`
}

type PTZConfigurationExtension struct {
	PTControlDirection PTControlDirection         `xml:"onvif:PTControlDirection"`
	Extension          PTZConfigurationExtension2 `xml:"onvif:Extension"`
}

type PTControlDirection struct {
	EFlip     EFlip                       `xml:"onvif:EFlip"`
	Reverse   Reverse                     `xml:"onvif:Reverse"`
	Extension PTControlDirectionExtension `xml:"onvif:Extension"`
}

type EFlip struct {
	Mode EFlipMode `xml:"onvif:Mode"`
}

type EFlipMode xsd.String

type Reverse struct {
	Mode ReverseMode `xml:"onvif:Mode"`
}

type ReverseMode xsd.String

type PTControlDirectionExtension xsd.AnyType

type PTZConfigurationExtension2 xsd.AnyType

type MetadataConfiguration struct {
	ConfigurationEntity
	CompressionType              string                         `xml:"CompressionType,attr"`
	PTZStatus                    PTZFilter                      `xml:"onvif:PTZStatus"`
	Events                       EventSubscription              `xml:"onvif:Events"`
	Analytics                    xsd.Boolean                    `xml:"onvif:Analytics"`
	Multicast                    MulticastConfiguration         `xml:"onvif:Multicast"`
	SessionTimeout               xsd.Duration                   `xml:"onvif:SessionTimeout"`
	AnalyticsEngineConfiguration AnalyticsEngineConfiguration   `xml:"onvif:AnalyticsEngineConfiguration"`
	Extension                    MetadataConfigurationExtension `xml:"onvif:Extension"`
}

type PTZFilter struct {
	Status   bool `xml:"onvif:Status"`
	Position bool `xml:"onvif:Position"`
}

type EventSubscription struct {
	Filter             FilterType `xml:"onvif:Filter"`
	SubscriptionPolicy `xml:"onvif:SubscriptionPolicy"`
}

type FilterType xsd.AnyType

type SubscriptionPolicy xsd.AnyType

type MetadataConfigurationExtension xsd.AnyType

type ProfileExtension struct {
	AudioOutputConfiguration  AudioOutputConfiguration
	AudioDecoderConfiguration AudioDecoderConfiguration
	Extension                 ProfileExtension2
}

type AudioOutputConfiguration struct {
	ConfigurationEntity
	OutputToken ReferenceToken `xml:"onvif:OutputToken"`
	SendPrimacy xsd.AnyURI     `xml:"onvif:SendPrimacy"`
	OutputLevel int            `xml:"onvif:OutputLevel"`
}

type AudioDecoderConfiguration struct {
	ConfigurationEntity
}

type ProfileExtension2 xsd.AnyType

type VideoSourceConfigurationOptions struct {
	MaximumNumberOfProfiles    int `xml:"MaximumNumberOfProfiles,attr"`
	BoundsRange                IntRectangleRange
	VideoSourceTokensAvailable ReferenceToken
	Extension                  VideoSourceConfigurationOptionsExtension
}

type VideoSourceConfigurationOptionsExtension struct {
	Rotate    RotateOptions
	Extension VideoSourceConfigurationOptionsExtension2
}

type RotateOptions struct {
	Mode       RotateMode
	DegreeList IntList
	Extension  RotateOptionsExtension
}

type IntList struct {
	Items []int
}

type RotateOptionsExtension xsd.AnyType

type VideoSourceConfigurationOptionsExtension2 struct {
	SceneOrientationMode SceneOrientationMode
}

type VideoEncoderConfigurationOptions struct {
	QualityRange IntRange
	JPEG         JpegOptions
	MPEG4        Mpeg4Options
	H264         H264Options
	Extension    VideoEncoderOptionsExtension
}

type JpegOptions struct {
	ResolutionsAvailable  VideoResolution
	FrameRateRange        IntRange
	EncodingIntervalRange IntRange
}

type Mpeg4Options struct {
	ResolutionsAvailable   VideoResolution
	GovLengthRange         IntRange
	FrameRateRange         IntRange
	EncodingIntervalRange  IntRange
	Mpeg4ProfilesSupported Mpeg4Profile
}

type H264Options struct {
	ResolutionsAvailable  VideoResolution
	GovLengthRange        IntRange
	FrameRateRange        IntRange
	EncodingIntervalRange IntRange
	H264ProfilesSupported H264Profile
}

type VideoEncoderOptionsExtension struct {
	JPEG      JpegOptions2
	MPEG4     Mpeg4Options2
	H264      H264Options2
	Extension VideoEncoderOptionsExtension2
}

type JpegOptions2 struct {
	JpegOptions
	BitrateRange IntRange
}

type Mpeg4Options2 struct {
	Mpeg4Options
	BitrateRange IntRange
}

type H264Options2 struct {
	H264Options
	BitrateRange IntRange
}

type VideoEncoderOptionsExtension2 xsd.AnyType

type AudioSourceConfigurationOptions struct {
	InputTokensAvailable ReferenceToken
	Extension            AudioSourceOptionsExtension
}

type AudioSourceOptionsExtension xsd.AnyType

type AudioEncoderConfigurationOptions struct {
	Options AudioEncoderConfigurationOption
}

type AudioEncoderConfigurationOption struct {
	Encoding       AudioEncoding
	BitrateList    IntList
	SampleRateList IntList
}

type MetadataConfigurationOptions struct {
	PTZStatusFilterOptions PTZStatusFilterOptions
	Extension              MetadataConfigurationOptionsExtension
}

type PTZStatusFilterOptions struct {
	PanTiltStatusSupported   bool
	ZoomStatusSupported      bool
	PanTiltPositionSupported bool
	ZoomPositionSupported    bool
	Extension                PTZStatusFilterOptionsExtension
}

type PTZStatusFilterOptionsExtension xsd.AnyType

type MetadataConfigurationOptionsExtension struct {
	CompressionType string
	Extension       MetadataConfigurationOptionsExtension2
}

type MetadataConfigurationOptionsExtension2 xsd.AnyType

type AudioOutputConfigurationOptions struct {
	OutputTokensAvailable ReferenceToken
	SendPrimacyOptions    xsd.AnyURI
	OutputLevelRange      IntRange
}

type AudioDecoderConfigurationOptions struct {
	AACDecOptions  AACDecOptions
	G711DecOptions G711DecOptions
	G726DecOptions G726DecOptions
	Extension      AudioDecoderConfigurationOptionsExtension
}

type AACDecOptions struct {
	Bitrate         IntList
	SampleRateRange IntList
}

type G711DecOptions struct {
	Bitrate         IntList
	SampleRateRange IntList
}

type G726DecOptions struct {
	Bitrate         IntList
	SampleRateRange IntList
}

type AudioDecoderConfigurationOptionsExtension xsd.AnyType

type StreamSetup struct {
	Stream    StreamType `xml:"onvif:Stream"`
	Transport Transport  `xml:"onvif:Transport"`
}

type StreamType xsd.String

type Transport struct {
	Protocol TransportProtocol `xml:"onvif:Protocol"`
	Tunnel   *Transport        `xml:"onvif:Tunnel"`
}

//enum
type TransportProtocol xsd.String

type MediaUri struct {
	Uri                 xsd.AnyURI
	InvalidAfterConnect bool
	InvalidAfterReboot  bool
	Timeout             xsd.Duration
}

type VideoSourceMode struct {
	Token         ReferenceToken `xml:"token,attr"`
	Enabled       bool           `xml:"Enabled,attr"`
	MaxFramerate  float64
	MaxResolution VideoResolution
	Encodings     EncodingTypes
	Reboot        bool
	Description   Description
	Extension     VideoSourceModeExtension
}

type EncodingTypes struct {
	EncodingTypes []string
}

type Description struct {
	Description string
}

type VideoSourceModeExtension xsd.AnyType

type OSDConfigurationOptions struct {
	MaximumNumberOfOSDs MaximumNumberOfOSDs
	Type                OSDType
	PositionOption      string
	TextOption          OSDTextOptions
	ImageOption         OSDImgOptions
	Extension           OSDConfigurationOptionsExtension
}

type MaximumNumberOfOSDs struct {
	Total       int `xml:"Total,attr"`
	Image       int `xml:"Image,attr"`
	PlainText   int `xml:"PlainText,attr"`
	Date        int `xml:"Date,attr"`
	Time        int `xml:"Time,attr"`
	DateAndTime int `xml:"DateAndTime,attr"`
}

type OSDTextOptions struct {
	Type            string
	FontSizeRange   IntRange
	DateFormat      string
	TimeFormat      string
	FontColor       OSDColorOptions
	BackgroundColor OSDColorOptions
	Extension       OSDTextOptionsExtension
}

type OSDColorOptions struct {
	Color       ColorOptions
	Transparent IntRange
	Extension   OSDColorOptionsExtension
}

type ColorOptions struct {
	ColorList       Color
	ColorspaceRange ColorspaceRange
}

type ColorspaceRange struct {
	X          FloatRange
	Y          FloatRange
	Z          FloatRange
	Colorspace xsd.AnyURI
}

type OSDColorOptionsExtension xsd.AnyType

type OSDTextOptionsExtension xsd.AnyType

type OSDImgOptions struct {
	FormatsSupported StringAttrList `xml:"FormatsSupported,attr"`
	MaxSize          int            `xml:"MaxSize,attr"`
	MaxWidth         int            `xml:"MaxWidth,attr"`
	MaxHeight        int            `xml:"MaxHeight,attr"`

	ImagePath xsd.AnyURI
	Extension OSDImgOptionsExtension
}

type StringAttrList struct {
	AttrList []string
}

type OSDImgOptionsExtension xsd.AnyType

type OSDConfigurationOptionsExtension xsd.AnyType

//PTZ

type PTZNode struct {
	DeviceEntity
	FixedHomePosition      xsd.Boolean `xml:"FixedHomePosition,attr"`
	GeoMove                xsd.Boolean `xml:"GeoMove,attr"`
	Name                   Name
	SupportedPTZSpaces     PTZSpaces
	MaximumNumberOfPresets int
	HomeSupported          xsd.Boolean
	AuxiliaryCommands      AuxiliaryData
	Extension              PTZNodeExtension
}

type PTZSpaces struct {
	AbsolutePanTiltPositionSpace    Space2DDescription
	AbsoluteZoomPositionSpace       Space1DDescription
	RelativePanTiltTranslationSpace Space2DDescription
	RelativeZoomTranslationSpace    Space1DDescription
	ContinuousPanTiltVelocitySpace  Space2DDescription
	ContinuousZoomVelocitySpace     Space1DDescription
	PanTiltSpeedSpace               Space1DDescription
	ZoomSpeedSpace                  Space1DDescription
	Extension                       PTZSpacesExtension
}

type PTZSpacesExtension xsd.AnyType

//TODO: restriction
type AuxiliaryData xsd.String

type PTZNodeExtension struct {
	SupportedPresetTour PTZPresetTourSupported
	Extension           PTZNodeExtension2
}

type PTZPresetTourSupported struct {
	MaximumNumberOfPresetTours int
	PTZPresetTourOperation     PTZPresetTourOperation
	Extension                  PTZPresetTourSupportedExtension
}

type PTZPresetTourOperation xsd.String
type PTZPresetTourSupportedExtension xsd.AnyType

type PTZNodeExtension2 xsd.AnyType

type PTZConfigurationOptions struct {
	PTZRamps           IntAttrList `xml:"PTZRamps,attr"`
	Spaces             PTZSpaces
	PTZTimeout         DurationRange
	PTControlDirection PTControlDirectionOptions
	Extension          PTZConfigurationOptions2
}

type IntAttrList struct {
	IntAttrList []int
}

type DurationRange struct {
	Min xsd.Duration
	Max xsd.Duration
}

type PTControlDirectionOptions struct {
	EFlip     EFlipOptions
	Reverse   ReverseOptions
	Extension PTControlDirectionOptionsExtension
}

type EFlipOptions struct {
	Mode      EFlipMode
	Extension EFlipOptionsExtension
}

type EFlipOptionsExtension xsd.AnyType

type ReverseOptions struct {
	Mode      ReverseMode
	Extension ReverseOptionsExtension
}

type ReverseOptionsExtension xsd.AnyType

type PTControlDirectionOptionsExtension xsd.AnyType

type PTZConfigurationOptions2 xsd.AnyType

type PTZPreset struct {
	Token       ReferenceToken `xml:"token,attr"`
	Name        Name
	PTZPosition PTZVector
}

type PTZVector struct {
	PanTilt Vector2D `xml:"onvif:PanTilt"`
	Zoom    Vector1D `xml:"onvif:Zoom"`
}

type PTZStatus struct {
	Position   PTZVector
	MoveStatus PTZMoveStatus
	Error      string
	UtcTime    xsd.DateTime
}

type PTZMoveStatus struct {
	PanTilt MoveStatus
	Zoom    MoveStatus
}

type MoveStatus struct {
	Status string
}

type GeoLocation struct {
	Lon       xsd.Double `xml:"lon,attr"`
	Lat       xsd.Double `xml:"lat,attr"`
	Elevation xsd.Float  `xml:"elevation,attr"`
}

type PresetTour struct {
	Token             ReferenceToken                 `xml:"token,attr"`
	Name              Name                           `xml:"onvif:Name"`
	Status            PTZPresetTourStatus            `xml:"onvif:Status"`
	AutoStart         xsd.Boolean                    `xml:"onvif:AutoStart"`
	StartingCondition PTZPresetTourStartingCondition `xml:"onvif:StartingCondition"`
	TourSpot          PTZPresetTourSpot              `xml:"onvif:TourSpot"`
	Extension         PTZPresetTourExtension         `xml:"onvif:Extension"`
}

type PTZPresetTourStatus struct {
	State           PTZPresetTourState           `xml:"onvif:State"`
	CurrentTourSpot PTZPresetTourSpot            `xml:"onvif:CurrentTourSpot"`
	Extension       PTZPresetTourStatusExtension `xml:"onvif:Extension"`
}

type PTZPresetTourState xsd.String

type PTZPresetTourSpot struct {
	PresetDetail PTZPresetTourPresetDetail  `xml:"onvif:PresetDetail"`
	Speed        PTZSpeed                   `xml:"onvif:Speed"`
	StayTime     xsd.Duration               `xml:"onvif:StayTime"`
	Extension    PTZPresetTourSpotExtension `xml:"onvif:Extension"`
}

type PTZPresetTourPresetDetail struct {
	PresetToken   ReferenceToken             `xml:"onvif:PresetToken"`
	Home          xsd.Boolean                `xml:"onvif:Home"`
	PTZPosition   PTZVector                  `xml:"onvif:PTZPosition"`
	TypeExtension PTZPresetTourTypeExtension `xml:"onvif:TypeExtension"`
}

type PTZPresetTourTypeExtension xsd.AnyType

type PTZPresetTourSpotExtension xsd.AnyType

type PTZPresetTourStatusExtension xsd.AnyType

type PTZPresetTourStartingCondition struct {
	RandomPresetOrder xsd.Boolean                             `xml:"RandomPresetOrder,attr"`
	RecurringTime     xsd.Int                                 `xml:"onvif:RecurringTime"`
	RecurringDuration xsd.Duration                            `xml:"onvif:RecurringDuration"`
	Direction         PTZPresetTourDirection                  `xml:"onvif:Direction"`
	Extension         PTZPresetTourStartingConditionExtension `xml:"onvif:Extension"`
}

type PTZPresetTourDirection xsd.String

type PTZPresetTourStartingConditionExtension xsd.AnyType

type PTZPresetTourExtension xsd.AnyType

type PTZPresetTourOptions struct {
	AutoStart         xsd.Boolean
	StartingCondition PTZPresetTourStartingConditionOptions
	TourSpot          PTZPresetTourSpotOptions
}

type PTZPresetTourStartingConditionOptions struct {
	RecurringTime     IntRange
	RecurringDuration DurationRange
	Direction         PTZPresetTourDirection
	Extension         PTZPresetTourStartingConditionOptionsExtension
}

type PTZPresetTourStartingConditionOptionsExtension xsd.AnyType

type PTZPresetTourSpotOptions struct {
	PresetDetail PTZPresetTourPresetDetailOptions
	StayTime     DurationRange
}

type PTZPresetTourPresetDetailOptions struct {
	PresetToken          ReferenceToken
	Home                 xsd.Boolean
	PanTiltPositionSpace Space2DDescription
	ZoomPositionSpace    Space1DDescription
	Extension            PTZPresetTourPresetDetailOptionsExtension
}

type PTZPresetTourPresetDetailOptionsExtension xsd.AnyType

//Device

type OnvifVersion struct {
	Major int
	Minor int
}

type SetDateTimeType xsd.String

type TimeZone struct {
	TZ xsd.Token `xml:"onvif:TZ"`
}

type SystemDateTime struct {
	DateTimeType    SetDateTimeType
	DaylightSavings xsd.Boolean
	TimeZone        TimeZone
	UTCDateTime     xsd.DateTime
	LocalDateTime   xsd.DateTime
	Extension       SystemDateTimeExtension
}

type SystemDateTimeExtension xsd.AnyType

type FactoryDefaultType xsd.String

type AttachmentData struct {
	ContentType ContentType `xml:"contentType,attr"`
	Include     Include     `xml:"inc:Include"`
}

type Include struct {
	Href xsd.AnyURI `xml:"href,attr"`
}

type BackupFile struct {
	Name string         `xml:"onvif:Name"`
	Data AttachmentData `xml:"onvif:Data"`
}

type SystemLogType xsd.String

type SystemLog struct {
	Binary AttachmentData
	String string
}

type SupportInformation struct {
	Binary AttachmentData
	String string
}

type Scope struct {
	ScopeDef  ScopeDefinition
	ScopeItem xsd.AnyURI
}

type ScopeDefinition xsd.String

type DiscoveryMode xsd.String

type NetworkHost struct {
	Type        NetworkHostType      `xml:"onvif:Type"`
	IPv4Address IPv4Address          `xml:"onvif:IPv4Address"`
	IPv6Address IPv6Address          `xml:"onvif:IPv6Address"`
	DNSname     DNSName              `xml:"onvif:DNSname"`
	Extension   NetworkHostExtension `xml:"onvif:Extension"`
}

type NetworkHostType xsd.String

type NetworkHostExtension xsd.String

type RemoteUser struct {
	Username           string      `xml:"onvif:Username"`
	Password           string      `xml:"onvif:Password"`
	UseDerivedPassword xsd.Boolean `xml:"onvif:UseDerivedPassword"`
}

type User struct {
	Username  string        `xml:"onvif:Username"`
	Password  string        `xml:"onvif:Password"`
	UserLevel UserLevel     `xml:"onvif:UserLevel"`
	Extension UserExtension `xml:"onvif:Extension"`
}

type UserLevel xsd.String

type UserExtension xsd.String

type CapabilityCategory xsd.String

//Capabilities of device
type Capabilities struct {
	Analytics AnalyticsCapabilities
	Device    DeviceCapabilities
	Events    EventCapabilities
	Imaging   ImagingCapabilities
	Media     MediaCapabilities
	PTZ       PTZCapabilities
	Extension CapabilitiesExtension
}

//AnalyticsCapabilities Check
type AnalyticsCapabilities struct {
	XAddr                  xsd.AnyURI
	RuleSupport            xsd.Boolean
	AnalyticsModuleSupport xsd.Boolean
}

//DeviceCapabilities Check
type DeviceCapabilities struct {
	XAddr     xsd.AnyURI
	Network   NetworkCapabilities
	System    SystemCapabilities
	IO        IOCapabilities
	Security  SecurityCapabilities
	Extension DeviceCapabilitiesExtension
}

//NetworkCapabilities Check
type NetworkCapabilities struct {
	IPFilter          xsd.Boolean
	ZeroConfiguration xsd.Boolean
	IPVersion6        xsd.Boolean
	DynDNS            xsd.Boolean
	Extension         NetworkCapabilitiesExtension
}

//NetworkCapabilitiesExtension Check
type NetworkCapabilitiesExtension struct {
	Dot11Configuration xsd.Boolean
	Extension          NetworkCapabilitiesExtension2
}

//NetworkCapabilitiesExtension2 Extension2
type NetworkCapabilitiesExtension2 xsd.AnyType

//SystemCapabilities check
type SystemCapabilities struct {
	DiscoveryResolve  xsd.Boolean
	DiscoveryBye      xsd.Boolean
	RemoteDiscovery   xsd.Boolean
	SystemBackup      xsd.Boolean
	SystemLogging     xsd.Boolean
	FirmwareUpgrade   xsd.Boolean
	SupportedVersions OnvifVersion
	Extension         SystemCapabilitiesExtension
}

type SystemCapabilitiesExtension struct {
	HttpFirmwareUpgrade    xsd.Boolean
	HttpSystemBackup       xsd.Boolean
	HttpSystemLogging      xsd.Boolean
	HttpSupportInformation xsd.Boolean
	Extension              SystemCapabilitiesExtension2
}

type SystemCapabilitiesExtension2 xsd.AnyType

type IOCapabilities struct {
	InputConnectors int
	RelayOutputs    int
	Extension       IOCapabilitiesExtension
}

type IOCapabilitiesExtension struct {
	Auxiliary         xsd.Boolean
	AuxiliaryCommands AuxiliaryData
	Extension         IOCapabilitiesExtension2
}

type IOCapabilitiesExtension2 xsd.AnyType

type SecurityCapabilities struct {
	TLS1_1               xsd.Boolean
	TLS1_2               xsd.Boolean
	OnboardKeyGeneration xsd.Boolean
	AccessPolicyConfig   xsd.Boolean
	X_509Token           xsd.Boolean
	SAMLToken            xsd.Boolean
	KerberosToken        xsd.Boolean
	RELToken             xsd.Boolean
	Extension            SecurityCapabilitiesExtension
}

type SecurityCapabilitiesExtension struct {
	TLS1_0    xsd.Boolean
	Extension SecurityCapabilitiesExtension2
}

type SecurityCapabilitiesExtension2 struct {
	Dot1X              xsd.Boolean
	SupportedEAPMethod int
	RemoteUserHandling xsd.Boolean
}

type DeviceCapabilitiesExtension xsd.AnyType

type EventCapabilities struct {
	XAddr                                         xsd.AnyURI
	WSSubscriptionPolicySupport                   xsd.Boolean
	WSPullPointSupport                            xsd.Boolean
	WSPausableSubscriptionManagerInterfaceSupport xsd.Boolean
}

type ImagingCapabilities struct {
	XAddr xsd.AnyURI
}

type MediaCapabilities struct {
	XAddr                 xsd.AnyURI
	StreamingCapabilities RealTimeStreamingCapabilities
	Extension             MediaCapabilitiesExtension
}

type RealTimeStreamingCapabilities struct {
	RTPMulticast xsd.Boolean
	RTP_TCP      xsd.Boolean
	RTP_RTSP_TCP xsd.Boolean
	Extension    RealTimeStreamingCapabilitiesExtension
}

type RealTimeStreamingCapabilitiesExtension xsd.AnyType

type MediaCapabilitiesExtension struct {
	ProfileCapabilities ProfileCapabilities
}

type ProfileCapabilities struct {
	MaximumNumberOfProfiles int
}

type PTZCapabilities struct {
	XAddr xsd.AnyURI
}

type CapabilitiesExtension struct {
	DeviceIO        DeviceIOCapabilities
	Display         DisplayCapabilities
	Recording       RecordingCapabilities
	Search          SearchCapabilities
	Replay          ReplayCapabilities
	Receiver        ReceiverCapabilities
	AnalyticsDevice AnalyticsDeviceCapabilities
	Extensions      CapabilitiesExtension2
}

type DeviceIOCapabilities struct {
	XAddr        xsd.AnyURI
	VideoSources int
	VideoOutputs int
	AudioSources int
	AudioOutputs int
	RelayOutputs int
}

type DisplayCapabilities struct {
	XAddr       xsd.AnyURI
	FixedLayout xsd.Boolean
}

type RecordingCapabilities struct {
	XAddr              xsd.AnyURI
	ReceiverSource     xsd.Boolean
	MediaProfileSource xsd.Boolean
	DynamicRecordings  xsd.Boolean
	DynamicTracks      xsd.Boolean
	MaxStringLength    int
}

type SearchCapabilities struct {
	XAddr          xsd.AnyURI
	MetadataSearch xsd.Boolean
}

type ReplayCapabilities struct {
	XAddr xsd.AnyURI
}

type ReceiverCapabilities struct {
	XAddr                xsd.AnyURI
	RTP_Multicast        xsd.Boolean
	RTP_TCP              xsd.Boolean
	RTP_RTSP_TCP         xsd.Boolean
	SupportedReceivers   int
	MaximumRTSPURILength int
}

type AnalyticsDeviceCapabilities struct {
	XAddr       xsd.AnyURI
	RuleSupport xsd.Boolean
	Extension   AnalyticsDeviceExtension
}

type AnalyticsDeviceExtension xsd.AnyType

type CapabilitiesExtension2 xsd.AnyType

type HostnameInformation struct {
	FromDHCP  xsd.Boolean
	Name      xsd.Token
	Extension HostnameInformationExtension
}

type HostnameInformationExtension xsd.AnyType

type DNSInformation struct {
	FromDHCP     xsd.Boolean
	SearchDomain xsd.Token
	DNSFromDHCP  IPAddress
	DNSManual    IPAddress
	Extension    DNSInformationExtension
}

type DNSInformationExtension xsd.AnyType

type NTPInformation struct {
	FromDHCP    xsd.Boolean
	NTPFromDHCP NetworkHost
	NTPManual   NetworkHost
	Extension   NTPInformationExtension
}

type NTPInformationExtension xsd.AnyType

type DynamicDNSInformation struct {
	Type      DynamicDNSType
	Name      DNSName
	TTL       xsd.Duration
	Extension DynamicDNSInformationExtension
}

//TODO: enumeration
type DynamicDNSType xsd.String

type DynamicDNSInformationExtension xsd.AnyType

type NetworkInterface struct {
	DeviceEntity
	Enabled   xsd.Boolean
	Info      NetworkInterfaceInfo
	Link      NetworkInterfaceLink
	IPv4      IPv4NetworkInterface
	IPv6      IPv6NetworkInterface
	Extension NetworkInterfaceExtension
}

type NetworkInterfaceInfo struct {
	Name      xsd.String
	HwAddress HwAddress
	MTU       xsd.Int
}

type HwAddress xsd.Token

type NetworkInterfaceLink struct {
	AdminSettings NetworkInterfaceConnectionSetting
	OperSettings  NetworkInterfaceConnectionSetting
	InterfaceType IANA_IfTypes `xml:"IANA-IfTypes"`
}

type IANA_IfTypes xsd.Int

type NetworkInterfaceConnectionSetting struct {
	AutoNegotiation xsd.Boolean `xml:"onvif:AutoNegotiation"`
	Speed           xsd.Int     `xml:"onvif:Speed"`
	Duplex          Duplex      `xml:"onvif:Duplex"`
}

//TODO: enum
type Duplex xsd.String

type NetworkInterfaceExtension struct {
	InterfaceType IANA_IfTypes
	Dot3          Dot3Configuration
	Dot11         Dot11Configuration
	Extension     NetworkInterfaceExtension2
}

type NetworkInterfaceExtension2 xsd.AnyType

type Dot11Configuration struct {
	SSID     Dot11SSIDType                  `xml:"onvif:SSID"`
	Mode     Dot11StationMode               `xml:"onvif:Mode"`
	Alias    Name                           `xml:"onvif:Alias"`
	Priority NetworkInterfaceConfigPriority `xml:"onvif:Priority"`
	Security Dot11SecurityConfiguration     `xml:"onvif:Security"`
}

type Dot11SecurityConfiguration struct {
	Mode      Dot11SecurityMode                   `xml:"onvif:Mode"`
	Algorithm Dot11Cipher                         `xml:"onvif:Algorithm"`
	PSK       Dot11PSKSet                         `xml:"onvif:PSK"`
	Dot1X     ReferenceToken                      `xml:"onvif:Dot1X"`
	Extension Dot11SecurityConfigurationExtension `xml:"onvif:Extension"`
}

type Dot11SecurityConfigurationExtension xsd.AnyType

type Dot11PSKSet struct {
	Key        Dot11PSK             `xml:"onvif:Key"`
	Passphrase Dot11PSKPassphrase   `xml:"onvif:Passphrase"`
	Extension  Dot11PSKSetExtension `xml:"onvif:Extension"`
}

type Dot11PSKSetExtension xsd.AnyType

type Dot11PSKPassphrase xsd.String

type Dot11PSK xsd.HexBinary

//TODO: enumeration
type Dot11Cipher xsd.String

//TODO: enumeration
type Dot11SecurityMode xsd.String

//TODO: restrictions
type NetworkInterfaceConfigPriority xsd.Integer

//TODO: enumeration
type Dot11StationMode xsd.String

//TODO: restrictions
type Dot11SSIDType xsd.HexBinary

type Dot3Configuration xsd.String

type IPv6NetworkInterface struct {
	Enabled xsd.Boolean
	Config  IPv6Configuration
}

type IPv6Configuration struct {
	AcceptRouterAdvert xsd.Boolean
	DHCP               IPv6DHCPConfiguration
	Manual             PrefixedIPv6Address
	LinkLocal          PrefixedIPv6Address
	FromDHCP           PrefixedIPv6Address
	FromRA             PrefixedIPv6Address
	Extension          IPv6ConfigurationExtension
}

type IPv6ConfigurationExtension xsd.AnyType

type PrefixedIPv6Address struct {
	Address      IPv6Address `xml:"onvif:Address"`
	PrefixLength xsd.Int     `xml:"onvif:PrefixLength"`
}

//TODO: enumeration
type IPv6DHCPConfiguration xsd.String

type IPv4NetworkInterface struct {
	Enabled xsd.Boolean
	Config  IPv4Configuration
}

type IPv4Configuration struct {
	Manual    PrefixedIPv4Address
	LinkLocal PrefixedIPv4Address
	FromDHCP  PrefixedIPv4Address
	DHCP      xsd.Boolean
}

//optional, unbounded
type PrefixedIPv4Address struct {
	Address      IPv4Address `xml:"onvif:Address"`
	PrefixLength xsd.Int     `xml:"onvif:PrefixLength"`
}

type NetworkInterfaceSetConfiguration struct {
	Enabled   xsd.Boolean                               `xml:"onvif:Enabled"`
	Link      NetworkInterfaceConnectionSetting         `xml:"onvif:Link"`
	MTU       xsd.Int                                   `xml:"onvif:MTU"`
	IPv4      IPv4NetworkInterfaceSetConfiguration      `xml:"onvif:IPv4"`
	IPv6      IPv6NetworkInterfaceSetConfiguration      `xml:"onvif:IPv6"`
	Extension NetworkInterfaceSetConfigurationExtension `xml:"onvif:Extension"`
}

type NetworkInterfaceSetConfigurationExtension struct {
	Dot3      Dot3Configuration                          `xml:"onvif:Dot3"`
	Dot11     Dot11Configuration                         `xml:"onvif:Dot11"`
	Extension NetworkInterfaceSetConfigurationExtension2 `xml:"onvif:Extension"`
}

type NetworkInterfaceSetConfigurationExtension2 xsd.AnyType

type IPv6NetworkInterfaceSetConfiguration struct {
	Enabled            xsd.Boolean           `xml:"onvif:Enabled"`
	AcceptRouterAdvert xsd.Boolean           `xml:"onvif:AcceptRouterAdvert"`
	Manual             PrefixedIPv6Address   `xml:"onvif:Manual"`
	DHCP               IPv6DHCPConfiguration `xml:"onvif:DHCP"`
}

type IPv4NetworkInterfaceSetConfiguration struct {
	Enabled xsd.Boolean         `xml:"onvif:Enabled"`
	Manual  PrefixedIPv4Address `xml:"onvif:Manual"`
	DHCP    xsd.Boolean         `xml:"onvif:DHCP"`
}

type NetworkProtocol struct {
	Name      NetworkProtocolType      `xml:"onvif:Name"`
	Enabled   xsd.Boolean              `xml:"onvif:Enabled"`
	Port      xsd.Int                  `xml:"onvif:Port"`
	Extension NetworkProtocolExtension `xml:"onvif:Extension"`
}

type NetworkProtocolExtension xsd.AnyType

//TODO: enumeration
type NetworkProtocolType xsd.String

type NetworkGateway struct {
	IPv4Address IPv4Address
	IPv6Address IPv6Address
}

type NetworkZeroConfiguration struct {
	InterfaceToken ReferenceToken
	Enabled        xsd.Boolean
	Addresses      IPv4Address
	Extension      NetworkZeroConfigurationExtension
}

type NetworkZeroConfigurationExtension struct {
	Additional *NetworkZeroConfiguration
	Extension  NetworkZeroConfigurationExtension2
}

type NetworkZeroConfigurationExtension2 xsd.AnyType

type IPAddressFilter struct {
	Type        IPAddressFilterType      `xml:"onvif:Type"`
	IPv4Address PrefixedIPv4Address      `xml:"onvif:IPv4Address,omitempty"`
	IPv6Address PrefixedIPv6Address      `xml:"onvif:IPv6Address,omitempty"`
	Extension   IPAddressFilterExtension `xml:"onvif:Extension,omitempty"`
}

type IPAddressFilterExtension xsd.AnyType

//enum { 'Allow', 'Deny' }
//TODO: enumeration
type IPAddressFilterType xsd.String

//TODO: attribite <xs:attribute ref="xmime:contentType" use="optional"/>
type BinaryData struct {
	X    ContentType      `xml:"xmime:contentType,attr"`
	Data xsd.Base64Binary `xml:"onvif:Data"`
}

type Certificate struct {
	CertificateID xsd.Token  `xml:"onvif:CertificateID"`
	Certificate   BinaryData `xml:"onvif:Certificate"`
}

type CertificateStatus struct {
	CertificateID xsd.Token   `xml:"onvif:CertificateID"`
	Status        xsd.Boolean `xml:"onvif:Status"`
}

type RelayOutput struct {
	DeviceEntity
	Properties RelayOutputSettings
}

type RelayOutputSettings struct {
	Mode      RelayMode      `xml:"onvif:Mode"`
	DelayTime xsd.Duration   `xml:"onvif:DelayTime"`
	IdleState RelayIdleState `xml:"onvif:IdleState"`
}

//TODO:enumeration
type RelayIdleState xsd.String

//TODO: enumeration
type RelayMode xsd.String

//TODO: enumeration
type RelayLogicalState xsd.String

type CertificateWithPrivateKey struct {
	CertificateID xsd.Token  `xml:"onvif:CertificateID"`
	Certificate   BinaryData `xml:"onvif:Certificate"`
	PrivateKey    BinaryData `xml:"onvif:PrivateKey"`
}

type CertificateInformation struct {
	CertificateID      xsd.Token
	IssuerDN           xsd.String
	SubjectDN          xsd.String
	KeyUsage           CertificateUsage
	ExtendedKeyUsage   CertificateUsage
	KeyLength          xsd.Int
	Version            xsd.String
	SerialNum          xsd.String
	SignatureAlgorithm xsd.String
	Validity           DateTimeRange
	Extension          CertificateInformationExtension
}

type CertificateInformationExtension xsd.AnyType

type DateTimeRange struct {
	From  xsd.DateTime
	Until xsd.DateTime
}

type CertificateUsage struct {
	Critical         xsd.Boolean `xml:"Critical,attr"`
	CertificateUsage xsd.String
}

type Dot1XConfiguration struct {
	Dot1XConfigurationToken ReferenceToken              `xml:"onvif:Dot1XConfigurationToken"`
	Identity                xsd.String                  `xml:"onvif:Identity"`
	AnonymousID             xsd.String                  `xml:"onvif:AnonymousID,omitempty"`
	EAPMethod               xsd.Int                     `xml:"onvif:EAPMethod"`
	CACertificateID         xsd.Token                   `xml:"onvif:CACertificateID,omitempty"`
	EAPMethodConfiguration  EAPMethodConfiguration      `xml:"onvif:EAPMethodConfiguration,omitempty"`
	Extension               Dot1XConfigurationExtension `xml:"onvif:Extension,omitempty"`
}

type Dot1XConfigurationExtension xsd.AnyType

type EAPMethodConfiguration struct {
	TLSConfiguration TLSConfiguration   `xml:"onvif:TLSConfiguration,omitempty"`
	Password         xsd.String         `xml:"onvif:Password,omitempty"`
	Extension        EapMethodExtension `xml:"onvif:Extension,omitempty"`
}

type EapMethodExtension xsd.AnyType

type TLSConfiguration struct {
	CertificateID xsd.Token `xml:"onvif:CertificateID,omitempty"`
}

type Dot11Capabilities struct {
	TKIP                  xsd.Boolean
	ScanAvailableNetworks xsd.Boolean
	MultipleConfiguration xsd.Boolean
	AdHocStationMode      xsd.Boolean
	WEP                   xsd.Boolean
}

type Dot11Status struct {
	SSID              Dot11SSIDType
	BSSID             xsd.String
	PairCipher        Dot11Cipher
	GroupCipher       Dot11Cipher
	SignalStrength    Dot11SignalStrength
	ActiveConfigAlias ReferenceToken
}

//TODO: enumeration
type Dot11SignalStrength xsd.String

type Dot11AvailableNetworks struct {
	SSID                  Dot11SSIDType
	BSSID                 xsd.String
	AuthAndMangementSuite Dot11AuthAndMangementSuite
	PairCipher            Dot11Cipher
	GroupCipher           Dot11Cipher
	SignalStrength        Dot11SignalStrength
	Extension             Dot11AvailableNetworksExtension
}

type Dot11AvailableNetworksExtension xsd.AnyType

//TODO: enumeration
type Dot11AuthAndMangementSuite xsd.String

type SystemLogUriList struct {
	SystemLog SystemLogUri
}

type SystemLogUri struct {
	Type SystemLogType
	Uri  xsd.AnyURI
}

type LocationEntity struct {
	Entity    xsd.String     `xml:"Entity,attr"`
	Token     ReferenceToken `xml:"Token,attr"`
	Fixed     xsd.Boolean    `xml:"Fixed,attr"`
	GeoSource xsd.AnyURI     `xml:"GeoSource,attr"`
	AutoGeo   xsd.Boolean    `xml:"AutoGeo,attr"`

	GeoLocation      GeoLocation      `xml:"onvif:GeoLocation"`
	GeoOrientation   GeoOrientation   `xml:"onvif:GeoOrientation"`
	LocalLocation    LocalLocation    `xml:"onvif:LocalLocation"`
	LocalOrientation LocalOrientation `xml:"onvif:LocalOrientation"`
}

type LocalOrientation struct {
	Lon       xsd.Double `xml:"lon,attr"`
	Lat       xsd.Double `xml:"lat,attr"`
	Elevation xsd.Float  `xml:"elevation,attr"`
}

type LocalLocation struct {
	X xsd.Float `xml:"x,attr"`
	Y xsd.Float `xml:"y,attr"`
	Z xsd.Float `xml:"z,attr"`
}

type GeoOrientation struct {
	Roll  xsd.Float `xml:"roll,attr"`
	Pitch xsd.Float `xml:"pitch,attr"`
	Yaw   xsd.Float `xml:"yaw,attr"`
}

type FocusMove struct {
	Absolute   AbsoluteFocus   `xml:"onvif:Absolute"`
	Relative   RelativeFocus   `xml:"onvif:Relative"`
	Continuous ContinuousFocus `xml:"onvif:Continuous"`
}

type ContinuousFocus struct {
	Speed xsd.Float `xml:"onvif:Speed"`
}

type RelativeFocus struct {
	Distance xsd.Float `xml:"onvif:Distance"`
	Speed    xsd.Float `xml:"onvif:Speed"`
}

type AbsoluteFocus struct {
	Position xsd.Float `xml:"onvif:Position"`
	Speed    xsd.Float `xml:"onvif:Speed"`
}

type DateTime struct {
	Time Time `xml:"onvif:Time"`
	Date Date `xml:"onvif:Date"`
}

type Time struct {
	Hour   xsd.Int `xml:"onvif:Hour"`
	Minute xsd.Int `xml:"onvif:Minute"`
	Second xsd.Int `xml:"onvif:Second"`
}

type Date struct {
	Year  xsd.Int `xml:"onvif:Year"`
	Month xsd.Int `xml:"onvif:Month"`
	Day   xsd.Int `xml:"onvif:Day"`
}
