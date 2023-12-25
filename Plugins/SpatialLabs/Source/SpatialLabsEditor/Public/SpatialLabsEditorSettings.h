#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"

#include "SpatialLabsEditorSettings.generated.h"

UCLASS(config = Engine)
class SPATIALLABSEDITOR_API USpatialLabsEditorSettings : public UObject
{
    GENERATED_UCLASS_BODY()

private:

    virtual void PostInitProperties() override;

    virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent) override;

    void Save();

    UPROPERTY(config, EditAnywhere, Category = General, meta = (
    ToolTip = "True if SpatialLabs device is available."),
    DisplayName = "Device Is Available")
    bool DeviceAvailable = false;

    UPROPERTY(config, EditAnywhere, Category = General, meta = (
    ToolTip = "This value is multiplied by each map's 'World to Meters' value (found in its World Settings) to allow applying a uniform scale to all maps in the project."),
    DisplayName = "World Scale Factor")
    float ScaleFactor = 1.0f;

    UPROPERTY(config, EditAnywhere, Category = Display, meta = (
    ToolTip = "Offset the active camera on the X axis relative to the display.\n\nWarning: this should be only used as a final solution. It is recommended to adjust the view frustum by moving the camera instead."),
    DisplayName = "Camera Offset")
    float CameraOffset = 0;

    UPROPERTY(config, EditAnywhere, Category = HeadTracking, meta = (
    ToolTip = "By default, the camera's FOV is calculated dynamically based on the user's eye position.Enabling this option allows the camera to use its own FOV value.\n\nDisable dynamic camera FOV fundamentally changes the viewer experience and is suitable for some but not all types of projects.It is recommended to test both behaviors to determine the more appropriate one for your project"),
    DisplayName = "Dynamic Camera FOV")
    bool UseDynamicCameraFOV = true;

    UPROPERTY(config, EditAnywhere, Category = HeadTracking, meta = (
    ToolTip = "This locks the head position to the position specified in 'Fixed Head Position'.\n\nThis is useful for experiences where the default behavior of the camera moving / rotating with the player's head is not desirable."),
    DisplayName = "Use Fixed Head Position")
    bool UseFixedHeadPosition = false;

    UPROPERTY(config, EditAnywhere, Category = HeadTracking, meta = (
    ToolTip = "The head position to use when 'Use Fixed Head Position' is enabled."),
    DisplayName = "Fixed Head Position")
    FVector FixedHeadPosition = FVector(-60, 0, 0);

    void SetScaleFactor(float InScaleFactor);
    void SetCameraOffset(float InCameraOffset);
    void SetUseDynamicCameraFOV(bool InUseDynamicCameraFOV);
    void SetUseFixedHeadPosition(bool InUseFixedHeadPosition);
    void SetFixedHeadPosition(FVector InFixedHeadPosition);

#if WITH_EDITOR
    virtual bool CanEditChange(const FProperty* InProperty) const override;
#endif

};
