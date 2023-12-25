// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "SRDisplayModule/Classes/Blueprint/SRDisplayManager.h"
#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable : 4883)
#endif
PRAGMA_DISABLE_DEPRECATION_WARNINGS
void EmptyLinkFunctionForGeneratedCodeSRDisplayManager() {}
// Cross Module References
	SRDISPLAYMODULE_API UEnum* Z_Construct_UEnum_SRDisplayModule_EGizmoSize();
	UPackage* Z_Construct_UPackage__Script_SRDisplayModule();
	SRDISPLAYMODULE_API UEnum* Z_Construct_UEnum_SRDisplayModule_EScalingMode();
	SRDISPLAYMODULE_API UEnum* Z_Construct_UEnum_SRDisplayModule_ECrosstalkCorrectionType();
	SRDISPLAYMODULE_API UClass* Z_Construct_UClass_ASRDisplayManager_NoRegister();
	SRDISPLAYMODULE_API UClass* Z_Construct_UClass_ASRDisplayManager();
	ENGINE_API UClass* Z_Construct_UClass_AActor();
	COREUOBJECT_API UScriptStruct* Z_Construct_UScriptStruct_FColor();
// End Cross Module References
	static UEnum* EGizmoSize_StaticEnum()
	{
		static UEnum* Singleton = nullptr;
		if (!Singleton)
		{
			Singleton = GetStaticEnum(Z_Construct_UEnum_SRDisplayModule_EGizmoSize, Z_Construct_UPackage__Script_SRDisplayModule(), TEXT("EGizmoSize"));
		}
		return Singleton;
	}
	template<> SRDISPLAYMODULE_API UEnum* StaticEnum<EGizmoSize>()
	{
		return EGizmoSize_StaticEnum();
	}
	static FCompiledInDeferEnum Z_CompiledInDeferEnum_UEnum_EGizmoSize(EGizmoSize_StaticEnum, TEXT("/Script/SRDisplayModule"), TEXT("EGizmoSize"), false, nullptr, nullptr);
	uint32 Get_Z_Construct_UEnum_SRDisplayModule_EGizmoSize_Hash() { return 438396200U; }
	UEnum* Z_Construct_UEnum_SRDisplayModule_EGizmoSize()
	{
#if WITH_HOT_RELOAD
		UPackage* Outer = Z_Construct_UPackage__Script_SRDisplayModule();
		static UEnum* ReturnEnum = FindExistingEnumIfHotReloadOrDynamic(Outer, TEXT("EGizmoSize"), 0, Get_Z_Construct_UEnum_SRDisplayModule_EGizmoSize_Hash(), false);
#else
		static UEnum* ReturnEnum = nullptr;
#endif // WITH_HOT_RELOAD
		if (!ReturnEnum)
		{
			static const UE4CodeGen_Private::FEnumeratorParam Enumerators[] = {
				{ "EGizmoSize::ELF_SR1_SIZE", (int64)EGizmoSize::ELF_SR1_SIZE },
				{ "EGizmoSize::ELF_SR2_SIZE", (int64)EGizmoSize::ELF_SR2_SIZE },
			};
#if WITH_METADATA
			const UE4CodeGen_Private::FMetaDataPairParam Enum_MetaDataParams[] = {
				{ "ELF_SR1_SIZE.DisplayName", "ELF-SR1" },
				{ "ELF_SR1_SIZE.Name", "EGizmoSize::ELF_SR1_SIZE" },
				{ "ELF_SR2_SIZE.DisplayName", "ELF-SR2" },
				{ "ELF_SR2_SIZE.Name", "EGizmoSize::ELF_SR2_SIZE" },
				{ "ModuleRelativePath", "Classes/Blueprint/SRDisplayManager.h" },
			};
#endif
			static const UE4CodeGen_Private::FEnumParams EnumParams = {
				(UObject*(*)())Z_Construct_UPackage__Script_SRDisplayModule,
				nullptr,
				"EGizmoSize",
				"EGizmoSize",
				Enumerators,
				UE_ARRAY_COUNT(Enumerators),
				RF_Public|RF_Transient|RF_MarkAsNative,
				EEnumFlags::None,
				UE4CodeGen_Private::EDynamicType::NotDynamic,
				(uint8)UEnum::ECppForm::EnumClass,
				METADATA_PARAMS(Enum_MetaDataParams, UE_ARRAY_COUNT(Enum_MetaDataParams))
			};
			UE4CodeGen_Private::ConstructUEnum(ReturnEnum, EnumParams);
		}
		return ReturnEnum;
	}
	static UEnum* EScalingMode_StaticEnum()
	{
		static UEnum* Singleton = nullptr;
		if (!Singleton)
		{
			Singleton = GetStaticEnum(Z_Construct_UEnum_SRDisplayModule_EScalingMode, Z_Construct_UPackage__Script_SRDisplayModule(), TEXT("EScalingMode"));
		}
		return Singleton;
	}
	template<> SRDISPLAYMODULE_API UEnum* StaticEnum<EScalingMode>()
	{
		return EScalingMode_StaticEnum();
	}
	static FCompiledInDeferEnum Z_CompiledInDeferEnum_UEnum_EScalingMode(EScalingMode_StaticEnum, TEXT("/Script/SRDisplayModule"), TEXT("EScalingMode"), false, nullptr, nullptr);
	uint32 Get_Z_Construct_UEnum_SRDisplayModule_EScalingMode_Hash() { return 496999229U; }
	UEnum* Z_Construct_UEnum_SRDisplayModule_EScalingMode()
	{
#if WITH_HOT_RELOAD
		UPackage* Outer = Z_Construct_UPackage__Script_SRDisplayModule();
		static UEnum* ReturnEnum = FindExistingEnumIfHotReloadOrDynamic(Outer, TEXT("EScalingMode"), 0, Get_Z_Construct_UEnum_SRDisplayModule_EScalingMode_Hash(), false);
#else
		static UEnum* ReturnEnum = nullptr;
#endif // WITH_HOT_RELOAD
		if (!ReturnEnum)
		{
			static const UE4CodeGen_Private::FEnumeratorParam Enumerators[] = {
				{ "EScalingMode::SCALED_SIZE", (int64)EScalingMode::SCALED_SIZE },
				{ "EScalingMode::ORIGINAL_SIZE", (int64)EScalingMode::ORIGINAL_SIZE },
			};
#if WITH_METADATA
			const UE4CodeGen_Private::FMetaDataPairParam Enum_MetaDataParams[] = {
				{ "ModuleRelativePath", "Classes/Blueprint/SRDisplayManager.h" },
				{ "ORIGINAL_SIZE.DisplayName", "Original Size" },
				{ "ORIGINAL_SIZE.Name", "EScalingMode::ORIGINAL_SIZE" },
				{ "SCALED_SIZE.DisplayName", "Scaled Size" },
				{ "SCALED_SIZE.Name", "EScalingMode::SCALED_SIZE" },
			};
#endif
			static const UE4CodeGen_Private::FEnumParams EnumParams = {
				(UObject*(*)())Z_Construct_UPackage__Script_SRDisplayModule,
				nullptr,
				"EScalingMode",
				"EScalingMode",
				Enumerators,
				UE_ARRAY_COUNT(Enumerators),
				RF_Public|RF_Transient|RF_MarkAsNative,
				EEnumFlags::None,
				UE4CodeGen_Private::EDynamicType::NotDynamic,
				(uint8)UEnum::ECppForm::EnumClass,
				METADATA_PARAMS(Enum_MetaDataParams, UE_ARRAY_COUNT(Enum_MetaDataParams))
			};
			UE4CodeGen_Private::ConstructUEnum(ReturnEnum, EnumParams);
		}
		return ReturnEnum;
	}
	static UEnum* ECrosstalkCorrectionType_StaticEnum()
	{
		static UEnum* Singleton = nullptr;
		if (!Singleton)
		{
			Singleton = GetStaticEnum(Z_Construct_UEnum_SRDisplayModule_ECrosstalkCorrectionType, Z_Construct_UPackage__Script_SRDisplayModule(), TEXT("ECrosstalkCorrectionType"));
		}
		return Singleton;
	}
	template<> SRDISPLAYMODULE_API UEnum* StaticEnum<ECrosstalkCorrectionType>()
	{
		return ECrosstalkCorrectionType_StaticEnum();
	}
	static FCompiledInDeferEnum Z_CompiledInDeferEnum_UEnum_ECrosstalkCorrectionType(ECrosstalkCorrectionType_StaticEnum, TEXT("/Script/SRDisplayModule"), TEXT("ECrosstalkCorrectionType"), false, nullptr, nullptr);
	uint32 Get_Z_Construct_UEnum_SRDisplayModule_ECrosstalkCorrectionType_Hash() { return 403108444U; }
	UEnum* Z_Construct_UEnum_SRDisplayModule_ECrosstalkCorrectionType()
	{
#if WITH_HOT_RELOAD
		UPackage* Outer = Z_Construct_UPackage__Script_SRDisplayModule();
		static UEnum* ReturnEnum = FindExistingEnumIfHotReloadOrDynamic(Outer, TEXT("ECrosstalkCorrectionType"), 0, Get_Z_Construct_UEnum_SRDisplayModule_ECrosstalkCorrectionType_Hash(), false);
#else
		static UEnum* ReturnEnum = nullptr;
#endif // WITH_HOT_RELOAD
		if (!ReturnEnum)
		{
			static const UE4CodeGen_Private::FEnumeratorParam Enumerators[] = {
				{ "ECrosstalkCorrectionType::GRADATION_CORRECTION_MEDIUM", (int64)ECrosstalkCorrectionType::GRADATION_CORRECTION_MEDIUM },
				{ "ECrosstalkCorrectionType::GRADATION_CORRECTION_ALL", (int64)ECrosstalkCorrectionType::GRADATION_CORRECTION_ALL },
				{ "ECrosstalkCorrectionType::GRADATION_CORRECTION_HIGH_PRECISE", (int64)ECrosstalkCorrectionType::GRADATION_CORRECTION_HIGH_PRECISE },
			};
#if WITH_METADATA
			const UE4CodeGen_Private::FMetaDataPairParam Enum_MetaDataParams[] = {
				{ "GRADATION_CORRECTION_ALL.Comment", "/*\n\x09""Corrects crosstalk and make it less noticeable at all gradation.\n\x09GPU load will be higher than that of \"Medium gradation correction\".\n\x09*/" },
				{ "GRADATION_CORRECTION_ALL.DisplayName", "Mid" },
				{ "GRADATION_CORRECTION_ALL.Name", "ECrosstalkCorrectionType::GRADATION_CORRECTION_ALL" },
				{ "GRADATION_CORRECTION_ALL.ToolTip", "Corrects crosstalk and make it less noticeable at all gradation.\nGPU load will be higher than that of \"Medium gradation correction\"." },
				{ "GRADATION_CORRECTION_HIGH_PRECISE.Comment", "/*\n\x09""Corrects crosstalk at all gradation.\n\x09""Crosstalk will be less noticeable than \"All gradation correction\".\n\x09GPU load will be higher than when \"All gradation correction\".\n\x09*/" },
				{ "GRADATION_CORRECTION_HIGH_PRECISE.DisplayName", "High" },
				{ "GRADATION_CORRECTION_HIGH_PRECISE.Name", "ECrosstalkCorrectionType::GRADATION_CORRECTION_HIGH_PRECISE" },
				{ "GRADATION_CORRECTION_HIGH_PRECISE.ToolTip", "Corrects crosstalk at all gradation.\nCrosstalk will be less noticeable than \"All gradation correction\".\nGPU load will be higher than when \"All gradation correction\"." },
				{ "GRADATION_CORRECTION_MEDIUM.Comment", "/*\n\x09""Corrects crosstalk and make it less noticeable at medium gradation.\n\x09GPU load will be a little higher than when crosstalk correction is not used.\n\x09*/" },
				{ "GRADATION_CORRECTION_MEDIUM.DisplayName", "Low" },
				{ "GRADATION_CORRECTION_MEDIUM.Name", "ECrosstalkCorrectionType::GRADATION_CORRECTION_MEDIUM" },
				{ "GRADATION_CORRECTION_MEDIUM.ToolTip", "Corrects crosstalk and make it less noticeable at medium gradation.\nGPU load will be a little higher than when crosstalk correction is not used." },
				{ "ModuleRelativePath", "Classes/Blueprint/SRDisplayManager.h" },
			};
#endif
			static const UE4CodeGen_Private::FEnumParams EnumParams = {
				(UObject*(*)())Z_Construct_UPackage__Script_SRDisplayModule,
				nullptr,
				"ECrosstalkCorrectionType",
				"ECrosstalkCorrectionType",
				Enumerators,
				UE_ARRAY_COUNT(Enumerators),
				RF_Public|RF_Transient|RF_MarkAsNative,
				EEnumFlags::None,
				UE4CodeGen_Private::EDynamicType::NotDynamic,
				(uint8)UEnum::ECppForm::EnumClass,
				METADATA_PARAMS(Enum_MetaDataParams, UE_ARRAY_COUNT(Enum_MetaDataParams))
			};
			UE4CodeGen_Private::ConstructUEnum(ReturnEnum, EnumParams);
		}
		return ReturnEnum;
	}
	void ASRDisplayManager::StaticRegisterNativesASRDisplayManager()
	{
	}
	UClass* Z_Construct_UClass_ASRDisplayManager_NoRegister()
	{
		return ASRDisplayManager::StaticClass();
	}
	struct Z_Construct_UClass_ASRDisplayManager_Statics
	{
		static UObject* (*const DependentSingletons[])();
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam Class_MetaDataParams[];
#endif
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_ShowCameraWindow_MetaData[];
#endif
		static void NewProp_ShowCameraWindow_SetBit(void* Obj);
		static const UE4CodeGen_Private::FBoolPropertyParams NewProp_ShowCameraWindow;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_CrosstalkCorrection_MetaData[];
#endif
		static void NewProp_CrosstalkCorrection_SetBit(void* Obj);
		static const UE4CodeGen_Private::FBoolPropertyParams NewProp_CrosstalkCorrection;
		static const UE4CodeGen_Private::FUnsizedIntPropertyParams NewProp_CorrectionType_Underlying;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_CorrectionType_MetaData[];
#endif
		static const UE4CodeGen_Private::FEnumPropertyParams NewProp_CorrectionType;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_SpatialClipping_MetaData[];
#endif
		static void NewProp_SpatialClipping_SetBit(void* Obj);
		static const UE4CodeGen_Private::FBoolPropertyParams NewProp_SpatialClipping;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_IsSRRenderingActive_MetaData[];
#endif
		static void NewProp_IsSRRenderingActive_SetBit(void* Obj);
		static const UE4CodeGen_Private::FBoolPropertyParams NewProp_IsSRRenderingActive;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_FarClip_MetaData[];
#endif
		static const UE4CodeGen_Private::FFloatPropertyParams NewProp_FarClip;
		static const UE4CodeGen_Private::FUnsizedIntPropertyParams NewProp_ScalingMode_Underlying;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_ScalingMode_MetaData[];
#endif
		static const UE4CodeGen_Private::FEnumPropertyParams NewProp_ScalingMode;
		static const UE4CodeGen_Private::FUnsizedIntPropertyParams NewProp_GizmoSize_Underlying;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_GizmoSize_MetaData[];
#endif
		static const UE4CodeGen_Private::FEnumPropertyParams NewProp_GizmoSize;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_BoxColor_MetaData[];
#endif
		static const UE4CodeGen_Private::FStructPropertyParams NewProp_BoxColor;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_DisplayColor_MetaData[];
#endif
		static const UE4CodeGen_Private::FStructPropertyParams NewProp_DisplayColor;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_LineThickness_MetaData[];
#endif
		static const UE4CodeGen_Private::FFloatPropertyParams NewProp_LineThickness;
		static const UE4CodeGen_Private::FPropertyParamsBase* const PropPointers[];
		static const FCppClassTypeInfoStatic StaticCppClassTypeInfo;
		static const UE4CodeGen_Private::FClassParams ClassParams;
	};
	UObject* (*const Z_Construct_UClass_ASRDisplayManager_Statics::DependentSingletons[])() = {
		(UObject* (*)())Z_Construct_UClass_AActor,
		(UObject* (*)())Z_Construct_UPackage__Script_SRDisplayModule,
	};
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_ASRDisplayManager_Statics::Class_MetaDataParams[] = {
		{ "IncludePath", "Blueprint/SRDisplayManager.h" },
		{ "ModuleRelativePath", "Classes/Blueprint/SRDisplayManager.h" },
	};
#endif
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_ASRDisplayManager_Statics::NewProp_ShowCameraWindow_MetaData[] = {
		{ "Category", "SRDisplay|Tracking" },
		{ "ModuleRelativePath", "Classes/Blueprint/SRDisplayManager.h" },
		{ "ToolTip", "If checked, a window opens and shows images captured by SRDisplay camera." },
	};
#endif
	void Z_Construct_UClass_ASRDisplayManager_Statics::NewProp_ShowCameraWindow_SetBit(void* Obj)
	{
		((ASRDisplayManager*)Obj)->ShowCameraWindow = 1;
	}
	const UE4CodeGen_Private::FBoolPropertyParams Z_Construct_UClass_ASRDisplayManager_Statics::NewProp_ShowCameraWindow = { "ShowCameraWindow", nullptr, (EPropertyFlags)0x0010000000000005, UE4CodeGen_Private::EPropertyGenFlags::Bool | UE4CodeGen_Private::EPropertyGenFlags::NativeBool, RF_Public|RF_Transient|RF_MarkAsNative, 1, sizeof(bool), sizeof(ASRDisplayManager), &Z_Construct_UClass_ASRDisplayManager_Statics::NewProp_ShowCameraWindow_SetBit, METADATA_PARAMS(Z_Construct_UClass_ASRDisplayManager_Statics::NewProp_ShowCameraWindow_MetaData, UE_ARRAY_COUNT(Z_Construct_UClass_ASRDisplayManager_Statics::NewProp_ShowCameraWindow_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_ASRDisplayManager_Statics::NewProp_CrosstalkCorrection_MetaData[] = {
		{ "Category", "SRDisplay|Rendering|Crosstalk Correction (ELF-SR1 exclusive)" },
		{ "ModuleRelativePath", "Classes/Blueprint/SRDisplayManager.h" },
		{ "ToolTip", "If checked, crosstalk will be reduced." },
	};
#endif
	void Z_Construct_UClass_ASRDisplayManager_Statics::NewProp_CrosstalkCorrection_SetBit(void* Obj)
	{
		((ASRDisplayManager*)Obj)->CrosstalkCorrection = 1;
	}
	const UE4CodeGen_Private::FBoolPropertyParams Z_Construct_UClass_ASRDisplayManager_Statics::NewProp_CrosstalkCorrection = { "CrosstalkCorrection", nullptr, (EPropertyFlags)0x0010000000000005, UE4CodeGen_Private::EPropertyGenFlags::Bool | UE4CodeGen_Private::EPropertyGenFlags::NativeBool, RF_Public|RF_Transient|RF_MarkAsNative, 1, sizeof(bool), sizeof(ASRDisplayManager), &Z_Construct_UClass_ASRDisplayManager_Statics::NewProp_CrosstalkCorrection_SetBit, METADATA_PARAMS(Z_Construct_UClass_ASRDisplayManager_Statics::NewProp_CrosstalkCorrection_MetaData, UE_ARRAY_COUNT(Z_Construct_UClass_ASRDisplayManager_Statics::NewProp_CrosstalkCorrection_MetaData)) };
	const UE4CodeGen_Private::FUnsizedIntPropertyParams Z_Construct_UClass_ASRDisplayManager_Statics::NewProp_CorrectionType_Underlying = { "UnderlyingType", nullptr, (EPropertyFlags)0x0000000000000000, UE4CodeGen_Private::EPropertyGenFlags::Int, RF_Public|RF_Transient|RF_MarkAsNative, 1, 0, METADATA_PARAMS(nullptr, 0) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_ASRDisplayManager_Statics::NewProp_CorrectionType_MetaData[] = {
		{ "Category", "SRDisplay|Rendering|Crosstalk Correction (ELF-SR1 exclusive)" },
		{ "EditCondition", "CrosstalkCorrection" },
		{ "ModuleRelativePath", "Classes/Blueprint/SRDisplayManager.h" },
		{ "ToolTip", "Crosstalk Correction level can be selected." },
	};
#endif
	const UE4CodeGen_Private::FEnumPropertyParams Z_Construct_UClass_ASRDisplayManager_Statics::NewProp_CorrectionType = { "CorrectionType", nullptr, (EPropertyFlags)0x0010000000000005, UE4CodeGen_Private::EPropertyGenFlags::Enum, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(ASRDisplayManager, CorrectionType), Z_Construct_UEnum_SRDisplayModule_ECrosstalkCorrectionType, METADATA_PARAMS(Z_Construct_UClass_ASRDisplayManager_Statics::NewProp_CorrectionType_MetaData, UE_ARRAY_COUNT(Z_Construct_UClass_ASRDisplayManager_Statics::NewProp_CorrectionType_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_ASRDisplayManager_Statics::NewProp_SpatialClipping_MetaData[] = {
		{ "Category", "SRDisplay|Rendering" },
		{ "ModuleRelativePath", "Classes/Blueprint/SRDisplayManager.h" },
		{ "ToolTip", "If checked, contents will be clipped at front of SR Display." },
	};
#endif
	void Z_Construct_UClass_ASRDisplayManager_Statics::NewProp_SpatialClipping_SetBit(void* Obj)
	{
		((ASRDisplayManager*)Obj)->SpatialClipping = 1;
	}
	const UE4CodeGen_Private::FBoolPropertyParams Z_Construct_UClass_ASRDisplayManager_Statics::NewProp_SpatialClipping = { "SpatialClipping", nullptr, (EPropertyFlags)0x0010000000000005, UE4CodeGen_Private::EPropertyGenFlags::Bool | UE4CodeGen_Private::EPropertyGenFlags::NativeBool, RF_Public|RF_Transient|RF_MarkAsNative, 1, sizeof(bool), sizeof(ASRDisplayManager), &Z_Construct_UClass_ASRDisplayManager_Statics::NewProp_SpatialClipping_SetBit, METADATA_PARAMS(Z_Construct_UClass_ASRDisplayManager_Statics::NewProp_SpatialClipping_MetaData, UE_ARRAY_COUNT(Z_Construct_UClass_ASRDisplayManager_Statics::NewProp_SpatialClipping_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_ASRDisplayManager_Statics::NewProp_IsSRRenderingActive_MetaData[] = {
		{ "Category", "SRDisplay|Rendering" },
		{ "ModuleRelativePath", "Classes/Blueprint/SRDisplayManager.h" },
		{ "ToolTip", "If checked, display in 2D on SR Display." },
	};
#endif
	void Z_Construct_UClass_ASRDisplayManager_Statics::NewProp_IsSRRenderingActive_SetBit(void* Obj)
	{
		((ASRDisplayManager*)Obj)->IsSRRenderingActive = 1;
	}
	const UE4CodeGen_Private::FBoolPropertyParams Z_Construct_UClass_ASRDisplayManager_Statics::NewProp_IsSRRenderingActive = { "IsSRRenderingActive", nullptr, (EPropertyFlags)0x0010000000000005, UE4CodeGen_Private::EPropertyGenFlags::Bool | UE4CodeGen_Private::EPropertyGenFlags::NativeBool, RF_Public|RF_Transient|RF_MarkAsNative, 1, sizeof(bool), sizeof(ASRDisplayManager), &Z_Construct_UClass_ASRDisplayManager_Statics::NewProp_IsSRRenderingActive_SetBit, METADATA_PARAMS(Z_Construct_UClass_ASRDisplayManager_Statics::NewProp_IsSRRenderingActive_MetaData, UE_ARRAY_COUNT(Z_Construct_UClass_ASRDisplayManager_Statics::NewProp_IsSRRenderingActive_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_ASRDisplayManager_Statics::NewProp_FarClip_MetaData[] = {
		{ "Category", "SRDisplay|Rendering" },
		{ "ClampMin", "0.0" },
		{ "DisplayName", "Far Clip (cm)" },
		{ "ModuleRelativePath", "Classes/Blueprint/SRDisplayManager.h" },
		{ "ToolTip", "Far clipt distance (cm)." },
		{ "UIMin", "0.0" },
	};
#endif
	const UE4CodeGen_Private::FFloatPropertyParams Z_Construct_UClass_ASRDisplayManager_Statics::NewProp_FarClip = { "FarClip", nullptr, (EPropertyFlags)0x0010000000000005, UE4CodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(ASRDisplayManager, FarClip), METADATA_PARAMS(Z_Construct_UClass_ASRDisplayManager_Statics::NewProp_FarClip_MetaData, UE_ARRAY_COUNT(Z_Construct_UClass_ASRDisplayManager_Statics::NewProp_FarClip_MetaData)) };
	const UE4CodeGen_Private::FUnsizedIntPropertyParams Z_Construct_UClass_ASRDisplayManager_Statics::NewProp_ScalingMode_Underlying = { "UnderlyingType", nullptr, (EPropertyFlags)0x0000000000000000, UE4CodeGen_Private::EPropertyGenFlags::Int, RF_Public|RF_Transient|RF_MarkAsNative, 1, 0, METADATA_PARAMS(nullptr, 0) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_ASRDisplayManager_Statics::NewProp_ScalingMode_MetaData[] = {
		{ "Category", "SRDisplay|Rendering|Scaling Mode" },
		{ "ModuleRelativePath", "Classes/Blueprint/SRDisplayManager.h" },
		{ "ToolTip", "Scaled mode or Original mode can be selected." },
	};
#endif
	const UE4CodeGen_Private::FEnumPropertyParams Z_Construct_UClass_ASRDisplayManager_Statics::NewProp_ScalingMode = { "ScalingMode", nullptr, (EPropertyFlags)0x0010000000000005, UE4CodeGen_Private::EPropertyGenFlags::Enum, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(ASRDisplayManager, ScalingMode), Z_Construct_UEnum_SRDisplayModule_EScalingMode, METADATA_PARAMS(Z_Construct_UClass_ASRDisplayManager_Statics::NewProp_ScalingMode_MetaData, UE_ARRAY_COUNT(Z_Construct_UClass_ASRDisplayManager_Statics::NewProp_ScalingMode_MetaData)) };
	const UE4CodeGen_Private::FUnsizedIntPropertyParams Z_Construct_UClass_ASRDisplayManager_Statics::NewProp_GizmoSize_Underlying = { "UnderlyingType", nullptr, (EPropertyFlags)0x0000000000000000, UE4CodeGen_Private::EPropertyGenFlags::Int, RF_Public|RF_Transient|RF_MarkAsNative, 1, 0, METADATA_PARAMS(nullptr, 0) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_ASRDisplayManager_Statics::NewProp_GizmoSize_MetaData[] = {
		{ "Category", "SRDisplay|Rendering|Scaling Mode" },
		{ "EditCondition", "ScalingMode == EScalingMode::ORIGINAL_SIZE" },
		{ "ModuleRelativePath", "Classes/Blueprint/SRDisplayManager.h" },
		{ "ToolTip", "size of SRDisplayManager gizmo can be selected." },
	};
#endif
	const UE4CodeGen_Private::FEnumPropertyParams Z_Construct_UClass_ASRDisplayManager_Statics::NewProp_GizmoSize = { "GizmoSize", nullptr, (EPropertyFlags)0x0010000000000005, UE4CodeGen_Private::EPropertyGenFlags::Enum, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(ASRDisplayManager, GizmoSize), Z_Construct_UEnum_SRDisplayModule_EGizmoSize, METADATA_PARAMS(Z_Construct_UClass_ASRDisplayManager_Statics::NewProp_GizmoSize_MetaData, UE_ARRAY_COUNT(Z_Construct_UClass_ASRDisplayManager_Statics::NewProp_GizmoSize_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_ASRDisplayManager_Statics::NewProp_BoxColor_MetaData[] = {
		{ "Category", "SRDisplay|Appearance" },
		{ "Comment", "// Appearance\n" },
		{ "ModuleRelativePath", "Classes/Blueprint/SRDisplayManager.h" },
		{ "ToolTip", "Appearance" },
	};
#endif
	const UE4CodeGen_Private::FStructPropertyParams Z_Construct_UClass_ASRDisplayManager_Statics::NewProp_BoxColor = { "BoxColor", nullptr, (EPropertyFlags)0x0010020000000005, UE4CodeGen_Private::EPropertyGenFlags::Struct, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(ASRDisplayManager, BoxColor), Z_Construct_UScriptStruct_FColor, METADATA_PARAMS(Z_Construct_UClass_ASRDisplayManager_Statics::NewProp_BoxColor_MetaData, UE_ARRAY_COUNT(Z_Construct_UClass_ASRDisplayManager_Statics::NewProp_BoxColor_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_ASRDisplayManager_Statics::NewProp_DisplayColor_MetaData[] = {
		{ "Category", "SRDisplay|Appearance" },
		{ "ModuleRelativePath", "Classes/Blueprint/SRDisplayManager.h" },
	};
#endif
	const UE4CodeGen_Private::FStructPropertyParams Z_Construct_UClass_ASRDisplayManager_Statics::NewProp_DisplayColor = { "DisplayColor", nullptr, (EPropertyFlags)0x0010000000000005, UE4CodeGen_Private::EPropertyGenFlags::Struct, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(ASRDisplayManager, DisplayColor), Z_Construct_UScriptStruct_FColor, METADATA_PARAMS(Z_Construct_UClass_ASRDisplayManager_Statics::NewProp_DisplayColor_MetaData, UE_ARRAY_COUNT(Z_Construct_UClass_ASRDisplayManager_Statics::NewProp_DisplayColor_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_ASRDisplayManager_Statics::NewProp_LineThickness_MetaData[] = {
		{ "Category", "SRDisplay|Appearance" },
		{ "ClampMax", "200.0" },
		{ "ClampMin", "1.0" },
		{ "ModuleRelativePath", "Classes/Blueprint/SRDisplayManager.h" },
		{ "UIMax", "200.0" },
		{ "UIMin", "1.0" },
	};
#endif
	const UE4CodeGen_Private::FFloatPropertyParams Z_Construct_UClass_ASRDisplayManager_Statics::NewProp_LineThickness = { "LineThickness", nullptr, (EPropertyFlags)0x0010000000000005, UE4CodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(ASRDisplayManager, LineThickness), METADATA_PARAMS(Z_Construct_UClass_ASRDisplayManager_Statics::NewProp_LineThickness_MetaData, UE_ARRAY_COUNT(Z_Construct_UClass_ASRDisplayManager_Statics::NewProp_LineThickness_MetaData)) };
	const UE4CodeGen_Private::FPropertyParamsBase* const Z_Construct_UClass_ASRDisplayManager_Statics::PropPointers[] = {
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_ASRDisplayManager_Statics::NewProp_ShowCameraWindow,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_ASRDisplayManager_Statics::NewProp_CrosstalkCorrection,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_ASRDisplayManager_Statics::NewProp_CorrectionType_Underlying,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_ASRDisplayManager_Statics::NewProp_CorrectionType,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_ASRDisplayManager_Statics::NewProp_SpatialClipping,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_ASRDisplayManager_Statics::NewProp_IsSRRenderingActive,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_ASRDisplayManager_Statics::NewProp_FarClip,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_ASRDisplayManager_Statics::NewProp_ScalingMode_Underlying,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_ASRDisplayManager_Statics::NewProp_ScalingMode,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_ASRDisplayManager_Statics::NewProp_GizmoSize_Underlying,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_ASRDisplayManager_Statics::NewProp_GizmoSize,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_ASRDisplayManager_Statics::NewProp_BoxColor,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_ASRDisplayManager_Statics::NewProp_DisplayColor,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_ASRDisplayManager_Statics::NewProp_LineThickness,
	};
	const FCppClassTypeInfoStatic Z_Construct_UClass_ASRDisplayManager_Statics::StaticCppClassTypeInfo = {
		TCppClassTypeTraits<ASRDisplayManager>::IsAbstract,
	};
	const UE4CodeGen_Private::FClassParams Z_Construct_UClass_ASRDisplayManager_Statics::ClassParams = {
		&ASRDisplayManager::StaticClass,
		"Engine",
		&StaticCppClassTypeInfo,
		DependentSingletons,
		nullptr,
		Z_Construct_UClass_ASRDisplayManager_Statics::PropPointers,
		nullptr,
		UE_ARRAY_COUNT(DependentSingletons),
		0,
		UE_ARRAY_COUNT(Z_Construct_UClass_ASRDisplayManager_Statics::PropPointers),
		0,
		0x008000A4u,
		METADATA_PARAMS(Z_Construct_UClass_ASRDisplayManager_Statics::Class_MetaDataParams, UE_ARRAY_COUNT(Z_Construct_UClass_ASRDisplayManager_Statics::Class_MetaDataParams))
	};
	UClass* Z_Construct_UClass_ASRDisplayManager()
	{
		static UClass* OuterClass = nullptr;
		if (!OuterClass)
		{
			UE4CodeGen_Private::ConstructUClass(OuterClass, Z_Construct_UClass_ASRDisplayManager_Statics::ClassParams);
		}
		return OuterClass;
	}
	IMPLEMENT_CLASS(ASRDisplayManager, 457966079);
	template<> SRDISPLAYMODULE_API UClass* StaticClass<ASRDisplayManager>()
	{
		return ASRDisplayManager::StaticClass();
	}
	static FCompiledInDefer Z_CompiledInDefer_UClass_ASRDisplayManager(Z_Construct_UClass_ASRDisplayManager, &ASRDisplayManager::StaticClass, TEXT("/Script/SRDisplayModule"), TEXT("ASRDisplayManager"), false, nullptr, nullptr, nullptr);
	DEFINE_VTABLE_PTR_HELPER_CTOR(ASRDisplayManager);
PRAGMA_ENABLE_DEPRECATION_WARNINGS
#ifdef _MSC_VER
#pragma warning (pop)
#endif
