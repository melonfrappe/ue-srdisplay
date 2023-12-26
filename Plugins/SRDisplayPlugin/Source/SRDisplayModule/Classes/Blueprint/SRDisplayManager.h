/*
 * Copyright 2019,2020,2021,2022 Sony Corporation
 */
#pragma once

#include <CoreMinimal.h>
#include "misc_util_wrapper.h"

#include "SRDisplayManager.generated.h"

class USRDisplayDrawBoxComponent;

UENUM()
enum class ECrosstalkCorrectionType
{
	/*
	Corrects crosstalk and make it less noticeable at medium gradation.
	GPU load will be a little higher than when crosstalk correction is not used.
	*/
	GRADATION_CORRECTION_MEDIUM UMETA(DisplayName = "Low"),
	/*
	Corrects crosstalk and make it less noticeable at all gradation.
	GPU load will be higher than that of "Medium gradation correction".
	*/
	GRADATION_CORRECTION_ALL UMETA(DisplayName = "Mid"),
	/*
	Corrects crosstalk at all gradation.
	Crosstalk will be less noticeable than "All gradation correction".
	GPU load will be higher than when "All gradation correction".
	*/
	GRADATION_CORRECTION_HIGH_PRECISE UMETA(DisplayName = "High")
};

UENUM()
enum class EScalingMode
{
	SCALED_SIZE UMETA(DisplayName = "Scaled Size"),
	ORIGINAL_SIZE UMETA(DisplayName = "Original Size")
};

UENUM()
enum class EGizmoSize
{
	ELF_SR1_SIZE UMETA(DisplayName = "ELF-SR1"),
	ELF_SR2_SIZE UMETA(DisplayName = "ELF-SR2")
};

UCLASS()
class ASRDisplayManager : public AActor
{
	GENERATED_BODY()

public:

	ASRDisplayManager();

	// Tracking
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SRDisplay|Tracking",
		meta = (ToolTip = "If checked, a window opens and shows images captured by SRDisplay camera."))
	bool ShowCameraWindow = false;

	// Rendering
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SRDisplay|Rendering|Crosstalk Correction (ELF-SR1 exclusive)",
		meta = (ToolTip = "If checked, crosstalk will be reduced."))
	bool CrosstalkCorrection = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SRDisplay|Rendering|Crosstalk Correction (ELF-SR1 exclusive)",
		meta = (ToolTip = "Crosstalk Correction level can be selected.", EditCondition = "CrosstalkCorrection"))
	ECrosstalkCorrectionType CorrectionType = ECrosstalkCorrectionType::GRADATION_CORRECTION_MEDIUM;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SRDisplay|Rendering",
		meta = (ToolTip = "If checked, contents will be clipped at front of SR Display."))
	bool SpatialClipping = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SRDisplay|Rendering",
		meta = (ToolTip = "If checked, display in 2D on SR Display."))
	bool IsSRRenderingActive = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SRDisplay|Rendering",
		meta = (DisplayName = "Far Clip (cm)", ClampMin = "0.0", UIMin = "0.0", ToolTip = "Far clipt distance (cm)."))
	float FarClip = 100000000.f;

	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "SRDisplay|Rendering|Scaling Mode",
		meta = (ToolTip = "Scaled mode or Original mode can be selected."))
	EScalingMode ScalingMode = EScalingMode::SCALED_SIZE;

	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "SRDisplay|Rendering|Scaling Mode",
		meta = (ToolTip = "size of SRDisplayManager gizmo can be selected.",
						EditCondition = "ScalingMode == EScalingMode::ORIGINAL_SIZE"))
	EGizmoSize GizmoSize = EGizmoSize::ELF_SR1_SIZE;

	// Appearance
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SimpleDisplay, Category = "SRDisplay|Appearance")
	FColor BoxColor = FColor::Blue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SRDisplay|Appearance")
	FColor DisplayColor = FColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SRDisplay|Appearance", meta = (ClampMin = "1.0", ClampMax = "200.0", UIMin = "1.0", UIMax = "200.0"))
	float LineThickness = 1.f;

public:
	virtual void Tick(float DeltaSeconds) override;

	float GetRealToVirtualScale();

	float GetPanelSizeScale(sony::oz::xr_runtime::SupportDevice ConnectedDevice) const;

#if WITH_EDITOR
	void PostEditChangeProperty(struct FPropertyChangedEvent& e) override;
#endif

protected:
	virtual void OnConstruction(const FTransform& Transform) override;

private:
	USRDisplayDrawBoxComponent* Box = nullptr;
	UStaticMeshComponent* StaticMeshComponent;
	UStaticMesh* StaticMesh;
};
