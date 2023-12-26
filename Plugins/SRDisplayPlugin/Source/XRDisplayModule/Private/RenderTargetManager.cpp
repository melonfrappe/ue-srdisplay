/*
 * Copyright 2019,2020,2021,2022 Sony Corporation
 */

#include "RenderTargetManager.h"

#include <Engine.h>
#include <IXRTrackingSystem.h>

namespace xr_display
{
	FRenderTargetManager::FRenderTargetManager()
	{
	}

	FRenderTargetManager::~FRenderTargetManager()
	{
	}

	bool FRenderTargetManager::ShouldUseSeparateRenderTarget() const
	{
		bool bStereoEnabled = false;
		if (GEngine->XRSystem.IsValid())
		{
			bStereoEnabled = GEngine->XRSystem->GetStereoRenderingDevice()->IsStereoEnabled();
		}

		return bStereoEnabled;
	}

	void FRenderTargetManager::UpdateViewportRHIBridge(bool bUseSeparateRenderTarget, const class FViewport& Viewport, FRHIViewport* const ViewportRHI)
	{
	}

} // namespace xr_display
