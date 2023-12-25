/*
 * Copyright 2019,2020,2021 Sony Corporation
 */

#pragma once

#include "IXRDisplayInterface.h"
#include "Blueprint/SRDisplayManager.h"

#include <CoreMinimal.h>
#include <HeadMountedDisplayBase.h>
#include <SceneViewExtension.h>

#include "misc_util_wrapper.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSRDisplay, Log, All);

namespace srdisplay_module
{
	class SRDISPLAYMODULE_API FSRDisplaySystem : public xr_display::IXRDisplayInterface
	{

	public:

		FSRDisplaySystem();
		~FSRDisplaySystem();

		// Begin IXRSystemIdentifier
		virtual FName GetSystemName() const override
		{
			static const FName Name(TEXT("SRDisplay"));
			return Name;
		}
		// End IXRSystemIdentifier

		// Begin IXRTrackingSystem
		virtual FString GetVersionString() const override;
		virtual void OnBeginPlay(FWorldContext& InWorldContext) override;
		virtual void OnEndPlay(FWorldContext& InWorldContext) override;
		virtual bool OnStartGameFrame(FWorldContext& WorldContext) override;
		virtual void OnBeginRendering_RenderThread(FRHICommandListImmediate& RHICmdList, FSceneViewFamily& ViewFamily) override;
		// End IXRTrackingSystem

		// Begin IStereoRendering
		virtual void RenderTexture_RenderThread(FRHICommandListImmediate& RHICmdList, FRHITexture2D* BackBuffer, FRHITexture2D* SrcTexture, FVector2D WindowSize, bool skipHomography) const override;
		void HomographyTransform(FRHICommandListImmediate& RHICmdList, FRHITexture2D* BackBuffer, FRHITexture2D* SrcTexture, FVector2D WindowSize) const;
		void LowPassFilter(FRHICommandListImmediate& RHICmdList, FRHITexture2D* BackBuffer, FRHITexture2D* SrcTexture, FVector2D WindowSize) const;
		// End IStereoRendering

		virtual const char* GetPlatformId() override;
		virtual uint32_t GetDeviceIndex(const SonyOzDeviceInfo* DeviceList, const uint64_t Size) override;

		virtual void ConvertPoseFromXrRuntimeToGameWorld(FTransform Transform, FQuat& DstOrientation, FVector& DstPosition) const override;

		// Get projection matrix and clipping distance, and calculate stereo projection matrix.
		// Please do SetDisplaySpec before execution.
		virtual void UpdateProjectionMatrix(FMatrix& LeftViewProjectionMatrix, FMatrix& RightViewProjectionMatrix) override;

		virtual void SetDisplaySpec(xr_display::XrDisplaySpec InDisplaySpec) override;

		virtual bool DeprojectScreenToWorld(FSceneViewProjectionData ProjectionData, const FVector2D& ScreenPosition, FVector& WorldPosition, FVector& WorldDirection, FVector& CameraPosition) override;

		virtual float GetNearClip() override;
		virtual float GetFarClip() override;

		virtual SonyOzPosef GetStubHeadPose() override;
		virtual void SetStubHeadPose(const SonyOzPosef pose) override;

		virtual xr_display::FDisplayCorners GetDisplayCornners() override;

	public:
		static const float DEFAULT_SRD_VIEW_SPACE_SCALE;
		static const float DEFAULT_SRD_FAR_CLIP;
		static const float METER_TO_CENTIMETER;

	private:
		ASRDisplayManager* GetSRDisplayManagerActor();
		void UpdateSRDisplayManagerState();
		void UpdateSRDisplayPosition();
		bool GetViewMatrix(FMatrix& LeftViewMatrix, FMatrix& RightViewMatrix);
		FMatrix GetSpatialClippedProjectionMatrix(FVector ClipPlanePositionInWorldCoord, FVector ClipPlaneNormalVecInWorldCoord, FMatrix ViewMatrix, FMatrix ProjectionMatrix);
		FMatrix GetNearClipPlaneConvertedProjectionMatrix(FMatrix SrcProjMat, FVector4 PlaneInViewSpace);
		bool GetDisplayCornersPosition(FVector4& LeftBottomPosition, FVector4& LeftTopPosition, FVector4& RightBottomPosition, FVector4& RightTopPosition) const;

	private:
		IRendererModule* RendererModule;
		bool preShowCameraWindow = false;
		bool preIsSRRenderingActive = true;
		sony::oz::xr_runtime::SupportDevice ConnectedDevice = sony::oz::xr_runtime::SupportDevice::ELF_SR1;

		// Actor "SRDisplayManager" and it's params.
		ASRDisplayManager* SRDisplayManager = nullptr;
		float SRDisplayViewSpaceScale = DEFAULT_SRD_VIEW_SPACE_SCALE;
		FTransform SRDisplayTransform = FTransform::Identity;
		float SRDisplayFarClip = DEFAULT_SRD_FAR_CLIP;
		bool SpatialClipping = false;

		// SR Display's specs.
		xr_display::XrDisplaySpec DisplaySpec;
		FVector2D DisplayResolution;
		xr_display::FDisplayCorners DisplayCorners;

		FMatrix LeftViewProjectionMatrix_RenderingThread;
		FMatrix RightViewProjectionMatrix_RenderingThread;
	}; // class FSRDisplaySystem

} // namespace srdisplay_module
