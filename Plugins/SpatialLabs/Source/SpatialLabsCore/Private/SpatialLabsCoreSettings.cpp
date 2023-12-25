#include "SpatialLabsCoreSettings.h"

USpatialLabsCoreSettings::USpatialLabsCoreSettings(class FObjectInitializer const& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void USpatialLabsCoreSettings::PostInitProperties()
{
	Super::PostInitProperties();
}

void USpatialLabsCoreSettings::Save()
{
    if (GetClass()->HasAnyClassFlags(CLASS_DefaultConfig))
    {
        UpdateDefaultConfigFile();
    }
    else if (GetClass()->HasAnyClassFlags(CLASS_GlobalUserConfig))
    {
        UpdateGlobalUserConfigFile();
    }
    else
    {
        SaveConfig();
    }
}

// ScaleFactor
float USpatialLabsCoreSettings::GetScaleFactor() const
{
    return ScaleFactor;
}

void USpatialLabsCoreSettings::SetScaleFactor(float InScaleFactor)
{
    ScaleFactor = InScaleFactor;
}

// CameraOffset
float USpatialLabsCoreSettings::GetCameraOffset() const
{
    return CameraOffset;
}

void USpatialLabsCoreSettings::SetCameraOffset(float InCameraOffset)
{
    CameraOffset = InCameraOffset;
}

// UseDynamicCameraFOV
bool USpatialLabsCoreSettings::GetUseDynamicCameraFOV() const
{
    return UseDynamicCameraFOV;
}

void USpatialLabsCoreSettings::SetUseDynamicCameraFOV(bool InUseDynamicCameraFOV)
{
    UseDynamicCameraFOV = InUseDynamicCameraFOV;
}

// UseFixedHeadPosition
bool USpatialLabsCoreSettings::GetUseFixedHeadPosition() const
{
    return UseFixedHeadPosition;
}

void USpatialLabsCoreSettings::SetUseFixedHeadPosition(bool InUseFixedHeadPosition) 
{
    UseFixedHeadPosition = InUseFixedHeadPosition;
}

// FixedHeadPosition
FVector USpatialLabsCoreSettings::GetFixedHeadPosition() const
{
    return FixedHeadPosition;
}

void USpatialLabsCoreSettings::SetFixedHeadPosition(FVector InFixedHeadPosition) 
{
    FixedHeadPosition = InFixedHeadPosition;
}

// IsDeviceAvailable
bool USpatialLabsCoreSettings::GetIsDeviceAvailable() const
{
    return SpatialLabsCoreLib::SpatialLabsCoreLibAPI::IsSpatialLabsDeviceAvailable();
}

void USpatialLabsCoreSettings::GetProjectionMatrix(SpatialLabsCoreLib::ProjectionInfo info, float* ProjectionMatrix)
{
    SpatialLabsCoreLib::SpatialLabsCoreLibAPI::GetProjectionMatrix(info, ProjectionMatrix);
}

void USpatialLabsCoreSettings::GetRelativeEyePosition(SpatialLabsCoreLib::ProjectionInfo info, float* RelativeEyePosition)
{
    SpatialLabsCoreLib::SpatialLabsCoreLibAPI::GetRelativeEyePosition(info, RelativeEyePosition);
}

void USpatialLabsCoreSettings::GetCachedEyePosition(int EyeIndex, float* EyePosition)
{
    SpatialLabsCoreLib::SpatialLabsCoreLibAPI::GetCachedEyePosition(EyeIndex, EyePosition);
}

void USpatialLabsCoreSettings::GetCachedEyePositionScreenSpace(int EyeIndex, float* EyePosition)
{
    SpatialLabsCoreLib::SpatialLabsCoreLibAPI::GetCachedEyePositionScreenSpace(EyeIndex, EyePosition);
}