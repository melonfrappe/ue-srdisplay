// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "SRDisplayModule/Classes/Blueprint/SRDisplayProjectSettings.h"
#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable : 4883)
#endif
PRAGMA_DISABLE_DEPRECATION_WARNINGS
void EmptyLinkFunctionForGeneratedCodeSRDisplayProjectSettings() {}
// Cross Module References
	SRDISPLAYMODULE_API UClass* Z_Construct_UClass_USRDisplayProjectSettings_NoRegister();
	SRDISPLAYMODULE_API UClass* Z_Construct_UClass_USRDisplayProjectSettings();
	DEVELOPERSETTINGS_API UClass* Z_Construct_UClass_UDeveloperSettings();
	UPackage* Z_Construct_UPackage__Script_SRDisplayModule();
// End Cross Module References
	void USRDisplayProjectSettings::StaticRegisterNativesUSRDisplayProjectSettings()
	{
	}
	UClass* Z_Construct_UClass_USRDisplayProjectSettings_NoRegister()
	{
		return USRDisplayProjectSettings::StaticClass();
	}
	struct Z_Construct_UClass_USRDisplayProjectSettings_Statics
	{
		static UObject* (*const DependentSingletons[])();
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam Class_MetaDataParams[];
#endif
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_RunWithoutSRDisplayMode_MetaData[];
#endif
		static void NewProp_RunWithoutSRDisplayMode_SetBit(void* Obj);
		static const UE4CodeGen_Private::FBoolPropertyParams NewProp_RunWithoutSRDisplayMode;
		static const UE4CodeGen_Private::FPropertyParamsBase* const PropPointers[];
		static const FCppClassTypeInfoStatic StaticCppClassTypeInfo;
		static const UE4CodeGen_Private::FClassParams ClassParams;
	};
	UObject* (*const Z_Construct_UClass_USRDisplayProjectSettings_Statics::DependentSingletons[])() = {
		(UObject* (*)())Z_Construct_UClass_UDeveloperSettings,
		(UObject* (*)())Z_Construct_UPackage__Script_SRDisplayModule,
	};
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_USRDisplayProjectSettings_Statics::Class_MetaDataParams[] = {
		{ "IncludePath", "Blueprint/SRDisplayProjectSettings.h" },
		{ "ModuleRelativePath", "Classes/Blueprint/SRDisplayProjectSettings.h" },
	};
#endif
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_USRDisplayProjectSettings_Statics::NewProp_RunWithoutSRDisplayMode_MetaData[] = {
		{ "Category", "SRDisplay" },
		{ "ModuleRelativePath", "Classes/Blueprint/SRDisplayProjectSettings.h" },
	};
#endif
	void Z_Construct_UClass_USRDisplayProjectSettings_Statics::NewProp_RunWithoutSRDisplayMode_SetBit(void* Obj)
	{
		((USRDisplayProjectSettings*)Obj)->RunWithoutSRDisplayMode = 1;
	}
	const UE4CodeGen_Private::FBoolPropertyParams Z_Construct_UClass_USRDisplayProjectSettings_Statics::NewProp_RunWithoutSRDisplayMode = { "RunWithoutSRDisplayMode", nullptr, (EPropertyFlags)0x0010000000004001, UE4CodeGen_Private::EPropertyGenFlags::Bool | UE4CodeGen_Private::EPropertyGenFlags::NativeBool, RF_Public|RF_Transient|RF_MarkAsNative, 1, sizeof(bool), sizeof(USRDisplayProjectSettings), &Z_Construct_UClass_USRDisplayProjectSettings_Statics::NewProp_RunWithoutSRDisplayMode_SetBit, METADATA_PARAMS(Z_Construct_UClass_USRDisplayProjectSettings_Statics::NewProp_RunWithoutSRDisplayMode_MetaData, UE_ARRAY_COUNT(Z_Construct_UClass_USRDisplayProjectSettings_Statics::NewProp_RunWithoutSRDisplayMode_MetaData)) };
	const UE4CodeGen_Private::FPropertyParamsBase* const Z_Construct_UClass_USRDisplayProjectSettings_Statics::PropPointers[] = {
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_USRDisplayProjectSettings_Statics::NewProp_RunWithoutSRDisplayMode,
	};
	const FCppClassTypeInfoStatic Z_Construct_UClass_USRDisplayProjectSettings_Statics::StaticCppClassTypeInfo = {
		TCppClassTypeTraits<USRDisplayProjectSettings>::IsAbstract,
	};
	const UE4CodeGen_Private::FClassParams Z_Construct_UClass_USRDisplayProjectSettings_Statics::ClassParams = {
		&USRDisplayProjectSettings::StaticClass,
		"Game",
		&StaticCppClassTypeInfo,
		DependentSingletons,
		nullptr,
		Z_Construct_UClass_USRDisplayProjectSettings_Statics::PropPointers,
		nullptr,
		UE_ARRAY_COUNT(DependentSingletons),
		0,
		UE_ARRAY_COUNT(Z_Construct_UClass_USRDisplayProjectSettings_Statics::PropPointers),
		0,
		0x001000A6u,
		METADATA_PARAMS(Z_Construct_UClass_USRDisplayProjectSettings_Statics::Class_MetaDataParams, UE_ARRAY_COUNT(Z_Construct_UClass_USRDisplayProjectSettings_Statics::Class_MetaDataParams))
	};
	UClass* Z_Construct_UClass_USRDisplayProjectSettings()
	{
		static UClass* OuterClass = nullptr;
		if (!OuterClass)
		{
			UE4CodeGen_Private::ConstructUClass(OuterClass, Z_Construct_UClass_USRDisplayProjectSettings_Statics::ClassParams);
		}
		return OuterClass;
	}
	IMPLEMENT_CLASS(USRDisplayProjectSettings, 2970912658);
	template<> SRDISPLAYMODULE_API UClass* StaticClass<USRDisplayProjectSettings>()
	{
		return USRDisplayProjectSettings::StaticClass();
	}
	static FCompiledInDefer Z_CompiledInDefer_UClass_USRDisplayProjectSettings(Z_Construct_UClass_USRDisplayProjectSettings, &USRDisplayProjectSettings::StaticClass, TEXT("/Script/SRDisplayModule"), TEXT("USRDisplayProjectSettings"), false, nullptr, nullptr, nullptr);
	DEFINE_VTABLE_PTR_HELPER_CTOR(USRDisplayProjectSettings);
PRAGMA_ENABLE_DEPRECATION_WARNINGS
#ifdef _MSC_VER
#pragma warning (pop)
#endif
