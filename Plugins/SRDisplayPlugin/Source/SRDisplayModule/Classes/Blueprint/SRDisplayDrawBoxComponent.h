/*
 * Copyright 2019,2020,2021,2022 Sony Corporation
 */
#pragma once

#include <CoreMinimal.h>
#include <Components/PrimitiveComponent.h>

#include "SRDisplayDrawBoxComponent.generated.h"

class FPrimitiveSceneProxy;

enum class DrawBoxSize {
	ELF_SR1_Size = 0,
	ELF_SR2_Size = 1
};

UCLASS(collapsecategories, hidecategories = Object, editinlinenew, MinimalAPI)
class USRDisplayDrawBoxComponent : public UPrimitiveComponent
{
	GENERATED_UCLASS_BODY()

	UPROPERTY(EditAnywhere, Category = DrawBoxComponent)
	FColor BoxColor;

	UPROPERTY(EditAnywhere, Category = DrawBoxComponent)
	FColor DisplayColor;

	UPROPERTY(EditAnywhere, Category = DrawBoxComponent)
	float LineThickness;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = DrawBoxComponent)
	float TargetScale;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = DrawBoxComponent)
	mutable float DisplayWidth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = DrawBoxComponent)
	mutable float DisplayHeight;

	mutable float DisplayTilt;

	DrawBoxSize BoxSize;

	//~ Begin UPrimitiveComponent Interface.
	virtual FPrimitiveSceneProxy* CreateSceneProxy() override;
	virtual FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override;
	//~ End UPrimitiveComponent Interface.

private:
	void UpdateDisplaySpec() const;
};
