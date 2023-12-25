#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"

#include "SpatialLabsCoreLib.h"

#include "SpatialLabsCoreSettings.generated.h"

UCLASS(config=Game, defaultconfig)
class SPATIALLABSCORE_API USpatialLabsCoreSettings : public UObject
{
    GENERATED_UCLASS_BODY()

public:

    void Save();

    virtual void PostInitProperties() override;

    float GetScaleFactor() const;
    void SetScaleFactor(float InScaleFactor);

    float GetCameraOffset() const;
    void SetCameraOffset(float InCameraOffset);

    bool GetUseDynamicCameraFOV() const;
    void SetUseDynamicCameraFOV(bool InUseDynamicCameraFOV);

    bool GetUseFixedHeadPosition() const;
    void SetUseFixedHeadPosition(bool InUseFixedHeadPosition);

    FVector GetFixedHeadPosition() const;
    void SetFixedHeadPosition(FVector InFixedHeadPosition);

    bool GetIsDeviceAvailable() const;

    void GetProjectionMatrix(SpatialLabsCoreLib::ProjectionInfo info, float* ProjectionMatrix);

    void GetRelativeEyePosition(SpatialLabsCoreLib::ProjectionInfo info, float* RelativeEyePosition);

    void GetCachedEyePosition(int EyeIndex, float* EyePosition);

    void GetCachedEyePositionScreenSpace(int EyeIndex, float* EyePosition);

private:
    UPROPERTY(config)
    float ScaleFactor = 1.0;

    UPROPERTY(config)
    float CameraOffset = 0.0;

    UPROPERTY(config)
    bool UseDynamicCameraFOV = true;

    UPROPERTY(config)
    bool UseFixedHeadPosition = false;

    UPROPERTY(config)
    FVector FixedHeadPosition = FVector(-60, 0, 0);
};
