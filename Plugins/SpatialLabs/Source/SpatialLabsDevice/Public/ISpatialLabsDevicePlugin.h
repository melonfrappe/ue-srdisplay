// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "IHeadMountedDisplayModule.h"

class SPATIALLABSDEVICE_API ISpatialLabsDevicePlugin : public IHeadMountedDisplayModule
{

public:

	static inline ISpatialLabsDevicePlugin& Get()
	{
		return FModuleManager::LoadModuleChecked< ISpatialLabsDevicePlugin >("SpatialLabsDevice");
	}

	static inline bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded("SpatialLabsDevice");
	}

	virtual bool IsExtensionAvailable(const FString& Name) const = 0;
	virtual bool IsExtensionEnabled(const FString& Name) const = 0;

	virtual bool IsLayerAvailable(const FString& Name) const = 0;
	virtual bool IsLayerEnabled(const FString& Name) const = 0;
};
