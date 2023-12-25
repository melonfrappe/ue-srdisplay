/*
 * Copyright 2019,2020,2021,2022 Sony Corporation
 */
#include "Blueprint/SRDisplayDrawBoxComponent.h"
#include "XRDisplaySystem.h"

#include <Engine/CollisionProfile.h>
#include <PrimitiveSceneProxy.h>

#include "misc_util_wrapper.h"

/** Represents a draw frustum to the scene manager. */
class FSRDisplayDrawBoxSceneProxy final : public FPrimitiveSceneProxy
{
public:
	SIZE_T GetTypeHash() const override
	{
		static size_t UniquePointer;
		return reinterpret_cast<size_t>(&UniquePointer);
	}

	FSRDisplayDrawBoxSceneProxy(const USRDisplayDrawBoxComponent* InComponent)
		: FPrimitiveSceneProxy(InComponent)
		, BoxColor(InComponent->BoxColor)
		, DisplayColor(InComponent->DisplayColor)
		, LineThickness(InComponent->LineThickness)
		, TargetScale(InComponent->TargetScale)
		, DisplayWidth(InComponent->DisplayWidth)
		, DisplayHeight(InComponent->DisplayHeight)
		, DisplayTilt(InComponent->DisplayTilt)
	{
		bWillEverBeLit = false;
	}

	// FPrimitiveSceneProxy interface.
	virtual void GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, FMeshElementCollector& Collector) const override
	{
		QUICK_SCOPE_CYCLE_COUNTER(STAT_DrawFrustumSceneProxy_DrawDynamicElements);

		FVector Verts[8];
		const float SRDisplayTile = FMath::DegreesToRadians(DisplayTilt);
		const float SRDisplayDepthByTilt = DisplayHeight * FMath::Sin(SRDisplayTile);
		const float SRDisplayHeightByTilt = DisplayHeight * FMath::Cos(SRDisplayTile);

		Verts[0] = FVector(0				, -DisplayWidth / 2	, 0);
		Verts[1] = FVector(SRDisplayHeightByTilt	, -DisplayWidth / 2	, 0);
		Verts[2] = FVector(SRDisplayHeightByTilt	, DisplayWidth / 2	, 0);
		Verts[3] = FVector(0				, DisplayWidth / 2	, 0);
		Verts[4] = FVector(0				, -DisplayWidth / 2	, SRDisplayDepthByTilt);
		Verts[5] = FVector(SRDisplayHeightByTilt	, -DisplayWidth / 2	, SRDisplayDepthByTilt);
		Verts[6] = FVector(SRDisplayHeightByTilt	, DisplayWidth / 2	, SRDisplayDepthByTilt);
		Verts[7] = FVector(0				, DisplayWidth / 2	, SRDisplayDepthByTilt);

		for (int32 X = 0; X < 8; ++X)
		{
			Verts[X] = GetLocalToWorld().TransformPosition(Verts[X] * TargetScale);
		}

		for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
		{
			if (VisibilityMap & (1 << ViewIndex))
			{
				FPrimitiveDrawInterface* PDI = Collector.GetPDI(ViewIndex);
				const FSceneView* View = Views[ViewIndex];

				const uint8 DepthPriorityGroup = GetDepthPriorityGroup(View);
				PDI->DrawLine(Verts[0], Verts[1], BoxColor, DepthPriorityGroup, LineThickness);
				PDI->DrawLine(Verts[1], Verts[2], BoxColor, DepthPriorityGroup, LineThickness);
				PDI->DrawLine(Verts[2], Verts[3], BoxColor, DepthPriorityGroup, LineThickness);
				PDI->DrawLine(Verts[3], Verts[0], BoxColor, DepthPriorityGroup, LineThickness);

				PDI->DrawLine(Verts[0], Verts[4], BoxColor, DepthPriorityGroup, LineThickness);
				PDI->DrawLine(Verts[1], Verts[5], BoxColor, DepthPriorityGroup, LineThickness);
				PDI->DrawLine(Verts[2], Verts[6], BoxColor, DepthPriorityGroup, LineThickness);
				PDI->DrawLine(Verts[3], Verts[7], BoxColor, DepthPriorityGroup, LineThickness);

				PDI->DrawLine(Verts[4], Verts[5], BoxColor, DepthPriorityGroup, LineThickness);
				PDI->DrawLine(Verts[5], Verts[6], BoxColor, DepthPriorityGroup, LineThickness);
				PDI->DrawLine(Verts[6], Verts[7], BoxColor, DepthPriorityGroup, LineThickness);
				PDI->DrawLine(Verts[7], Verts[4], BoxColor, DepthPriorityGroup, LineThickness);

				//display surface
				PDI->DrawLine(Verts[0], Verts[5], DisplayColor, DepthPriorityGroup, LineThickness);
				PDI->DrawLine(Verts[5], Verts[6], DisplayColor, DepthPriorityGroup, LineThickness);
				PDI->DrawLine(Verts[6], Verts[3], DisplayColor, DepthPriorityGroup, LineThickness);
				PDI->DrawLine(Verts[3], Verts[0], DisplayColor, DepthPriorityGroup, LineThickness);
			}
		}
	}

	virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const override
	{
		FPrimitiveViewRelevance Result;
		Result.bDrawRelevance = IsShown(View) && true;
		Result.bDynamicRelevance = true;
		Result.bShadowRelevance = IsShadowCast(View);
		Result.bEditorPrimitiveRelevance = UseEditorCompositing(View);
		return Result;
	}

	virtual uint32 GetMemoryFootprint(void) const override { return (sizeof(*this) + GetAllocatedSize()); }
	uint32 GetAllocatedSize(void) const { return (FPrimitiveSceneProxy::GetAllocatedSize()); }

private:
	FColor BoxColor;
	FColor DisplayColor;
	float LineThickness;
	float TargetScale;
	float DisplayWidth;
	float DisplayHeight;
	float DisplayTilt;
};

USRDisplayDrawBoxComponent::USRDisplayDrawBoxComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Set Defaults
	BoxColor = FColor::Blue;
	DisplayColor = FColor::White;
	LineThickness = 1.f;
	TargetScale = 1.f;

	bUseEditorCompositing = true;
	bHiddenInGame = true;
	SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
	SetGenerateOverlapEvents(false);

	BoxSize = DrawBoxSize::ELF_SR1_Size;
}

static const float defaultDisplayWidth_cm = 59.6736f;
static const float defaultDisplayHeight_cm = 33.5664f;
static const float defaultDisplayTilt_rad = 0.785398f;

FPrimitiveSceneProxy* USRDisplayDrawBoxComponent::CreateSceneProxy()
{
	UpdateDisplaySpec();
	return new FSRDisplayDrawBoxSceneProxy(this);
}

FBoxSphereBounds USRDisplayDrawBoxComponent::CalcBounds(const FTransform& LocalToWorld) const
{
	UpdateDisplaySpec();
	FVector origin(FVector(0, -DisplayWidth / 2, 0) * TargetScale);
	FVector extent(FVector(DisplayHeight * FMath::Sin(DisplayTilt), DisplayWidth / 2, DisplayHeight * FMath::Cos(DisplayTilt)) * TargetScale);
	float radious = (extent - origin).Size();
	return FBoxSphereBounds(LocalToWorld.TransformPosition(origin), extent, radious);
}

void USRDisplayDrawBoxComponent::UpdateDisplaySpec() const
{
	constexpr float METER_TO_CENTIMETER = 100.f;
	DisplayWidth = defaultDisplayWidth_cm;
	DisplayHeight = defaultDisplayHeight_cm;
	DisplayTilt = FMath::RadiansToDegrees(defaultDisplayTilt_rad);

	xr_display::FXRDisplaySystem* XRDisplaySystem = static_cast<xr_display::FXRDisplaySystem*>(GEngine->XRSystem.Get());
	if (!XRDisplaySystem) {
		return;
	}

	int32_t Size = 0;
	if (!sony::oz::xr_runtime::GetPanelSpecOfSupportedDevices(XRDisplaySystem->GetXRDisplay()->GetPlatformId(), nullptr, &Size) || Size < 1) {
		return;
	}
	auto PanelSpecSize = MakeUnique<sony::oz::xr_runtime::supported_panel_spec[]>(Size);
	if (!sony::oz::xr_runtime::GetPanelSpecOfSupportedDevices(XRDisplaySystem->GetXRDisplay()->GetPlatformId(), PanelSpecSize.Get(), &Size)) {
		return;
	}

	sony::oz::xr_runtime::supported_panel_spec spec = PanelSpecSize[static_cast<int>(BoxSize)];
	DisplayWidth = spec.width * METER_TO_CENTIMETER;
	DisplayHeight = spec.height * METER_TO_CENTIMETER;
	DisplayTilt = FMath::RadiansToDegrees(spec.angle);
}
