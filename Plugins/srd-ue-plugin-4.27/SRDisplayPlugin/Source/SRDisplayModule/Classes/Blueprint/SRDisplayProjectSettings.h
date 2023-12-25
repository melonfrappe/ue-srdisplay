#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"

#include "SRDisplayProjectSettings.generated.h"

UCLASS(config = Game, defaultconfig)
class SRDISPLAYMODULE_API USRDisplayProjectSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Config, Category = "SRDisplay")
	bool RunWithoutSRDisplayMode = false;

public:
	virtual FName GetContainerName() const override {
		return Super::GetContainerName();
	}
	virtual FName GetCategoryName() const override {
		return "Plugins";
	}
	virtual FName GetSectionName() const override {
		return Super::GetSectionName();
	}

#if WITH_EDITOR
	virtual FText GetSectionText() const override {
		return FText::FromString(TEXT("SRDisplay"));
	}
	virtual FText GetSectionDescription() const override {
		return FText::FromString(TEXT("Set up SR Display Plugin"));
	}
#endif
};
