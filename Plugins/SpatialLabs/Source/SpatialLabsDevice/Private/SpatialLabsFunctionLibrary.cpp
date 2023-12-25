#include "SpatialLabsFunctionLibrary.h"
#include "Misc/ConfigCacheIni.h"
#include "RHI.h"
#include "Engine/Engine.h"
#include "GameFramework/WorldSettings.h"
#include "SpatialLabsDevice.h"
#include "SpatialLabsCoreModule.h"
#include "SpatialLabsConstants.h"

USpatialLabsFunctionLibrary::FOnWidgetAddToSpatialLabsViewport USpatialLabsFunctionLibrary::OnWidgetAddToSpatialLabsViewport;
USpatialLabsFunctionLibrary::FOnWidgetRemoveFromSpatialLabsViewport USpatialLabsFunctionLibrary::OnWidgetRemoveFromSpatialLabsViewport;
USpatialLabsFunctionLibrary::FOnClearSpatialLabsViewport USpatialLabsFunctionLibrary::OnClearSpatialLabsViewport;
TArray<UUserWidget*> USpatialLabsFunctionLibrary::NativeWidgets;

bool IsWorldPIE(UObject* WorldContextObject)
{
    UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
    if (World != nullptr)
    {
        return World->WorldType == EWorldType::PIE;
    }

    return false;
}

void USpatialLabsFunctionLibrary::AddToSpatialLabsViewport(UObject* WorldContextObject, UUserWidget* Widget, int32 ZOrder)
{
    OnWidgetAddToSpatialLabsViewport.ExecuteIfBound(Widget, ZOrder);
}

void USpatialLabsFunctionLibrary::RemoveFromSpatialLabsViewport(UObject* WorldContextObject, UUserWidget* Widget)
{
    OnWidgetRemoveFromSpatialLabsViewport.ExecuteIfBound(Widget);
}

void USpatialLabsFunctionLibrary::ClearSpatialLabsViewport(UObject* WorldContextObject)
{
    UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
    if (World != nullptr)
    {
        OnClearSpatialLabsViewport.ExecuteIfBound(World);
    }
}

FVector USpatialLabsFunctionLibrary::GetHeadLocation()
{
    FVector LeftEye = FSpatialLabsDevice::GetCachedEyeLocation(EStereoscopicPass::eSSP_LEFT_EYE);
    FVector RightEye = FSpatialLabsDevice::GetCachedEyeLocation(EStereoscopicPass::eSSP_RIGHT_EYE);

    return (RightEye + LeftEye) / 2;
}

FVector USpatialLabsFunctionLibrary::GetHeadScreenSpaceLocation()
{
    FVector LeftEye = FSpatialLabsDevice::GetCachedEyeScreenSpaceLocation(EStereoscopicPass::eSSP_LEFT_EYE);
    FVector RightEye = FSpatialLabsDevice::GetCachedEyeScreenSpaceLocation(EStereoscopicPass::eSSP_RIGHT_EYE);

    return (RightEye + LeftEye) / 2;
}

float USpatialLabsFunctionLibrary::GetScaledWorldToMetersScale(UObject* WorldContextObject)
{
    UWorld* World = WorldContextObject->GetWorld();
    if (World == nullptr) return 100.0f;

    FSpatialLabsCoreModule* SpatialLabsCore = static_cast<FSpatialLabsCoreModule*>(FModuleManager::Get().GetModule("SpatialLabsCore"));
    if (SpatialLabsCore == nullptr) return 100.0f;
    float ScaleFactor = SpatialLabsCore->GetCoreSettings()->GetScaleFactor();

    float WorldToMeters = World->GetWorldSettings()->WorldToMeters;
    float ScaledWorldToMeters = WorldToMeters * ScaleFactor;

    return ScaledWorldToMeters;
}

FVector USpatialLabsFunctionLibrary::GetMouseWorldPosition(UObject* WorldContextObject)
{
    FSpatialLabsCoreModule* SpatialLabsCore = static_cast<FSpatialLabsCoreModule*>(FModuleManager::Get().GetModule("SpatialLabsCore"));
    if (SpatialLabsCore == nullptr) FVector::ZeroVector;
    float CameraOffset = SpatialLabsCore->GetCoreSettings()->GetCameraOffset();

    return GetMouseWorldPositionOffset(WorldContextObject, CameraOffset);
}

FVector USpatialLabsFunctionLibrary::GetMouseWorldPositionOffset(UObject* WorldContextObject, float CameraOffset)
{
    if (WorldContextObject == nullptr) return FVector::ZeroVector;

    float ScaledWorldToMetersScale = GetScaledWorldToMetersScale(WorldContextObject);

    UWorld* World = WorldContextObject->GetWorld();
    if (World == nullptr) return FVector::ZeroVector;

    APlayerController* PlayerController = World->GetFirstPlayerController();
    if (PlayerController == nullptr) return FVector::ZeroVector;

    APlayerCameraManager* PlayerCameraManager = PlayerController->PlayerCameraManager;
    if (PlayerCameraManager == nullptr) return FVector::ZeroVector;
    const FTransform& CameraTransform = PlayerCameraManager->GetActorTransform();
    FRotator CameraRotator = CameraTransform.Rotator();
    float FOVAngle = PlayerCameraManager->GetFOVAngle();

    FVector MouseWorldLocation;
    FVector MouseWorldDirection;
    PlayerController->DeprojectMousePositionToWorld(MouseWorldLocation, MouseWorldDirection);

    float NearClippingPlane = GNearClippingPlane;

    FSpatialLabsCoreModule* SpatialLabsCore = static_cast<FSpatialLabsCoreModule*>(FModuleManager::Get().GetModule("SpatialLabsCore"));
    if (SpatialLabsCore == nullptr) FVector::ZeroVector;
    bool UseDynamicCameraFOV = SpatialLabsCore->GetCoreSettings()->GetUseDynamicCameraFOV();

    float TanHalfFOV = FMath::Tan(PI / (180.f) * (FOVAngle * 0.5));

    float DistanceHeadToScreen = (SPATIALLABS_SCREEN_WIDTH_CM * 0.01 / 2) / TanHalfFOV * ScaledWorldToMetersScale;
    FVector HeadToScreen(DistanceHeadToScreen, 0, 0);

    FTransform MouseToCamera(CameraRotator, MouseWorldLocation, FVector(1.0, 1.0, 1.0));
    MouseToCamera = CameraTransform.GetRelativeTransform(MouseToCamera);
    FVector NewLocation = MouseToCamera.GetLocation();

    FVector MousePostionYZ = FVector(0.0, NewLocation.Y, NewLocation.Z) * -1 * (1.0 / NearClippingPlane) * DistanceHeadToScreen;
    FVector MousePositionXOffset = FVector(1.0, 0.0, 0.0) * CameraOffset * ScaledWorldToMetersScale * 0.01;

    FVector MousePositionCameraSpace;
    if (UseDynamicCameraFOV)
    {
        MousePositionCameraSpace = MousePostionYZ + MousePositionXOffset;
    }
    else
    {
        MousePositionCameraSpace = MousePostionYZ + MousePositionXOffset + HeadToScreen;
    }

    FVector MousePositionWorldSpace = CameraTransform.TransformPosition(MousePositionCameraSpace);

    return MousePositionWorldSpace;
}

FVector USpatialLabsFunctionLibrary::GetHeadToMouseRay(UObject* WorldContextObject)
{
    if (WorldContextObject == nullptr) return FVector::ZeroVector;

    UWorld* World = WorldContextObject->GetWorld();
    if (World == nullptr) return FVector::ZeroVector;

    APlayerController* PlayerController = World->GetFirstPlayerController();
    if (PlayerController == nullptr) return FVector::ZeroVector;

    APlayerCameraManager* PlayerCameraManager = PlayerController->PlayerCameraManager;
    const FTransform& CameraTransform = PlayerCameraManager->GetActorTransform();

    FSpatialLabsCoreModule* SpatialLabsCore = static_cast<FSpatialLabsCoreModule*>(FModuleManager::Get().GetModule("SpatialLabsCore"));
    bool UseDynamicCameraFOV = SpatialLabsCore->GetCoreSettings()->GetUseDynamicCameraFOV();

    FVector HeadLocationCameraSpace = FVector::ZeroVector;
    if (UseDynamicCameraFOV)
    {
        HeadLocationCameraSpace = GetHeadLocation();
    }

    FVector MouseWorldPosition = GetMouseWorldPositionOffset(WorldContextObject, 0);// Ray end
    FVector HeadLocationWoldSpace = CameraTransform.TransformPosition(HeadLocationCameraSpace);// Ray start

    return (MouseWorldPosition - HeadLocationWoldSpace).GetSafeNormal();
}
