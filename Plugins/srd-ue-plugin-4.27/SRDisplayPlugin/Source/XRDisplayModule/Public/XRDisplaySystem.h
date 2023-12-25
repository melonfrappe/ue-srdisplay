/*
 * Copyright 2019,2020,2021,2022 Sony Corporation
 */

#pragma once

#include "XrRuntime/XrRuntimeWrapper.h"
#include "IXRDisplayInterface.h"

#include <Engine.h>
#include <HeadMountedDisplayBase.h>
#include <SceneViewExtension.h>

DECLARE_LOG_CATEGORY_EXTERN(LogXRDisplay, Log, All);

namespace xr_display
{
	class FRenderTargetManager;

	class XRDISPLAYMODULE_API FXRDisplaySystem : public FHeadMountedDisplayBase, public FSceneViewExtensionBase
	{

	public:

		FXRDisplaySystem(const FAutoRegister&);
		~FXRDisplaySystem();

		static void SetXRDisplay(IXRDisplayInterface* InXRDisplay);
		static IXRDisplayInterface* GetXRDisplay();

		// Begin IXRSystemIdentifier
		virtual FName GetSystemName() const override
		{
			static const FName Name(TEXT("Sony XR Display"));
			return Name;
		}
		// End IXRSystemIdentifier

		// Begin IXRTrackingSystem
		virtual FString GetVersionString() const override;

#if ENGINE_MAJOR_VERSION == 5 || ENGINE_MINOR_VERSION >= 26
		virtual int32 GetXRSystemFlags() const override;
#endif // ENGINE_MAJOR_VERSION == 5 || ENGINE_MINOR_VERSION >= 26
		virtual bool DoesSupportPositionalTracking() const override { return true; };
		virtual bool EnumerateTrackedDevices(TArray<int32>& OutDevices, EXRTrackedDeviceType Type = EXRTrackedDeviceType::Any) override;
		virtual bool GetCurrentPose(int32 DeviceId, FQuat& OutOrientation, FVector& OutPosition) override;
		virtual void ResetOrientationAndPosition(float Yaw = 0.f) override {};
		virtual class IHeadMountedDisplay* GetHMDDevice() override { return this; }
		virtual class TSharedPtr< class IStereoRendering, ESPMode::ThreadSafe > GetStereoRenderingDevice() override { return SharedThis(this); }
		virtual bool IsHeadTrackingAllowed() const override;
		virtual void OnBeginPlay(FWorldContext& InWorldContext) override;
		virtual void OnEndPlay(FWorldContext& InWorldContext) override;
		virtual bool OnStartGameFrame(FWorldContext& WorldContext) override;
		virtual void OnBeginRendering_RenderThread(FRHICommandListImmediate& RHICmdList, FSceneViewFamily& ViewFamily) override;
		// End IXRTrackingSystem

		// Begin FXRTrackingSystemBase
		virtual float GetWorldToMetersScale() const override;
		// End FXRTrackingSystemBase

		// Begin IHeadMountedDisplay
		virtual bool IsHMDConnected() override;
		virtual bool IsHMDEnabled() const override { return true; };
		virtual void EnableHMD(bool bEnable = true) override {};
		virtual bool GetHMDMonitorInfo(MonitorInfo&) override;
		virtual void GetFieldOfView(float& InOutHFOVInDegrees, float& InOutVFOVInDegrees) const override {};
		virtual void SetInterpupillaryDistance(float NewInterpupillaryDistance) override {};
		virtual float GetInterpupillaryDistance() const override { return 0.f; };
		virtual FIntPoint GetIdealRenderTargetSize() const override;
		virtual bool IsChromaAbCorrectionEnabled() const override { return false; };
		// End IHeadMountedDisplay

		// Begin IStereoRendering
		virtual bool IsStereoEnabled() const override;
		virtual bool EnableStereo(bool stereo = true) override;

#if ENGINE_MAJOR_VERSION == 5
		virtual void AdjustViewRect(const int32 ViewIndex, int32& X, int32& Y, uint32& SizeX, uint32& SizeY) const override;
		virtual void CalculateStereoViewOffset(const int32 ViewIndex, FRotator& ViewRotation, const float WorldToMeters, FVector& ViewLocation) override;
		virtual FMatrix GetStereoProjectionMatrix(const int32 ViewIndex) const override;
#else
		virtual void AdjustViewRect(enum EStereoscopicPass StereoPass, int32& X, int32& Y, uint32& SizeX, uint32& SizeY) const override;
		virtual void CalculateStereoViewOffset(const enum EStereoscopicPass StereoPassType, FRotator& ViewRotation, const float WorldToMeters, FVector& ViewLocation) override;
		virtual FMatrix GetStereoProjectionMatrix(const enum EStereoscopicPass StereoPassType) const override;
#endif
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 1
		virtual void RenderTexture_RenderThread(class FRHICommandListImmediate& RHICmdList, class FRHITexture* BackBuffer, class FRHITexture* SrcTexture, FVector2D WindowSize) const override;
#else
		virtual void RenderTexture_RenderThread(class FRHICommandListImmediate& RHICmdList, class FRHITexture2D* BackBuffer, class FRHITexture2D* SrcTexture, FVector2D WindowSize) const override;
#endif

		virtual IStereoRenderTargetManager* GetRenderTargetManager() override;
		// End IStereoRendering

		// Begin ISceneViewExtension
		virtual void SetupViewFamily(FSceneViewFamily& InViewFamily) override;
		virtual void SetupView(FSceneViewFamily& InViewFamily, FSceneView& InView) override;
		virtual void BeginRenderViewFamily(FSceneViewFamily& InViewFamily) override;
		virtual void PreRenderViewFamily_RenderThread(FRHICommandListImmediate& RHICmdList, FSceneViewFamily& InViewFamily) override;
		virtual void PreRenderView_RenderThread(FRHICommandListImmediate& RHICmdList, FSceneView& InView) override;
#if ENGINE_MAJOR_VERSION < 5
		virtual bool IsActiveThisFrame(class FViewport* InViewport) const override;
#endif // ENGINE_MAJOR_VERSION < 5
		// End ISceneViewExtension

	public:
		// Begin SR Display APIs.
		bool GetMousePosition(float& LocationX, float& LocationY);
		bool DeprojectScreenToWorld(APlayerController const* Player, const FVector2D& ScreenPosition, FVector& WorldPosition, FVector& WorldDirection, FVector& CameraPosition);
		bool GetViewMatrix(FMatrix& LeftViewMatrix, FMatrix& RightViewMatrix);
		bool isWithoutXRDisplayMode = false;
		void SetSRRenderingActive(bool isActive);
	private:
		FVector2D PrevMousePos = FVector2D(-1, -1);
		bool isMousePressed = false;
		SonyOzPosef CurrentHeadPose;
		bool isLensShiftEnabled = false;
		bool isPerformancePriorityEnabled = false;
		// End SR Display APIs.

	public:
		static const float DEFAULT_VIEW_SPACE_SCALE;
		static const float DEFAULT_FAR_CLIP;

	private:
		bool InitializeXrRuntime();
		bool FinalizeXrRuntime();

		bool StartXrRuntime();
		bool StopXrRuntime();

		// Whether or not stereo rendering is allowed
		bool IsStereoRenderingAllowed(FSceneViewport** OutSceneViewport);

		bool IsProjectPlayModeSupported();
		bool IsProjectGraphicsAPISupported();
		FString GetCurrentGraphicsAPI() const;

		// Get projection matrix and clipping distance, and calculate stereo projection matrix.
		void UpdateProjectionMatrix();
		FMatrix GetSpatialClippedProjectionMatrix(FVector ClipPlanePositionInWorldCoord, FVector ClipPlaneNormalVecInWorldCoord, FMatrix ViewMatrix, FMatrix ProjectionMatrix);
		FMatrix GetNearClipPlaneConvertedProjectionMatrix(FMatrix SrcProjMat, FVector4 PlaneInViewSpace);

		FMatrix GetDefaultProjectionMatrix() const;

		void CreateWindowOnXRDisplay();
		void DestroyWindowOnXRDisplay();

		void CompositeStereoImages(FRHICommandListImmediate& RHICmdList, FRHITexture2D* BackBuffer, FRHITexture2D* SrcSideBySide, uint32 SourceWidth, uint32 SourceHeight, DXGI_FORMAT Format) const;

		void ConvertPoseFromXrRuntimeToGameWorld(SonyOzPosef SrcPose, FQuat& DstOrientation, FVector& DstPosition) const;

		void ShowXrSystemErrorDialog();
		bool HandleXRRuntimeResult(SonyOzResult Result);

	private:
		static IXRDisplayInterface* XRDisplay;

		FRenderTargetManager* RenderTargetManager = nullptr;

		// For application life time management.
		bool bQuitApplication = false;

		// For XR Runtime management.
		bool bIsSessionBegun = false;
		bool bStereoEnabled = false;

		// SR Display's specs.
		SonyOzDisplaySpec DisplaySpec;

		// View projection matrix used in game thread.
		FMatrix LeftViewProjectionMatrix;
		FMatrix RightViewProjectionMatrix;

		// Critical sections.
		FCriticalSection CSGetCurrentPose;

		//using FXrRuntimeWrapper = sony::oz::xr_runtime::unrealengine::XrRuntimeUnrealEngineWrapper;
		TUniquePtr<FXrRuntimeWrapper> XrRuntimeWrapper;

		IRendererModule* RendererModule;

		FDisplayCorners DisplayCorners;

		FMatrix LeftViewProjectionMatrix_RenderingThread;
		FMatrix RightViewProjectionMatrix_RenderingThread;
	}; // class FARHeadMountedDisplay

} // namespace xr_display
