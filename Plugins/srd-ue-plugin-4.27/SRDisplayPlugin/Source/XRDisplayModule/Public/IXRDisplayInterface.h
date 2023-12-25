/*
 * Copyright 2019,2020,2021 Sony Corporation
 */

#pragma once

#include <CoreMinimal.h>
#include "ScreenRendering.h"

#include "xr-runtime-common/xr_api_defs.h"

XR_DEFINE_HANDLE(SonyOzSessionHandle);

namespace xr_display
{
	struct FDisplayCorners {
		FVector4 LeftBottom;
		FVector4 LeftTop;
		FVector4 RightBottom;
		FVector4 RightTop;
	};

	struct XrDisplaySpec {
		struct DisplaySize {
			float width_m;
			float height_m;
		} display_size;

		struct DisplayResolution {
			uint32_t width;
			uint32_t height;
			uint32_t area;
		} display_resolution;

		float display_tilt_rad;

		XrDisplaySpec()
			: display_size({ 0.f, 0.f }),
			display_resolution({ 0, 0, 0 }),
			display_tilt_rad(0.f) {}

		XrDisplaySpec(DisplaySize in_display_size,
			DisplayResolution in_display_resolution,
			float in_display_tilt_rad)
			: display_size(in_display_size),
			display_resolution(in_display_resolution),
			display_tilt_rad(in_display_tilt_rad) {}
	};

	class IXRDisplayInterface
	{
	public:
		virtual ~IXRDisplayInterface() {}

		virtual FName GetSystemName() const = 0;

		// Begin IXRTrackingSystem
		virtual FString GetVersionString() const = 0;
		virtual void OnBeginPlay(FWorldContext& InWorldContext) = 0;
		virtual void OnEndPlay(FWorldContext& InWorldContext) = 0;
		virtual bool OnStartGameFrame(FWorldContext& WorldContext) = 0;
		virtual void OnBeginRendering_RenderThread(FRHICommandListImmediate& RHICmdList, FSceneViewFamily& ViewFamily) {};
		// End IXRTrackingSystem

		// Begin IStereoRendering
		virtual void RenderTexture_RenderThread(FRHICommandListImmediate& RHICmdList, FRHITexture2D* BackBuffer, FRHITexture2D* SrcTexture, FVector2D WindowSize, bool skipHomography) const {};
		// End IStereoRendering

	public:
		virtual const char* GetPlatformId() = 0;
		virtual uint32_t GetDeviceIndex(const SonyOzDeviceInfo* DeviceList, const uint64_t Size) = 0;
		virtual void ConvertPoseFromXrRuntimeToGameWorld(FTransform Transform, FQuat& DstOrientation, FVector& DstPosition) const = 0;

		virtual float GetNearClip() = 0;
		virtual float GetFarClip() = 0;

		virtual SonyOzPosef GetStubHeadPose() = 0;
		virtual void SetStubHeadPose(const SonyOzPosef pose) = 0;

		virtual void SetSessionHandle(SonyOzSessionHandle InSessionHandle) { SessionHandle = InSessionHandle; };
		virtual void RemoveSessionHandle() { SessionHandle = nullptr; };
		virtual void UpdateProjectionMatrix(FMatrix& LeftViewProjectionMatrix, FMatrix& RightViewProjectionMatrix) {};
		virtual void SetDisplaySpec(XrDisplaySpec InDisplaySpec) {};
		virtual bool DeprojectScreenToWorld(FSceneViewProjectionData ProjectionData, const FVector2D& ScreenPosition, FVector& WorldPosition, FVector& WorldDirection, FVector& CameraPosition) { return false; };
		virtual FDisplayCorners GetDisplayCornners() { return FDisplayCorners(); };

	protected:
		SonyOzSessionHandle SessionHandle = nullptr;
	};// class IXRDisplayInterface
} // namespace xr_display
