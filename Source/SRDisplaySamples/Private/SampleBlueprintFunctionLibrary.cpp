// Fill out your copyright notice in the Description page of Project Settings.


#include "SampleBlueprintFunctionLibrary.h"

float USampleBlueprintFunctionLibrary::GetAverageFps()
{
	extern ENGINE_API float GAverageFPS;
	return GAverageFPS;
}
