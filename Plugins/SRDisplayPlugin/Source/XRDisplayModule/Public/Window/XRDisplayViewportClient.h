/*
 * Copyright 2019,2020,2021,2022 Sony Corporation
 */

#pragma once

// <-- Engine Header -->
#include <CoreMinimal.h>
#include <UnrealClient.h>

class UTextureRenderTarget2D;

namespace xr_display
{
	class FXRDisplayViewportClient : public FViewportClient
	{
	public:

		FXRDisplayViewportClient(float SizeX, float SizeY);

		void Shutdown();

		void Tick();

		virtual bool InputKey(const FInputKeyEventArgs& EventArgs) override;
		virtual bool InputAxis(FViewport* InViewport, int32 ControllerId, FKey Key, float Delta, float DeltaTime, int32 NumSamples = 1, bool bGamepad = false) override;
		virtual bool InputChar(FViewport* InViewport, int32 ControllerId, TCHAR Character) override;
		virtual bool InputTouch(FViewport* InViewport, int32 ControllerId, uint32 Handle, ETouchType::Type Type, const FVector2D& TouchLocation, float Force, FDateTime DeviceTimestamp, uint32 TouchpadIndex) override;
		virtual bool InputMotion(FViewport* INViewport, int32 ControllerId, const FVector& Tilt, const FVector& RotationRate, const FVector& Gravity, const FVector& Acceleration) override;

		virtual void MouseEnter(FViewport* InViewport, int32 x, int32 y) override;
		virtual void MouseLeave(FViewport* InViewport) override;
#if ENGINE_MAJOR_VERSION == 5 || ENGINE_MINOR_VERSION >= 26
		virtual EMouseCaptureMode GetMouseCaptureMode() const override
#else // ENGINE_MAJOR_VERSION == 5 || ENGINE_MINOR_VERSION >= 26
		virtual EMouseCaptureMode CaptureMouseOnClick() override
#endif // ENGINE_MAJOR_VERSION == 5 || ENGINE_MINOR_VERSION >= 26
		{
			return MouseCaptureMode;
		}

		UTextureRenderTarget2D* GetRenderTarget();

		// Draw canvas in RenderThread
		void RenderTexture_RenderThread(FRHICommandListImmediate& RHICmdList);

		bool GetMouseRightButtonPressed() const;
		bool GetMousePosition(FVector2D& MousePosition) const;

	private:

		void ShowMouseCursor(bool bShow);

	public:

		FViewport* Viewport;

	private:

		UTextureRenderTarget2D* RenderTarget;

		bool bShowMouseCursor;
		bool bMouseRightButtonPressed = false;

		// Used to prevent the cursor from being hidden when it is leave from the Window
		bool bLeaveMouseCursor;
		EMouseCaptureMode MouseCaptureMode;
	};

} // namespace xr_display
