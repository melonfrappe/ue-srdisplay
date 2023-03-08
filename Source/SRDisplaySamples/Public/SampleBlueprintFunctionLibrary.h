// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SampleBlueprintFunctionLibrary.generated.h"

/**
 *
 */
UCLASS()
class SRDISPLAYSAMPLES_API USampleBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	/**
	* @brief return fps value
	*/
	UFUNCTION(BlueprintCallable)
	static float GetAverageFps();

};
