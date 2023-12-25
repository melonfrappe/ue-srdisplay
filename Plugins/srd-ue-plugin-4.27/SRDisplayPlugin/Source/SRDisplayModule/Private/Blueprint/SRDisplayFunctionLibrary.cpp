/**
 * Copyright 2019,2020,2021,2022 Sony Corporation
 */
#include "Blueprint/SRDisplayFunctionLibrary.h"

USRDisplayFunctionLibrary::USRDisplayFunctionLibrary(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

static const FName XRDisplaySystemName(TEXT("Sony XR Display"));
xr_display::FXRDisplaySystem* USRDisplayFunctionLibrary::GetXRDisplaySystem()
{
	if (GEngine && GEngine->XRSystem.IsValid()) {
		if (GEngine->XRSystem->GetSystemName() == XRDisplaySystemName) {
			return static_cast<xr_display::FXRDisplaySystem*>(GEngine->XRSystem.Get());
		}
	}
	return nullptr;
}

bool USRDisplayFunctionLibrary::GetMousePosition(float& LocationX, float& LocationY)
{
#if PLATFORM_WINDOWS
	xr_display::FXRDisplaySystem* XRDisplaySystem = GetXRDisplaySystem();
	if (XRDisplaySystem)
	{
		return XRDisplaySystem->GetMousePosition(LocationX, LocationY);
	}
#endif // PLATFORM_WINDOWS

	LocationX = 0.f;
	LocationY = 0.f;
	return false;
}

bool USRDisplayFunctionLibrary::DeprojectScreenToWorld(APlayerController const* Player, const FVector2D& ScreenPosition, FVector& WorldPosition, FVector& WorldDirection, FVector& CameraPosition)
{
#if PLATFORM_WINDOWS
	xr_display::FXRDisplaySystem* XRDisplaySystem = GetXRDisplaySystem();
	if (XRDisplaySystem)
	{
		return XRDisplaySystem->DeprojectScreenToWorld(Player, ScreenPosition, WorldPosition, WorldDirection, CameraPosition);
	}
#endif // PLATFORM_WINDOWS

	// something went wrong, zero things and return false
	WorldPosition = FVector::ZeroVector;
	WorldDirection = FVector::ZeroVector;
	return false;
}

bool USRDisplayFunctionLibrary::ConvertMouseLocationToWorldSpace(APlayerController const* Player, FVector& WorldPosition, FVector& WorldDirection, FVector& CameraPosition)
{
#if PLATFORM_WINDOWS
	xr_display::FXRDisplaySystem* XRDisplaySystem = GetXRDisplaySystem();
	if (XRDisplaySystem)
	{
		float LocationX = 0.f;
		float LocationY = 0.f;
		if (XRDisplaySystem->GetMousePosition(LocationX, LocationY))
		{
			FVector2D ScreenPosition(LocationX, LocationY);
			return XRDisplaySystem->DeprojectScreenToWorld(Player, ScreenPosition, WorldPosition, WorldDirection, CameraPosition);
		}
	}
#endif // PLATFORM_WINDOWS

	// something went wrong, zero things and return false
	WorldPosition = FVector::ZeroVector;
	WorldDirection = FVector::ZeroVector;
	return false;
}
