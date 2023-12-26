/*
 * Copyright 2019,2020.2021,2022 Sony Corporation
 */

// <-- Plugin Header -->
#include "Window/XRDisplayWindow.h"
#include "Window/XRDisplaySceneViewport.h"

// <-- Engine Header -->
#include <Engine/Engine.h>
#include <Engine/GameViewportClient.h>
#include <UnrealEngine.h>
#include <Framework/Application/SlateApplication.h>
#include <Windows/WindowsHWrapper.h>

namespace xr_display
{
	TSharedPtr<FXRDisplayWindow> FXRDisplayWindow::PlayWindow = nullptr;

	FXRDisplayWindow::FXRDisplayWindow(FVector2D Resolution)
	{

		// Hide and disable engine window
#if PLATFORM_WINDOWS
		HWND WindowHandle = (HWND)GEngine->GameViewport->GetWindow()->GetNativeWindow()->GetOSWindowHandle();
		if (WindowHandle)
		{
			::EnableWindow(WindowHandle, false);
			::SetParent(WindowHandle, HWND_MESSAGE);
		}
#endif

		// Create original window
		Window = SNew(SWindow)
			.Type(EWindowType::GameWindow)
			.Title(FText::FromString("Sony XR Display Window"))
			.AutoCenter(EAutoCenter::PrimaryWorkArea)
			.ScreenPosition(FVector2D(0, 0))
			.ClientSize(Resolution)
			.SizingRule(ESizingRule::UserSized)
			.IsTopmostWindow(true)
			.ActivationPolicy(EWindowActivationPolicy::FirstShown)
			.FocusWhenFirstShown(true)
			.UseOSWindowBorder(true)
			.IsInitiallyMinimized(true)
			.SaneWindowPlacement(false);


		InDelegate.BindRaw(this, &FXRDisplayWindow::OnWindowClosed);
		Window->SetOnWindowClosed(InDelegate);

		PlaySceneViewport = SNew(SXRDisplaySceneViewport);
		PlaySceneViewport->Init(Resolution.X, Resolution.Y);
		Window->SetContent(PlaySceneViewport.ToSharedRef());
	}

	FXRDisplayWindow::~FXRDisplayWindow()
	{
		InDelegate.Unbind();
	}

	TSharedPtr<SWindow> FXRDisplayWindow::GetWindow()
	{
		if (Window.IsValid())
		{
			return Window;
		}
		return nullptr;
	}

	UTextureRenderTarget2D* FXRDisplayWindow::GetRenderTarget()
	{
		if (PlaySceneViewport.IsValid())
		{
			return PlaySceneViewport->GetRenderTarget();
		}
		return nullptr;
	}

	void FXRDisplayWindow::RenderTexture_RenderThread(FRHICommandListImmediate& RHICmdList)
	{
		if (PlaySceneViewport.IsValid())
		{
			PlaySceneViewport->RenderTexture_RenderThread(RHICmdList);
		}
	}

	void FXRDisplayWindow::Initialize(FVector2D Resolution, FIntRect ScreenPosition)
	{
		if (!PlayWindow.IsValid())
		{
			FXRDisplayWindow::PlayWindow = MakeShareable(new FXRDisplayWindow(Resolution));
		}

		if (GEngine && FXRDisplayWindow::PlayWindow && FXRDisplayWindow::PlayWindow->GetWindow())
		{
			const float WindowPosX = static_cast<float>(ScreenPosition.Min.X);
			const float WindowPosY = static_cast<float>(ScreenPosition.Min.Y);
			const FVector2D Position(WindowPosX, WindowPosY);
			const float WindowWidth = static_cast<float>(ScreenPosition.Width());
			const float WindowHeight = static_cast<float>(ScreenPosition.Height());
			const FVector2D Size(WindowWidth, WindowHeight);

			// Don't move and resize window after AddWindowAsNativeChild and before SetWindowMode. If do so, it will make window Vsync ON.
			FXRDisplayWindow::GetPlayWindow()->GetWindow()->MoveWindowTo(Position);
			FXRDisplayWindow::GetPlayWindow()->GetWindow()->Resize(Size);

			FSlateApplication::Get().AddWindowAsNativeChild(
				FXRDisplayWindow::GetPlayWindow()->GetWindow().ToSharedRef(), GEngine->GameViewport->GetWindow().ToSharedRef());
			FXRDisplayWindow::GetPlayWindow()->GetWindow()->SetWindowMode(EWindowMode::WindowedFullscreen);
		}

		if (GEngine)
		{
			UGameUserSettings* UserSettings = GEngine->GetGameUserSettings();
			UserSettings->SetFullscreenMode(EWindowMode::Windowed);
			UserSettings->ApplySettings(false);
			UserSettings->SetFullscreenMode(EWindowMode::WindowedFullscreen);
			UserSettings->ApplySettings(false);
		}
	}

	void FXRDisplayWindow::Finalize()
	{
		if (FXRDisplayWindow::PlayWindow.IsValid())
		{
			if (FXRDisplayWindow::PlayWindow->PlaySceneViewport.IsValid())
			{
				FXRDisplayWindow::PlayWindow->PlaySceneViewport->Shutdown();
				FXRDisplayWindow::PlayWindow->PlaySceneViewport.Reset();
				FXRDisplayWindow::PlayWindow->PlaySceneViewport = nullptr;
			}

			FXRDisplayWindow::PlayWindow.Reset();
			FXRDisplayWindow::PlayWindow = nullptr;

			if (GEngine && GEngine->GameViewport)
			{
				TSharedPtr<SWindow> EngineWindow = GEngine->GameViewport->GetWindow();
				EngineWindow->RequestDestroyWindow();
			}
		}
	}

	FXRDisplayWindow* FXRDisplayWindow::GetPlayWindow()
	{
		if (PlayWindow.IsValid())
		{
			return PlayWindow.Get();
		}
		return nullptr;
	}

	void FXRDisplayWindow::OnWindowClosed(const TSharedRef<SWindow>& InWindow)
	{
		FXRDisplayWindow::Finalize();
	}

	bool FXRDisplayWindow::GetMouseRightButtonPressed() const
	{
		if (PlaySceneViewport.IsValid()) {
			return PlaySceneViewport->GetMouseRightButtonPressed();
		}
		return false;
	}


	bool FXRDisplayWindow::GetMousePosition(FVector2D& MousePosition) const
	{
		if (PlaySceneViewport.IsValid())
		{
			return PlaySceneViewport->GetMousePosition(MousePosition);
		}
		return false;
	}

	FViewport* FXRDisplayWindow::GetViewport()
	{
		if (PlaySceneViewport.IsValid())
		{
			return PlaySceneViewport->GetViewport();
		}
		return nullptr;
	}

} // namespace xr_display
