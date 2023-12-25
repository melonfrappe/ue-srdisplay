#include "SpatialLabsEditorSettings.h"
#include "SpatialLabsCoreModule.h"
#include "SpatialLabsCoreSettings.h"


USpatialLabsEditorSettings::USpatialLabsEditorSettings(class FObjectInitializer const& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void USpatialLabsEditorSettings::Save()
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

void USpatialLabsEditorSettings::PostInitProperties()
{
    Super::PostInitProperties();

    const auto& CoreSettings = FSpatialLabsCoreModule::Get().GetCoreSettings();

    DeviceAvailable = CoreSettings->GetIsDeviceAvailable();
    ScaleFactor = CoreSettings->GetScaleFactor();
    CameraOffset = CoreSettings->GetCameraOffset();
    UseDynamicCameraFOV = CoreSettings->GetUseDynamicCameraFOV();
    UseFixedHeadPosition = CoreSettings->GetUseFixedHeadPosition();
    FixedHeadPosition = CoreSettings->GetFixedHeadPosition();

    SetScaleFactor(ScaleFactor);
    SetCameraOffset(CameraOffset);
    SetUseDynamicCameraFOV(UseDynamicCameraFOV);
    SetUseFixedHeadPosition(UseFixedHeadPosition);
    SetFixedHeadPosition(FixedHeadPosition);
}

void USpatialLabsEditorSettings::PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent)
{
    auto CurrentProperty = PropertyChangedEvent.PropertyChain.GetTail();

    while (true)
    {
        if (CurrentProperty == nullptr) break;
        if (CurrentProperty->GetValue() == nullptr) break;

        const FName& PropertyName(CurrentProperty->GetValue()->GetFName());

        if (PropertyName == GET_MEMBER_NAME_CHECKED(USpatialLabsEditorSettings, ScaleFactor))
        {
            SetScaleFactor(ScaleFactor);
        }
        else if (PropertyName == GET_MEMBER_NAME_CHECKED(USpatialLabsEditorSettings, CameraOffset))
        {
            SetCameraOffset(CameraOffset);
        }
        else if (PropertyName == GET_MEMBER_NAME_CHECKED(USpatialLabsEditorSettings, UseDynamicCameraFOV))
        {
            SetUseDynamicCameraFOV(UseDynamicCameraFOV);
        }
        else if (PropertyName == GET_MEMBER_NAME_CHECKED(USpatialLabsEditorSettings, UseFixedHeadPosition))
        {
            SetUseFixedHeadPosition(UseFixedHeadPosition);
        }
        else if (PropertyName == GET_MEMBER_NAME_CHECKED(USpatialLabsEditorSettings, FixedHeadPosition))
        {
            SetFixedHeadPosition(FixedHeadPosition);
        }

        CurrentProperty = CurrentProperty->GetPrevNode();
    }

    Super::PostEditChangeChainProperty(PropertyChangedEvent);

}

void USpatialLabsEditorSettings::SetScaleFactor(float InScaleFactor)
{
    const auto& CoreSettings = FSpatialLabsCoreModule::Get().GetCoreSettings();
    CoreSettings->SetScaleFactor(InScaleFactor);
    CoreSettings->Save();
    ScaleFactor = CoreSettings->GetScaleFactor();
}

void USpatialLabsEditorSettings::SetCameraOffset(float InCameraOffset)
{
    const auto& CoreSettings = FSpatialLabsCoreModule::Get().GetCoreSettings();
    CoreSettings->SetCameraOffset(InCameraOffset);
    CoreSettings->Save();
    CameraOffset = CoreSettings->GetCameraOffset();
}

void USpatialLabsEditorSettings::SetUseDynamicCameraFOV(bool InUseDynamicCameraFOV)
{
	const auto& CoreSettings = FSpatialLabsCoreModule::Get().GetCoreSettings();
	CoreSettings->SetUseDynamicCameraFOV(InUseDynamicCameraFOV);
	CoreSettings->Save();
	UseDynamicCameraFOV = CoreSettings->GetUseDynamicCameraFOV();
}

void USpatialLabsEditorSettings::SetUseFixedHeadPosition(bool InUseFixedHeadPosition)
{
    const auto& CoreSettings = FSpatialLabsCoreModule::Get().GetCoreSettings();
    CoreSettings->SetUseFixedHeadPosition(InUseFixedHeadPosition);
    CoreSettings->Save();
    UseFixedHeadPosition = CoreSettings->GetUseFixedHeadPosition();
}

void USpatialLabsEditorSettings::SetFixedHeadPosition(FVector InFixedHeadPosition)
{
    const auto& CoreSettings = FSpatialLabsCoreModule::Get().GetCoreSettings();
    CoreSettings->SetFixedHeadPosition(InFixedHeadPosition);
    CoreSettings->Save();
    FixedHeadPosition = CoreSettings->GetFixedHeadPosition();
}

#if WITH_EDITOR
bool USpatialLabsEditorSettings::CanEditChange(const FProperty* InProperty) const
{
    if (!DeviceAvailable)
    {
        return false;// Disable all fields if device is not available
    }

    if (InProperty->GetFName() == GET_MEMBER_NAME_CHECKED(USpatialLabsEditorSettings, DeviceAvailable))
    {
        return false;// Field DeviceAvailable is not editable
    }

    if (InProperty->GetFName() == GET_MEMBER_NAME_CHECKED(USpatialLabsEditorSettings, FixedHeadPosition))
    {
        return UseFixedHeadPosition;
    }

    return true;
}
#endif