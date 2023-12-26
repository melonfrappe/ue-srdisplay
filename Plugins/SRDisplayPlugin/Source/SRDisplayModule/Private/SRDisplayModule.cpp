/**
 * Copyright 2019,2020,2021,2022 Sony Corporation
 */
#include "SRDisplayModule.h"

#include <Interfaces/IPluginManager.h>

#define LOCTEXT_NAMESPACE "FSRDisplayModule"

void FSRDisplayModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
	SRDispalySystem = MakeUnique<srdisplay_module::FSRDisplaySystem>();

	FString PluginShaderDir = FPaths::Combine(IPluginManager::Get().FindPlugin(TEXT("SRDisplayPlugin"))->GetBaseDir(), TEXT("Shaders"));
	AddShaderSourceDirectoryMapping(TEXT("/Plugin/SRDisplayPlugin"), PluginShaderDir);
}

void FSRDisplayModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FSRDisplayModule, SRDisplayModule)
