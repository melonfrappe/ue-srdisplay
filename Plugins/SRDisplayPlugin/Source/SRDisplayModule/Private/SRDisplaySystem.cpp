/*
 * Copyright 2019,2020,2021 Sony Corporation
 */

#include "SRDisplaySystem.h"
#include "XRDisplaySystem.h"

#include "GlobalShader.h"
#include "CommonRenderResources.h"

#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 2
#include "DataDrivenShaderPlatformInfo.h"
#endif

#include "ClearQuad.h"
#include "SceneRendering.h"
#include "ScreenRendering.h"
#include "PostProcess/PostProcessHMD.h"

#include <Slate/SceneViewport.h>
#include <Modules/ModuleManager.h>

#include <SRDisplayModule/Classes/Blueprint/SRDisplayProjectSettings.h>

#include "xr_basic_api_wrapper.h"

#if PLATFORM_WINDOWS
#include <Windows/AllowWindowsPlatformTypes.h>
#include <Windows/HideWindowsPlatformTypes.h>
#endif


DEFINE_LOG_CATEGORY(LogSRDisplay);

class FSRDisplayHomographyTransformPS : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FSRDisplayHomographyTransformPS, Global);

public:
	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
	}

	FSRDisplayHomographyTransformPS() { }

	FSRDisplayHomographyTransformPS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FGlobalShader(Initializer)
	{
		// Bind shader inputs.
		InTexture.Bind(Initializer.ParameterMap, TEXT("InTexture"));
		InTextureSampler.Bind(Initializer.ParameterMap, TEXT("InTextureSampler"));
		InLeftHomographyMatrix.Bind(Initializer.ParameterMap, TEXT("InLeftHomographyMatrix"));
		InRightHomographyMatrix.Bind(Initializer.ParameterMap, TEXT("InRightHomographyMatrix"));
	}

	template<typename TShaderRHIParamRef>
	void SetParameters(FRHICommandList& RHICmdList, const TShaderRHIParamRef ShaderRHI, FRHITexture* Texture, FMatrix LeftHomographyMatrix, FMatrix RightHomographyMatrix)
	{
		FRHISamplerState* SamplerStateRHI = TStaticSamplerState<SF_Point>::GetRHI();

		SetTextureParameter(RHICmdList, ShaderRHI, InTexture, InTextureSampler, SamplerStateRHI, Texture);
#if ENGINE_MAJOR_VERSION == 4
		SetShaderValue(RHICmdList, ShaderRHI, InLeftHomographyMatrix, LeftHomographyMatrix);
		SetShaderValue(RHICmdList, ShaderRHI, InRightHomographyMatrix, RightHomographyMatrix);
#else
		SetShaderValue(RHICmdList, ShaderRHI, InLeftHomographyMatrix, (FMatrix44f)LeftHomographyMatrix);
		SetShaderValue(RHICmdList, ShaderRHI, InRightHomographyMatrix, (FMatrix44f)RightHomographyMatrix);
#endif
	}

private:
	// Shader parameters.
	LAYOUT_FIELD(FShaderResourceParameter, InTexture);
	LAYOUT_FIELD(FShaderResourceParameter, InTextureSampler);
	LAYOUT_FIELD(FShaderParameter, InLeftHomographyMatrix);
	LAYOUT_FIELD(FShaderParameter, InRightHomographyMatrix);
};

IMPLEMENT_SHADER_TYPE(, FSRDisplayHomographyTransformPS, TEXT("/Plugin/SRDisplayPlugin/Private/HomographyTransform.usf"), TEXT("MainPS"), SF_Pixel)

class FSRDisplayLowPassFilterPS : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FSRDisplayLowPassFilterPS, Global);

public:
	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
	}

	FSRDisplayLowPassFilterPS() {}

	FSRDisplayLowPassFilterPS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FGlobalShader(Initializer)
	{
		// Bind shader inputs.
		InTexture.Bind(Initializer.ParameterMap, TEXT("InTexture"));
		InTextureSampler.Bind(Initializer.ParameterMap, TEXT("InTextureSampler"));
		Is9tap.Bind(Initializer.ParameterMap, TEXT("Is9tap"));
	}

	template<typename TShaderRHIParamRef>
	void SetParameters(FRHICommandList& RHICmdList, const TShaderRHIParamRef ShaderRHI, FRHITexture* Texture, bool is9tap)
	{
		FRHISamplerState* SamplerStateRHI = TStaticSamplerState<SF_Point>::GetRHI();

		SetTextureParameter(RHICmdList, ShaderRHI, InTexture, InTextureSampler, SamplerStateRHI, Texture);
		if (is9tap)
		{
			SetShaderValue(RHICmdList, ShaderRHI, Is9tap, 1);
		}
		else
		{
			SetShaderValue(RHICmdList, ShaderRHI, Is9tap, 0);
		}
	}

private:
	// Shader parameters.
	LAYOUT_FIELD(FShaderResourceParameter, InTexture);
	LAYOUT_FIELD(FShaderResourceParameter, InTextureSampler);
	LAYOUT_FIELD(FShaderParameter, Is9tap);
};

IMPLEMENT_SHADER_TYPE(, FSRDisplayLowPassFilterPS, TEXT("/Plugin/SRDisplayPlugin/Private/LowPassFilter.usf"), TEXT("MainPS"), SF_Pixel)


namespace srdisplay_module
{
	const float FSRDisplaySystem::DEFAULT_SRD_VIEW_SPACE_SCALE = 1.f;
	const float FSRDisplaySystem::DEFAULT_SRD_FAR_CLIP = 100000000.f;
	const float FSRDisplaySystem::METER_TO_CENTIMETER = 100.f;

	FTexture2DRHIRef TmpRenderTexture = nullptr;

	namespace {
		FVector2D ConvertWorldPositionToScreen(const FVector4& WorldPosition, const FMatrix& ViewProjectionMatrix)
		{
			FPlane Result = ViewProjectionMatrix.TransformFVector4(WorldPosition);

			// the result of this will be x and y coords in -1..1 projection space
			const float RHW = 1.f / Result.W;
			FPlane PosInScreenSpace = FPlane(Result.X * RHW, Result.Y * RHW, Result.Z * RHW, Result.W);

			// Move from projection space to normalized 0..1 UI space
			const float NormalizedX = (PosInScreenSpace.X + 1.f) * 0.5f;
			const float NormalizedY = 1.f - ((PosInScreenSpace.Y + 1.f) * 0.5f);

			return FVector2D(NormalizedX, NormalizedY);
		}

		void CalculateHomographyMatrix(FVector2D InLeftBottomPosition, FVector2D InLeftTopPosition, FVector2D InRightBottomPosition, FVector2D InRightTopPosition, FMatrix& OutMatrix)
		{
			FVector2D LeftBottomPosition = InLeftBottomPosition;
			FVector2D LeftTopPosition = InLeftTopPosition;
			FVector2D RightBottomPosition = InRightBottomPosition;
			FVector2D RightTopPosition = InRightTopPosition;

			// Conversion factor
			float A = RightTopPosition.X - RightBottomPosition.X;
			float B = LeftBottomPosition.X - RightBottomPosition.X;
			float C = LeftTopPosition.X - LeftBottomPosition.X - RightTopPosition.X + RightBottomPosition.X;
			float D = RightTopPosition.Y - RightBottomPosition.Y;
			float E = LeftBottomPosition.Y - RightBottomPosition.Y;
			float F = LeftTopPosition.Y - LeftBottomPosition.Y - RightTopPosition.Y + RightBottomPosition.Y;

			float Out02 = LeftTopPosition.X;
			float Out12 = LeftTopPosition.Y;
			float Out21 = (C * D - A * F) / (B * D - A * E);
			float Out20 = (C * E - B * F) / (A * E - B * D);
			float Out00 = RightTopPosition.X - LeftTopPosition.X + Out20 * RightTopPosition.X;
			float Out01 = LeftBottomPosition.X - LeftTopPosition.X + Out21 * LeftBottomPosition.X;
			float Out10 = RightTopPosition.Y - LeftTopPosition.Y + Out20 * RightTopPosition.Y;
			float Out11 = LeftBottomPosition.Y - LeftTopPosition.Y + Out21 * LeftBottomPosition.Y;

			OutMatrix = FMatrix(
				FPlane(Out00, Out01, Out02, 0.f),
				FPlane(Out10, Out11, Out12, 0.f),
				FPlane(Out20, Out21, 1.f, 0.f),
				FPlane(0.f, 0.f, 0.f, 0.f)
			);
		}

		FMatrix CalculateHomographyMatrixFromViewProjectionMatrix(
			const xr_display::FDisplayCorners& DisplayCorners,
			const FMatrix& ViewProjectionMatrix) {
			// 4 corners of LFB(Screen Position)[range: 0.0 ~ 1.0]
			// Calculate screen position
			const FVector2D LeftBottomPositionInScreen = ConvertWorldPositionToScreen(
				DisplayCorners.LeftBottom, ViewProjectionMatrix);
			const FVector2D LeftTopPositionInScreen = ConvertWorldPositionToScreen(
				DisplayCorners.LeftTop, ViewProjectionMatrix);
			const FVector2D RightBottomPositionInScreen = ConvertWorldPositionToScreen(
				DisplayCorners.RightBottom, ViewProjectionMatrix);
			const FVector2D RightTopPositionInScreen = ConvertWorldPositionToScreen(
				DisplayCorners.RightTop, ViewProjectionMatrix);

			FMatrix Result;
			CalculateHomographyMatrix(LeftBottomPositionInScreen, LeftTopPositionInScreen,
				RightBottomPositionInScreen,
				RightTopPositionInScreen, Result);
			return Result;
		}
	}

	FSRDisplaySystem::FSRDisplaySystem()
	{
		xr_display::FXRDisplaySystem::SetXRDisplay(this);

		static const FName RendererModuleName("Renderer");
		RendererModule = FModuleManager::GetModulePtr<IRendererModule>(RendererModuleName);
	}

	FSRDisplaySystem::~FSRDisplaySystem()
	{
	}

	FString FSRDisplaySystem::GetVersionString() const
	{
		FString Version =
			FString::Printf(TEXT("SRDisplay - %s, built %s, %s"),
				*FEngineVersion::Current().ToString(),
				UTF8_TO_TCHAR(__DATE__), UTF8_TO_TCHAR(__TIME__));
		return Version;
	}

	void FSRDisplaySystem::OnBeginPlay(FWorldContext& InWorldContext)
	{
		// initialize parameter
		SRDisplayManager = nullptr;
		SRDisplayViewSpaceScale = DEFAULT_SRD_VIEW_SPACE_SCALE;
		preShowCameraWindow = false;
		preIsSRRenderingActive = true;

		xr_display::FXRDisplaySystem* XRDisplaySystem = static_cast<xr_display::FXRDisplaySystem*>(GEngine->XRSystem.Get());
		if (XRDisplaySystem) {
			XRDisplaySystem->isWithoutXRDisplayMode = GetDefault<USRDisplayProjectSettings>()->RunWithoutSRDisplayMode;
		}

		UpdateSRDisplayPosition();
	}

	void FSRDisplaySystem::OnEndPlay(FWorldContext& InWorldContext)
	{
		SRDisplayManager = nullptr;

		if (TmpRenderTexture != nullptr)
		{
			TmpRenderTexture.SafeRelease();
		}
	}

	bool FSRDisplaySystem::OnStartGameFrame(FWorldContext& WorldContext)
	{
		UpdateSRDisplayManagerState();
		UpdateSRDisplayPosition();

		return true;
	}

	void FSRDisplaySystem::OnBeginRendering_RenderThread(FRHICommandListImmediate& RHICmdList, FSceneViewFamily& ViewFamily)
	{
		for (const FSceneView* SceneView : ViewFamily.Views)
		{
#if ENGINE_MAJOR_VERSION == 4
			if (SceneView->StereoPass == eSSP_LEFT_EYE)
			{
				LeftViewProjectionMatrix_RenderingThread = SceneView->ViewMatrices.GetViewProjectionMatrix();
			}
			else if (SceneView->StereoPass == eSSP_RIGHT_EYE)
			{
				RightViewProjectionMatrix_RenderingThread = SceneView->ViewMatrices.GetViewProjectionMatrix();
			}
#else
			if (SceneView->StereoViewIndex == eSSE_LEFT_EYE) {
				LeftViewProjectionMatrix_RenderingThread = SceneView->ViewMatrices.GetViewProjectionMatrix();
			}
			else if (SceneView->StereoViewIndex == eSSE_RIGHT_EYE) {
				RightViewProjectionMatrix_RenderingThread = SceneView->ViewMatrices.GetViewProjectionMatrix();
			}
#endif
		}
	}

	void FSRDisplaySystem::RenderTexture_RenderThread(FRHICommandListImmediate& RHICmdList, FRHITexture2D* BackBuffer, FRHITexture2D* SrcTexture, FVector2D WindowSize, bool skipHomography) const
	{
		if (!skipHomography) {
			if (TmpRenderTexture == nullptr)
			{
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 3
				const FRHITextureCreateDesc StagingDesc =
					FRHITextureCreateDesc::Create2D(TEXT("DisplayProjectorTempTexture"), BackBuffer->GetSizeX(), BackBuffer->GetSizeY(), BackBuffer->GetFormat())
					.SetFlags(TexCreate_None | TexCreate_ShaderResource | TexCreate_RenderTargetable);
				TmpRenderTexture = RHICreateTexture(StagingDesc);
#else

#if ENGINE_MAJOR_VERSION == 5
				FRHIResourceCreateInfo TmpInfo(TEXT("DisplayProjectorTempTexture"));
#else // ENGINE_MAJOR_VERSION == 5
				FRHIResourceCreateInfo TmpInfo;
#endif // ENGINE_MAJOR_VERSION == 5
				TmpRenderTexture = RHICreateTexture2D(
					BackBuffer->GetSizeX(), BackBuffer->GetSizeY(), BackBuffer->GetFormat(), 1, 1,
					TexCreate_None | TexCreate_ShaderResource | TexCreate_RenderTargetable, TmpInfo);

#endif
			}
			HomographyTransform(RHICmdList, TmpRenderTexture, SrcTexture, WindowSize);
		}
		if (skipHomography)
		{
			LowPassFilter(RHICmdList, BackBuffer, SrcTexture, WindowSize);
		}
		else
		{
			LowPassFilter(RHICmdList, BackBuffer, TmpRenderTexture, WindowSize);
		}
		
	}

	void FSRDisplaySystem::HomographyTransform(FRHICommandListImmediate& RHICmdList, FRHITexture2D* BackBuffer, FRHITexture2D* SrcTexture, FVector2D WindowSize) const
	{
		FRHIRenderPassInfo RPInfoTempRight(BackBuffer, ERenderTargetActions::Load_Store);
		RHICmdList.BeginRenderPass(RPInfoTempRight, TEXT("FSRDisplaySystem_ProcessHomographyTransform"));
		{
			const FVector2D ViewportSize = FVector2D(WindowSize.X, WindowSize.Y);

			// Calculate homography matrix
			const FMatrix LeftHomographyMatrix = CalculateHomographyMatrixFromViewProjectionMatrix(DisplayCorners, LeftViewProjectionMatrix_RenderingThread);
			const FMatrix RightHomographyMatrix = CalculateHomographyMatrixFromViewProjectionMatrix(DisplayCorners, RightViewProjectionMatrix_RenderingThread);

			DrawClearQuad(RHICmdList, FLinearColor(0.0f, 0.0f, 0.0f, 1.0f));
			RHICmdList.SetViewport(0, 0, 0.0f, ViewportSize.X, ViewportSize.Y, 1.0f);

			FGraphicsPipelineStateInitializer GraphicsPSOInit;
			RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);

			const auto FeatureLevel = GMaxRHIFeatureLevel;
			FGlobalShaderMap* ShaderMap = GetGlobalShaderMap(FeatureLevel);
			TShaderMapRef<FScreenVS> VertexShader(ShaderMap);
			TShaderMapRef<FSRDisplayHomographyTransformPS> PixelShader(ShaderMap);

			GraphicsPSOInit.BlendState = TStaticBlendState<>::GetRHI();
			GraphicsPSOInit.RasterizerState = TStaticRasterizerState<>::GetRHI();
			GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, CF_Always>::GetRHI();
			GraphicsPSOInit.PrimitiveType = PT_TriangleList;
			GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GFilterVertexDeclaration.VertexDeclarationRHI;
			GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
			GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();
			SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit);

			PixelShader->SetParameters(RHICmdList, PixelShader.GetPixelShader(), SrcTexture, LeftHomographyMatrix, RightHomographyMatrix);

			RendererModule->DrawRectangle(
				RHICmdList,
				0.f, 0.f,
				ViewportSize.X, ViewportSize.Y,
				0.f, 0.f,
				1.f, 1.f,
				FIntPoint(ViewportSize.X, ViewportSize.Y),
				FIntPoint(1, 1),
				VertexShader,
				EDRF_Default);
		}
		RHICmdList.EndRenderPass();
	}

	void FSRDisplaySystem::LowPassFilter(FRHICommandListImmediate& RHICmdList, FRHITexture2D* BackBuffer, FRHITexture2D* SrcTexture, FVector2D WindowSize) const
	{
		FRHIRenderPassInfo RPInfoTempRight(BackBuffer, ERenderTargetActions::Load_Store);
		RHICmdList.BeginRenderPass(RPInfoTempRight, TEXT("FSRDisplaySystem_ProcessLowPassFilter"));
		{
			const FVector2D ViewportSize = FVector2D(WindowSize.X, WindowSize.Y);

			DrawClearQuad(RHICmdList, FLinearColor(0.0f, 0.0f, 0.0f, 1.0f));
			RHICmdList.SetViewport(0, 0, 0.0f, ViewportSize.X, ViewportSize.Y, 1.0f);

			FGraphicsPipelineStateInitializer GraphicsPSOInit;
			RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);

			const auto FeatureLevel = GMaxRHIFeatureLevel;
			FGlobalShaderMap* ShaderMap = GetGlobalShaderMap(FeatureLevel);
			TShaderMapRef<FScreenVS> VertexShader(ShaderMap);
			TShaderMapRef<FSRDisplayLowPassFilterPS> PixelShader(ShaderMap);

			GraphicsPSOInit.BlendState = TStaticBlendState<>::GetRHI();
			GraphicsPSOInit.RasterizerState = TStaticRasterizerState<>::GetRHI();
			GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, CF_Always>::GetRHI();
			GraphicsPSOInit.PrimitiveType = PT_TriangleList;
			GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GFilterVertexDeclaration.VertexDeclarationRHI;
			GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
			GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();
			SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit);

			constexpr int SideBySide4KWidth = 7680;
			if(ViewportSize.X == SideBySide4KWidth)
			{
				PixelShader->SetParameters(RHICmdList, PixelShader.GetPixelShader(), SrcTexture, true);
			}
			else
			{
				PixelShader->SetParameters(RHICmdList, PixelShader.GetPixelShader(), SrcTexture, false);
			}

			RendererModule->DrawRectangle(
				RHICmdList,
				0.f, 0.f,
				ViewportSize.X, ViewportSize.Y,
				0.f, 0.f,
				1.f, 1.f,
				FIntPoint(ViewportSize.X, ViewportSize.Y),
				FIntPoint(1, 1),
				VertexShader,
				EDRF_Default);
		}
		RHICmdList.EndRenderPass();
	}

	const char* FSRDisplaySystem::GetPlatformId()
	{
		return "Spatial Reality Display";
	}

	uint32_t FSRDisplaySystem::GetDeviceIndex(const SonyOzDeviceInfo* DeviceList, const uint64_t Size)
	{
		uint64_t DeviceIndex = 0;
		if (GetDefault<USRDisplayProjectSettings>()->RunWithoutSRDisplayMode) {
			for (int i = 0; i < Size; i++) {
				SonyOzDeviceInfo DeviceInfo = DeviceList[i];
				if (strcmp(DeviceInfo.product_id, "Deviceless") == 0) {
					DeviceIndex = i;
				}
			}
		}
		else {
			auto ItemList = MakeUnique<const wchar_t* []>(Size);
			int ItemCount = 0;
			for (int i = 0; i < Size; i++) {
				SonyOzDeviceInfo DeviceInfo = DeviceList[i];
				if (strcmp(DeviceInfo.product_id, "Deviceless") == 0) {
					continue;
				}
				const size_t ItemStrSize = sizeof(DeviceInfo.product_id) + sizeof(DeviceInfo.device_serial_number) + 1;
				char Str[ItemStrSize] = "";
				strcat_s(Str, ItemStrSize, DeviceInfo.product_id);
				strcat_s(Str, ItemStrSize, " ");
				strcat_s(Str, ItemStrSize, DeviceInfo.device_serial_number);
				wchar_t* ItemStr = new wchar_t[ItemStrSize];
				size_t ret = 0;
				mbstowcs_s(&ret, ItemStr, ItemStrSize, Str, _TRUNCATE);
				ItemList.Get()[i] = ItemStr;
				ItemCount++;
			}
			if (ItemCount > 1) {
				DeviceIndex = sony::oz::xr_runtime::ShowComboBoxDialog(GetPlatformId(), NULL, ItemList.Get(), Size);
			}
		}
		if (strcmp(DeviceList[DeviceIndex].product_id, "ELF-SR2") == 0) {
			ConnectedDevice = sony::oz::xr_runtime::SupportDevice::ELF_SR2;
		}
		else {
			ConnectedDevice = sony::oz::xr_runtime::SupportDevice::ELF_SR1;
		}

		return DeviceIndex;
	}

	void FSRDisplaySystem::ConvertPoseFromXrRuntimeToGameWorld(FTransform Transform, FQuat& DstOrientation, FVector& DstPosition) const
	{
		DstOrientation = SRDisplayTransform.TransformRotation(Transform.GetRotation());
		DstPosition = SRDisplayTransform.GetRotation().RotateVector(Transform.GetLocation() * SRDisplayViewSpaceScale) + SRDisplayTransform.GetLocation();
	}

	void FSRDisplaySystem::UpdateProjectionMatrix(FMatrix& LeftViewProjectionMatrix, FMatrix& RightViewProjectionMatrix)
	{
		FMatrix LeftViewMatrix;
		FMatrix RightViewMatrix;
		if (SpatialClipping && GetViewMatrix(LeftViewMatrix, RightViewMatrix))
		{
			FVector4 LeftBottomPosition =
				FVector4(0.f, -1.f * DisplaySpec.display_size.width_m * METER_TO_CENTIMETER / 2.f, 0.f, 1.f);
			LeftBottomPosition = SRDisplayTransform.GetRotation().RotateVector(LeftBottomPosition) + FVector4(SRDisplayTransform.GetLocation(), 0.f);

			static const FVector4 ClipPlaneOffset(5.f, 0.f, 0.f, 1.f);
			FVector ClipPlanePositionInWorldCoord = LeftBottomPosition - ClipPlaneOffset;
			FVector ClipPlaneNormalVecInWorldCoord = SRDisplayTransform.GetRotation().RotateVector(FVector(1, 0, 0));

			LeftViewProjectionMatrix = GetSpatialClippedProjectionMatrix(ClipPlanePositionInWorldCoord, ClipPlaneNormalVecInWorldCoord, LeftViewMatrix, LeftViewProjectionMatrix);
			RightViewProjectionMatrix = GetSpatialClippedProjectionMatrix(ClipPlanePositionInWorldCoord, ClipPlaneNormalVecInWorldCoord, RightViewMatrix, RightViewProjectionMatrix);
		}
	}

	void FSRDisplaySystem::SetDisplaySpec(xr_display::XrDisplaySpec InDisplaySpec)
	{
		DisplaySpec = InDisplaySpec;
		DisplayResolution = FVector2D(InDisplaySpec.display_resolution.width, InDisplaySpec.display_resolution.height);
	}

	bool FSRDisplaySystem::DeprojectScreenToWorld(FSceneViewProjectionData ProjectionData, const FVector2D& ScreenPosition, FVector& WorldPosition, FVector& WorldDirection, FVector& CameraPosition)
	{
		// Calculate WorldPosition
		FVector4 LeftBottomPosition;
		FVector4 LeftTopPosition;
		FVector4 RightBottomPosition;
		FVector4 RightTopPosition;
		if (!GetDisplayCornersPosition(LeftBottomPosition, LeftTopPosition, RightBottomPosition, RightTopPosition))
		{
			WorldPosition = FVector::ZeroVector;
			WorldDirection = FVector::ZeroVector;
			return false;
		}

		FVector2D screenRate = FVector2D(ScreenPosition.X / DisplayResolution.X, ScreenPosition.Y / DisplayResolution.Y);
		if (GetDefault<USRDisplayProjectSettings>()->RunWithoutSRDisplayMode)
		{
			screenRate = FVector2D(ScreenPosition.X / (ProjectionData.GetViewRect().Max.X * 2), 
														 ScreenPosition.Y / ProjectionData.GetViewRect().Max.Y);
		}
		FVector4 yDirection = RightTopPosition - LeftTopPosition;
		FVector4 zDirection = LeftBottomPosition - LeftTopPosition;
		WorldPosition = LeftTopPosition + yDirection * screenRate.X + zDirection * screenRate.Y;

		// Calculate WorldDirection
		WorldDirection = (WorldPosition - ProjectionData.ViewOrigin).GetSafeNormal();

		// Calculate CameraPosition
		CameraPosition = ProjectionData.ViewOrigin;

		return true;
	}

	float FSRDisplaySystem::GetNearClip()
	{
		const float DEFAULT_NEAR_Z = 0.15f;
		return DEFAULT_NEAR_Z * SRDisplayViewSpaceScale;
	}

	float FSRDisplaySystem::GetFarClip()
	{
		const float CENTIMETER_TO_METER = 0.01f;
		return SRDisplayFarClip * SRDisplayViewSpaceScale * CENTIMETER_TO_METER;
	}

	SonyOzPosef FSRDisplaySystem::GetStubHeadPose()
	{
		SonyOzPosef pose;
		sony::oz::xr_runtime::GetStubHeadPose(SessionHandle, &pose);
		return pose;
	}

	void FSRDisplaySystem::SetStubHeadPose(const SonyOzPosef pose)
	{
		sony::oz::xr_runtime::SetStubHeadPose(SessionHandle, pose);
	}

	xr_display::FDisplayCorners FSRDisplaySystem::GetDisplayCornners()
	{
		return DisplayCorners;
	}

	ASRDisplayManager* FSRDisplaySystem::GetSRDisplayManagerActor()
	{
		TArray<AActor*> FoundActors;
		UGameplayStatics::GetAllActorsOfClass(GWorld->GetWorld(), ASRDisplayManager::StaticClass(), FoundActors);
		if (FoundActors.Num() <= 0)
		{
			return nullptr;
		}

		if (FoundActors.Num() > 1)
		{
			UE_LOG(LogSRDisplay, Warning, TEXT("There are more than two SRDisplayManager placed on the level. Only one of them is available."));
		}

		ASRDisplayManager* Display = Cast<ASRDisplayManager>(FoundActors[FoundActors.Num() - 1]);
		if (!Display->HasActorBegunPlay())
		{
			return nullptr;
		}

		return Display;
	}

	void FSRDisplaySystem::UpdateSRDisplayManagerState()
	{
		static const float FAR_CLIP_MIN = 1.f;

		SRDisplayManager = GetSRDisplayManagerActor();
		if (SRDisplayManager)
		{
			SRDisplayViewSpaceScale = SRDisplayManager->GetRealToVirtualScale() / SRDisplayManager->GetPanelSizeScale(ConnectedDevice);
			SRDisplayTransform = SRDisplayManager->GetActorTransform();
			SpatialClipping = SRDisplayManager->SpatialClipping;
			SRDisplayFarClip = SRDisplayManager->FarClip;
			if (SRDisplayFarClip < FAR_CLIP_MIN)
			{
				SRDisplayFarClip = FAR_CLIP_MIN;
			}
			if (SRDisplayManager->ShowCameraWindow != preShowCameraWindow)
			{
				preShowCameraWindow = SRDisplayManager->ShowCameraWindow;
				sony::oz::xr_runtime::SetCameraWindowEnabled(GetPlatformId(), SessionHandle, preShowCameraWindow);
			}
			if (SRDisplayManager->IsSRRenderingActive != preIsSRRenderingActive)
			{
				preIsSRRenderingActive = SRDisplayManager->IsSRRenderingActive;
				xr_display::FXRDisplaySystem* XRDisplaySystem = static_cast<xr_display::FXRDisplaySystem*>(GEngine->XRSystem.Get());
				if (XRDisplaySystem) {
					XRDisplaySystem->SetSRRenderingActive(preIsSRRenderingActive);
				}
			}

			bool enableCrosstalkCorrection = SRDisplayManager->CrosstalkCorrection;
			sony::oz::srd_base_settings::SrdXrCrosstalkCorrectionMode mode;
			if (enableCrosstalkCorrection) {
				switch (SRDisplayManager->CorrectionType) {
				case ECrosstalkCorrectionType::GRADATION_CORRECTION_MEDIUM:
					mode = sony::oz::srd_base_settings::SrdXrCrosstalkCorrectionMode::GRADATION_CORRECTION_MEDIUM;
					break;
				case ECrosstalkCorrectionType::GRADATION_CORRECTION_ALL:
					mode = sony::oz::srd_base_settings::SrdXrCrosstalkCorrectionMode::GRADATION_CORRECTION_ALL;
					break;
				case ECrosstalkCorrectionType::GRADATION_CORRECTION_HIGH_PRECISE:
					mode = sony::oz::srd_base_settings::SrdXrCrosstalkCorrectionMode::GRADATION_CORRECTION_HIGH_PRECISE;
					break;
				default:
					mode = sony::oz::srd_base_settings::SrdXrCrosstalkCorrectionMode::GRADATION_CORRECTION_MEDIUM;
					break;
				}
			}
			else {
				mode = sony::oz::srd_base_settings::SrdXrCrosstalkCorrectionMode::DISABLED;
			}
			sony::oz::xr_runtime::SetCrosstalkCorrectionSettings(GetPlatformId(), SessionHandle, mode);
		}
	}

	void FSRDisplaySystem::UpdateSRDisplayPosition()
	{
		GetDisplayCornersPosition(DisplayCorners.LeftBottom, DisplayCorners.LeftTop, DisplayCorners.RightBottom, DisplayCorners.RightTop);
	}

	bool FSRDisplaySystem::GetViewMatrix(FMatrix& LeftViewMatrix, FMatrix& RightViewMatrix)
	{
		if (GEngine && GEngine->GameViewport && GEngine->GameViewport->GetWorld())
		{
			APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GEngine->GameViewport->GetWorld(), 0);
			ULocalPlayer* const LocalPlayer = PlayerController ? PlayerController->GetLocalPlayer() : nullptr;
			//if (LocalPlayer && FSRDisplaySystemWindow::GetPlayWindow() && FSRDisplaySystemWindow::GetPlayWindow()->GetViewport())
			//{
			//	FSceneViewProjectionData LeftProjectionData;
			//	FSceneViewProjectionData RightProjectionData;
			//	if (LocalPlayer->GetProjectionData(FSRDisplaySystemWindow::GetPlayWindow()->GetViewport(), eSSP_LEFT_EYE, /*out*/ LeftProjectionData)
			//		&& LocalPlayer->GetProjectionData(FSRDisplaySystemWindow::GetPlayWindow()->GetViewport(), eSSP_RIGHT_EYE, /*out*/ RightProjectionData))
			//	{
			//		LeftViewMatrix = FTranslationMatrix(-LeftProjectionData.ViewOrigin) * LeftProjectionData.ViewRotationMatrix;
			//		RightViewMatrix = FTranslationMatrix(-RightProjectionData.ViewOrigin) * RightProjectionData.ViewRotationMatrix;

			//		return true;
			//	}
			//}
		}
		xr_display::FXRDisplaySystem* XRDisplaySystem = static_cast<xr_display::FXRDisplaySystem*>(GEngine->XRSystem.Get());
		return XRDisplaySystem->GetViewMatrix(LeftViewMatrix, RightViewMatrix);
	}

	FMatrix FSRDisplaySystem::GetSpatialClippedProjectionMatrix(FVector ClipPlanePositionInWorldCoord, FVector ClipPlaneNormalVecInWorldCoord, FMatrix ViewMatrix, FMatrix ProjectionMatrix)
	{
		// Caluculate clipping plane(view)
		FVector NormalInCameraCoord = ViewMatrix.TransformVector(ClipPlaneNormalVecInWorldCoord);
		NormalInCameraCoord.Normalize();
		FVector PosInCameraCoord = ViewMatrix.TransformPosition(ClipPlanePositionInWorldCoord);
		FVector4 ClipPlane = FVector4(NormalInCameraCoord.X, NormalInCameraCoord.Y, NormalInCameraCoord.Z,
			-FVector::DotProduct(PosInCameraCoord, NormalInCameraCoord));

		return GetNearClipPlaneConvertedProjectionMatrix(ProjectionMatrix, ClipPlane);
	}

	FMatrix FSRDisplaySystem::GetNearClipPlaneConvertedProjectionMatrix(FMatrix SrcProjMat, FVector4 PlaneInViewSpace)
	{
		auto sgn = [](float a) {
			if (a > 0.f) { return (1.f); }
			if (a < 0.f) { return (-1.f); }
			return (0.f);
		};

		FMatrix ZReverseMatrix;
		ZReverseMatrix.SetIdentity();
		ZReverseMatrix.M[2][2] = -1.f;
		ZReverseMatrix.M[3][2] = 1.f;

		FMatrix Test = ZReverseMatrix * ZReverseMatrix;
		FMatrix UnZReversedProjMat = SrcProjMat * ZReverseMatrix;

		FVector4 CornerPlane(
			(sgn(PlaneInViewSpace.X) - UnZReversedProjMat.M[2][0]) / UnZReversedProjMat.M[0][0],
			(sgn(PlaneInViewSpace.Y) - UnZReversedProjMat.M[2][1]) / UnZReversedProjMat.M[1][1],
			1.f,
			(1.f - UnZReversedProjMat.M[2][2]) / UnZReversedProjMat.M[3][2]
		);

		FVector4 ProjPlane(PlaneInViewSpace * (1.f / Dot4(PlaneInViewSpace, CornerPlane)));

		UnZReversedProjMat.M[0][2] = ProjPlane.X;
		UnZReversedProjMat.M[1][2] = ProjPlane.Y;
		UnZReversedProjMat.M[2][2] = ProjPlane.Z;
		UnZReversedProjMat.M[3][2] = ProjPlane.W;

		return UnZReversedProjMat * ZReverseMatrix;
	}

	bool FSRDisplaySystem::GetDisplayCornersPosition(FVector4& LeftBottomPosition, FVector4& LeftTopPosition, FVector4& RightBottomPosition, FVector4& RightTopPosition) const
	{
		float DisplayWidth = DisplaySpec.display_size.width_m * METER_TO_CENTIMETER;
		float DisplayHeight = DisplaySpec.display_size.height_m * METER_TO_CENTIMETER;

		DisplayWidth *= SRDisplayViewSpaceScale;
		DisplayHeight *= SRDisplayViewSpaceScale;

		const float LFBDepthByTilt = DisplayHeight * FMath::Sin(DisplaySpec.display_tilt_rad);
		const float LFBHeightByTilt = DisplayHeight * FMath::Cos(DisplaySpec.display_tilt_rad);

		LeftBottomPosition = FVector4(0.f, -DisplayWidth / 2.f, 0.f, 1.f);
		LeftBottomPosition = SRDisplayTransform.GetRotation().RotateVector(LeftBottomPosition) + FVector4(SRDisplayTransform.GetLocation(), 0.f);

		LeftTopPosition = FVector4(LFBDepthByTilt, -DisplayWidth / 2.f, LFBHeightByTilt, 1.f);
		LeftTopPosition = SRDisplayTransform.GetRotation().RotateVector(LeftTopPosition) + FVector4(SRDisplayTransform.GetLocation(), 0.f);

		RightBottomPosition = FVector4(0.f, DisplayWidth / 2.f, 0.f, 1.f);
		RightBottomPosition = SRDisplayTransform.GetRotation().RotateVector(RightBottomPosition) + FVector4(SRDisplayTransform.GetLocation(), 0.f);

		RightTopPosition = FVector4(LFBDepthByTilt, DisplayWidth / 2.f, LFBHeightByTilt, 1.f);
		RightTopPosition = SRDisplayTransform.GetRotation().RotateVector(RightTopPosition) + FVector4(SRDisplayTransform.GetLocation(), 0.f);

		return true;
	}
} // namespace srdisplay_module
