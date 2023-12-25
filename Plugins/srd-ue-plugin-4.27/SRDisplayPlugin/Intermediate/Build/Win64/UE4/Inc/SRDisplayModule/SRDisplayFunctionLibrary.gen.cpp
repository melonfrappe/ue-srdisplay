// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "SRDisplayModule/Public/Blueprint/SRDisplayFunctionLibrary.h"
#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable : 4883)
#endif
PRAGMA_DISABLE_DEPRECATION_WARNINGS
void EmptyLinkFunctionForGeneratedCodeSRDisplayFunctionLibrary() {}
// Cross Module References
	SRDISPLAYMODULE_API UClass* Z_Construct_UClass_USRDisplayFunctionLibrary_NoRegister();
	SRDISPLAYMODULE_API UClass* Z_Construct_UClass_USRDisplayFunctionLibrary();
	ENGINE_API UClass* Z_Construct_UClass_UBlueprintFunctionLibrary();
	UPackage* Z_Construct_UPackage__Script_SRDisplayModule();
	ENGINE_API UClass* Z_Construct_UClass_APlayerController_NoRegister();
	COREUOBJECT_API UScriptStruct* Z_Construct_UScriptStruct_FVector();
	COREUOBJECT_API UScriptStruct* Z_Construct_UScriptStruct_FVector2D();
// End Cross Module References
	DEFINE_FUNCTION(USRDisplayFunctionLibrary::execConvertMouseLocationToWorldSpace)
	{
		P_GET_OBJECT(APlayerController,Z_Param_Player);
		P_GET_STRUCT_REF(FVector,Z_Param_Out_WorldPosition);
		P_GET_STRUCT_REF(FVector,Z_Param_Out_WorldDirection);
		P_GET_STRUCT_REF(FVector,Z_Param_Out_CameraPosition);
		P_FINISH;
		P_NATIVE_BEGIN;
		*(bool*)Z_Param__Result=USRDisplayFunctionLibrary::ConvertMouseLocationToWorldSpace(Z_Param_Player,Z_Param_Out_WorldPosition,Z_Param_Out_WorldDirection,Z_Param_Out_CameraPosition);
		P_NATIVE_END;
	}
	DEFINE_FUNCTION(USRDisplayFunctionLibrary::execDeprojectScreenToWorld)
	{
		P_GET_OBJECT(APlayerController,Z_Param_Player);
		P_GET_STRUCT_REF(FVector2D,Z_Param_Out_ScreenPosition);
		P_GET_STRUCT_REF(FVector,Z_Param_Out_WorldPosition);
		P_GET_STRUCT_REF(FVector,Z_Param_Out_WorldDirection);
		P_GET_STRUCT_REF(FVector,Z_Param_Out_CameraPosition);
		P_FINISH;
		P_NATIVE_BEGIN;
		*(bool*)Z_Param__Result=USRDisplayFunctionLibrary::DeprojectScreenToWorld(Z_Param_Player,Z_Param_Out_ScreenPosition,Z_Param_Out_WorldPosition,Z_Param_Out_WorldDirection,Z_Param_Out_CameraPosition);
		P_NATIVE_END;
	}
	DEFINE_FUNCTION(USRDisplayFunctionLibrary::execGetMousePosition)
	{
		P_GET_PROPERTY_REF(FFloatProperty,Z_Param_Out_LocationX);
		P_GET_PROPERTY_REF(FFloatProperty,Z_Param_Out_LocationY);
		P_FINISH;
		P_NATIVE_BEGIN;
		*(bool*)Z_Param__Result=USRDisplayFunctionLibrary::GetMousePosition(Z_Param_Out_LocationX,Z_Param_Out_LocationY);
		P_NATIVE_END;
	}
	void USRDisplayFunctionLibrary::StaticRegisterNativesUSRDisplayFunctionLibrary()
	{
		UClass* Class = USRDisplayFunctionLibrary::StaticClass();
		static const FNameNativePtrPair Funcs[] = {
			{ "ConvertMouseLocationToWorldSpace", &USRDisplayFunctionLibrary::execConvertMouseLocationToWorldSpace },
			{ "DeprojectScreenToWorld", &USRDisplayFunctionLibrary::execDeprojectScreenToWorld },
			{ "GetMousePosition", &USRDisplayFunctionLibrary::execGetMousePosition },
		};
		FNativeFunctionRegistrar::RegisterFunctions(Class, Funcs, UE_ARRAY_COUNT(Funcs));
	}
	struct Z_Construct_UFunction_USRDisplayFunctionLibrary_ConvertMouseLocationToWorldSpace_Statics
	{
		struct SRDisplayFunctionLibrary_eventConvertMouseLocationToWorldSpace_Parms
		{
			APlayerController* Player;
			FVector WorldPosition;
			FVector WorldDirection;
			FVector CameraPosition;
			bool ReturnValue;
		};
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_Player_MetaData[];
#endif
		static const UE4CodeGen_Private::FObjectPropertyParams NewProp_Player;
		static const UE4CodeGen_Private::FStructPropertyParams NewProp_WorldPosition;
		static const UE4CodeGen_Private::FStructPropertyParams NewProp_WorldDirection;
		static const UE4CodeGen_Private::FStructPropertyParams NewProp_CameraPosition;
		static void NewProp_ReturnValue_SetBit(void* Obj);
		static const UE4CodeGen_Private::FBoolPropertyParams NewProp_ReturnValue;
		static const UE4CodeGen_Private::FPropertyParamsBase* const PropPointers[];
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam Function_MetaDataParams[];
#endif
		static const UE4CodeGen_Private::FFunctionParams FuncParams;
	};
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UFunction_USRDisplayFunctionLibrary_ConvertMouseLocationToWorldSpace_Statics::NewProp_Player_MetaData[] = {
		{ "NativeConst", "" },
	};
#endif
	const UE4CodeGen_Private::FObjectPropertyParams Z_Construct_UFunction_USRDisplayFunctionLibrary_ConvertMouseLocationToWorldSpace_Statics::NewProp_Player = { "Player", nullptr, (EPropertyFlags)0x0010000000000080, UE4CodeGen_Private::EPropertyGenFlags::Object, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(SRDisplayFunctionLibrary_eventConvertMouseLocationToWorldSpace_Parms, Player), Z_Construct_UClass_APlayerController_NoRegister, METADATA_PARAMS(Z_Construct_UFunction_USRDisplayFunctionLibrary_ConvertMouseLocationToWorldSpace_Statics::NewProp_Player_MetaData, UE_ARRAY_COUNT(Z_Construct_UFunction_USRDisplayFunctionLibrary_ConvertMouseLocationToWorldSpace_Statics::NewProp_Player_MetaData)) };
	const UE4CodeGen_Private::FStructPropertyParams Z_Construct_UFunction_USRDisplayFunctionLibrary_ConvertMouseLocationToWorldSpace_Statics::NewProp_WorldPosition = { "WorldPosition", nullptr, (EPropertyFlags)0x0010000000000180, UE4CodeGen_Private::EPropertyGenFlags::Struct, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(SRDisplayFunctionLibrary_eventConvertMouseLocationToWorldSpace_Parms, WorldPosition), Z_Construct_UScriptStruct_FVector, METADATA_PARAMS(nullptr, 0) };
	const UE4CodeGen_Private::FStructPropertyParams Z_Construct_UFunction_USRDisplayFunctionLibrary_ConvertMouseLocationToWorldSpace_Statics::NewProp_WorldDirection = { "WorldDirection", nullptr, (EPropertyFlags)0x0010000000000180, UE4CodeGen_Private::EPropertyGenFlags::Struct, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(SRDisplayFunctionLibrary_eventConvertMouseLocationToWorldSpace_Parms, WorldDirection), Z_Construct_UScriptStruct_FVector, METADATA_PARAMS(nullptr, 0) };
	const UE4CodeGen_Private::FStructPropertyParams Z_Construct_UFunction_USRDisplayFunctionLibrary_ConvertMouseLocationToWorldSpace_Statics::NewProp_CameraPosition = { "CameraPosition", nullptr, (EPropertyFlags)0x0010000000000180, UE4CodeGen_Private::EPropertyGenFlags::Struct, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(SRDisplayFunctionLibrary_eventConvertMouseLocationToWorldSpace_Parms, CameraPosition), Z_Construct_UScriptStruct_FVector, METADATA_PARAMS(nullptr, 0) };
	void Z_Construct_UFunction_USRDisplayFunctionLibrary_ConvertMouseLocationToWorldSpace_Statics::NewProp_ReturnValue_SetBit(void* Obj)
	{
		((SRDisplayFunctionLibrary_eventConvertMouseLocationToWorldSpace_Parms*)Obj)->ReturnValue = 1;
	}
	const UE4CodeGen_Private::FBoolPropertyParams Z_Construct_UFunction_USRDisplayFunctionLibrary_ConvertMouseLocationToWorldSpace_Statics::NewProp_ReturnValue = { "ReturnValue", nullptr, (EPropertyFlags)0x0010000000000580, UE4CodeGen_Private::EPropertyGenFlags::Bool | UE4CodeGen_Private::EPropertyGenFlags::NativeBool, RF_Public|RF_Transient|RF_MarkAsNative, 1, sizeof(bool), sizeof(SRDisplayFunctionLibrary_eventConvertMouseLocationToWorldSpace_Parms), &Z_Construct_UFunction_USRDisplayFunctionLibrary_ConvertMouseLocationToWorldSpace_Statics::NewProp_ReturnValue_SetBit, METADATA_PARAMS(nullptr, 0) };
	const UE4CodeGen_Private::FPropertyParamsBase* const Z_Construct_UFunction_USRDisplayFunctionLibrary_ConvertMouseLocationToWorldSpace_Statics::PropPointers[] = {
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_USRDisplayFunctionLibrary_ConvertMouseLocationToWorldSpace_Statics::NewProp_Player,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_USRDisplayFunctionLibrary_ConvertMouseLocationToWorldSpace_Statics::NewProp_WorldPosition,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_USRDisplayFunctionLibrary_ConvertMouseLocationToWorldSpace_Statics::NewProp_WorldDirection,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_USRDisplayFunctionLibrary_ConvertMouseLocationToWorldSpace_Statics::NewProp_CameraPosition,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_USRDisplayFunctionLibrary_ConvertMouseLocationToWorldSpace_Statics::NewProp_ReturnValue,
	};
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UFunction_USRDisplayFunctionLibrary_ConvertMouseLocationToWorldSpace_Statics::Function_MetaDataParams[] = {
		{ "Category", "SRDisplay" },
		{ "Comment", "/**\n\x09 * @brief Convert current mouse 2D position to World Space 3D position and direction. Returns false if unable to determine value (The reference camera position is also acquired).\n\x09 */" },
		{ "DisplayName", "ConvertMouseLocationToWorldSpace" },
		{ "ModuleRelativePath", "Public/Blueprint/SRDisplayFunctionLibrary.h" },
		{ "ToolTip", "@brief Convert current mouse 2D position to World Space 3D position and direction. Returns false if unable to determine value (The reference camera position is also acquired)." },
	};
#endif
	const UE4CodeGen_Private::FFunctionParams Z_Construct_UFunction_USRDisplayFunctionLibrary_ConvertMouseLocationToWorldSpace_Statics::FuncParams = { (UObject*(*)())Z_Construct_UClass_USRDisplayFunctionLibrary, nullptr, "ConvertMouseLocationToWorldSpace", nullptr, nullptr, sizeof(SRDisplayFunctionLibrary_eventConvertMouseLocationToWorldSpace_Parms), Z_Construct_UFunction_USRDisplayFunctionLibrary_ConvertMouseLocationToWorldSpace_Statics::PropPointers, UE_ARRAY_COUNT(Z_Construct_UFunction_USRDisplayFunctionLibrary_ConvertMouseLocationToWorldSpace_Statics::PropPointers), RF_Public|RF_Transient|RF_MarkAsNative, (EFunctionFlags)0x14C22401, 0, 0, METADATA_PARAMS(Z_Construct_UFunction_USRDisplayFunctionLibrary_ConvertMouseLocationToWorldSpace_Statics::Function_MetaDataParams, UE_ARRAY_COUNT(Z_Construct_UFunction_USRDisplayFunctionLibrary_ConvertMouseLocationToWorldSpace_Statics::Function_MetaDataParams)) };
	UFunction* Z_Construct_UFunction_USRDisplayFunctionLibrary_ConvertMouseLocationToWorldSpace()
	{
		static UFunction* ReturnFunction = nullptr;
		if (!ReturnFunction)
		{
			UE4CodeGen_Private::ConstructUFunction(ReturnFunction, Z_Construct_UFunction_USRDisplayFunctionLibrary_ConvertMouseLocationToWorldSpace_Statics::FuncParams);
		}
		return ReturnFunction;
	}
	struct Z_Construct_UFunction_USRDisplayFunctionLibrary_DeprojectScreenToWorld_Statics
	{
		struct SRDisplayFunctionLibrary_eventDeprojectScreenToWorld_Parms
		{
			APlayerController* Player;
			FVector2D ScreenPosition;
			FVector WorldPosition;
			FVector WorldDirection;
			FVector CameraPosition;
			bool ReturnValue;
		};
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_Player_MetaData[];
#endif
		static const UE4CodeGen_Private::FObjectPropertyParams NewProp_Player;
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam NewProp_ScreenPosition_MetaData[];
#endif
		static const UE4CodeGen_Private::FStructPropertyParams NewProp_ScreenPosition;
		static const UE4CodeGen_Private::FStructPropertyParams NewProp_WorldPosition;
		static const UE4CodeGen_Private::FStructPropertyParams NewProp_WorldDirection;
		static const UE4CodeGen_Private::FStructPropertyParams NewProp_CameraPosition;
		static void NewProp_ReturnValue_SetBit(void* Obj);
		static const UE4CodeGen_Private::FBoolPropertyParams NewProp_ReturnValue;
		static const UE4CodeGen_Private::FPropertyParamsBase* const PropPointers[];
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam Function_MetaDataParams[];
#endif
		static const UE4CodeGen_Private::FFunctionParams FuncParams;
	};
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UFunction_USRDisplayFunctionLibrary_DeprojectScreenToWorld_Statics::NewProp_Player_MetaData[] = {
		{ "NativeConst", "" },
	};
#endif
	const UE4CodeGen_Private::FObjectPropertyParams Z_Construct_UFunction_USRDisplayFunctionLibrary_DeprojectScreenToWorld_Statics::NewProp_Player = { "Player", nullptr, (EPropertyFlags)0x0010000000000080, UE4CodeGen_Private::EPropertyGenFlags::Object, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(SRDisplayFunctionLibrary_eventDeprojectScreenToWorld_Parms, Player), Z_Construct_UClass_APlayerController_NoRegister, METADATA_PARAMS(Z_Construct_UFunction_USRDisplayFunctionLibrary_DeprojectScreenToWorld_Statics::NewProp_Player_MetaData, UE_ARRAY_COUNT(Z_Construct_UFunction_USRDisplayFunctionLibrary_DeprojectScreenToWorld_Statics::NewProp_Player_MetaData)) };
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UFunction_USRDisplayFunctionLibrary_DeprojectScreenToWorld_Statics::NewProp_ScreenPosition_MetaData[] = {
		{ "NativeConst", "" },
	};
#endif
	const UE4CodeGen_Private::FStructPropertyParams Z_Construct_UFunction_USRDisplayFunctionLibrary_DeprojectScreenToWorld_Statics::NewProp_ScreenPosition = { "ScreenPosition", nullptr, (EPropertyFlags)0x0010000008000182, UE4CodeGen_Private::EPropertyGenFlags::Struct, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(SRDisplayFunctionLibrary_eventDeprojectScreenToWorld_Parms, ScreenPosition), Z_Construct_UScriptStruct_FVector2D, METADATA_PARAMS(Z_Construct_UFunction_USRDisplayFunctionLibrary_DeprojectScreenToWorld_Statics::NewProp_ScreenPosition_MetaData, UE_ARRAY_COUNT(Z_Construct_UFunction_USRDisplayFunctionLibrary_DeprojectScreenToWorld_Statics::NewProp_ScreenPosition_MetaData)) };
	const UE4CodeGen_Private::FStructPropertyParams Z_Construct_UFunction_USRDisplayFunctionLibrary_DeprojectScreenToWorld_Statics::NewProp_WorldPosition = { "WorldPosition", nullptr, (EPropertyFlags)0x0010000000000180, UE4CodeGen_Private::EPropertyGenFlags::Struct, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(SRDisplayFunctionLibrary_eventDeprojectScreenToWorld_Parms, WorldPosition), Z_Construct_UScriptStruct_FVector, METADATA_PARAMS(nullptr, 0) };
	const UE4CodeGen_Private::FStructPropertyParams Z_Construct_UFunction_USRDisplayFunctionLibrary_DeprojectScreenToWorld_Statics::NewProp_WorldDirection = { "WorldDirection", nullptr, (EPropertyFlags)0x0010000000000180, UE4CodeGen_Private::EPropertyGenFlags::Struct, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(SRDisplayFunctionLibrary_eventDeprojectScreenToWorld_Parms, WorldDirection), Z_Construct_UScriptStruct_FVector, METADATA_PARAMS(nullptr, 0) };
	const UE4CodeGen_Private::FStructPropertyParams Z_Construct_UFunction_USRDisplayFunctionLibrary_DeprojectScreenToWorld_Statics::NewProp_CameraPosition = { "CameraPosition", nullptr, (EPropertyFlags)0x0010000000000180, UE4CodeGen_Private::EPropertyGenFlags::Struct, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(SRDisplayFunctionLibrary_eventDeprojectScreenToWorld_Parms, CameraPosition), Z_Construct_UScriptStruct_FVector, METADATA_PARAMS(nullptr, 0) };
	void Z_Construct_UFunction_USRDisplayFunctionLibrary_DeprojectScreenToWorld_Statics::NewProp_ReturnValue_SetBit(void* Obj)
	{
		((SRDisplayFunctionLibrary_eventDeprojectScreenToWorld_Parms*)Obj)->ReturnValue = 1;
	}
	const UE4CodeGen_Private::FBoolPropertyParams Z_Construct_UFunction_USRDisplayFunctionLibrary_DeprojectScreenToWorld_Statics::NewProp_ReturnValue = { "ReturnValue", nullptr, (EPropertyFlags)0x0010000000000580, UE4CodeGen_Private::EPropertyGenFlags::Bool | UE4CodeGen_Private::EPropertyGenFlags::NativeBool, RF_Public|RF_Transient|RF_MarkAsNative, 1, sizeof(bool), sizeof(SRDisplayFunctionLibrary_eventDeprojectScreenToWorld_Parms), &Z_Construct_UFunction_USRDisplayFunctionLibrary_DeprojectScreenToWorld_Statics::NewProp_ReturnValue_SetBit, METADATA_PARAMS(nullptr, 0) };
	const UE4CodeGen_Private::FPropertyParamsBase* const Z_Construct_UFunction_USRDisplayFunctionLibrary_DeprojectScreenToWorld_Statics::PropPointers[] = {
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_USRDisplayFunctionLibrary_DeprojectScreenToWorld_Statics::NewProp_Player,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_USRDisplayFunctionLibrary_DeprojectScreenToWorld_Statics::NewProp_ScreenPosition,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_USRDisplayFunctionLibrary_DeprojectScreenToWorld_Statics::NewProp_WorldPosition,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_USRDisplayFunctionLibrary_DeprojectScreenToWorld_Statics::NewProp_WorldDirection,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_USRDisplayFunctionLibrary_DeprojectScreenToWorld_Statics::NewProp_CameraPosition,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_USRDisplayFunctionLibrary_DeprojectScreenToWorld_Statics::NewProp_ReturnValue,
	};
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UFunction_USRDisplayFunctionLibrary_DeprojectScreenToWorld_Statics::Function_MetaDataParams[] = {
		{ "Category", "SRDisplay" },
		{ "Comment", "/**\n\x09 * @brief Transforms the given 2D screen space coordinate into a 3D world-space point and direction (The reference camera position is also acquired).\n\x09 */" },
		{ "ModuleRelativePath", "Public/Blueprint/SRDisplayFunctionLibrary.h" },
		{ "ToolTip", "@brief Transforms the given 2D screen space coordinate into a 3D world-space point and direction (The reference camera position is also acquired)." },
	};
#endif
	const UE4CodeGen_Private::FFunctionParams Z_Construct_UFunction_USRDisplayFunctionLibrary_DeprojectScreenToWorld_Statics::FuncParams = { (UObject*(*)())Z_Construct_UClass_USRDisplayFunctionLibrary, nullptr, "DeprojectScreenToWorld", nullptr, nullptr, sizeof(SRDisplayFunctionLibrary_eventDeprojectScreenToWorld_Parms), Z_Construct_UFunction_USRDisplayFunctionLibrary_DeprojectScreenToWorld_Statics::PropPointers, UE_ARRAY_COUNT(Z_Construct_UFunction_USRDisplayFunctionLibrary_DeprojectScreenToWorld_Statics::PropPointers), RF_Public|RF_Transient|RF_MarkAsNative, (EFunctionFlags)0x14C22401, 0, 0, METADATA_PARAMS(Z_Construct_UFunction_USRDisplayFunctionLibrary_DeprojectScreenToWorld_Statics::Function_MetaDataParams, UE_ARRAY_COUNT(Z_Construct_UFunction_USRDisplayFunctionLibrary_DeprojectScreenToWorld_Statics::Function_MetaDataParams)) };
	UFunction* Z_Construct_UFunction_USRDisplayFunctionLibrary_DeprojectScreenToWorld()
	{
		static UFunction* ReturnFunction = nullptr;
		if (!ReturnFunction)
		{
			UE4CodeGen_Private::ConstructUFunction(ReturnFunction, Z_Construct_UFunction_USRDisplayFunctionLibrary_DeprojectScreenToWorld_Statics::FuncParams);
		}
		return ReturnFunction;
	}
	struct Z_Construct_UFunction_USRDisplayFunctionLibrary_GetMousePosition_Statics
	{
		struct SRDisplayFunctionLibrary_eventGetMousePosition_Parms
		{
			float LocationX;
			float LocationY;
			bool ReturnValue;
		};
		static const UE4CodeGen_Private::FFloatPropertyParams NewProp_LocationX;
		static const UE4CodeGen_Private::FFloatPropertyParams NewProp_LocationY;
		static void NewProp_ReturnValue_SetBit(void* Obj);
		static const UE4CodeGen_Private::FBoolPropertyParams NewProp_ReturnValue;
		static const UE4CodeGen_Private::FPropertyParamsBase* const PropPointers[];
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam Function_MetaDataParams[];
#endif
		static const UE4CodeGen_Private::FFunctionParams FuncParams;
	};
	const UE4CodeGen_Private::FFloatPropertyParams Z_Construct_UFunction_USRDisplayFunctionLibrary_GetMousePosition_Statics::NewProp_LocationX = { "LocationX", nullptr, (EPropertyFlags)0x0010000000000180, UE4CodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(SRDisplayFunctionLibrary_eventGetMousePosition_Parms, LocationX), METADATA_PARAMS(nullptr, 0) };
	const UE4CodeGen_Private::FFloatPropertyParams Z_Construct_UFunction_USRDisplayFunctionLibrary_GetMousePosition_Statics::NewProp_LocationY = { "LocationY", nullptr, (EPropertyFlags)0x0010000000000180, UE4CodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(SRDisplayFunctionLibrary_eventGetMousePosition_Parms, LocationY), METADATA_PARAMS(nullptr, 0) };
	void Z_Construct_UFunction_USRDisplayFunctionLibrary_GetMousePosition_Statics::NewProp_ReturnValue_SetBit(void* Obj)
	{
		((SRDisplayFunctionLibrary_eventGetMousePosition_Parms*)Obj)->ReturnValue = 1;
	}
	const UE4CodeGen_Private::FBoolPropertyParams Z_Construct_UFunction_USRDisplayFunctionLibrary_GetMousePosition_Statics::NewProp_ReturnValue = { "ReturnValue", nullptr, (EPropertyFlags)0x0010000000000580, UE4CodeGen_Private::EPropertyGenFlags::Bool | UE4CodeGen_Private::EPropertyGenFlags::NativeBool, RF_Public|RF_Transient|RF_MarkAsNative, 1, sizeof(bool), sizeof(SRDisplayFunctionLibrary_eventGetMousePosition_Parms), &Z_Construct_UFunction_USRDisplayFunctionLibrary_GetMousePosition_Statics::NewProp_ReturnValue_SetBit, METADATA_PARAMS(nullptr, 0) };
	const UE4CodeGen_Private::FPropertyParamsBase* const Z_Construct_UFunction_USRDisplayFunctionLibrary_GetMousePosition_Statics::PropPointers[] = {
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_USRDisplayFunctionLibrary_GetMousePosition_Statics::NewProp_LocationX,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_USRDisplayFunctionLibrary_GetMousePosition_Statics::NewProp_LocationY,
		(const UE4CodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_USRDisplayFunctionLibrary_GetMousePosition_Statics::NewProp_ReturnValue,
	};
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UFunction_USRDisplayFunctionLibrary_GetMousePosition_Statics::Function_MetaDataParams[] = {
		{ "Category", "SRDisplay" },
		{ "Comment", "/**\n\x09 * @brief Retrieves the X and Y screen coordinates of the mouse cursor. Returns false if the mouse cursor is not over the window.\n\x09 */" },
		{ "ModuleRelativePath", "Public/Blueprint/SRDisplayFunctionLibrary.h" },
		{ "ToolTip", "@brief Retrieves the X and Y screen coordinates of the mouse cursor. Returns false if the mouse cursor is not over the window." },
	};
#endif
	const UE4CodeGen_Private::FFunctionParams Z_Construct_UFunction_USRDisplayFunctionLibrary_GetMousePosition_Statics::FuncParams = { (UObject*(*)())Z_Construct_UClass_USRDisplayFunctionLibrary, nullptr, "GetMousePosition", nullptr, nullptr, sizeof(SRDisplayFunctionLibrary_eventGetMousePosition_Parms), Z_Construct_UFunction_USRDisplayFunctionLibrary_GetMousePosition_Statics::PropPointers, UE_ARRAY_COUNT(Z_Construct_UFunction_USRDisplayFunctionLibrary_GetMousePosition_Statics::PropPointers), RF_Public|RF_Transient|RF_MarkAsNative, (EFunctionFlags)0x14422401, 0, 0, METADATA_PARAMS(Z_Construct_UFunction_USRDisplayFunctionLibrary_GetMousePosition_Statics::Function_MetaDataParams, UE_ARRAY_COUNT(Z_Construct_UFunction_USRDisplayFunctionLibrary_GetMousePosition_Statics::Function_MetaDataParams)) };
	UFunction* Z_Construct_UFunction_USRDisplayFunctionLibrary_GetMousePosition()
	{
		static UFunction* ReturnFunction = nullptr;
		if (!ReturnFunction)
		{
			UE4CodeGen_Private::ConstructUFunction(ReturnFunction, Z_Construct_UFunction_USRDisplayFunctionLibrary_GetMousePosition_Statics::FuncParams);
		}
		return ReturnFunction;
	}
	UClass* Z_Construct_UClass_USRDisplayFunctionLibrary_NoRegister()
	{
		return USRDisplayFunctionLibrary::StaticClass();
	}
	struct Z_Construct_UClass_USRDisplayFunctionLibrary_Statics
	{
		static UObject* (*const DependentSingletons[])();
		static const FClassFunctionLinkInfo FuncInfo[];
#if WITH_METADATA
		static const UE4CodeGen_Private::FMetaDataPairParam Class_MetaDataParams[];
#endif
		static const FCppClassTypeInfoStatic StaticCppClassTypeInfo;
		static const UE4CodeGen_Private::FClassParams ClassParams;
	};
	UObject* (*const Z_Construct_UClass_USRDisplayFunctionLibrary_Statics::DependentSingletons[])() = {
		(UObject* (*)())Z_Construct_UClass_UBlueprintFunctionLibrary,
		(UObject* (*)())Z_Construct_UPackage__Script_SRDisplayModule,
	};
	const FClassFunctionLinkInfo Z_Construct_UClass_USRDisplayFunctionLibrary_Statics::FuncInfo[] = {
		{ &Z_Construct_UFunction_USRDisplayFunctionLibrary_ConvertMouseLocationToWorldSpace, "ConvertMouseLocationToWorldSpace" }, // 1993287118
		{ &Z_Construct_UFunction_USRDisplayFunctionLibrary_DeprojectScreenToWorld, "DeprojectScreenToWorld" }, // 620325090
		{ &Z_Construct_UFunction_USRDisplayFunctionLibrary_GetMousePosition, "GetMousePosition" }, // 1992069175
	};
#if WITH_METADATA
	const UE4CodeGen_Private::FMetaDataPairParam Z_Construct_UClass_USRDisplayFunctionLibrary_Statics::Class_MetaDataParams[] = {
		{ "Comment", "/**\n * @brief TBD: Add brief description for this Blueprint Library Class.\n */" },
		{ "IncludePath", "Blueprint/SRDisplayFunctionLibrary.h" },
		{ "ModuleRelativePath", "Public/Blueprint/SRDisplayFunctionLibrary.h" },
		{ "ObjectInitializerConstructorDeclared", "" },
		{ "ToolTip", "@brief TBD: Add brief description for this Blueprint Library Class." },
	};
#endif
	const FCppClassTypeInfoStatic Z_Construct_UClass_USRDisplayFunctionLibrary_Statics::StaticCppClassTypeInfo = {
		TCppClassTypeTraits<USRDisplayFunctionLibrary>::IsAbstract,
	};
	const UE4CodeGen_Private::FClassParams Z_Construct_UClass_USRDisplayFunctionLibrary_Statics::ClassParams = {
		&USRDisplayFunctionLibrary::StaticClass,
		nullptr,
		&StaticCppClassTypeInfo,
		DependentSingletons,
		FuncInfo,
		nullptr,
		nullptr,
		UE_ARRAY_COUNT(DependentSingletons),
		UE_ARRAY_COUNT(FuncInfo),
		0,
		0,
		0x001000A0u,
		METADATA_PARAMS(Z_Construct_UClass_USRDisplayFunctionLibrary_Statics::Class_MetaDataParams, UE_ARRAY_COUNT(Z_Construct_UClass_USRDisplayFunctionLibrary_Statics::Class_MetaDataParams))
	};
	UClass* Z_Construct_UClass_USRDisplayFunctionLibrary()
	{
		static UClass* OuterClass = nullptr;
		if (!OuterClass)
		{
			UE4CodeGen_Private::ConstructUClass(OuterClass, Z_Construct_UClass_USRDisplayFunctionLibrary_Statics::ClassParams);
		}
		return OuterClass;
	}
	IMPLEMENT_CLASS(USRDisplayFunctionLibrary, 1098173015);
	template<> SRDISPLAYMODULE_API UClass* StaticClass<USRDisplayFunctionLibrary>()
	{
		return USRDisplayFunctionLibrary::StaticClass();
	}
	static FCompiledInDefer Z_CompiledInDefer_UClass_USRDisplayFunctionLibrary(Z_Construct_UClass_USRDisplayFunctionLibrary, &USRDisplayFunctionLibrary::StaticClass, TEXT("/Script/SRDisplayModule"), TEXT("USRDisplayFunctionLibrary"), false, nullptr, nullptr, nullptr);
	DEFINE_VTABLE_PTR_HELPER_CTOR(USRDisplayFunctionLibrary);
PRAGMA_ENABLE_DEPRECATION_WARNINGS
#ifdef _MSC_VER
#pragma warning (pop)
#endif
