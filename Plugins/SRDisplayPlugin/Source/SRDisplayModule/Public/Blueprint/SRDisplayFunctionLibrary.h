/**
 * Copyright 2019,2020,2021,2022 Sony Corporation
 */
#pragma once

#include <CoreMinimal.h>
#include <Kismet/BlueprintFunctionLibrary.h>

#include "XRDisplaySystem.h"

#include "SRDisplayFunctionLibrary.generated.h"

/**
 * @brief TBD: Add brief description for this Blueprint Library Class.
 */
UCLASS()
class SRDISPLAYMODULE_API USRDisplayFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

private:

	static xr_display::FXRDisplaySystem* GetXRDisplaySystem();

public:

	USRDisplayFunctionLibrary(const FObjectInitializer& ObjectInitializer);

	/**
	 * @brief Retrieves the X and Y screen coordinates of the mouse cursor. Returns false if the mouse cursor is not over the window.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "SRDisplay")
	static bool GetMousePosition(float& LocationX, float& LocationY);

	/**
	 * @brief Transforms the given 2D screen space coordinate into a 3D world-space point and direction (The reference camera position is also acquired).
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "SRDisplay")
	static bool DeprojectScreenToWorld(APlayerController const* Player, const FVector2D& ScreenPosition, FVector& WorldPosition, FVector& WorldDirection, FVector& CameraPosition);

	/**
	 * @brief Convert current mouse 2D position to World Space 3D position and direction. Returns false if unable to determine value (The reference camera position is also acquired).
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "SRDisplay", meta = (DisplayName = "ConvertMouseLocationToWorldSpace"))
	static bool ConvertMouseLocationToWorldSpace(APlayerController const* Player, FVector& WorldPosition, FVector& WorldDirection, FVector& CameraPosition);

	/**
	 * @brief Returns if SRDisplay detects a face
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "SRDisplay")
	static bool IsSRDisplayDetectedFace();

};
