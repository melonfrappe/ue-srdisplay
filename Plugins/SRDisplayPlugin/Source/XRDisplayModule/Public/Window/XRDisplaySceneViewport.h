/*
 * Copyright 2019,2020,2021,2022 Sony Corporation
 */

#pragma once

// <-- Engine Header -->
#include <CoreMinimal.h>
#include <Widgets/SCompoundWidget.h>
#include <Widgets/DeclarativeSyntaxSupport.h>

class SViewport;
class FSceneViewport;
class UTextureRenderTarget2D;
class FRHICommandListImmediate;

namespace xr_display
{
	class FXRDisplayViewportClient;

	class SXRDisplaySceneViewport : public SCompoundWidget
	{
	public:

		SLATE_BEGIN_ARGS(SXRDisplaySceneViewport) { }
		SLATE_END_ARGS()

	public:

		void Construct(const FArguments& InArgs);

		void Init(float SizeX, float SizeY);

		void Shutdown();

		virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

		UTextureRenderTarget2D* GetRenderTarget();

		void RenderTexture_RenderThread(FRHICommandListImmediate& RHICmdList);

		bool GetMouseRightButtonPressed() const;
		bool GetMousePosition(FVector2D& MousePosition) const;

		FViewport* GetViewport();

	private:

		TSharedPtr<SViewport> Viewport;
		TSharedPtr<FSceneViewport> SceneViewport;
		TSharedPtr<FXRDisplayViewportClient> PlayViewportClient;
	};

} // namespace xr_display
