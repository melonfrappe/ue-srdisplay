/*
 * Copyright 2019,2020,2021,2022 Sony Corporation
 */

#include "XRDisplayModule.h"
#include "XRDisplaySystem.h"

#include <SceneViewExtension.h>
#include <Interfaces/IPluginManager.h>

 //-------------------------------------------------------------------------------------------------
 // FSRDisplayModule
 //-------------------------------------------------------------------------------------------------

namespace xr_display
{
	class FSRDisplayModule : public IXRDisplayModule
	{
		/** IHeadMountedDisplayModule implementation */
		virtual TSharedPtr< class IXRTrackingSystem, ESPMode::ThreadSafe > CreateTrackingSystem() override;

		virtual void StartupModule() override
		{
			IHeadMountedDisplayModule::StartupModule();
			// This code will execute after your module is loaded into memory (but after global variables are initialized, of course.)
		}

		virtual void ShutdownModule() override
		{
			// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
			// we call this function before unloading the module.
		}

		virtual FString GetModuleKeyName() const override
		{
			return FString(TEXT("XRDisplayModule"));
		}

	};

	TSharedPtr< class IXRTrackingSystem, ESPMode::ThreadSafe > FSRDisplayModule::CreateTrackingSystem()
	{
		TSharedRef<xr_display::FXRDisplaySystem, ESPMode::ThreadSafe> ARHeadMountedDisplay = FSceneViewExtensions::NewExtension<xr_display::FXRDisplaySystem>();
		return ARHeadMountedDisplay;
	}

} // namespace xr_display

IMPLEMENT_MODULE(xr_display::FSRDisplayModule, XRDisplayModule)
