/**
 * Copyright 2019,2020,2021,2022 Sony Corporation
 */
#include "Blueprint/SRDisplayManager.h"
#include "Blueprint/SRDisplayDrawBoxComponent.h"

#include "XRDisplaySystem.h"

ASRDisplayManager::ASRDisplayManager()
{
	Box = CreateDefaultSubobject<USRDisplayDrawBoxComponent>(TEXT("Box"));
	PrimaryActorTick.bCanEverTick = true;
}

void ASRDisplayManager::OnConstruction(const FTransform& Transform)
{
	Box->BoxSize = (ScalingMode == EScalingMode::ORIGINAL_SIZE && GizmoSize == EGizmoSize::ELF_SR2_SIZE)
								? DrawBoxSize::ELF_SR2_Size : DrawBoxSize::ELF_SR1_Size;
}

void ASRDisplayManager::Tick(float DeltaSeconds)
{
	xr_display::FXRDisplaySystem* XRDisplaySystem = static_cast<xr_display::FXRDisplaySystem*>(GEngine->XRSystem.Get());
}

float ASRDisplayManager::GetRealToVirtualScale()
{
	float RealToVirtualScale = 1.f;
	if (Box)
	{
		RealToVirtualScale = Box->GetComponentScale().X; // X, Y and Z is always same value.
	}
	return RealToVirtualScale;
}

float ASRDisplayManager::GetPanelSizeScale(sony::oz::xr_runtime::SupportDevice ConnectedDevice) const
{
	if (ScalingMode == EScalingMode::SCALED_SIZE) {
		xr_display::FXRDisplaySystem* XRDisplaySystem = static_cast<xr_display::FXRDisplaySystem*>(GEngine->XRSystem.Get());
		if (!XRDisplaySystem) {
			return 1.0f;
		}
		int32_t Size = 0;
		if (!sony::oz::xr_runtime::GetPanelSpecOfSupportedDevices(XRDisplaySystem->GetXRDisplay()->GetPlatformId(), nullptr, &Size) || Size < 1) {
			return 1.0f;
		}
		auto PanelSpecSizeList = MakeUnique<sony::oz::xr_runtime::supported_panel_spec[]>(Size);
		if (!sony::oz::xr_runtime::GetPanelSpecOfSupportedDevices(XRDisplaySystem->GetXRDisplay()->GetPlatformId(), PanelSpecSizeList.Get(), &Size)) {
			return 1.0f;
		}
		return PanelSpecSizeList[static_cast<int>(ConnectedDevice)].width / PanelSpecSizeList[0].width;
	}
	return 1.0f;
}

#if WITH_EDITOR
void ASRDisplayManager::PostEditChangeProperty(struct FPropertyChangedEvent& e)
{
	FName PropertyName = (e.Property != NULL) ? e.Property->GetFName() : NAME_None;
	FName MemberPropertyName = (e.Property != NULL) ? e.MemberProperty->GetFName() : NAME_None;

	if (MemberPropertyName == "RelativeScale3D")
	{
		FVector Current = Box->GetComponentScale();
		float scaling = 1.f;
		if (PropertyName == "X")
		{
			scaling = Current.X;
		}
		else if (PropertyName == "Y")
		{
			scaling = Current.Y;
		}
		else if (PropertyName == "Z")
		{
			scaling = Current.Z;
		}
		else if (PropertyName == "RelativeScale3D")
		{
			scaling = Current.X;
		}

		Box->SetWorldScale3D(FVector(scaling, scaling, scaling));
		LineThickness = scaling == 0.f ? 1.f : roundf(scaling / 10);
	}

	Box->BoxColor = BoxColor;
	Box->DisplayColor = DisplayColor;
	Box->LineThickness = LineThickness;
	Box->BoxSize = (ScalingMode == EScalingMode::ORIGINAL_SIZE && GizmoSize == EGizmoSize::ELF_SR2_SIZE)
		? DrawBoxSize::ELF_SR2_Size : DrawBoxSize::ELF_SR1_Size;

	Super::PostEditChangeProperty(e);
}

#endif
