/*
 * Copyright 2019,2020,2021,2022 Sony Corporation
 */

#pragma once

// <-- Engine Header -->
#include <Engine.h>
#include <CoreMinimal.h>
#include <Widgets/SWindow.h>

class UTextureRenderTarget2D;
class FRHICommandListImmediate;

namespace xr_display
{
	class SXRDisplaySceneViewport;

	class FXRDisplayWindow
	{
	public:

		static void Initialize(FVector2D Resolution, FIntRect ScreenPosition);
		static void Finalize();
		static FXRDisplayWindow* GetPlayWindow();

	public:

		FXRDisplayWindow(FVector2D Resolution);
		~FXRDisplayWindow();

		TSharedPtr<SWindow> GetWindow();
		UTextureRenderTarget2D* GetRenderTarget();

		void RenderTexture_RenderThread(FRHICommandListImmediate& RHICmdList);

		bool GetMouseRightButtonPressed() const;
		bool GetMousePosition(FVector2D& MousePosition) const;

		FViewport* GetViewport();

	private:

		void OnWindowClosed(const TSharedRef<SWindow>& InWindow);

	private:

		static TSharedPtr<FXRDisplayWindow> PlayWindow;

		TSharedPtr<SWindow> Window;
		FOnWindowClosed InDelegate;
		TSharedPtr<SXRDisplaySceneViewport> PlaySceneViewport;
	};

} // namespace xr_display
