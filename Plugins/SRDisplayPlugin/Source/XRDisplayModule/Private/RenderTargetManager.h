/*
 * Copyright 2019,2020,2021,2022 Sony Corporation
 */

#pragma once

#include "XRRenderTargetManager.h"

namespace xr_display
{
	class XRDISPLAYMODULE_API FRenderTargetManager : public FXRRenderTargetManager
	{

	public:

		FRenderTargetManager();
		virtual ~FRenderTargetManager();

		// Begin IStereoRenderTargetManager interface
		virtual bool ShouldUseSeparateRenderTarget() const override;
		// End IStereoRenderTargetManager interface

	protected:

		// Begin FXRRenderTargetManager interface
		virtual void UpdateViewportRHIBridge(bool bUseSeparateRenderTarget, const class FViewport& Viewport, FRHIViewport* const ViewportRHI) override;
		// End FXRRenderTargetManager interface

	private:

		bool bInitialized;

	};

} // namespace xr_display
