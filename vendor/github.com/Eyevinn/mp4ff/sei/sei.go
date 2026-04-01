package sei

import (
	"encoding/hex"
	"errors"
	"fmt"
	"io"

	"github.com/Eyevinn/mp4ff/bits"
)

var ErrRbspTrailingBitsMissing = errors.New("rbsp_trailing_bits byte 0x80 is missing")

const (
	// Combined definition of AVC and HEVC messages. When names collide for the same number, AVC or HEVC is added in the name.
	// AVC defined in ISO/IEC 14496-10:2020 Annex D and HEVC in ISO/IEC 23008-2:2020 Annex D.
	// SEIBufferingPeriodType is defined in AVC D.1.2 and HEVC D.2.2. Definitions differ.
	SEIBufferingPeriodType = 0
	// SEIPicTimingType is defined in AVC D.1.3 and HEVC D.2.3. Definitions differ.
	SEIPicTimingType = 1
	// SEIPanScanRectType is defined in AVC D.1.4 and HEVC D.2.4. Definitions differ.
	SEIPanScanRectType = 2
	// SEIFillerPayloadType is defined in AVC D.1.5 and HEVC D.2.5. Definitions agree.
	SEIFillerPayloadType = 3
	// SEIUserDataRegisteredITUtT35Type is defined in AVC D.1.6 and HEVC D.2.6. Definitions agree.
	SEIUserDataRegisteredITUtT35Type = 4
	// SEIUserDataUnregisteredType is defined in AVC D.1.7 and HEVC D.2.7. Definitions agree.
	SEIUserDataUnregisteredType = 5
	// SEIRecoveryPointType is defined in AVC D.1.8 and HEVC D.2.8. Definitions differ.
	SEIRecoveryPointType = 6
	// SEIDecRefPicMarkingRepetitionType is defined in AVC D.1.9. Absent in HEVC.
	SEIDecRefPicMarkingRepetitionType = 7
	// SEISparePicType is defined in AVC D.1.10. Absent in HEVC.
	SEISparePicType = 8
	// SEISceneInfoType is defined in AVC D.1.11 and HEVC 2.9. Definitions differ.
	SEISceneInfoType = 9
	// SEISubSeqInfoType is defined in AVC D.1.12. Absent in HEVC.
	SEISubSeqInfoType = 10
	// SEISubSeqLayerCharacteristicsType is defined in AVC D.1.13. Absent in HEVC.
	SEISubSeqLayerCharacteristicsType = 11
	// SEISubSeqCharacteristicsType is defined in AVC D.1.14. Absent in HEVC.
	SEISubSeqCharacteristicsType = 12
	// SEIFullFrameFreezeType is defined in AVC D.1.15. Absent in HEVC.
	SEIFullFrameFreezeType = 13
	// SEIFullFrameFreezeReleaseType is defined in AVC D.1.16. Absent in HEVC.
	SEIFullFrameFreezeReleaseType = 14
	// SEIPictureSnapShotType is defined in AVC D.1.17 and HEVC D.2.10. Definitions agree, but called FullFrameSnapshot in AVC.
	SEIPictureSnapShotType = 15
	// SEIProgressiveRefinementSegmentStartType is defined in AVC D.1.18 and HEVC D.2.11. Definitions differ.
	SEIProgressiveRefinementSegmentStartType = 16
	// SEIProgressiveRefinementSegmentStartEnd is defined in AVC D.1.19 and HEVC D.2.12. Definitions agree.
	SEIProgressiveRefinementSegmentStartEnd = 17 // AVC and HEVC. Same definition
	// SEIMotionConstrainedSliceGroupSetType is defined in AVC D.1.20. Absent in HEVC.
	SEIMotionConstrainedSliceGroupSetType = 18
	// SEIFilmGrainCharacteristicsType is defined in AVC D.1.21 and HEVC D.2.13. Definitions differ.
	SEIFilmGrainCharacteristicsType = 19
	// SEIDeblockingFilterDisplayPreferenceType is defined in AVC D.1.22. Absent in HEVC.
	SEIDeblockingFilterDisplayPreferenceType = 20
	// SEIStereoVideoInfoType is defined in AVC D.1.23. Absent in HEVC.
	SEIStereoVideoInfoType = 21
	// SEIPostFilterHintType is defined in AVC D.1.24 and HEVC D.2.14. Definitions differ.
	SEIPostFilterHintType = 22
	// SEIToneMappingInfoType is defined in AVC D.1.25 and HEVC D.15. Definitions agree.
	SEIToneMappingInfoType = 23 // AVC and HEVC. Same definition
	// SEIScalabilityInfoType is defined in AVC Annex F. Absent in HEVC.
	SEIScalabilityInfoType = 24
	// SEISubPicScalableLayerType is defined in AVC Annex F. Absent in HEVC.
	SEISubPicScalableLayerType = 25
	// SEINonRequiredLayerRepType is defined in AVC Annex F. Absent in HEVC.
	SEINonRequiredLayerRepType = 26
	// SEIPriorityLayerInfoType is defined in AVC Annex F. Absent in HEVC.
	SEIPriorityLayerInfoType = 27
	// SEILayersNotPresentAVCType is defined in AVC Annex F. Absent in HEVC.
	SEILayersNotPresentAVCType = 28
	// SEILayerDependencyChangeType is defined in AVC Annex F. Absent in HEVC.
	SEILayerDependencyChangeType = 29
	// SEIScalableNestingAVCType is defined in AVC Annex F. Absent in HEVC.
	SEIScalableNestingAVCType = 30
	// SEIBaseLayerTemporalHrdType is defined in AVC Annex F. Absent in HEVC.
	SEIBaseLayerTemporalHrdType = 31
	// SEIQualityLayerIntegrityCheckTpe is defined in AVC Annex F. Absent in HEVC.
	SEIQualityLayerIntegrityCheckTpe = 32
	// SEIRedundantPicPropertyType is defined in AVC Annex F. Absent in HEVC.
	SEIRedundantPicPropertyType = 33
	// SEITl0DepRepIndexType is defined in AVC Annex F. Absent in HEVC.
	SEITl0DepRepIndexType = 34
	// SEITlSwitchingPointType is defined in AVC Annex F. Absent in HEVC.
	SEITlSwitchingPointType = 35
	// SEIParallelDecodingInfoType is defined in AVC Annex G. Absent in HEVC.
	SEIParallelDecodingInfoType = 36
	// SEIMVCScalableNestingType is defined in AVC Annex G. Absent in HEVC.
	SEIMVCScalableNestingType = 37
	// SEIViewScalabilityInfoType is defined in AVC Annex G. Absent in HEVC.
	SEIViewScalabilityInfoType = 38
	// SEIMultiviewSceneInfoAVCType is defined in AVC Annex G. Absent in HEVC.
	SEIMultiviewSceneInfoAVCType = 39
	// SEIMultiviewAcquisitionInfoAVCType is defined in AVC Annex G. Absent in HEVC.
	SEIMultiviewAcquisitionInfoAVCType = 40
	// SEINonRequiredViewComponentType is defined in AVC Annex G. Absent in HEVC.
	SEINonRequiredViewComponentType = 41
	// SEIViewDependencyChangeType is defined in AVC Annex G. Absent in HEVC.C.
	SEIViewDependencyChangeType = 42
	// SEIOperationPointsNotPresentType is defined in AVC Annex G. Absent in HEVC.
	SEIOperationPointsNotPresentType = 43
	// SEIBaseViewTemporalHrdType is defined in AVC Annex G. Absent in HEVC.
	SEIBaseViewTemporalHrdType = 44
	// SEIFramePackingArrangementType is defined in AVC D.1.26 and HEVC D.2.16. Definitions differ.
	SEIFramePackingArrangementType = 45
	// SEIMultiviewViewPositionAVCType is defined in AVC Annex G. Absent in HEVC.
	SEIMultiviewViewPositionAVCType = 46
	// SEIDisplayOrientationType is defined in AVC D.1.27 and HEVC D.2.17. Definitions differ.
	SEIDisplayOrientationType = 47
	// SEIMvcdScalableNestingType is defined in AVC Annex H. Absent in HEVC.
	SEIMvcdScalableNestingType = 48
	// SEIMvcdViewScalabilityInfoType is defined in AVC Annex H. Absent in HEVC.
	SEIMvcdViewScalabilityInfoType = 49
	// SEIDepthRepresentationInfoAVCType is defined in AVC Annex H. Absent in HEVC.
	SEIDepthRepresentationInfoAVCType = 50
	// SEIThreeDimensionalReferenceDisplaysInfoAVCType is defined in AVC Annex H. Absent in HEVC.
	SEIThreeDimensionalReferenceDisplaysInfoAVCType = 51
	// SEIDepthTimingType is defined in AVC Annex H. Absent in HEVC.
	SEIDepthTimingType = 52
	// SEIDepthSamplingInfoType is defined in AVC Annex H. Absent in HEVC.
	SEIDepthSamplingInfoType = 53
	// SEIConstrainedDepthParameterSetIdentifierType is defined in AVC Annex H. Absent in HEVC.
	SEIConstrainedDepthParameterSetIdentifierType = 54
	// SEIGreenMetaDataType is defined in AVC D.1.28 and HEVC D.2.17. Definitions agree and point to ISO/IEC 23001-11.
	SEIGreenMetaDataType = 56
	// SEIStructureOfPicturesInfoType is defined in HEVC D.2.19. Absent in AVC.
	SEIStructureOfPicturesInfoType = 128
	// SEIActiveParameterSetsType is defined in HEVC D.2.21. Absent in AVC.
	SEIActiveParameterSetsType = 129
	// SEIDecodingUnitInfoType is defined in HEVC D.2.22. Absent in AVC.
	SEIDecodingUnitInfoType = 130
	// SEITemporalSubLayerZeroIndexType is defined in HEVC D.2.23. Absent in AVC.
	SEITemporalSubLayerZeroIndexType = 131
	// SEIDecodedPictureHashType is defined in HEVC D.2.20. Only used as suffix SEI. Absent in AVC.
	SEIDecodedPictureHashType = 132
	// SEIScalableNestingHEVCType is defined in HEVC D.2.24. Absent in AVC.
	SEIScalableNestingHEVCType = 133
	// SEIRegionRefreshInfoType is defined in HEVC D.2.25. Absent in AVC.
	SEIRegionRefreshInfoType = 134
	// SEINoDisplayType is defined in HEVC D.2.26. Absent in AVC.
	SEINoDisplayType = 135
	// SEITimeCodeType is defined in HEVC D.2.27. Absent in AVC.
	SEITimeCodeType = 136
	// SEIMasteringDisplayColourVolumeType is defined in AVC D.1.29 and HEVC D.2.28. Definitions agree.
	SEIMasteringDisplayColourVolumeType = 137
	// SEISegmentedRectFramePackingArrangementType is defined in HEVC D.2.29. Absent in AVC.
	SEISegmentedRectFramePackingArrangementType = 138
	// SEITemporalMotionConstrainedTileSetsType is defined in HEVC D.2.30. Absent in AVC.
	SEITemporalMotionConstrainedTileSetsType = 139
	// SEIChromaResamplingFilterHintType is defined in HEVC D.2.31. Absent in AVC.
	SEIChromaResamplingFilterHintType = 140
	// SEIKneeFunctionInfoType is defined in HEVC D.2.32. Absent in AVC.
	SEIKneeFunctionInfoType = 141
	// SEIColourRemappingInfoType is defined in AVC D.1.30 and HEVC D.2.33. Definitions differ.
	SEIColourRemappingInfoType = 142
	// SEIDeinterlacedFieldIdentificationType is defined in HEVC D.2.34. Absent in AVC.
	SEIDeinterlacedFieldIdentificationType = 143
	// SEIContentLightLevelInformationType is defined in AVC D.1.31 and HEVC D.2.35. Definitions agree.
	SEIContentLightLevelInformationType = 144
	// SEIDependentRapIndicationType is defined in HEVC D.2.36. Absent in AVC.
	SEIDependentRapIndicationType = 145
	// SEICodedRegionCompletionType is defined in HEVC D.2.37. Absent in AVC.
	SEICodedRegionCompletionType = 146
	// SEIAlternativeTransferCharacteristicsType is defined in AVC D.1.32 and HEVC D.2.38. Definitions agree.
	SEIAlternativeTransferCharacteristicsType = 147
	// SEIAmbientViewingEnvironmentType is defined in HEVC D.2.39. Absent in AVC.
	SEIAmbientViewingEnvironmentType = 148
	// SEIContentColourVolumeType is defined in AVC D.1.33 and HEVC D.2.40. Definitions agree.
	SEIContentColourVolumeType = 149
	// SEIEquirectangularProjectionType is defined in AVC D.1.35.1 and HEVC D.2.41.1. Definitions agree.
	SEIEquirectangularProjectionType = 150
	// SEICubemapProjectionType is defined in AVC D.1.35.2 and HEVC D.2.41.2. Definitions agree.
	SEICubemapProjectionType = 151
	// SEIFisheyeVideoInfoType is defined in HEVC D.41.3. Absent in AVC.
	SEIFisheyeVideoInfoType = 152
	// SEISphereRotationType is defined in AVC D.1.35.3 and HEVC D.2.41.4. Definitions agree.
	SEISphereRotationType = 154
	// SEIRegionwisePackingType is defined in AVC D.1.35.4 and HEVC D.2.41.5. Definitions agree.
	SEIRegionwisePackingType = 155
	// SEIOmniViewportType is defined in AVC D.1.35.5 and HEVC D.2.41.6. Definitions agree.
	SEIOmniViewportType = 156
	// SEIRegionalNestingType is defined in HEVC D.2.42. Absent in AVC.
	SEIRegionalNestingType = 157
	// SEIMctsExtractionInfoSetsType is defined in HEVC D.2.43. Absent in AVC.
	SEIMctsExtractionInfoSetsType = 158
	// SEIMctsExtractionInfoNesting is defined in HEVC D.2.44. Absent in AVC.
	SEIMctsExtractionInfoNesting = 159
	// SEILayersNotPresentHEVCType is defined in HEVC Annex F. Absent in AVC.
	SEILayersNotPresentHEVCType = 160
	// SEIInterLayerConstrainedTileSetsType is defined in HEVC Annex F. Absent in AVC.
	SEIInterLayerConstrainedTileSetsType = 161
	// SEIBspNestingType is defined in HEVC Annex F. Absent in AVC.
	SEIBspNestingType = 162
	// SEIBspInitialArrivalTimeType is defined in HEVC Annex F. Absent in AVC.
	SEIBspInitialArrivalTimeType = 163
	// SEISubBitstreamPropertyType is defined in HEVC Annex F. Absent in AVC.
	SEISubBitstreamPropertyType = 164
	// SEIAlphaChannelInfoType is defined in HEVC Annex F. Absent in AVC.
	SEIAlphaChannelInfoType = 165
	// SEIOverlayInfoType is defined in HEVC Annex F. Absent in AVC.
	SEIOverlayInfoType = 166
	// SEITemporalMvPredictionConstraintsType is defined in HEVC Annex F. Absent in AVC.
	SEITemporalMvPredictionConstraintsType = 167
	// SEIFrameFieldInfoType is defined in HEVC Annex F. Absent in AVC.
	SEIFrameFieldInfoType = 168
	// SEIThreeDimensionalReferenceDisplaysInfoHEVCType is defined in HEVC Annex G. Absent in AVC.
	SEIThreeDimensionalReferenceDisplaysInfoHEVCType = 176
	// SEIDepthRepresentationInfoHEVCType is defined in HEVC Annex G. Absent in AVC.
	SEIDepthRepresentationInfoHEVCType = 177
	// SEIMultiviewSceneInfoHEVCType is defined in HEVC Annex G. Absent in AVC.
	SEIMultiviewSceneInfoHEVCType = 178
	// SEIMultiviewAcquisitionInfoHEVCType is defined in HEVC Annex G. Absent in AVC.
	SEIMultiviewAcquisitionInfoHEVCType = 179
	// SEIMultiviewViewPositionHEVCType is defined in HEVC Annex G. Absent in AVC.
	SEIMultiviewViewPositionHEVCType = 180
	// SEIAlternativeDepthInfoType is defined in HEVC Annex I. Absent in AVC.
	SEIAlternativeDepthInfoType = 181
	// SEISeiManifestType is defined in AVC D.1.36 and HEVC D.2.45. Definitions agree.
	SEISeiManifestType = 200
	// SEISeiPrefixIndicationType is defined in HEVC D.2.46. Absent in AVC.
	SEISeiPrefixIndicationType = 201
	// SEIAnnotatedRegionsType is defined in HEVC D.2.47. Absent in AVC.
	SEIAnnotatedRegionsType = 202
)

// SEIType is SEI payload type in AVC or HEVC.
type SEIType uint

// String provides the camel-case name for the SEIType.
func (h SEIType) String() string {
	name := ""
	switch h {
	case SEIBufferingPeriodType:
		name = "SEIBufferingPeriodType"
	case SEIPicTimingType:
		name = "SEIPicTimingType"
	case SEIPanScanRectType:
		name = "SEIPanScanRectType"
	case SEIFillerPayloadType:
		name = "SEIFillerPayloadType"
	case SEIUserDataRegisteredITUtT35Type:
		name = "SEIUserDataRegisteredITUtT35Type"
	case SEIUserDataUnregisteredType:
		name = "SEIUserDataUnregisteredType"
	case SEIRecoveryPointType:
		name = "SEIRecoveryPointType"
	case SEIDecRefPicMarkingRepetitionType:
		name = "SEIDecRefPicMarkingRepetitionType"
	case SEISparePicType:
		name = "SEISparePicType"
	case SEISceneInfoType:
		name = "SEISceneInfoType"
	case SEISubSeqInfoType:
		name = "SEISubSeqInfoType"
	case SEISubSeqLayerCharacteristicsType:
		name = "SEISubSeqLayerCharacteristicsType"
	case SEISubSeqCharacteristicsType:
		name = "SEISubSeqCharacteristicsType"
	case SEIFullFrameFreezeType:
		name = "SEIFullFrameFreezeType"
	case SEIFullFrameFreezeReleaseType:
		name = "SEIFullFrameFreezeReleaseType"
	case SEIPictureSnapShotType:
		name = "SEIPictureSnapShotType"
	case SEIProgressiveRefinementSegmentStartType:
		name = "SEIProgressiveRefinementSegmentStartType"
	case SEIProgressiveRefinementSegmentStartEnd:
		name = "SEIProgressiveRefinementSegmentStartEnd"
	case SEIMotionConstrainedSliceGroupSetType:
		name = "SEIMotionConstrainedSliceGroupSetType"
	case SEIFilmGrainCharacteristicsType:
		name = "SEIFilmGrainCharacteristicsType"
	case SEIDeblockingFilterDisplayPreferenceType:
		name = "SEIDeblockingFilterDisplayPreferenceType"
	case SEIStereoVideoInfoType:
		name = "SEIStereoVideoInfoType"
	case SEIPostFilterHintType:
		name = "SEIPostFilterHintType"
	case SEIToneMappingInfoType:
		name = "SEIToneMappingInfoType"
	case SEIScalabilityInfoType:
		name = "SEIScalabilityInfoType"
	case SEISubPicScalableLayerType:
		name = "SEISubPicScalableLayerType"
	case SEINonRequiredLayerRepType:
		name = "SEINonRequiredLayerRepType"
	case SEIPriorityLayerInfoType:
		name = "SEIPriorityLayerInfoType"
	case SEILayersNotPresentAVCType:
		name = "SEILayersNotPresentAVCType"
	case SEILayerDependencyChangeType:
		name = "SEILayerDependencyChangeType"
	case SEIScalableNestingAVCType:
		name = "SEIScalableNestingAVCType"
	case SEIBaseLayerTemporalHrdType:
		name = "SEIBaseLayerTemporalHrdType"
	case SEIQualityLayerIntegrityCheckTpe:
		name = "SEIQualityLayerIntegrityCheckTpe"
	case SEIRedundantPicPropertyType:
		name = "SEIRedundantPicPropertyType"
	case SEITl0DepRepIndexType:
		name = "SEITl0DepRepIndexType"
	case SEITlSwitchingPointType:
		name = "SEITlSwitchingPointType"
	case SEIParallelDecodingInfoType:
		name = "SEIParallelDecodingInfoType"
	case SEIMVCScalableNestingType:
		name = "SEIMVCScalableNestingType"
	case SEIViewScalabilityInfoType:
		name = "SEIViewScalabilityInfoType"
	case SEIMultiviewSceneInfoAVCType:
		name = "SEIMultiviewSceneInfoAVCType"
	case SEIMultiviewAcquisitionInfoAVCType:
		name = "SEIMultiviewAcquisitionInfoAVCType"
	case SEINonRequiredViewComponentType:
		name = "SEINonRequiredViewComponentType"
	case SEIViewDependencyChangeType:
		name = "SEIViewDependencyChangeType"
	case SEIOperationPointsNotPresentType:
		name = "SEIOperationPointsNotPresentType"
	case SEIBaseViewTemporalHrdType:
		name = "SEIBaseViewTemporalHrdType"
	case SEIFramePackingArrangementType:
		name = "SEIFramePackingArrangementType"
	case SEIMultiviewViewPositionAVCType:
		name = "SEIMultiviewViewPositionAVCType"
	case SEIDisplayOrientationType:
		name = "SEIDisplayOrientationType"
	case SEIMvcdScalableNestingType:
		name = "SEIMvcdScalableNestingType"
	case SEIMvcdViewScalabilityInfoType:
		name = "SEIMvcdViewScalabilityInfoType"
	case SEIDepthRepresentationInfoAVCType:
		name = "SEIDepthRepresentationInfoAVCType"
	case SEIThreeDimensionalReferenceDisplaysInfoAVCType:
		name = "SEIThreeDimensionalReferenceDisplaysInfoAVCType"
	case SEIDepthTimingType:
		name = "SEIDepthTimingType"
	case SEIDepthSamplingInfoType:
		name = "SEIDepthSamplingInfoType"
	case SEIConstrainedDepthParameterSetIdentifierType:
		name = "SEIConstrainedDepthParameterSetIdentifierType"
	case SEIGreenMetaDataType:
		name = "SEIGreenMetaDataType"
	case SEIStructureOfPicturesInfoType:
		name = "SEIStructureOfPicturesInfoType"
	case SEIActiveParameterSetsType:
		name = "SEIActiveParameterSetsType"
	case SEIDecodingUnitInfoType:
		name = "SEIDecodingUnitInfoType"
	case SEITemporalSubLayerZeroIndexType:
		name = "SEITemporalSubLayerZeroIndexType"
	case SEIDecodedPictureHashType:
		name = "SEIDecodedPictureHashType"
	case SEIScalableNestingHEVCType:
		name = "SEIScalableNestingHEVCType"
	case SEIRegionRefreshInfoType:
		name = "SEIRegionRefreshInfoType"
	case SEINoDisplayType:
		name = "SEINoDisplayType"
	case SEITimeCodeType:
		name = "SEITimeCodeType"
	case SEIMasteringDisplayColourVolumeType:
		name = "SEIMasteringDisplayColourVolumeType"
	case SEISegmentedRectFramePackingArrangementType:
		name = "SEISegmentedRectFramePackingArrangementType"
	case SEITemporalMotionConstrainedTileSetsType:
		name = "SEITemporalMotionConstrainedTileSetsType"
	case SEIChromaResamplingFilterHintType:
		name = "SEIChromaResamplingFilterHintType"
	case SEIKneeFunctionInfoType:
		name = "SEIKneeFunctionInfoType"
	case SEIColourRemappingInfoType:
		name = "SEIColourRemappingInfoType"
	case SEIDeinterlacedFieldIdentificationType:
		name = "SEIDeinterlacedFieldIdentificationType"
	case SEIContentLightLevelInformationType:
		name = "SEIContentLightLevelInformationType"
	case SEIDependentRapIndicationType:
		name = "SEIDependentRapIndicationType"
	case SEICodedRegionCompletionType:
		name = "SEICodedRegionCompletionType"
	case SEIAlternativeTransferCharacteristicsType:
		name = "SEIAlternativeTransferCharacteristicsType"
	case SEIAmbientViewingEnvironmentType:
		name = "SEIAmbientViewingEnvironmentType"
	case SEIContentColourVolumeType:
		name = "SEIContentColourVolumeType"
	case SEIEquirectangularProjectionType:
		name = "SEIEquirectangularProjectionType"
	case SEICubemapProjectionType:
		name = "SEICubemapProjectionType"
	case SEIFisheyeVideoInfoType:
		name = "SEIFisheyeVideoInfoType"
	case SEISphereRotationType:
		name = "SEISphereRotationType"
	case SEIRegionwisePackingType:
		name = "SEIRegionwisePackingType"
	case SEIOmniViewportType:
		name = "SEIOmniViewportType"
	case SEIRegionalNestingType:
		name = "SEIRegionalNestingType"
	case SEIMctsExtractionInfoSetsType:
		name = "SEIMctsExtractionInfoSetsType"
	case SEIMctsExtractionInfoNesting:
		name = "SEIMctsExtractionInfoNesting"
	case SEILayersNotPresentHEVCType:
		name = "SEILayersNotPresentHEVCType"
	case SEIInterLayerConstrainedTileSetsType:
		name = "SEIInterLayerConstrainedTileSetsType"
	case SEIBspNestingType:
		name = "SEIBspNestingType"
	case SEIBspInitialArrivalTimeType:
		name = "SEIBspInitialArrivalTimeType"
	case SEISubBitstreamPropertyType:
		name = "SEISubBitstreamPropertyType"
	case SEIAlphaChannelInfoType:
		name = "SEIAlphaChannelInfoType"
	case SEIOverlayInfoType:
		name = "SEIOverlayInfoType"
	case SEITemporalMvPredictionConstraintsType:
		name = "SEITemporalMvPredictionConstraintsType"
	case SEIFrameFieldInfoType:
		name = "SEIFrameFieldInfoType"
	case SEIThreeDimensionalReferenceDisplaysInfoHEVCType:
		name = "SEIThreeDimensionalReferenceDisplaysInfoHEVCType"
	case SEIDepthRepresentationInfoHEVCType:
		name = "SEIDepthRepresentationInfoHEVCType"
	case SEIMultiviewSceneInfoHEVCType:
		name = "SEIMultiviewSceneInfoHEVCType"
	case SEIMultiviewAcquisitionInfoHEVCType:
		name = "SEIMultiviewAcquisitionInfoHEVCType"
	case SEIMultiviewViewPositionHEVCType:
		name = "SEIMultiviewViewPositionHEVCType"
	case SEIAlternativeDepthInfoType:
		name = "SEIAlternativeDepthInfoType"
	case SEISeiManifestType:
		name = "SEISeiManifestType"
	case SEISeiPrefixIndicationType:
		name = "SEISeiPrefixIndicationType"
	case SEIAnnotatedRegionsType:
		name = "SEIAnnotatedRegionsType"
	default:
		name = "Reserved SEI type"
	}
	return fmt.Sprintf("%s (%d)", name, h)
}

type Codec uint

const (
	AVC Codec = iota
	HEVC
)

// SEI is Supplementary Enhancement Information.
// High level syntax in ISO/IEC 14496-10 Section 7.3.2.3.
// The actual types are listed in Annex D.
type SEI struct {
	SEIMessages []SEIMessage
}

// SEIMessage is common part of any SEI message.
type SEIMessage interface {
	Type() uint
	Size() uint
	String() string
	Payload() []byte
}

// DecodeSEIMessage decodes or at least provides some information about an SEIMessage.
func DecodeSEIMessage(sd *SEIData, codec Codec) (SEIMessage, error) {
	switch codec {
	case AVC:
		switch sd.Type() {
		case SEIPicTimingType:
			return DecodePicTimingAvcSEI(sd)
		case SEIUserDataRegisteredITUtT35Type:
			return DecodeUserDataRegisteredSEI(sd)
		case SEIUserDataUnregisteredType:
			return DecodeUserDataUnregisteredSEI(sd)
		default:
			return DecodeGeneralSEI(sd), nil
		}
	case HEVC:
		switch sd.Type() {
		case SEIUserDataRegisteredITUtT35Type:
			return DecodeUserDataRegisteredSEI(sd)
		case SEIUserDataUnregisteredType:
			return DecodeUserDataUnregisteredSEI(sd)
		case SEITimeCodeType:
			return DecodeTimeCodeSEI(sd)
		case SEIMasteringDisplayColourVolumeType:
			return DecodeMasteringDisplayColourVolumeSEI(sd)
		case SEIContentLightLevelInformationType:
			return DecodeContentLightLevelInformationSEI(sd)
		default:
			return DecodeGeneralSEI(sd), nil
		}
	default:
		return nil, fmt.Errorf("unknown codec type %d", codec)
	}
}

// DecodeGeneralSEI is a fallback decoder for non-implemented SEI message types.
func DecodeGeneralSEI(sd *SEIData) SEIMessage {
	return &SEIData{
		sd.Type(),
		sd.Payload(),
	}
}

// SEIData is raw parsed SEI message including payload rbsp data.
type SEIData struct {
	payloadType uint
	payload     []byte
}

// NewSEIData returns SEIData struct.
func NewSEIData(msgType uint, payload []byte) *SEIData {
	return &SEIData{msgType, payload}
}

// Type returns the SEI payload type.
func (s *SEIData) Type() uint {
	return s.payloadType
}

// Payload returns the SEI raw rbsp payload.
func (s *SEIData) Payload() []byte {
	return s.payload
}

// String provides a description of the SEI message.
func (s *SEIData) String() string {
	msgType := SEIType(s.Type())
	return fmt.Sprintf("%s, size=%d, %q", msgType, s.Size(), hex.EncodeToString(s.payload))
}

// Size is the size in bytes of the raw SEI message rbsp payload.
func (s *SEIData) Size() uint {
	return uint(len(s.payload))
}

// ExtractSEIData parses ebsp (after NALU header) and returns a slice of SEIData in rbsp format.
// In case the rbsp_trailing_bits 0x80 byte is missing at end, []seiData and
// an ErrMissingRbspTrailingBits error are both returned.
func ExtractSEIData(r io.ReadSeeker) (seiData []SEIData, err error) {
	ar := bits.NewEBSPReader(r)
	for {
		payloadType := uint(0)
		for {
			nextByte := ar.Read(8)
			payloadType += uint(nextByte)
			if nextByte != 0xff {
				break
			}
		}
		payloadSize := uint32(0)
		for {
			nextByte := ar.Read(8)
			payloadSize += uint32(nextByte)
			if nextByte != 0xff {
				break
			}
		}
		payload := ar.ReadBytes(int(payloadSize))
		if ar.AccError() != nil {
			return nil, ar.AccError()
		}

		seiData = append(seiData, SEIData{payloadType, payload})
		if ar.AccError() != nil {
			return nil, ar.AccError()
		}
		// Break loop if no more rbsp data (end of sei messages)
		more, err := ar.MoreRbspData()
		if err != nil {
			return nil, err
		}
		if ar.AccError() == io.EOF {
			return seiData, ErrRbspTrailingBitsMissing
		}
		if ar.AccError() != nil {
			return nil, ar.AccError()
		}
		if !more {
			break
		}
	}
	return seiData, nil
}

// WriteSEIMessages writes the messages in EBSP format with RBSPTrailing bits.
// The output corresponds to an SEI NAL unit payload.
func WriteSEIMessages(w io.Writer, msgs []SEIMessage) error {
	bw := bits.NewEBSPWriter(w)
	for _, msg := range msgs {
		bw.WriteSEIValue(msg.Type())
		bw.WriteSEIValue(msg.Size())
		pl := msg.Payload()
		for _, b := range pl {
			bw.Write(uint(b), 8)
		}
	}
	bw.WriteRbspTrailingBits()
	return bw.AccError()
}
