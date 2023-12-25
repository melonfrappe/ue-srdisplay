/*
 * Copyright 2019,2020,2021,2022 Sony Corporation
 */

// <-- Plugin Header -->
#include "Window/XRDisplaySceneViewport.h"
#include "Window/XRDisplayViewportClient.h"

// <-- Engine Header -->
#include <Slate/SceneViewport.h>
#include <Widgets/SViewport.h>

namespace xr_display
{
	void SXRDisplaySceneViewport::Construct(const FArguments& InArgs)
	{
		// Create Viewport Widget
		Viewport = SNew(SViewport)
			.IsEnabled(true)
			.EnableGammaCorrection(false)
			.ShowEffectWhenDisabled(false)
			.EnableBlending(true);
	}

	void SXRDisplaySceneViewport::Init(float SizeX, float SizeY)
	{
		// Create Viewport Client
		PlayViewportClient = MakeShareable(new FXRDisplayViewportClient(SizeX, SizeY));

		// Create Scene Viewport
		SceneViewport = MakeShareable(new FSceneViewport(PlayViewportClient.Get(), Viewport));

		// Assign SceneViewport to Viewport widget. It needed for rendering
		Viewport->SetViewportInterface(SceneViewport.ToSharedRef());
		PlayViewportClient->Viewport = SceneViewport.Get();

		// Assing Viewport widget for our custom PlayScene Viewport
		this->ChildSlot
		[
			Viewport.ToSharedRef()
		];
	}

	UTextureRenderTarget2D* SXRDisplaySceneViewport::GetRenderTarget()
	{
		return PlayViewportClient->GetRenderTarget();
	}

	void SXRDisplaySceneViewport::Shutdown()
	{
		if (SceneViewport.IsValid())
		{
			SceneViewport.Reset();
			SceneViewport = nullptr;
		}

		if (PlayViewportClient.IsValid())
		{
			PlayViewportClient->Shutdown();
			PlayViewportClient.Reset();
			PlayViewportClient = nullptr;
		}

		if (Viewport.IsValid())
		{
			Viewport.Reset();
			Viewport = nullptr;
		}
	}

	void SXRDisplaySceneViewport::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
	{
		SWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

		PlayViewportClient->Tick();

		static bool bFirstTime = true;
		if (bFirstTime)
		{
			bFirstTime = false;
			FSlateApplication::Get().RegisterGameViewport(Viewport.ToSharedRef());
		}
	}

	void SXRDisplaySceneViewport::RenderTexture_RenderThread(FRHICommandListImmediate& RHICmdList)
	{
		PlayViewportClient->RenderTexture_RenderThread(RHICmdList);
	}

	bool SXRDisplaySceneViewport::GetMouseRightButtonPressed() const
	{
		return PlayViewportClient->GetMouseRightButtonPressed();
	}

	bool SXRDisplaySceneViewport::GetMousePosition(FVector2D& MousePosition) const
	{
		return PlayViewportClient->GetMousePosition(MousePosition);
	}

	FViewport* SXRDisplaySceneViewport::GetViewport()
	{
		if (PlayViewportClient.IsValid())
		{
			return PlayViewportClient->Viewport;
		}
		return nullptr;
	}

} // namespace xr_display
