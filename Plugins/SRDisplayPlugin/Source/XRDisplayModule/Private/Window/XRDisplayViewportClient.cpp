/*
 * Copyright 2019,2020,2021,2022 Sony Corporation
 */

// <-- Plugin Header -->
#include "Window/XRDisplayViewportClient.h"

// <-- Engine Header -->
#include <Engine/Engine.h>
#include <Engine/TextureRenderTarget2D.h>
#include <Engine/GameViewportClient.h>
#include <CanvasTypes.h>
#include <CanvasItem.h>
#include <Framework/Application/SlateApplication.h>
#include <GameFramework/InputSettings.h>
#include <GenericPlatform/GenericApplication.h>
#include <InputCoreTypes.h>
#include <Windows/WindowsHWrapper.h>

#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"


namespace xr_display
{
	FXRDisplayViewportClient::FXRDisplayViewportClient(float SizeX, float SizeY)
		: Viewport(nullptr)
		, bShowMouseCursor(true)
		, bLeaveMouseCursor(false)
		, MouseCaptureMode(EMouseCaptureMode::CapturePermanently)
	{
		RenderTarget = NewObject<UTextureRenderTarget2D>(GetTransientPackage(), NAME_None, RF_Transient);
		RenderTarget->RenderTargetFormat = RTF_RGBA16f;
		RenderTarget->OverrideFormat = PF_A2B10G10R10;
		// [issue #26] TODO: Get display size from device
		RenderTarget->InitAutoFormat(SizeX, SizeY);
		RenderTarget->ClearColor = FLinearColor::Black;
		RenderTarget->UpdateResource();
		RenderTarget->AddToRoot();

		ShowMouseCursor(true);

		MouseCaptureMode = GetDefault<UInputSettings>()->DefaultViewportMouseCaptureMode;
		FString DefaultViewportMouseCaptureMode;
		if (FParse::Value(FCommandLine::Get(), TEXT("DefaultViewportMouseCaptureMode="), DefaultViewportMouseCaptureMode))
		{
			const UEnum* EnumPtr = StaticEnum<EMouseCaptureMode>();
			checkf(EnumPtr, TEXT("Unable to find EMouseCaptureMode enum"));
			if (EnumPtr)
			{
				int64 EnumValue = EnumPtr->GetValueByName(FName(*DefaultViewportMouseCaptureMode));
				if (EnumValue != INDEX_NONE)
				{
					MouseCaptureMode = static_cast<EMouseCaptureMode>(EnumValue);
				}
				else
				{
					UE_LOG(LogInit, Warning, TEXT("Unknown DefaultViewportMouseCaptureMode %s. Command line setting will be ignored."), *DefaultViewportMouseCaptureMode);
				}
			}
		}
	}

	void FXRDisplayViewportClient::Shutdown()
	{
		if (RenderTarget)
		{
			RenderTarget->RemoveFromRoot();
			RenderTarget = nullptr;
		}

		bShowMouseCursor = true;
		bLeaveMouseCursor = true;
		ShowMouseCursor(true);
		Viewport = nullptr;
	}

	void FXRDisplayViewportClient::Tick()
	{
		if (GEngine && GEngine->GameViewport)
		{
			UGameViewportClient* ViewportClient = GEngine->GameViewport;
			if (ViewportClient->GetWorld())
			{
				APlayerController* PlayerController = UGameplayStatics::GetPlayerController(ViewportClient->GetWorld(), 0);
				if (PlayerController)
				{
					if (!bLeaveMouseCursor && bShowMouseCursor != PlayerController->bShowMouseCursor)
					{
						ShowMouseCursor(PlayerController->bShowMouseCursor);
						bShowMouseCursor = PlayerController->bShowMouseCursor;
					}
				}
			}
		}
	}

	bool FXRDisplayViewportClient::InputKey(const FInputKeyEventArgs& EventArgs)
	{
		// Prohibit release of fullscreen mode by key operation
		if ((EventArgs.Key == EKeys::Enter && EventArgs.Event == EInputEvent::IE_Pressed && FSlateApplication::Get().GetModifierKeys().IsAltDown() && GetDefault<UInputSettings>()->bAltEnterTogglesFullscreen)
			|| (IsRunningGame() && EventArgs.Key == EKeys::F11 && EventArgs.Event == EInputEvent::IE_Pressed && GetDefault<UInputSettings>()->bF11TogglesFullscreen && !FSlateApplication::Get().GetModifierKeys().AreModifersDown(EModifierKey::Control | EModifierKey::Alt)))
		{
			return false;
		}

		if (EventArgs.Key == EKeys::RightMouseButton) {
			bMouseRightButtonPressed = EventArgs.Event == EInputEvent::IE_Pressed;
		}

		UGameViewportClient* ViewportClient = GEngine->GameViewport;
		return ViewportClient->InputKey(EventArgs);
	}

	bool FXRDisplayViewportClient::InputAxis(FViewport* InViewport, int32 ControllerId, FKey Key, float Delta, float DeltaTime, int32 NumSamples, bool bGamepad)
	{
		UGameViewportClient* ViewportClient = GEngine->GameViewport;
		return ViewportClient->InputAxis(InViewport, ControllerId, Key, Delta, DeltaTime, NumSamples, bGamepad);
	}

	bool FXRDisplayViewportClient::InputChar(FViewport* InViewport, int32 ControllerId, TCHAR Character)
	{
		UGameViewportClient* ViewportClient = GEngine->GameViewport;
		return ViewportClient->InputChar(InViewport, ControllerId, Character);
	}

	bool FXRDisplayViewportClient::InputTouch(FViewport* InViewport, int32 ControllerId, uint32 Handle, ETouchType::Type Type, const FVector2D& TouchLocation, float Force, FDateTime DeviceTimestamp, uint32 TouchpadIndex)
	{
		UGameViewportClient* ViewportClient = GEngine->GameViewport;
		return ViewportClient->InputTouch(InViewport, ControllerId, Handle, Type, TouchLocation, Force, DeviceTimestamp, TouchpadIndex);
	}

	bool FXRDisplayViewportClient::InputMotion(FViewport* InViewport, int32 ControllerId, const FVector& Tilt, const FVector& RotationRate, const FVector& Gravity, const FVector& Acceleration)
	{
		UGameViewportClient* ViewportClient = GEngine->GameViewport;
		return ViewportClient->InputMotion(InViewport, ControllerId, Tilt, RotationRate, Gravity, Acceleration);
	}

	void FXRDisplayViewportClient::MouseEnter(FViewport* InViewport, int32 x, int32 y)
	{
		bLeaveMouseCursor = false;
		if (GEngine && GEngine->GameViewport)
		{
			UGameViewportClient* ViewportClient = GEngine->GameViewport;
			if (ViewportClient->GetWorld())
			{
				APlayerController* PlayerController = UGameplayStatics::GetPlayerController(ViewportClient->GetWorld(), 0);
				if (PlayerController)
				{
					ShowMouseCursor(PlayerController->bShowMouseCursor);
				}
			}
		}
	}

	void FXRDisplayViewportClient::MouseLeave(FViewport* InViewport)
	{
		bLeaveMouseCursor = true;
		ShowMouseCursor(true);
	}

	UTextureRenderTarget2D* FXRDisplayViewportClient::GetRenderTarget()
	{
		return RenderTarget;
	}

	void FXRDisplayViewportClient::RenderTexture_RenderThread(FRHICommandListImmediate& RHICmdList)
	{
		if (RenderTarget && RenderTarget->Resource && Viewport)
		{
			if (Viewport->GetSizeXY().X == 0 || Viewport->GetSizeXY().Y == 0) {
				return;
			}

			RenderTarget->Resource->bIgnoreGammaConversions = true;

			UWorld* ViewportWorld = GetWorld();
			FCanvas Canvas(Viewport, nullptr, ViewportWorld, ViewportWorld ? ViewportWorld->FeatureLevel.GetValue() : GMaxRHIFeatureLevel, FCanvas::CDM_DeferDrawing, ShouldDPIScaleSceneCanvas() ? GetDPIScale() : 1.f);
			Canvas.SetRenderTargetRect(FIntRect(0, 0, Viewport->GetSizeXY().X, Viewport->GetSizeXY().Y));
			{
				Canvas.SetAllowedModes(0);
				FCanvasTileItem TileItem(
					FVector2D::ZeroVector,
					RenderTarget->Resource,
					Viewport->GetSizeXY(),
					FLinearColor::White);
				TileItem.BlendMode = ESimpleElementBlendMode::SE_BLEND_Opaque;
				Canvas.DrawItem(TileItem);
			}
			Canvas.Flush_RenderThread(RHICmdList, true);
		}
	}

	bool FXRDisplayViewportClient::GetMouseRightButtonPressed() const
	{
		return bMouseRightButtonPressed;
	}

	bool FXRDisplayViewportClient::GetMousePosition(FVector2D& MousePosition) const
	{
		bool bGotMousePosition = false;

		if (Viewport && FSlateApplication::Get().IsMouseAttached())
		{
			if (bLeaveMouseCursor)
			{
				return false;
			}
			FIntPoint MousePos;
			Viewport->GetMousePos(MousePos);
			if (MousePos.X >= 0 && MousePos.Y >= 0)
			{
				MousePosition = FVector2D(MousePos);
				bGotMousePosition = true;
			}
		}

		return bGotMousePosition;
	}

	void FXRDisplayViewportClient::ShowMouseCursor(bool bShow)
	{
#if PLATFORM_WINDOWS
		if (bShow)
		{
			// Show mouse cursor. Each time ShowCursor(true) is called an internal value is incremented so we
			// call ShowCursor until the cursor is actually shown (>= 0 value returned by showcursor)
			while (::ShowCursor(true) < 0);
		}
		else
		{
			// Disable the cursor.  Wait until its actually disabled.
			while (::ShowCursor(false) >= 0);
		}
#endif
	}

} // namespace xr_display
