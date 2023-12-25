#pragma once
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Blueprint/UserWidget.h"
#include "Components/SceneCaptureComponent2D.h"
#include "SpatialLabsDevice.h"
#include "SpatialLabsFunctionLibrary.generated.h"

UCLASS()
class SPATIALLABSDEVICE_API USpatialLabsFunctionLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:

    /**
     * Add the widget to a SpatialLabs compatible viewport.
     */
    UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "SpatialLabs|Display", meta = (WorldContext = "WorldContextObject", AdvancedDisplay = "ZOrder", DisplayName = "Add To SpatialLabs Viewport"))
    static void AddToSpatialLabsViewport(UObject* WorldContextObject, UUserWidget* Widget, int32 ZOrder = 0);
    
    /**
     * Remove the widget from the SpatialLabs compatible viewport.
     */
    UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "SpatialLabs|Display", meta = (WorldContext = "WorldContextObject", DisplayName = "Remove From SpatialLabs Viewport"))
    static void RemoveFromSpatialLabsViewport(UObject* WorldContextObject, UUserWidget* Widget);

    /**
     * Remove all widgets from the SpatialLabs compatible viewport.
     */
    UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "SpatialLabs|Display", meta = (WorldContext = "WorldContextObject", DisplayName = "Clear SpatialLabs Viewport"))
    static void ClearSpatialLabsViewport(UObject* WorldContextObject);

    DECLARE_DELEGATE_TwoParams(FOnWidgetAddToSpatialLabsViewport, UUserWidget*, int32)
    static FOnWidgetAddToSpatialLabsViewport OnWidgetAddToSpatialLabsViewport;

    DECLARE_DELEGATE_OneParam(FOnWidgetRemoveFromSpatialLabsViewport, UUserWidget*)
    static FOnWidgetRemoveFromSpatialLabsViewport OnWidgetRemoveFromSpatialLabsViewport;

    DECLARE_DELEGATE_OneParam(FOnClearSpatialLabsViewport, UWorld*)
    static FOnClearSpatialLabsViewport OnClearSpatialLabsViewport;

    /**
     * Retrieves the position of the center of the user's eyes relative to the camera component in local space coordinates.
     */
    UFUNCTION(BlueprintPure, Category = "SpatialLabs|Display")
    static FVector GetHeadLocation();

    /**
     * Retrieves the position of the center of the user's eyes relative to the tracking sensor component in local space coordinates.
     */
    UFUNCTION(BlueprintPure, Category = "SpatialLabs|Display")
    static FVector GetHeadScreenSpaceLocation();

    static float GetScaledWorldToMetersScale(UObject* WorldContextObject);

    /**
     * Retrieves the mouse position relative to the display plane in world space coordinates.
     */
    UFUNCTION(BlueprintPure, Category = "SpatialLabs|Display", meta = (WorldContext = "WorldContextObject"))
    static FVector GetMouseWorldPosition(UObject* WorldContextObject);

    /**
     * Projects a ray from the center of the user's eyes towards the mouse position in world space coordinates.
     */
    UFUNCTION(BlueprintPure, Category = "SpatialLabs|Display", meta = (WorldContext = "WorldContextObject"))
    static FVector GetHeadToMouseRay(UObject* WorldContextObject);

private:
    static TArray<UUserWidget*> NativeWidgets;

    static FVector GetMouseWorldPositionOffset(UObject* WorldContextObject, float CameraOffset);
};
