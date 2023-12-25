// Copyright Epic Games, Inc. All Rights Reserved.

#include "SpatialLabsDevice.h"
#include "SpatialLabsDevice_Swapchain.h"
#include "SpatialLabsDevice_RenderBridge.h"
#include "OpenXRCore.h"
#include "OpenXRPlatformRHI.h"
#include "SpatialLabsFunctionLibrary.h"
#include "ISpatialLabsDevicePlugin.h"
#include "SpatialLabsConstants.h"

#include "Misc/App.h"
#include "Modules/ModuleManager.h"
#include "EngineGlobals.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "Engine/LocalPlayer.h"
#include "ISpatialLabsDevicePlugin.h"
#include "SceneRendering.h"
#include "PostProcess/PostProcessHMD.h"
#include "GameFramework/WorldSettings.h"
#include "Misc/CString.h"
#include "ClearQuad.h"
#include "XRThreadUtils.h"
#include "RenderUtils.h"
#include "PipelineStateCache.h"
#include "Slate/SceneViewport.h"
#include "Engine/GameEngine.h"
#include "BuildSettings.h"
#include "ARSystem.h"
#include "IHandTracker.h"

#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Engine/UserInterfaceSettings.h"

// Widget Layers
#include "Engine/TextureRenderTarget2D.h"
#include "Input/HittestGrid.h"

#if WITH_EDITOR
#include "Editor/UnrealEd/Classes/Editor/EditorEngine.h"
#endif

#define OPENXR_PAUSED_IDLE_FPS 10
static const int64 OPENXR_SWAPCHAIN_WAIT_TIMEOUT = 100000000ll;		// 100ms in nanoseconds.

#include "SpatialLabsCoreModule.h"
#include "SpatialLabsCoreSettings.h"

static TAutoConsoleVariable<int32> CVarEnableOpenXRValidationLayer(
	TEXT("xr.EnableOpenXRValidationLayer"),
	0,
	TEXT("If true, enables the OpenXR validation layer, which will provide extended validation of\nOpenXR API calls. This should only be used for debugging purposes.\n")
	TEXT("Changes will only take effect in new game/editor instances - can't be changed at runtime.\n"),
	ECVF_Default);		// @todo: Should we specify ECVF_Cheat here so this doesn't show up in release builds?

float FSpatialLabsDevice::WorldToMetersCache = 100.0f;

namespace {
	static TSet<XrEnvironmentBlendMode> SupportedBlendModes{ XR_ENVIRONMENT_BLEND_MODE_ALPHA_BLEND, XR_ENVIRONMENT_BLEND_MODE_ADDITIVE, XR_ENVIRONMENT_BLEND_MODE_OPAQUE };
	static TSet<XrViewConfigurationType> SupportedViewConfigurations{ XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO, XR_VIEW_CONFIGURATION_TYPE_PRIMARY_QUAD_VARJO };

	/** Helper function for acquiring the appropriate FSceneViewport */
	FSceneViewport* FindSceneViewport()
	{
		if (!GIsEditor)
		{
			UGameEngine* GameEngine = Cast<UGameEngine>(GEngine);
			return GameEngine->SceneViewport.Get();
		}
	#if WITH_EDITOR
		else
		{
			UEditorEngine* EditorEngine = CastChecked<UEditorEngine>(GEngine);
			FSceneViewport* PIEViewport = (FSceneViewport*)EditorEngine->GetPIEViewport();
			if (PIEViewport != nullptr && PIEViewport->IsStereoRenderingAllowed())
			{
				// PIE is setup for stereo rendering
				return PIEViewport;
			}
			else
			{
				// Check to see if the active editor viewport is drawing in stereo mode
				// @todo vreditor: Should work with even non-active viewport!
				FSceneViewport* EditorViewport = (FSceneViewport*)EditorEngine->GetActiveViewport();
				if (EditorViewport != nullptr && EditorViewport->IsStereoRenderingAllowed())
				{
					return EditorViewport;
				}
			}
		}
	#endif
		return nullptr;
	}
}

//---------------------------------------------------
// SpatialLabsDevice Plugin Implementation
//---------------------------------------------------

class FSpatialLabsDevicePlugin : public ISpatialLabsDevicePlugin
{
public:
	FSpatialLabsDevicePlugin()
		: LoaderHandle(nullptr)
		, Instance(XR_NULL_HANDLE)
		, System(XR_NULL_SYSTEM_ID)
		, RenderBridge(nullptr)
	{ }

	~FSpatialLabsDevicePlugin()
	{
	}

	/** IHeadMountedDisplayModule implementation */
	virtual TSharedPtr< class IXRTrackingSystem, ESPMode::ThreadSafe > CreateTrackingSystem() override;
	virtual TSharedPtr< IHeadMountedDisplayVulkanExtensions, ESPMode::ThreadSafe > GetVulkanExtensions() override;
	virtual uint64 GetGraphicsAdapterLuid() override;

	FString GetModuleKeyName() const override
	{
		return FString(TEXT("SpatialLabsDevice"));
	}

	void GetModuleAliases(TArray<FString>& AliasesOut) const override
	{
		AliasesOut.Add(TEXT("SpatialLabs"));
	}

	void ShutdownModule() override
	{
		if (Instance)
		{
			XR_ENSURE(xrDestroyInstance(Instance));
		}

		if (LoaderHandle)
		{
			FPlatformProcess::FreeDllHandle(LoaderHandle);
			LoaderHandle = nullptr;
		}
	}

	virtual bool IsHMDConnected() override { return true; }
	virtual bool IsStandaloneStereoOnlyDevice() override;
	virtual bool IsExtensionAvailable(const FString& Name) const override { return AvailableExtensions.Contains(Name); }
	virtual bool IsExtensionEnabled(const FString& Name) const override { return EnabledExtensions.Contains(Name); }
	virtual bool IsLayerAvailable(const FString& Name) const override { return EnabledLayers.Contains(Name); }
	virtual bool IsLayerEnabled(const FString& Name) const override { return EnabledLayers.Contains(Name); }

private:

	void *LoaderHandle;
	XrInstance Instance;
	XrSystemId System;
	TSet<FString> AvailableExtensions;
	TSet<FString> AvailableLayers;
	TArray<const char*> EnabledExtensions;
	TArray<const char*> EnabledLayers;
	TArray<IOpenXRExtensionPlugin*> ExtensionPlugins;
	TRefCountPtr<FOpenXRRenderBridge> RenderBridge;
	TSharedPtr< IHeadMountedDisplayVulkanExtensions, ESPMode::ThreadSafe > VulkanExtensions;

	bool EnumerateExtensions();
	bool EnumerateLayers();
	bool InitRenderBridge();
	bool InitInstanceAndSystem();
	bool InitInstance();
	bool InitSystem();
	PFN_xrGetInstanceProcAddr GetDefaultLoader();
	bool EnableExtensions(const TArray<const ANSICHAR*>& RequiredExtensions, const TArray<const ANSICHAR*>& OptionalExtensions, TArray<const ANSICHAR*>& OutExtensions);
	bool GetRequiredExtensions(TArray<const ANSICHAR*>& OutExtensions);
	bool GetOptionalExtensions(TArray<const ANSICHAR*>& OutExtensions);
};

IMPLEMENT_MODULE( FSpatialLabsDevicePlugin, SpatialLabsDevice )

TSharedPtr< class IXRTrackingSystem, ESPMode::ThreadSafe > FSpatialLabsDevicePlugin::CreateTrackingSystem()
{
	const auto& CoreSettings = FSpatialLabsCoreModule::Get().GetCoreSettings();
	bool IsDeviceAvailable = CoreSettings->GetIsDeviceAvailable();
	if (!IsDeviceAvailable)
	{
		return nullptr;
	}

	if (!RenderBridge)
	{
		if (!InitRenderBridge())
		{
			return nullptr;
		}
	}

	auto SpatialLabsDevice = FSceneViewExtensions::NewExtension<FSpatialLabsDevice>(Instance, System, RenderBridge, EnabledExtensions, ExtensionPlugins);
	if (SpatialLabsDevice->IsInitialized())
	{
		return SpatialLabsDevice;
	}

	return nullptr;
}

uint64 FSpatialLabsDevicePlugin::GetGraphicsAdapterLuid()
{
	if (!RenderBridge)
	{
		if (!InitRenderBridge())
		{
			return 0;
		}
	}
	return RenderBridge->GetGraphicsAdapterLuid();
}

TSharedPtr< IHeadMountedDisplayVulkanExtensions, ESPMode::ThreadSafe > FSpatialLabsDevicePlugin::GetVulkanExtensions()
{
#ifdef XR_USE_GRAPHICS_API_VULKAN
	if (InitInstanceAndSystem() && IsExtensionEnabled(XR_KHR_VULKAN_ENABLE_EXTENSION_NAME))
	{
		if (!VulkanExtensions.IsValid())
		{
			VulkanExtensions = MakeShareable(new FSpatialLabsDevice::FVulkanExtensions(Instance, System));
		}
		return VulkanExtensions;
	}
#endif//XR_USE_GRAPHICS_API_VULKAN
	return nullptr;
}

bool FSpatialLabsDevicePlugin::IsStandaloneStereoOnlyDevice()
{
	if (InitInstanceAndSystem())
	{

	}
	return false;
}

bool FSpatialLabsDevicePlugin::EnumerateExtensions()
{
	uint32_t ExtensionsCount = 0;
	if (XR_FAILED(xrEnumerateInstanceExtensionProperties(nullptr, 0, &ExtensionsCount, nullptr)))
	{
		// If it fails this early that means there's no runtime installed
		return false;
	}

	TArray<XrExtensionProperties> Properties;
	Properties.SetNum(ExtensionsCount);
	for (auto& Prop : Properties)
	{
		Prop = XrExtensionProperties{ XR_TYPE_EXTENSION_PROPERTIES };
	}

	if (XR_ENSURE(xrEnumerateInstanceExtensionProperties(nullptr, ExtensionsCount, &ExtensionsCount, Properties.GetData())))
	{
		for (const XrExtensionProperties& Prop : Properties)
		{
			AvailableExtensions.Add(Prop.extensionName);
		}
		return true;
	}
	return false;
}

bool FSpatialLabsDevicePlugin::EnumerateLayers()
{
	uint32 LayerPropertyCount = 0;
	if (XR_FAILED(xrEnumerateApiLayerProperties(0, &LayerPropertyCount, nullptr)))
	{
		// As per EnumerateExtensions - a failure here means no runtime installed.
		return false;
	}

	if (!LayerPropertyCount)
	{
		// It's still legit if we have no layers, so early out here (and return success) if so.
		return true;
	}

	TArray<XrApiLayerProperties> LayerProperties;
	LayerProperties.SetNum(LayerPropertyCount);
	for (auto& Prop : LayerProperties)
	{
		Prop = XrApiLayerProperties{ XR_TYPE_API_LAYER_PROPERTIES };
	}

	if (XR_ENSURE(xrEnumerateApiLayerProperties(LayerPropertyCount, &LayerPropertyCount, LayerProperties.GetData())))
	{
		for (const auto& Prop : LayerProperties)
		{
			AvailableLayers.Add(Prop.layerName);
		}
		return true;
	}

	return false;
}

struct AnsiKeyFunc : BaseKeyFuncs<const ANSICHAR*, const ANSICHAR*, false>
{
	typedef typename TTypeTraits<const ANSICHAR*>::ConstPointerType KeyInitType;
	typedef typename TCallTraits<const ANSICHAR*>::ParamType ElementInitType;

	/**
	 * @return The key used to index the given element.
	 */
	static FORCEINLINE KeyInitType GetSetKey(ElementInitType Element)
	{
		return Element;
	}

	/**
	 * @return True if the keys match.
	 */
	static FORCEINLINE bool Matches(KeyInitType A, KeyInitType B)
	{
		return FCStringAnsi::Strcmp(A, B) == 0;
	}

	/** Calculates a hash index for a key. */
	static FORCEINLINE uint32 GetKeyHash(KeyInitType Key)
	{
		return GetTypeHash(Key);
	}
};

bool FSpatialLabsDevicePlugin::InitRenderBridge()
{
	// Get all extension plugins
	TSet<const ANSICHAR*, AnsiKeyFunc> ExtensionSet;

	// Query all extension plugins to see if we need to use a custom render bridge
	PFN_xrGetInstanceProcAddr GetProcAddr = nullptr;

	FString RHIString = FApp::GetGraphicsRHI();
	if (RHIString.IsEmpty())
	{
		return false;
	}

	if (!InitInstanceAndSystem())
	{
		return false;
	}

#ifdef XR_USE_GRAPHICS_API_D3D11
	if (RHIString == TEXT("DirectX 11") && IsExtensionEnabled(XR_KHR_D3D11_ENABLE_EXTENSION_NAME))
	{
		RenderBridge = CreateRenderBridge_D3D11(Instance, System);
	}
	else
#endif
#ifdef XR_USE_GRAPHICS_API_D3D12
		if (RHIString == TEXT("DirectX 12") && IsExtensionEnabled(XR_KHR_D3D12_ENABLE_EXTENSION_NAME))
		{
			RenderBridge = CreateRenderBridge_D3D12(Instance, System);
		}
		else
#endif
#ifdef XR_USE_GRAPHICS_API_OPENGL
			if (RHIString == TEXT("OpenGL") && IsExtensionEnabled(XR_KHR_OPENGL_ENABLE_EXTENSION_NAME))
			{
				RenderBridge = CreateRenderBridge_OpenGL(Instance, System);
			}
			else
#endif
#ifdef XR_USE_GRAPHICS_API_VULKAN
				if (RHIString == TEXT("Vulkan") && IsExtensionEnabled(XR_KHR_VULKAN_ENABLE_EXTENSION_NAME))
				{
					RenderBridge = CreateRenderBridge_Vulkan(Instance, System);
				}
				else
#endif
				{
					UE_LOG(LogHMD, Warning, TEXT("%s is not currently supported by the OpenXR runtime"), *RHIString);
					return false;
				}
	return true;
}

PFN_xrGetInstanceProcAddr FSpatialLabsDevicePlugin::GetDefaultLoader()
{
#if PLATFORM_WINDOWS
#if !PLATFORM_CPU_X86_FAMILY
#error Windows platform does not currently support this CPU family. A OpenXR loader binary for this CPU family is needed.
#endif

#if PLATFORM_64BITS
	FString BinariesPath = FPaths::EngineDir() / FString(TEXT("Binaries/ThirdParty/OpenXR/win64"));
#else
	FString BinariesPath = FPaths::EngineDir() / FString(TEXT("Binaries/ThirdParty/OpenXR/win32"));
#endif

	FString LoaderName = "openxr_loader.dll";
	FPlatformProcess::PushDllDirectory(*BinariesPath);
	LoaderHandle = FPlatformProcess::GetDllHandle(*LoaderName);
	FPlatformProcess::PopDllDirectory(*BinariesPath);
#elif PLATFORM_LINUX
	FString BinariesPath = FPaths::EngineDir() / FString(TEXT("Binaries/ThirdParty/OpenXR/linux/x86_64-unknown-linux-gnu"));
	LoaderHandle = FPlatformProcess::GetDllHandle(*(BinariesPath / TEXT("libopenxr_loader.so")));
#elif PLATFORM_HOLOLENS
#ifndef PLATFORM_64BITS
#error HoloLens platform does not currently support 32-bit. 32-bit OpenXR loader binaries are needed.
#endif

#if PLATFORM_CPU_ARM_FAMILY
	FString BinariesPath = FPaths::EngineDir() / FString(TEXT("Binaries/ThirdParty/OpenXR/hololens/arm64"));
#elif PLATFORM_CPU_X86_FAMILY
	FString BinariesPath = FPaths::EngineDir() / FString(TEXT("Binaries/ThirdParty/OpenXR/hololens/x64"));
#else
#error Unsupported CPU family for the HoloLens platform.
#endif

	LoaderHandle = FPlatformProcess::GetDllHandle(*(BinariesPath / "openxr_loader.dll"));
#endif

	if (!LoaderHandle)
	{
		UE_LOG(LogHMD, Log, TEXT("Failed to load openxr_loader.dll"));
		return nullptr;
	}
	return (PFN_xrGetInstanceProcAddr)FPlatformProcess::GetDllExport(LoaderHandle, TEXT("xrGetInstanceProcAddr"));
}

bool FSpatialLabsDevicePlugin::EnableExtensions(const TArray<const ANSICHAR*>& RequiredExtensions, const TArray<const ANSICHAR*>& OptionalExtensions, TArray<const ANSICHAR*>& OutExtensions)
{
	// Query required extensions and check if they're all available
	bool ExtensionMissing = false;
	for (const ANSICHAR* Ext : RequiredExtensions)
	{
		if (AvailableExtensions.Contains(Ext))
		{
			UE_LOG(LogHMD, Verbose, TEXT("Required extension %S enabled"), Ext);
		}
		else
		{
			UE_LOG(LogHMD, Warning, TEXT("Required extension %S is not available"), Ext);
			ExtensionMissing = true;
		}
	}

	// If any required extensions are missing then we ignore the plugin
	if (ExtensionMissing)
	{
		return false;
	}

	// All required extensions are supported we can safely add them to our set and give the plugin callbacks
	OutExtensions.Append(RequiredExtensions);

	// Add all supported optional extensions to the set
	for (const ANSICHAR* Ext : OptionalExtensions)
	{
		if (AvailableExtensions.Contains(Ext))
		{
			UE_LOG(LogHMD, Verbose, TEXT("Optional extension %S enabled"), Ext);
			OutExtensions.Add(Ext);
		}
		else
		{
			UE_LOG(LogHMD, Log, TEXT("Optional extension %S is not available"), Ext);
		}
	}

	return true;
}

bool FSpatialLabsDevicePlugin::GetRequiredExtensions(TArray<const ANSICHAR*>& OutExtensions)
{
#if PLATFORM_ANDROID
	OutExtensions.Add(XR_KHR_ANDROID_CREATE_INSTANCE_EXTENSION_NAME);
#endif
	return true;
}

bool FSpatialLabsDevicePlugin::GetOptionalExtensions(TArray<const ANSICHAR*>& OutExtensions)
{
#ifdef XR_USE_GRAPHICS_API_D3D11
	OutExtensions.Add(XR_KHR_D3D11_ENABLE_EXTENSION_NAME);
#endif
#ifdef XR_USE_GRAPHICS_API_D3D12
	OutExtensions.Add(XR_KHR_D3D12_ENABLE_EXTENSION_NAME);
#endif
#ifdef XR_USE_GRAPHICS_API_OPENGL
	OutExtensions.Add(XR_KHR_OPENGL_ENABLE_EXTENSION_NAME);
#endif
#ifdef XR_USE_GRAPHICS_API_VULKAN
	OutExtensions.Add(XR_KHR_VULKAN_ENABLE_EXTENSION_NAME);
#endif
	OutExtensions.Add(XR_KHR_COMPOSITION_LAYER_DEPTH_EXTENSION_NAME);
	OutExtensions.Add(XR_VARJO_QUAD_VIEWS_EXTENSION_NAME);
	OutExtensions.Add(XR_KHR_VISIBILITY_MASK_EXTENSION_NAME);
	return true;
}

bool FSpatialLabsDevicePlugin::InitInstanceAndSystem()
{
	if (!Instance && !InitInstance())
	{
		return false;
	}

	if (!System && !InitSystem())
	{
		return false;
	}

	return true;
}

bool FSpatialLabsDevicePlugin::InitInstance()
{
	// This should only ever be called if we don't already have an instance.
	check(!Instance);

	// Get all extension plugins
	TSet<const ANSICHAR*, AnsiKeyFunc> ExtensionSet;

	// Query all extension plugins to see if we need to use a custom loader
	PFN_xrGetInstanceProcAddr GetProcAddr = nullptr;

	if (!GetProcAddr)
	{
		GetProcAddr = GetDefaultLoader();
	}

	if (!PreInitOpenXRCore(GetProcAddr))
	{
		UE_LOG(LogHMD, Log, TEXT("Failed to initialize core functions. Please check that you have a valid OpenXR runtime installed."));
		return false;
	}

	if (!EnumerateExtensions())
	{
		UE_LOG(LogHMD, Log, TEXT("Failed to enumerate extensions. Please check that you have a valid OpenXR runtime installed."));
		return false;
	}

	if (!EnumerateLayers())
	{
		UE_LOG(LogHMD, Log, TEXT("Failed to enumerate API layers. Please check that you have a valid OpenXR runtime installed."));
		return false;
	}

	// Enable any required and optional extensions that are not plugin specific (usually platform support extensions)
	{
		TArray<const ANSICHAR*> RequiredExtensions, OptionalExtensions, Extensions;
		// Query required extensions
		RequiredExtensions.Empty();
		if (!GetRequiredExtensions(RequiredExtensions))
		{
			UE_LOG(LogHMD, Error, TEXT("Could not get required OpenXR extensions."));
			return false;
		}

		// Query optional extensions
		OptionalExtensions.Empty();
		if (!GetOptionalExtensions(OptionalExtensions))
		{
			UE_LOG(LogHMD, Error, TEXT("Could not get optional OpenXR extensions."));
			return false;
		}

		if (!EnableExtensions(RequiredExtensions, OptionalExtensions, Extensions))
		{
			UE_LOG(LogHMD, Error, TEXT("Could not enable all required OpenXR extensions."));
			return false;
		}
		ExtensionSet.Append(Extensions);
	}

	if (AvailableExtensions.Contains(XR_EPIC_VIEW_CONFIGURATION_FOV_EXTENSION_NAME))
	{
		ExtensionSet.Add(XR_EPIC_VIEW_CONFIGURATION_FOV_EXTENSION_NAME);
	}

	EnabledExtensions.Reset();
	for (const ANSICHAR* Ext : ExtensionSet)
	{
		EnabledExtensions.Add(Ext);
	}

	// Enable layers, if specified by CVar.
	// Note: For the validation layer to work on Windows (as of latest OpenXR runtime, August 2019), the following are required:
	//   1. Download and build the OpenXR SDK from https://github.com/KhronosGroup/OpenXR-SDK-Source (follow instructions at https://github.com/KhronosGroup/OpenXR-SDK-Source/blob/master/BUILDING.md)
	//	 2. Add a registry key under HKEY_LOCAL_MACHINE\SOFTWARE\Khronos\OpenXR\1\ApiLayers\Explicit, containing the path to the manifest file
	//      (e.g. C:\OpenXR-SDK-Source-master\build\win64\src\api_layers\XrApiLayer_core_validation.json) <-- this file is downloaded as part of the SDK source, above
	//   3. Copy the DLL from the build target at, for example, C:\OpenXR-SDK-Source-master\build\win64\src\api_layers\XrApiLayer_core_validation.dll to
	//      somewhere in your system path (e.g. c:\windows\system32); the OpenXR loader currently doesn't use the path the json file is in (this is a bug)

	const bool bEnableOpenXRValidationLayer = CVarEnableOpenXRValidationLayer.GetValueOnAnyThread() != 0;
	TArray<const char*> Layers;
	if (bEnableOpenXRValidationLayer && AvailableLayers.Contains("XR_APILAYER_LUNARG_core_validation"))
	{
		Layers.Add("XR_APILAYER_LUNARG_core_validation");
	}

	// Engine registration can be disabled via console var.
	auto* CVarDisableEngineAndAppRegistration = IConsoleManager::Get().FindTConsoleVariableDataInt(TEXT("r.DisableEngineAndAppRegistration"));
	bool bDisableEngineRegistration = (CVarDisableEngineAndAppRegistration && CVarDisableEngineAndAppRegistration->GetValueOnAnyThread() != 0);

	FText ProjectName = FText();
	GConfig->GetText(TEXT("/Script/EngineSettings.GeneralProjectSettings"), TEXT("ProjectName"), ProjectName, GGameIni);

	FText ProjectVersion = FText();
	GConfig->GetText(TEXT("/Script/EngineSettings.GeneralProjectSettings"), TEXT("ProjectVersion"), ProjectVersion, GGameIni);

	// EngineName will be of the form "UnrealEngine4.21", with the minor version ("21" in this example)
	// updated with every quarterly release
	FString EngineName = bDisableEngineRegistration ? FString("") : FApp::GetEpicProductIdentifier() + FEngineVersion::Current().ToString(EVersionComponent::Minor);
	FString AppName = bDisableEngineRegistration ? TEXT("") : ProjectName.ToString() + ProjectVersion.ToString();

	XrInstanceCreateInfo Info;
	Info.type = XR_TYPE_INSTANCE_CREATE_INFO;
	Info.next = nullptr;
	Info.createFlags = 0;
	FTCHARToUTF8_Convert::Convert(Info.applicationInfo.applicationName, XR_MAX_APPLICATION_NAME_SIZE, *AppName, AppName.Len() + 1);
	Info.applicationInfo.applicationVersion = static_cast<uint32>(BuildSettings::GetCurrentChangelist()) | (BuildSettings::IsLicenseeVersion() ? 0x80000000 : 0);
	FTCHARToUTF8_Convert::Convert(Info.applicationInfo.engineName, XR_MAX_ENGINE_NAME_SIZE, *EngineName, EngineName.Len() + 1);
	Info.applicationInfo.engineVersion = (uint32)(FEngineVersion::Current().GetMinor() << 16 | FEngineVersion::Current().GetPatch());
	Info.applicationInfo.apiVersion = XR_CURRENT_API_VERSION;

	Info.enabledApiLayerCount = Layers.Num();
	Info.enabledApiLayerNames = Layers.GetData();

	Info.enabledExtensionCount = EnabledExtensions.Num();
	Info.enabledExtensionNames = EnabledExtensions.GetData();

#if PLATFORM_ANDROID
	XrInstanceCreateInfoAndroidKHR InstanceCreateInfoAndroid;
	InstanceCreateInfoAndroid.type = XR_TYPE_INSTANCE_CREATE_INFO_ANDROID_KHR;
	InstanceCreateInfoAndroid.next = nullptr;
	InstanceCreateInfoAndroid.applicationVM = GNativeAndroidApp->activity->vm;
	InstanceCreateInfoAndroid.applicationActivity = GNativeAndroidApp->activity->clazz;
	Info.next = &InstanceCreateInfoAndroid;
#endif // PLATFORM_ANDROID

	XrResult Result = xrCreateInstance(&Info, &Instance);
	if (XR_FAILED(Result))
	{
		UE_LOG(LogHMD, Log, TEXT("Failed to create an OpenXR instance, result is %s. Please check if you have an OpenXR runtime installed. The following extensions were enabled:"), OpenXRResultToString(Result));
		for (const char* Extension : EnabledExtensions)
		{
			UE_LOG(LogHMD, Log, TEXT("- %S"), Extension);
		}
		return false;
	}

	if (!InitOpenXRCore(Instance))
	{
		UE_LOG(LogHMD, Log, TEXT("Failed to initialize core functions. Please check that you have a valid OpenXR runtime installed."));
		return false;
	}

	return true;
}

bool FSpatialLabsDevicePlugin::InitSystem()
{
	XrSystemGetInfo SystemInfo;
	SystemInfo.type = XR_TYPE_SYSTEM_GET_INFO;
	SystemInfo.next = nullptr;
	SystemInfo.formFactor = XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY;

	XrResult Result = xrGetSystem(Instance, &SystemInfo, &System);
	if (XR_FAILED(Result))
	{
		UE_LOG(LogHMD, Log, TEXT("Failed to get an OpenXR system, result is %s. Please check that your runtime supports VR headsets."), OpenXRResultToString(Result));
		return false;
	}

	return true;
}

//---------------------------------------------------
// SpatialLabsDevice IHeadMountedDisplay Implementation
//---------------------------------------------------

bool FSpatialLabsDevice::FVulkanExtensions::GetVulkanInstanceExtensionsRequired(TArray<const ANSICHAR*>& Out)
{
#ifdef XR_USE_GRAPHICS_API_VULKAN
	TArray<VkExtensionProperties> Properties;
	{
		uint32_t PropertyCount;
		VulkanRHI::vkEnumerateInstanceExtensionProperties(nullptr, &PropertyCount, nullptr);
		Properties.SetNum(PropertyCount);
		VulkanRHI::vkEnumerateInstanceExtensionProperties(nullptr, &PropertyCount, Properties.GetData());
	}

	{
		PFN_xrGetVulkanInstanceExtensionsKHR GetVulkanInstanceExtensionsKHR;
		XR_ENSURE(xrGetInstanceProcAddr(Instance, "xrGetVulkanInstanceExtensionsKHR", (PFN_xrVoidFunction*)&GetVulkanInstanceExtensionsKHR));

		uint32 ExtensionCount = 0;
		XR_ENSURE(GetVulkanInstanceExtensionsKHR(Instance, System, 0, &ExtensionCount, nullptr));
		Extensions.SetNum(ExtensionCount);
		XR_ENSURE(GetVulkanInstanceExtensionsKHR(Instance, System, ExtensionCount, &ExtensionCount, Extensions.GetData()));
	}

	ANSICHAR* Context = nullptr;
	for (ANSICHAR* Tok = FCStringAnsi::Strtok(Extensions.GetData(), " ", &Context); Tok != nullptr; Tok = FCStringAnsi::Strtok(nullptr, " ", &Context))
	{
		bool ExtensionFound = false;
		for (int32 PropertyIndex = 0; PropertyIndex < Properties.Num(); PropertyIndex++)
		{
			const char* PropertyExtensionName = Properties[PropertyIndex].extensionName;

			if (!FCStringAnsi::Strcmp(PropertyExtensionName, Tok))
			{
				Out.Add(Tok);
				ExtensionFound = true;
				break;
			}
		}

		if (!ExtensionFound)
		{
			UE_LOG(LogHMD, Log, TEXT("Missing required Vulkan instance extension %S."), Tok);
			return false;
		}
	}
#endif
	return true;
}

bool FSpatialLabsDevice::FVulkanExtensions::GetVulkanDeviceExtensionsRequired(VkPhysicalDevice_T *pPhysicalDevice, TArray<const ANSICHAR*>& Out)
{
#ifdef XR_USE_GRAPHICS_API_VULKAN
	TArray<VkExtensionProperties> Properties;
	{
		uint32_t PropertyCount;
		VulkanRHI::vkEnumerateDeviceExtensionProperties((VkPhysicalDevice)pPhysicalDevice, nullptr, &PropertyCount, nullptr);
		Properties.SetNum(PropertyCount);
		VulkanRHI::vkEnumerateDeviceExtensionProperties((VkPhysicalDevice)pPhysicalDevice, nullptr, &PropertyCount, Properties.GetData());
	}

	{
		PFN_xrGetVulkanDeviceExtensionsKHR GetVulkanDeviceExtensionsKHR;
		XR_ENSURE(xrGetInstanceProcAddr(Instance, "xrGetVulkanDeviceExtensionsKHR", (PFN_xrVoidFunction*)&GetVulkanDeviceExtensionsKHR));

		uint32 ExtensionCount = 0;
		XR_ENSURE(GetVulkanDeviceExtensionsKHR(Instance, System, 0, &ExtensionCount, nullptr));
		DeviceExtensions.SetNum(ExtensionCount);
		XR_ENSURE(GetVulkanDeviceExtensionsKHR(Instance, System, ExtensionCount, &ExtensionCount, DeviceExtensions.GetData()));
	}

	ANSICHAR* Context = nullptr;
	for (ANSICHAR* Tok = FCStringAnsi::Strtok(DeviceExtensions.GetData(), " ", &Context); Tok != nullptr; Tok = FCStringAnsi::Strtok(nullptr, " ", &Context))
	{
		bool ExtensionFound = false;
		for (int32 PropertyIndex = 0; PropertyIndex < Properties.Num(); PropertyIndex++)
		{
			const char* PropertyExtensionName = Properties[PropertyIndex].extensionName;

			if (!FCStringAnsi::Strcmp(PropertyExtensionName, Tok))
			{
				Out.Add(Tok);
				ExtensionFound = true;
				break;
			}
		}

		if (!ExtensionFound)
		{
			UE_LOG(LogHMD, Log, TEXT("Missing required Vulkan device extension %S."), Tok);
			return false;
		}
	}
#endif
	return true;
}

void FSpatialLabsDevice::GetMotionControllerData(UObject* WorldContext, const EControllerHand Hand, FXRMotionControllerData& MotionControllerData)
{
	return;
}

float FSpatialLabsDevice::GetWorldToMetersScale() const
{
	float WorldToMeters = GWorld->GetWorldSettings()->WorldToMeters;

	return WorldToMeters;
}

FVector2D FSpatialLabsDevice::GetPlayAreaBounds(EHMDTrackingOrigin::Type Origin) const
{
	return FVector2D::ZeroVector;
}

float FSpatialLabsDevice::GetScaledWorldToMetersScale() const
{
	FSpatialLabsCoreModule* SpatialLabsCore = static_cast<FSpatialLabsCoreModule*>(FModuleManager::Get().GetModule("SpatialLabsCore"));
	float ScaleFactor = SpatialLabsCore->GetCoreSettings()->GetScaleFactor();

	float ScaleWorldToMeters = WorldToMetersCache * ScaleFactor;	

	return ScaleWorldToMeters;
}

FName FSpatialLabsDevice::GetHMDName() const
{
	return SystemProperties.systemName;
}

FString FSpatialLabsDevice::GetVersionString() const
{
	return FString::Printf(TEXT("%s: %d.%d.%d"),
		UTF8_TO_TCHAR(InstanceProperties.runtimeName),
		XR_VERSION_MAJOR(InstanceProperties.runtimeVersion),
		XR_VERSION_MINOR(InstanceProperties.runtimeVersion),
		XR_VERSION_PATCH(InstanceProperties.runtimeVersion));
}

bool FSpatialLabsDevice::IsHMDEnabled() const
{
	return true;
}

void FSpatialLabsDevice::EnableHMD(bool enable)
{
}

bool FSpatialLabsDevice::GetHMDMonitorInfo(MonitorInfo& MonitorDesc)
{
	MonitorDesc.MonitorName = UTF8_TO_TCHAR(SystemProperties.systemName);
	MonitorDesc.MonitorId = 0;

	FIntPoint RTSize = GetIdealRenderTargetSize();
	MonitorDesc.DesktopX = MonitorDesc.DesktopY = 0;
	MonitorDesc.ResolutionX = MonitorDesc.WindowSizeX = RTSize.X;//??
	MonitorDesc.ResolutionY = MonitorDesc.WindowSizeY = RTSize.Y;//??
	return true;
}

void FSpatialLabsDevice::GetFieldOfView(float& OutHFOVInDegrees, float& OutVFOVInDegrees) const
{
	const FPipelinedFrameState& FrameState = GetPipelinedFrameStateForThread();

	XrFovf UnifiedFov = { 0.0f };
	for (const XrView& View : FrameState.Views)
	{
		UnifiedFov.angleLeft = FMath::Min(UnifiedFov.angleLeft, View.fov.angleLeft);
		UnifiedFov.angleRight = FMath::Max(UnifiedFov.angleRight, View.fov.angleRight);
		UnifiedFov.angleUp = FMath::Max(UnifiedFov.angleUp, View.fov.angleUp);
		UnifiedFov.angleDown = FMath::Min(UnifiedFov.angleDown, View.fov.angleDown);
	}
	OutHFOVInDegrees = FMath::RadiansToDegrees(UnifiedFov.angleRight - UnifiedFov.angleLeft);
	OutVFOVInDegrees = FMath::RadiansToDegrees(UnifiedFov.angleUp - UnifiedFov.angleDown);
}

bool FSpatialLabsDevice::EnumerateTrackedDevices(TArray<int32>& OutDevices, EXRTrackedDeviceType Type)
{
	if (Type == EXRTrackedDeviceType::Any || Type == EXRTrackedDeviceType::HeadMountedDisplay)
	{
		OutDevices.Add(IXRTrackingSystem::HMDDeviceId);
	}
	if (Type == EXRTrackedDeviceType::Any || Type == EXRTrackedDeviceType::Controller)
	{
		FReadScopeLock DeviceLock(DeviceMutex);

		// Skip the HMD, we already added it to the list
		for (int32 i = 1; i < DeviceSpaces.Num(); i++)
		{
			OutDevices.Add(i);
		}
	}
	return OutDevices.Num() > 0;
}

void FSpatialLabsDevice::SetInterpupillaryDistance(float NewInterpupillaryDistance)
{
}

float FSpatialLabsDevice::GetInterpupillaryDistance() const
{
	const FPipelinedFrameState& FrameState = GetPipelinedFrameStateForThread();
	if (FrameState.Views.Num() < 2)
	{
		return 0.064f;
	}

	FVector leftPos = ToFVector(FrameState.Views[0].pose.position);
	FVector rightPos = ToFVector(FrameState.Views[1].pose.position);
	return FVector::Dist(leftPos, rightPos);
}

bool FSpatialLabsDevice::GetIsTracked(int32 DeviceId)
{
	// This function is called from both the game and rendering thread and each thread maintains separate pose
	// snapshots to prevent inconsistent poses (tearing) on the same frame.
	const FPipelinedFrameState& PipelineState = GetPipelinedFrameStateForThread();

	if (!PipelineState.DeviceLocations.IsValidIndex(DeviceId))
	{
		return false;
	}

	const XrSpaceLocation& Location = PipelineState.DeviceLocations[DeviceId];
	return Location.locationFlags & XR_SPACE_LOCATION_ORIENTATION_TRACKED_BIT &&
		Location.locationFlags & XR_SPACE_LOCATION_POSITION_TRACKED_BIT;
}

bool FSpatialLabsDevice::GetCurrentPose(int32 DeviceId, FQuat& CurrentOrientation, FVector& CurrentPosition)
{
	CurrentOrientation = FQuat::Identity;
	CurrentPosition = FVector::ZeroVector;

	return true;
}

bool FSpatialLabsDevice::GetPoseForTime(int32 DeviceId, FTimespan Timespan, FQuat& Orientation, FVector& Position, bool& bProvidedLinearVelocity, FVector& LinearVelocity, bool& bProvidedAngularVelocity, FVector& AngularVelocityRadPerSec)
{
	FPipelinedFrameState& PipelineState = GetPipelinedFrameStateForThread();

	FReadScopeLock DeviceLock(DeviceMutex);
	if (!DeviceSpaces.IsValidIndex(DeviceId))
	{
		return false;
	}

	XrTime TargetTime = ToXrTime(Timespan);

	const FDeviceSpace& DeviceSpace = DeviceSpaces[DeviceId];

	XrSpaceVelocity DeviceVelocity{ XR_TYPE_SPACE_VELOCITY };
	XrSpaceLocation DeviceLocation{ XR_TYPE_SPACE_LOCATION, &DeviceVelocity };

	XR_ENSURE(xrLocateSpace(DeviceSpace.Space, PipelineState.TrackingSpace, TargetTime, &DeviceLocation));

	if (DeviceLocation.locationFlags & XR_SPACE_LOCATION_ORIENTATION_VALID_BIT &&
		DeviceLocation.locationFlags & XR_SPACE_LOCATION_POSITION_VALID_BIT)
	{
		Orientation = ToFQuat(DeviceLocation.pose.orientation);
		Position = ToFVector(DeviceLocation.pose.position, GetWorldToMetersScale());

		if (DeviceVelocity.velocityFlags & XR_SPACE_VELOCITY_LINEAR_VALID_BIT)
		{
			bProvidedLinearVelocity = true;
			LinearVelocity = ToFVector(DeviceVelocity.linearVelocity, GetWorldToMetersScale());
		}
		if (DeviceVelocity.velocityFlags & XR_SPACE_VELOCITY_ANGULAR_VALID_BIT)
		{
			bProvidedAngularVelocity = true;
			AngularVelocityRadPerSec = ToFVector(DeviceVelocity.angularVelocity);
		}

		return true;
	}

	return false;
}

bool FSpatialLabsDevice::IsChromaAbCorrectionEnabled() const
{
	return false;
}

void FSpatialLabsDevice::ResetOrientationAndPosition(float yaw)
{
	ResetOrientation(yaw);
	ResetPosition();
}

void FSpatialLabsDevice::ResetOrientation(float Yaw)
{
}

void FSpatialLabsDevice::ResetPosition()
{
}

void FSpatialLabsDevice::SetBaseRotation(const FRotator& BaseRot)
{
}

FRotator FSpatialLabsDevice::GetBaseRotation() const
{
	return FRotator::ZeroRotator;
}

void FSpatialLabsDevice::SetBaseOrientation(const FQuat& BaseOrient)
{
}

FQuat FSpatialLabsDevice::GetBaseOrientation() const
{
	return FQuat::Identity;
}

bool FSpatialLabsDevice::IsStereoEnabled() const
{
	return bStereoEnabled;
}

bool FSpatialLabsDevice::EnableStereo(bool stereo)
{
	if (stereo == bStereoEnabled)
	{
		return true;
	}

	bStereoEnabled = stereo;
	if (stereo)
	{
		GEngine->bForceDisableFrameRateSmoothing = true;
		if (OnStereoStartup())
		{
			StartSession();

			FApp::SetUseVRFocus(true);
			FApp::SetHasVRFocus(true);

			return true;
		}
		return false;
	}
	else
	{
		GEngine->bForceDisableFrameRateSmoothing = false;

		FApp::SetUseVRFocus(false);
		FApp::SetHasVRFocus(false);

#if WITH_EDITOR
		if (GIsEditor)
		{
			if (FSceneViewport* SceneVP = FindSceneViewport())
			{
				TSharedPtr<SWindow> Window = SceneVP->FindWindow();
				if (Window.IsValid())
				{
					Window->SetViewportSizeDrivenByWindow(true);
				}
			}
		}
#endif // WITH_EDITOR

		return OnStereoTeardown();
	}
}

void FSpatialLabsDevice::AdjustViewRect(EStereoscopicPass StereoPass, int32& X, int32& Y, uint32& SizeX, uint32& SizeY) const
{
	const uint32 ViewIndex = GetViewIndexForPass(StereoPass);

	const FPipelinedFrameState& PipelineState = GetPipelinedFrameStateForThread();
	const XrViewConfigurationView& Config = PipelineState.ViewConfigs[ViewIndex];
	FIntPoint ViewRectMin(EForceInit::ForceInitToZero);

	// If Mobile Multi-View is active the first two views will share the same position
	// Thus the start index should be the second view if enabled
	for (uint32 i = bIsMobileMultiViewEnabled ? 1 : 0; i < ViewIndex; ++i)
	{
		ViewRectMin.X += PipelineState.ViewConfigs[i].recommendedImageRectWidth;
	}
	QuantizeSceneBufferSize(ViewRectMin, ViewRectMin);

	X = ViewRectMin.X;
	Y = ViewRectMin.Y;
	SizeX = Config.recommendedImageRectWidth;
	SizeY = Config.recommendedImageRectHeight;
}

void FSpatialLabsDevice::SetFinalViewRect(FRHICommandListImmediate& RHICmdList, const enum EStereoscopicPass StereoPass, const FIntRect& FinalViewRect)
{
	if (StereoPass == eSSP_FULL)
	{
		return;
	}

	int32 ViewIndex = GetViewIndexForPass(StereoPass);
	float NearZ = GNearClippingPlane / GetWorldToMetersScale();

	XrSwapchainSubImage& ColorImage = PipelinedLayerStateRendering.ColorImages[ViewIndex];
	ColorImage.swapchain = PipelinedLayerStateRendering.ColorSwapchain.IsValid() ? static_cast<FOpenXRSwapchain*>(PipelinedLayerStateRendering.ColorSwapchain.Get())->GetHandle() : XR_NULL_HANDLE;
	ColorImage.imageArrayIndex = bIsMobileMultiViewEnabled && ViewIndex < 2 ? ViewIndex : 0;
	ColorImage.imageRect = {
		{ FinalViewRect.Min.X, FinalViewRect.Min.Y },
		{ FinalViewRect.Width(), FinalViewRect.Height() }
	};

	XrSwapchainSubImage& DepthImage = PipelinedLayerStateRendering.DepthImages[ViewIndex];
	if (bDepthExtensionSupported)
	{
		DepthImage.swapchain = PipelinedLayerStateRendering.DepthSwapchain.IsValid() ? static_cast<FOpenXRSwapchain*>(PipelinedLayerStateRendering.DepthSwapchain.Get())->GetHandle() : XR_NULL_HANDLE;
		DepthImage.imageArrayIndex = bIsMobileMultiViewEnabled && ViewIndex < 2 ? ViewIndex : 0;
		DepthImage.imageRect = ColorImage.imageRect;
	}

	if (!PipelinedFrameStateRendering.PluginViews.IsValidIndex(ViewIndex))
	{
		// This plugin is no longer providing this view.
		return;
	}

	if (PipelinedFrameStateRendering.PluginViews[ViewIndex])
	{
		// Defer to the plugin to handle submission
		return;
	}

	XrCompositionLayerProjectionView& Projection = PipelinedLayerStateRendering.ProjectionLayers[ViewIndex];
	XrCompositionLayerDepthInfoKHR& DepthLayer = PipelinedLayerStateRendering.DepthLayers[ViewIndex];

	Projection.type = XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW;
	Projection.next = nullptr;
	Projection.subImage = ColorImage;

	if (bDepthExtensionSupported && PipelinedLayerStateRendering.DepthSwapchain.IsValid())
	{
		DepthLayer.type = XR_TYPE_COMPOSITION_LAYER_DEPTH_INFO_KHR;
		DepthLayer.next = nullptr;
		DepthLayer.subImage = DepthImage;
		DepthLayer.minDepth = 0.0f;
		DepthLayer.maxDepth = 1.0f;
		DepthLayer.nearZ = FLT_MAX;
		DepthLayer.farZ = NearZ;

		Projection.next = &DepthLayer;
	}

	RHICmdList.EnqueueLambda([this, LayerState = PipelinedLayerStateRendering](FRHICommandListImmediate& InRHICmdList)
		{
			PipelinedLayerStateRHI = LayerState;
		});
}

EStereoscopicPass FSpatialLabsDevice::GetViewPassForIndex(bool bStereoRequested, uint32 ViewIndex) const
{
	if (!bStereoRequested)
		return EStereoscopicPass::eSSP_FULL;

	return static_cast<EStereoscopicPass>(eSSP_LEFT_EYE + ViewIndex);
}

uint32 FSpatialLabsDevice::GetViewIndexForPass(EStereoscopicPass StereoPassType) const
{
	switch (StereoPassType)
	{
	case eSSP_LEFT_EYE:
	case eSSP_FULL:
		return 0;

	case eSSP_RIGHT_EYE:
		return 1;

	default:
		return StereoPassType - eSSP_LEFT_EYE;
	}
}

uint32 FSpatialLabsDevice::DeviceGetLODViewIndex() const
{
	if (SelectedViewConfigurationType == XR_VIEW_CONFIGURATION_TYPE_PRIMARY_QUAD_VARJO)
	{
		return GetViewIndexForPass(eSSP_LEFT_EYE_SIDE);
	}
	return IStereoRendering::DeviceGetLODViewIndex();
}

bool FSpatialLabsDevice::DeviceIsAPrimaryPass(EStereoscopicPass Pass)
{
	uint32 ViewIndex = GetViewIndexForPass(Pass);
	const FPipelinedFrameState& PipelineState = GetPipelinedFrameStateForThread();
	if (PipelineState.PluginViews.IsValidIndex(ViewIndex) && PipelineState.PluginViews[ViewIndex])
	{
		// Views provided by a plugin should be considered a new primary pass
		return true;
	}

	return Pass == EStereoscopicPass::eSSP_FULL || Pass == EStereoscopicPass::eSSP_LEFT_EYE;
}

int32 FSpatialLabsDevice::GetDesiredNumberOfViews(bool bStereoRequested) const
{
	const FPipelinedFrameState& FrameState = GetPipelinedFrameStateForThread();

	// FIXME: Monoscopic actually needs 2 views for quad vr
	return bStereoRequested ? FrameState.ViewConfigs.Num() : 1;
}

bool FSpatialLabsDevice::GetRelativeEyePose(int32 InDeviceId, EStereoscopicPass InEye, FQuat& OutOrientation, FVector& OutPosition)
{
	if (InDeviceId != IXRTrackingSystem::HMDDeviceId)
	{
		return false;
	}

	FString Message;

	const FPipelinedFrameState& FrameState = GetPipelinedFrameStateForThread();
	FSpatialLabsCoreModule* SpatialLabsCore = static_cast<FSpatialLabsCoreModule*>(FModuleManager::Get().GetModule("SpatialLabsCore"));
	
	UpdatePlayerCameraData();	

	const uint32 ViewIndex = GetViewIndexForPass(InEye);
	if (FrameState.ViewState.viewStateFlags & XR_VIEW_STATE_ORIENTATION_VALID_BIT &&
		FrameState.ViewState.viewStateFlags & XR_VIEW_STATE_POSITION_VALID_BIT &&
		FrameState.Views.IsValidIndex(ViewIndex))
	{
		OutOrientation = FQuat::Identity;

		SpatialLabsCoreLib::ProjectionInfo projectionInfo
		{
			GetViewIndexForPass(InEye),// EyeIndex
			{
				FrameState.Views[0].pose.position.x,
				FrameState.Views[0].pose.position.y,
				FrameState.Views[0].pose.position.z
			},// LeftEyePos
			{
				FrameState.Views[1].pose.position.x,
				FrameState.Views[1].pose.position.y,
				FrameState.Views[1].pose.position.z
			}, // RightEyePos
			SpatialLabsCore->GetCoreSettings()->GetCameraOffset(),// CameraOffset
			SpatialLabsCore->GetCoreSettings()->GetUseDynamicCameraFOV(),// UseDynamicCameraFOV
			SpatialLabsCore->GetCoreSettings()->GetUseFixedHeadPosition(),// UseFixedHeadPosition
			{
				SpatialLabsCore->GetCoreSettings()->GetFixedHeadPosition().X,
				SpatialLabsCore->GetCoreSettings()->GetFixedHeadPosition().Y,
				SpatialLabsCore->GetCoreSettings()->GetFixedHeadPosition().Z
			},// FixedHeadPosition
			FocalLengthPlayerCamera_CM,//FocalLengthPlayerCamera
			UseDynamicCameraFOVAroundSceenCenter,// UseDynamicCameraFOVAroundSceenCenter
			GetScaledWorldToMetersScale(),// ScaledWorldToMetersScale
			GNearClippingPlane// ZNear
		};

		float EyePosition[] = { 0.0, 0.0, 0.0 };// vector, 3 elements
		const auto& CoreSettings = FSpatialLabsCoreModule::Get().GetCoreSettings();
		CoreSettings->GetRelativeEyePosition(projectionInfo, EyePosition);

		OutPosition = FVector(
			EyePosition[0],
			EyePosition[1],
			EyePosition[2]
		);
		
		return true;
	}

	return false;
}

FMatrix FSpatialLabsDevice::GetStereoProjectionMatrix(const enum EStereoscopicPass StereoPassType) const
{
	const FPipelinedFrameState& FrameState = GetPipelinedFrameStateForThread();
	FSpatialLabsCoreModule* SpatialLabsCore = static_cast<FSpatialLabsCoreModule*>(FModuleManager::Get().GetModule("SpatialLabsCore"));

	SpatialLabsCoreLib::ProjectionInfo projectionInfo
	{
		GetViewIndexForPass(StereoPassType),// EyeIndex
		{		
			FrameState.Views[0].pose.position.x,
			FrameState.Views[0].pose.position.y,
			FrameState.Views[0].pose.position.z
		},// LeftEyePos
		{
			FrameState.Views[1].pose.position.x,
			FrameState.Views[1].pose.position.y,
			FrameState.Views[1].pose.position.z
		}, // RightEyePos
		SpatialLabsCore->GetCoreSettings()->GetCameraOffset(),// CameraOffset
		SpatialLabsCore->GetCoreSettings()->GetUseDynamicCameraFOV(),// UseDynamicCameraFOV
	    SpatialLabsCore->GetCoreSettings()->GetUseFixedHeadPosition(),// UseFixedHeadPosition
		{
			SpatialLabsCore->GetCoreSettings()->GetFixedHeadPosition().X,
			SpatialLabsCore->GetCoreSettings()->GetFixedHeadPosition().Y,
			SpatialLabsCore->GetCoreSettings()->GetFixedHeadPosition().Z
		},// FixedHeadPosition
	    FocalLengthPlayerCamera_CM,//FocalLengthPlayerCamera
		UseDynamicCameraFOVAroundSceenCenter,// UseDynamicCameraFOVAroundSceenCenter
		GetScaledWorldToMetersScale(),// ScaledWorldToMetersScale
		GNearClippingPlane// ZNear
	};

	float projectionMatrix[] = { 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0 };// 4x4 matrix, 16 elements
	const auto& CoreSettings = FSpatialLabsCoreModule::Get().GetCoreSettings();
	CoreSettings->GetProjectionMatrix(projectionInfo, projectionMatrix);

	FMatrix ProjectionMatrix = FMatrix(
		FPlane(projectionMatrix[0], projectionMatrix[1], projectionMatrix[2], projectionMatrix[3]),
		FPlane(projectionMatrix[4], projectionMatrix[5], projectionMatrix[6], projectionMatrix[7]),
		FPlane(projectionMatrix[8], projectionMatrix[9], projectionMatrix[10], projectionMatrix[11]),
		FPlane(projectionMatrix[12], projectionMatrix[13], projectionMatrix[14], projectionMatrix[15])
	);

	return ProjectionMatrix;
}

void FSpatialLabsDevice::GetEyeRenderParams_RenderThread(const FRenderingCompositePassContext& Context, FVector2D& EyeToSrcUVScaleValue, FVector2D& EyeToSrcUVOffsetValue) const
{
	EyeToSrcUVOffsetValue = FVector2D::ZeroVector;
	EyeToSrcUVScaleValue = FVector2D(1.0f, 1.0f);
}


void FSpatialLabsDevice::SetupViewFamily(FSceneViewFamily& InViewFamily)
{
	InViewFamily.EngineShowFlags.MotionBlur = 0;
	InViewFamily.EngineShowFlags.HMDDistortion = false;
	InViewFamily.EngineShowFlags.StereoRendering = IsStereoEnabled();

	// TODO: Handle dynamic resolution in the driver, so the runtime
	// can take advantage of the extra resolution in the distortion process.
	InViewFamily.EngineShowFlags.ScreenPercentage = 0;

	const FPipelinedFrameState& FrameState = GetPipelinedFrameStateForThread();
	if (FrameState.Views.Num() > 2)
	{
		InViewFamily.EngineShowFlags.Vignette = 0;
		InViewFamily.EngineShowFlags.Bloom = 0;
	}
}

void FSpatialLabsDevice::SetupView(FSceneViewFamily& InViewFamily, FSceneView& InView)
{
	WorldToMetersCache = GetWorldToMetersScale();
}

void FSpatialLabsDevice::BeginRenderViewFamily(FSceneViewFamily& InViewFamily)
{
	uint32 ViewConfigCount = 0;
	XR_ENSURE(xrEnumerateViewConfigurationViews(Instance, System, SelectedViewConfigurationType, 0, &ViewConfigCount, nullptr));

	PipelinedLayerStateRendering.ProjectionLayers.SetNum(ViewConfigCount);
	PipelinedLayerStateRendering.DepthLayers.SetNum(ViewConfigCount);

	PipelinedLayerStateRendering.ColorImages.SetNum(PipelinedFrameStateRendering.ViewConfigs.Num());
	PipelinedLayerStateRendering.DepthImages.SetNum(PipelinedFrameStateRendering.ViewConfigs.Num());

	if (SpectatorScreenController)
	{
		SpectatorScreenController->BeginRenderViewFamily();
	}
}

void FSpatialLabsDevice::PreRenderView_RenderThread(FRHICommandListImmediate& RHICmdList, FSceneView& InView)
{
	check(IsInRenderingThread());
}

void FSpatialLabsDevice::PreRenderViewFamily_RenderThread(FRHICommandListImmediate& RHICmdList, FSceneViewFamily& ViewFamily)
{
	check(IsInRenderingThread());

	if (SpectatorScreenController)
	{
		SpectatorScreenController->UpdateSpectatorScreenMode_RenderThread();
	}
}

bool FSpatialLabsDevice::IsActiveThisFrame_Internal(const FSceneViewExtensionContext& Context) const
{
	// Don't activate the SVE if xr is being used for tracking only purposes
	static const bool bXrTrackingOnly = FParse::Param(FCommandLine::Get(), TEXT("xrtrackingonly"));

	return GEngine && GEngine->IsStereoscopic3D(Context.Viewport) && !bXrTrackingOnly;
}

bool CheckPlatformDepthExtensionSupport(const XrInstanceProperties& InstanceProps)
{
	if (FCStringAnsi::Strstr(InstanceProps.runtimeName, "SteamVR/OpenXR") && (FApp::GetGraphicsRHI() == TEXT("Vulkan")))
	{
		return false;
	}
	else if (FCStringAnsi::Strstr(InstanceProps.runtimeName, "Oculus") && (FApp::GetGraphicsRHI() == TEXT("DirectX 12")))
	{
		// No PF_DepthStencil compatible formats offered yet
		return false;
	}
	return true;
}

FSpatialLabsDevice::FSpatialLabsDevice(
	const FAutoRegister& AutoRegister, 
	XrInstance InInstance, 
	XrSystemId InSystem, 
	TRefCountPtr<FOpenXRRenderBridge>& InRenderBridge,
	TArray<const char*> InEnabledExtensions, 
	TArray<IOpenXRExtensionPlugin*> InExtensionPlugins)
	: FHeadMountedDisplayBase(nullptr)
	, FSceneViewExtensionBase(AutoRegister)
	, bStereoEnabled(false)
	, bIsRunning(false)
	, bIsReady(false)
	, bIsRendering(false)
	, bIsSynchronized(false)
	, bNeedReAllocatedDepth(false)
	, bNeedReBuildOcclusionMesh(true)
	, bIsMobileMultiViewEnabled(false)
	, bSupportsHandTracking(false)
	, bIsStandaloneStereoOnlyDevice(false)
	, CurrentSessionState(XR_SESSION_STATE_UNKNOWN)
	, EnabledExtensions(std::move(InEnabledExtensions))
	, ExtensionPlugins(std::move(InExtensionPlugins))
	, Instance(InInstance)
	, System(InSystem)
	, Session(XR_NULL_HANDLE)
	, LocalSpace(XR_NULL_HANDLE)
	, StageSpace(XR_NULL_HANDLE)
	, TrackingSpaceType(XR_REFERENCE_SPACE_TYPE_STAGE)
	, SelectedViewConfigurationType(XR_VIEW_CONFIGURATION_TYPE_MAX_ENUM)
	, SelectedEnvironmentBlendMode(XR_ENVIRONMENT_BLEND_MODE_MAX_ENUM)
	, RenderBridge(InRenderBridge)
	, RendererModule(nullptr)
	, LastRequestedSwapchainFormat(0)
	, LastRequestedDepthSwapchainFormat(0)
{
	InstanceProperties = { XR_TYPE_INSTANCE_PROPERTIES, nullptr };
	XR_ENSURE(xrGetInstanceProperties(Instance, &InstanceProperties));

	bDepthExtensionSupported = IsExtensionEnabled(XR_KHR_COMPOSITION_LAYER_DEPTH_EXTENSION_NAME) && CheckPlatformDepthExtensionSupport(InstanceProperties);

	bHiddenAreaMaskSupported = IsExtensionEnabled(XR_KHR_VISIBILITY_MASK_EXTENSION_NAME) &&
		!FCStringAnsi::Strstr(InstanceProperties.runtimeName, "Oculus");
	bViewConfigurationFovSupported = IsExtensionEnabled(XR_EPIC_VIEW_CONFIGURATION_FOV_EXTENSION_NAME);

	// Retrieve system properties and check for hand tracking support
	XrSystemHandTrackingPropertiesEXT HandTrackingSystemProperties = { XR_TYPE_SYSTEM_HAND_TRACKING_PROPERTIES_EXT };
	SystemProperties = XrSystemProperties{ XR_TYPE_SYSTEM_PROPERTIES, &HandTrackingSystemProperties };
	XR_ENSURE(xrGetSystemProperties(Instance, System, &SystemProperties));
	bSupportsHandTracking = HandTrackingSystemProperties.supportsHandTracking == XR_TRUE;
	SystemProperties.next = nullptr;

	static const auto CVarMobileMultiView = IConsoleManager::Get().FindTConsoleVariableDataInt(TEXT("vr.MobileMultiView"));
	static const auto CVarMobileHDR = IConsoleManager::Get().FindTConsoleVariableDataInt(TEXT("r.MobileHDR"));
	const bool bMobileHDR = (CVarMobileHDR && CVarMobileHDR->GetValueOnAnyThread() != 0);
	const bool bMobileMultiView = !bMobileHDR && (CVarMobileMultiView && CVarMobileMultiView->GetValueOnAnyThread() != 0);
#if PLATFORM_HOLOLENS
	bIsMobileMultiViewEnabled = bMobileMultiView && GRHISupportsArrayIndexFromAnyShader;
#else
	bIsMobileMultiViewEnabled = bMobileMultiView && RHISupportsMobileMultiView(GMaxRHIShaderPlatform);
#endif

	static const auto CVarPropagateAlpha = IConsoleManager::Get().FindTConsoleVariableDataInt(TEXT("r.PostProcessing.PropagateAlpha"));
	bProjectionLayerAlphaEnabled = !IsMobilePlatform(GMaxRHIShaderPlatform) && CVarPropagateAlpha->GetValueOnAnyThread() != 0;

	// Enumerate the viewport configurations
	uint32 ConfigurationCount;
	TArray<XrViewConfigurationType> ViewConfigTypes;
	XR_ENSURE(xrEnumerateViewConfigurations(Instance, System, 0, &ConfigurationCount, nullptr));
	ViewConfigTypes.SetNum(ConfigurationCount);
	// Fill the initial array with valid enum types (this will fail in the validation layer otherwise).
	for (auto& TypeIter : ViewConfigTypes)
		TypeIter = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_MONO;
	XR_ENSURE(xrEnumerateViewConfigurations(Instance, System, ConfigurationCount, &ConfigurationCount, ViewConfigTypes.GetData()));

	// Select the first view configuration returned by the runtime that is supported.
	// This is the view configuration preferred by the runtime.
	for (XrViewConfigurationType ViewConfigType : ViewConfigTypes)
	{
		if (SupportedViewConfigurations.Contains(ViewConfigType))
		{
			SelectedViewConfigurationType = ViewConfigType;
			break;
		}
	}

	// If there is no supported view configuration type, use the first option as a last resort.
	if (!ensure(SelectedViewConfigurationType != XR_VIEW_CONFIGURATION_TYPE_MAX_ENUM))
	{
		UE_LOG(LogHMD, Error, TEXT("No compatible view configuration type found, falling back to runtime preferred type."));
		SelectedViewConfigurationType = ViewConfigTypes[0];
	}

	// Enumerate the views we will be simulating with.
	EnumerateViews(PipelinedFrameStateGame);

	// Enumerate environment blend modes and select the best one.
	{
		uint32 BlendModeCount;
		TArray<XrEnvironmentBlendMode> BlendModes;
		XR_ENSURE(xrEnumerateEnvironmentBlendModes(Instance, System, SelectedViewConfigurationType, 0, &BlendModeCount, nullptr));
		// Fill the initial array with valid enum types (this will fail in the validation layer otherwise).
		for (auto& TypeIter : BlendModes)
			TypeIter = XR_ENVIRONMENT_BLEND_MODE_OPAQUE;
		BlendModes.SetNum(BlendModeCount);
		XR_ENSURE(xrEnumerateEnvironmentBlendModes(Instance, System, SelectedViewConfigurationType, BlendModeCount, &BlendModeCount, BlendModes.GetData()));

		// Select the first blend mode returned by the runtime that is supported.
		// This is the environment blend mode preferred by the runtime.
		for (XrEnvironmentBlendMode BlendMode : BlendModes)
		{
			if (SupportedBlendModes.Contains(BlendMode) &&
				// On mobile platforms the alpha channel can contain depth information, so we can't use alpha blend.
				(BlendMode != XR_ENVIRONMENT_BLEND_MODE_ALPHA_BLEND || !IsMobilePlatform(GMaxRHIShaderPlatform)))
			{
				SelectedEnvironmentBlendMode = BlendMode;
				break;
			}
		}

		// If there is no supported environment blend mode, use the first option as a last resort.
		if (!ensure(SelectedEnvironmentBlendMode != XR_ENVIRONMENT_BLEND_MODE_MAX_ENUM))
		{
			SelectedEnvironmentBlendMode = BlendModes[0];
		}

		// Widgets
		FViewport::ViewportResizedEvent.AddRaw(this, &FSpatialLabsDevice::ViewportResized);

		FWorldDelegates::OnWorldTickStart.AddRaw(this, &FSpatialLabsDevice::OnTick);
	}

#if PLATFORM_HOLOLENS
	bIsStandaloneStereoOnlyDevice = true;
#endif

	// Add a device space for the HMD without an action handle and ensure it has the correct index
	XrPath UserHead = XR_NULL_PATH;
	XR_ENSURE(xrStringToPath(Instance, "/user/head", &UserHead));
	ensure(DeviceSpaces.Emplace(XR_NULL_HANDLE, UserHead) == HMDDeviceId);

	// Give the all frame states the same initial values.
	PipelinedFrameStateRHI = PipelinedFrameStateRendering = PipelinedFrameStateGame;

	// Bind Widget Event
	USpatialLabsFunctionLibrary::OnWidgetAddToSpatialLabsViewport.BindRaw(this, &FSpatialLabsDevice::AddWidget);
	USpatialLabsFunctionLibrary::OnWidgetRemoveFromSpatialLabsViewport.BindRaw(this, &FSpatialLabsDevice::RemoveWidget);
	USpatialLabsFunctionLibrary::OnClearSpatialLabsViewport.BindRaw(this, &FSpatialLabsDevice::ClearWidgets);
}

FSpatialLabsDevice::~FSpatialLabsDevice()
{
	DestroySession();
}

const FSpatialLabsDevice::FPipelinedFrameState& FSpatialLabsDevice::GetPipelinedFrameStateForThread() const
{
	// Relying on implicit selection of the RHI struct is hazardous since the RHI thread isn't always present
	check(!IsInRHIThread());

	if (IsInRenderingThread())
	{
		return PipelinedFrameStateRendering;
	}
	else
	{
		check(IsInGameThread());
		return PipelinedFrameStateGame;
	}
}

FSpatialLabsDevice::FPipelinedFrameState& FSpatialLabsDevice::GetPipelinedFrameStateForThread()
{
	// Relying on implicit selection of the RHI struct is hazardous since the RHI thread isn't always present
	check(!IsInRHIThread());

	if (IsInRenderingThread())
	{
		return PipelinedFrameStateRendering;
	}
	else
	{
		check(IsInGameThread());
		return PipelinedFrameStateGame;
	}
}

void FSpatialLabsDevice::UpdateDeviceLocations(bool bUpdateOpenXRExtensionPlugins)
{

}

void FSpatialLabsDevice::EnumerateViews(FPipelinedFrameState& PipelineState)
{
	SCOPED_NAMED_EVENT(EnumerateViews, FColor::Red);

	// Enumerate the viewport configuration views
	uint32 ViewConfigCount = 0;
	TArray<XrViewConfigurationViewFovEPIC> ViewFov;
	XR_ENSURE(xrEnumerateViewConfigurationViews(Instance, System, SelectedViewConfigurationType, 0, &ViewConfigCount, nullptr));
	ViewFov.SetNum(ViewConfigCount);
	PipelineState.ViewConfigs.Empty(ViewConfigCount);
	PipelineState.PluginViews.Empty(ViewConfigCount);
	for (uint32 ViewIndex = 0; ViewIndex < ViewConfigCount; ViewIndex++)
	{
		XrViewConfigurationView View;
		View.type = XR_TYPE_VIEW_CONFIGURATION_VIEW;

		ViewFov[ViewIndex].type = XR_TYPE_VIEW_CONFIGURATION_VIEW_FOV_EPIC;
		ViewFov[ViewIndex].next = nullptr;
		View.next = bViewConfigurationFovSupported ? &ViewFov[ViewIndex] : nullptr;

		// These are core views that don't have an associated plugin
		PipelineState.PluginViews.Add(nullptr);
		PipelineState.ViewConfigs.Add(View);
	}
	XR_ENSURE(xrEnumerateViewConfigurationViews(Instance, System, SelectedViewConfigurationType, ViewConfigCount, &ViewConfigCount, PipelinedFrameStateGame.ViewConfigs.GetData()));

	if (Session)
	{
		LocateViews(PipelineState, true);

		FReadScopeLock DeviceLock(DeviceMutex);
	}
	else if (bViewConfigurationFovSupported)
	{
		// We can't locate the views yet, but we can already retrieve their field-of-views
		PipelineState.Views.SetNum(PipelineState.ViewConfigs.Num());
		for (int ViewIndex = 0; ViewIndex < PipelineState.Views.Num(); ViewIndex++)
		{
			XrView& View = PipelineState.Views[ViewIndex];
			View.type = XR_TYPE_VIEW;
			View.next = nullptr;
			// FIXME: should be recommendedFov
			View.fov = ViewFov[ViewIndex].recommendedFov;
			View.pose = ToXrPose(FTransform::Identity);
		}
	}
	else
	{
		// Ensure the views have sane values before we locate them
		PipelineState.Views.SetNum(PipelineState.ViewConfigs.Num());
		for (XrView& View : PipelineState.Views)
		{
			View.type = XR_TYPE_VIEW;
			View.next = nullptr;
			View.fov = XrFovf{ -PI / 4.0f, PI / 4.0f, PI / 4.0f, -PI / 4.0f };
			View.pose = ToXrPose(FTransform::Identity);
		}
	}
}

#if !PLATFORM_HOLOLENS
void FSpatialLabsDevice::BuildOcclusionMeshes()
{

}

bool FSpatialLabsDevice::BuildOcclusionMesh(XrVisibilityMaskTypeKHR Type, int View, FHMDViewMesh& Mesh)
{
	return false;
}
#endif

bool FSpatialLabsDevice::OnStereoStartup()
{
	FWriteScopeLock Lock(SessionHandleMutex);
	FWriteScopeLock DeviceLock(DeviceMutex);

	XrSessionCreateInfo SessionInfo;
	SessionInfo.type = XR_TYPE_SESSION_CREATE_INFO;
	SessionInfo.next = RenderBridge->GetGraphicsBinding();
	SessionInfo.createFlags = 0;
	SessionInfo.systemId = System;

	//XR_ENSURE(xrCreateSession(Instance, &SessionInfo, &Session));
	XrResult rs = xrCreateSession(Instance, &SessionInfo, &Session);
	if (XR_FAILED(rs))
	{
		UE_LOG(LogHMD, Log, TEXT("Failed to create an OpenXR session."));
		return false;
	}

	uint32_t ReferenceSpacesCount;
	XR_ENSURE(xrEnumerateReferenceSpaces(Session, 0, &ReferenceSpacesCount, nullptr));

	TArray<XrReferenceSpaceType> Spaces;
	Spaces.SetNum(ReferenceSpacesCount);
	// Initialize spaces array with valid enum values (avoid triggering validation error).
	for (auto & SpaceIter : Spaces)
		SpaceIter = XR_REFERENCE_SPACE_TYPE_VIEW;
	XR_ENSURE(xrEnumerateReferenceSpaces(Session, (uint32_t)Spaces.Num(), &ReferenceSpacesCount, Spaces.GetData()));
	ensure(ReferenceSpacesCount == Spaces.Num());

	XrSpace HmdSpace = XR_NULL_HANDLE;
	XrReferenceSpaceCreateInfo SpaceInfo;

	ensure(Spaces.Contains(XR_REFERENCE_SPACE_TYPE_VIEW));
	SpaceInfo.type = XR_TYPE_REFERENCE_SPACE_CREATE_INFO;
	SpaceInfo.next = nullptr;
	SpaceInfo.referenceSpaceType = XR_REFERENCE_SPACE_TYPE_VIEW;
	SpaceInfo.poseInReferenceSpace = ToXrPose(FTransform::Identity);
	XR_ENSURE(xrCreateReferenceSpace(Session, &SpaceInfo, &HmdSpace));
	DeviceSpaces[HMDDeviceId].Space = HmdSpace;

	ensure(Spaces.Contains(XR_REFERENCE_SPACE_TYPE_LOCAL));
	SpaceInfo.referenceSpaceType = XR_REFERENCE_SPACE_TYPE_LOCAL;
	XR_ENSURE(xrCreateReferenceSpace(Session, &SpaceInfo, &LocalSpace));

	// Prefer a stage space over a local space
	if (Spaces.Contains(XR_REFERENCE_SPACE_TYPE_STAGE))
	{
		TrackingSpaceType = XR_REFERENCE_SPACE_TYPE_STAGE;
		SpaceInfo.referenceSpaceType = TrackingSpaceType;
		XR_ENSURE(xrCreateReferenceSpace(Session, &SpaceInfo, &StageSpace));
	}
	else
	{
		TrackingSpaceType = XR_REFERENCE_SPACE_TYPE_LOCAL;
	}

	// Create action spaces for all devices
	for (FDeviceSpace& DeviceSpace : DeviceSpaces)
	{
		DeviceSpace.CreateSpace(Session);
	}

	RenderBridge->SetSpatialLabsDevice(this);

	// grab a pointer to the renderer module for displaying our mirror window
	static const FName RendererModuleName("Renderer");
	RendererModule = FModuleManager::GetModulePtr<IRendererModule>(RendererModuleName);

	bool bUseExtensionSpectatorScreenController = false;

#if !PLATFORM_HOLOLENS
	if (!bUseExtensionSpectatorScreenController && !bIsStandaloneStereoOnlyDevice)
	{
		SpectatorScreenController = MakeUnique<FDefaultSpectatorScreenController>(this);
		UE_LOG(LogHMD, Verbose, TEXT("OpenXR using base spectator screen."));
	}
	else
#endif
	{
		if (SpectatorScreenController == nullptr)
		{
			UE_LOG(LogHMD, Verbose, TEXT("OpenXR disabling spectator screen."));
		}
		else
		{
			UE_LOG(LogHMD, Verbose, TEXT("OpenXR using extension spectator screen."));
		}
	}

	return true;
}

bool FSpatialLabsDevice::OnStereoTeardown()
{
	XrResult Result = XR_ERROR_SESSION_NOT_RUNNING;
	{
		FReadScopeLock Lock(SessionHandleMutex);
		if (Session != XR_NULL_HANDLE)
		{
			Result = xrRequestExitSession(Session);
		}
	}

	if (Result == XR_ERROR_SESSION_NOT_RUNNING)
	{
		// Session was never running - most likely PIE without putting the headset on.
		DestroySession();
	}
	else
	{
		XR_ENSURE(Result);
	}
	return true;
}

void FSpatialLabsDevice::DestroySession()
{
	// FlushRenderingCommands must be called outside of SessionLock since some rendering threads will also lock this mutex.
	FlushRenderingCommands();
	FWriteScopeLock SessionLock(SessionHandleMutex);

	if (Session != XR_NULL_HANDLE)
	{
		FWriteScopeLock DeviceLock(DeviceMutex);

		// We need to reset all swapchain references to ensure there are no attempts
		// to destroy swapchain handles after the session is already destroyed.
		ForEachLayer([&](uint32 /* unused */, FOpenXRLayer& Layer)
			{
				Layer.Swapchain.Reset();
				Layer.LeftSwapchain.Reset();
			});

		PipelinedLayerStateRendering.ColorSwapchain.Reset();
		PipelinedLayerStateRendering.DepthSwapchain.Reset();
		PipelinedLayerStateRendering.QuadSwapchains.Reset();

		// TODO: Once we handle OnFinishRendering_RHIThread + StopSession interactions
		// properly, we can release these shared pointers in that function, and use
		// `ensure` here to make sure these are released.
		PipelinedLayerStateRHI.ColorSwapchain.Reset();
		PipelinedLayerStateRHI.DepthSwapchain.Reset();
		PipelinedLayerStateRHI.QuadSwapchains.Reset();

		// Reset the frame state.
		PipelinedFrameStateGame.FrameState = XrFrameState{ XR_TYPE_FRAME_STATE };
		PipelinedFrameStateRendering.FrameState = XrFrameState{ XR_TYPE_FRAME_STATE };
		PipelinedFrameStateRHI.FrameState = XrFrameState{ XR_TYPE_FRAME_STATE };

		// VRFocus must be reset so FWindowsApplication::PollGameDeviceState does not incorrectly short-circuit.
		FApp::SetUseVRFocus(false);
		FApp::SetHasVRFocus(false);

		// Destroy device spaces, they will be recreated
		// when the session is created again.
		for (FDeviceSpace& Device : DeviceSpaces)
		{
			Device.DestroySpace();
		}

		// Close the session now we're allowed to.
		XR_ENSURE(xrDestroySession(Session));
		Session = XR_NULL_HANDLE;

		bStereoEnabled = false;
		bIsReady = false;
		bIsRunning = false;
		bIsRendering = false;
		bIsSynchronized = false;
		bNeedReAllocatedDepth = true;
		bNeedReBuildOcclusionMesh = true;
	}
}

int32 FSpatialLabsDevice::AddActionDevice(XrAction Action, XrPath Path)
{
	FWriteScopeLock DeviceLock(DeviceMutex);

	// Ensure the HMD device is already emplaced
	ensure(DeviceSpaces.Num() > 0);

	int32 DeviceId = DeviceSpaces.Emplace(Action, Path);

	FReadScopeLock Lock(SessionHandleMutex);
	if (Session)
	{
		DeviceSpaces[DeviceId].CreateSpace(Session);
	}

	return DeviceId;
}

void FSpatialLabsDevice::ResetActionDevices()
{
	FWriteScopeLock DeviceLock(DeviceMutex);

	// Index 0 is HMDDeviceId and is preserved. The remaining are action devices.
	if (DeviceSpaces.Num() > 0)
	{
		DeviceSpaces.RemoveAt(HMDDeviceId + 1, DeviceSpaces.Num() - 1);
	}
}

XrPath FSpatialLabsDevice::GetTrackedDevicePath(const int32 DeviceId)
{
	FReadScopeLock DeviceLock(DeviceMutex);
	if (DeviceSpaces.IsValidIndex(DeviceId))
	{
		return DeviceSpaces[DeviceId].Path;
	}
	return XR_NULL_PATH;
}

XrTime FSpatialLabsDevice::GetDisplayTime() const
{
	const FPipelinedFrameState& PipelineState = GetPipelinedFrameStateForThread();
	return PipelineState.FrameState.predictedDisplayTime;
}

bool FSpatialLabsDevice::IsInitialized() const
{
	return Instance != XR_NULL_HANDLE;
}

bool FSpatialLabsDevice::IsRunning() const
{
	return bIsRunning;
}

bool FSpatialLabsDevice::IsFocused() const
{
	return CurrentSessionState == XR_SESSION_STATE_FOCUSED;
}

bool FSpatialLabsDevice::StartSession()
{
	// If the session is not yet ready, we'll call into this function again when it is
	FWriteScopeLock Lock(SessionHandleMutex);
	if (!bIsReady || bIsRunning)
	{
		return false;
	}

	XrSessionBeginInfo Begin = { XR_TYPE_SESSION_BEGIN_INFO, nullptr, SelectedViewConfigurationType };

	bIsRunning = XR_ENSURE(xrBeginSession(Session, &Begin));
	return bIsRunning;
}

bool FSpatialLabsDevice::StopSession()
{
	FWriteScopeLock Lock(SessionHandleMutex);
	if (!bIsRunning)
	{
		return false;
	}

	bIsRunning = !XR_ENSURE(xrEndSession(Session));
	return !bIsRunning;
}

void FSpatialLabsDevice::OnBeginPlay(FWorldContext& InWorldContext)
{

}

void FSpatialLabsDevice::OnEndPlay(FWorldContext& InWorldContext)
{
	if (InWorldContext.World()) {
		ClearWidgets(InWorldContext.World());
	}
}

IStereoRenderTargetManager* FSpatialLabsDevice::GetRenderTargetManager()
{
	return this;
}

bool FSpatialLabsDevice::AllocateRenderTargetTexture(uint32 Index, uint32 SizeX, uint32 SizeY, uint8 Format, uint32 NumMips, ETextureCreateFlags Flags, ETextureCreateFlags TargetableTextureFlags, FTexture2DRHIRef& OutTargetableTexture, FTexture2DRHIRef& OutShaderResourceTexture, uint32 NumSamples)
{
	check(IsInRenderingThread());

	FReadScopeLock Lock(SessionHandleMutex);
	if (!Session)
	{
		return false;
	}

	// This is not a static swapchain
	Flags |= TexCreate_Dynamic;

	// We need to ensure we can sample from the texture in CopyTexture
	TargetableTextureFlags |= TexCreate_ShaderResource;

	// On mobile without HDR all render targets need to be marked sRGB
	bool MobileHWsRGB = IsMobileColorsRGB() && IsMobilePlatform(GMaxRHIShaderPlatform);
	if (MobileHWsRGB)
	{
		TargetableTextureFlags |= TexCreate_SRGB;
	}

	// Temporary workaround to swapchain formats - OpenXR doesn't support 10-bit sRGB swapchains, so prefer 8-bit sRGB instead.
	if (Format == PF_A2B10G10R10)
	{
		Format = PF_R8G8B8A8;
	}

	FClearValueBinding ClearColor = (SelectedEnvironmentBlendMode == XR_ENVIRONMENT_BLEND_MODE_OPAQUE) ? FClearValueBinding::Black : FClearValueBinding::Transparent;

	FXRSwapChainPtr& Swapchain = PipelinedLayerStateRendering.ColorSwapchain;

	const FRHITexture2D* const SwapchainTexture = Swapchain == nullptr ? nullptr : Swapchain->GetTexture2DArray() ? Swapchain->GetTexture2DArray() : Swapchain->GetTexture2D();
	if (Swapchain == nullptr || SwapchainTexture == nullptr || Format != LastRequestedSwapchainFormat || SwapchainTexture->GetSizeX() != SizeX || SwapchainTexture->GetSizeY() != SizeY)
	{
		ensureMsgf(NumSamples == 1, TEXT("OpenXR supports MSAA swapchains, but engine logic expects the swapchain target to be 1x."));

		Swapchain = RenderBridge->CreateSwapchain(Session, Format, SizeX, SizeY, bIsMobileMultiViewEnabled ? 2 : 1, NumMips, NumSamples, Flags, TargetableTextureFlags, ClearColor);
		if (!Swapchain)
		{
			return false;
		}

		// Acquire the first swapchain image
		Swapchain->IncrementSwapChainIndex_RHIThread();

#if WITH_EDITOR
		if (GIsEditor)
		{
			if (FSceneViewport* SceneVP = FindSceneViewport())
			{
				TSharedPtr<SWindow> Window = SceneVP->FindWindow();
				if (Window.IsValid())
				{
					// Window continues to be processed when PIE spectator window is minimized
					Window->SetIndependentViewportSize(FVector2D(SizeX, SizeY * 2));
				}
			}
		}
#endif
	}

	// Grab the presentation texture out of the swapchain.
	OutTargetableTexture = OutShaderResourceTexture = (FTexture2DRHIRef&)Swapchain->GetTextureRef();
	LastRequestedSwapchainFormat = Format;

	bNeedReAllocatedDepth = bDepthExtensionSupported;

	return true;
}

bool FSpatialLabsDevice::AllocateDepthTexture(uint32 Index, uint32 SizeX, uint32 SizeY, uint8 Format, uint32 NumMips, ETextureCreateFlags Flags, ETextureCreateFlags TargetableTextureFlags, FTexture2DRHIRef& OutTargetableTexture, FTexture2DRHIRef& OutShaderResourceTexture, uint32 NumSamples)
{
	check(IsInRenderingThread());

	// FIXME: UE4 constantly calls this function even when there is no reason to reallocate the depth texture
	FReadScopeLock Lock(SessionHandleMutex);
	if (!Session || !bDepthExtensionSupported)
	{
		return false;
	}

	// This is not a static swapchain
	Flags |= TexCreate_Dynamic;

	FXRSwapChainPtr& Swapchain = PipelinedLayerStateRendering.DepthSwapchain;
	const FRHITexture2D* const DepthSwapchainTexture = Swapchain == nullptr ? nullptr : Swapchain->GetTexture2DArray() ? Swapchain->GetTexture2DArray() : Swapchain->GetTexture2D();
	if (Swapchain == nullptr || DepthSwapchainTexture == nullptr || Format != LastRequestedDepthSwapchainFormat || DepthSwapchainTexture->GetSizeX() != SizeX || DepthSwapchainTexture->GetSizeY() != SizeY)
	{
		ensureMsgf(NumSamples == 1, TEXT("OpenXR supports MSAA swapchains, but engine logic expects the swapchain target to be 1x."));

		Swapchain = RenderBridge->CreateSwapchain(Session, PF_DepthStencil, SizeX, SizeY, bIsMobileMultiViewEnabled ? 2 : 1, FMath::Max(NumMips, 1u), NumSamples, Flags, TargetableTextureFlags, FClearValueBinding::DepthFar);
		if (!Swapchain)
		{
			return false;
		}

		// Acquire the first swapchain image
		Swapchain->IncrementSwapChainIndex_RHIThread();
	}

	bNeedReAllocatedDepth = false;

	OutTargetableTexture = OutShaderResourceTexture = (FTexture2DRHIRef&)Swapchain->GetTextureRef();
	LastRequestedDepthSwapchainFormat = Format;

	return true;
}

void FSpatialLabsDevice::OnBeginRendering_RenderThread(FRHICommandListImmediate& RHICmdList, FSceneViewFamily& ViewFamily)
{
	ensure(IsInRenderingThread());
	FReadScopeLock DeviceLock(DeviceMutex);

	const float WorldToMeters = GetWorldToMetersScale();
	const FTransform InvTrackingToWorld = GetTrackingToWorldTransform().Inverse();

	PipelinedFrameStateRendering = PipelinedFrameStateGame;

	for (int32 ViewIndex = 0; ViewIndex < PipelinedLayerStateRendering.ProjectionLayers.Num(); ViewIndex++)
	{
		const XrView& View = PipelinedFrameStateRendering.Views[ViewIndex];
		FTransform EyePose = ToFTransform(View.pose, WorldToMeters);

		// Apply the base HMD pose to each eye pose, we will late update this pose for late update in another callback
		FTransform BasePose(ViewFamily.Views[ViewIndex]->BaseHmdOrientation, ViewFamily.Views[ViewIndex]->BaseHmdLocation);
		XrCompositionLayerProjectionView& Projection = PipelinedLayerStateRendering.ProjectionLayers[ViewIndex];
		FTransform BasePoseTransform = EyePose * BasePose;
		BasePoseTransform.NormalizeRotation();
		Projection.pose = ToXrPose(BasePoseTransform, WorldToMeters);
		Projection.fov = View.fov;
	}

	// Manage the swapchains in the stereo layers
	auto CreateSwapchain = [this](FRHITexture2D* Texture, ETextureCreateFlags Flags)
	{
		return RenderBridge->CreateSwapchain(Session,
			PF_B8G8R8A8,
			Texture->GetSizeX(),
			Texture->GetSizeY(),
			1,
			Texture->GetNumMips(),
			Texture->GetNumSamples(),
			Texture->GetFlags() | Flags,
			TexCreate_RenderTargetable,
			Texture->GetClearBinding());
	};
	if (GetStereoLayersDirty())
	{
		ForEachLayer([&](uint32 /* unused */, FOpenXRLayer& Layer)
			{
				if (Layer.Desc.IsVisible() && Layer.Desc.HasShape<FQuadLayer>())
				{
					const ETextureCreateFlags Flags = Layer.Desc.Flags & IStereoLayers::LAYER_FLAG_TEX_CONTINUOUS_UPDATE ?
						TexCreate_Dynamic | TexCreate_SRGB : TexCreate_SRGB;

					if (Layer.NeedReAllocateTexture())
					{
						FRHITexture2D* Texture = Layer.Desc.Texture->GetTexture2D();
						Layer.Swapchain = CreateSwapchain(Texture, Flags);
						Layer.SwapchainSize = Texture->GetSizeXY();
						Layer.bUpdateTexture = true;
					}

					if (Layer.NeedReAllocateLeftTexture())
					{
						FRHITexture2D* Texture = Layer.Desc.LeftTexture->GetTexture2D();
						Layer.LeftSwapchain = CreateSwapchain(Texture, Flags);
						Layer.bUpdateTexture = true;
					}
				}
				else
				{
					// We retain references in FPipelinedLayerState to avoid premature destruction
					Layer.Swapchain.Reset();
					Layer.LeftSwapchain.Reset();
				}
			});
	}

	// Gather all active quad layers for composition sorted by priority
	TArray<FOpenXRLayer> StereoLayers;
	CopySortedLayers(StereoLayers);
	PipelinedLayerStateRendering.QuadLayers.Reset(StereoLayers.Num());
	PipelinedLayerStateRendering.QuadSwapchains.Reset(StereoLayers.Num());
	for (const FOpenXRLayer& Layer : StereoLayers)
	{
		const bool bNoAlpha = Layer.Desc.Flags & IStereoLayers::LAYER_FLAG_TEX_NO_ALPHA_CHANNEL;
		const bool bIsStereo = Layer.Desc.LeftTexture.IsValid();
		FTransform PositionTransform = Layer.Desc.PositionType == ELayerType::WorldLocked ?
			InvTrackingToWorld : FTransform::Identity;

		XrCompositionLayerQuad Quad = { XR_TYPE_COMPOSITION_LAYER_QUAD, nullptr };
		Quad.layerFlags = bNoAlpha ? 0 : XR_COMPOSITION_LAYER_UNPREMULTIPLIED_ALPHA_BIT |
			XR_COMPOSITION_LAYER_BLEND_TEXTURE_SOURCE_ALPHA_BIT;
		Quad.space = Layer.Desc.PositionType == ELayerType::FaceLocked ?
			DeviceSpaces[HMDDeviceId].Space : GetTrackingSpace();
		Quad.subImage.imageRect = ToXrRect(Layer.GetViewport());
		Quad.subImage.imageArrayIndex = 0;
		Quad.pose = ToXrPose(Layer.Desc.Transform * PositionTransform, WorldToMeters);
		Quad.size = ToXrExtent2D(Layer.GetQuadSize(), WorldToMeters);

		// We need to copy each layer into an OpenXR swapchain so they can be displayed by the compositor
		if (Layer.Swapchain.IsValid() && Layer.Desc.Texture.IsValid())
		{
			if (Layer.bUpdateTexture && bIsRunning)
			{
				FRHITexture2D* SrcTexture = Layer.Desc.Texture->GetTexture2D();
				FIntRect DstRect(FIntPoint(0, 0), Layer.SwapchainSize.IntPoint());
				CopyTexture_RenderThread(RHICmdList, SrcTexture, FIntRect(), Layer.Swapchain, DstRect, false, bNoAlpha);
			}

			Quad.eyeVisibility = bIsStereo ? XR_EYE_VISIBILITY_RIGHT : XR_EYE_VISIBILITY_BOTH;
			Quad.subImage.swapchain = static_cast<FOpenXRSwapchain*>(Layer.Swapchain.Get())->GetHandle();
			PipelinedLayerStateRendering.QuadLayers.Add(Quad);
			PipelinedLayerStateRendering.QuadSwapchains.Add(Layer.Swapchain);
		}

		if (Layer.LeftSwapchain.IsValid() && Layer.Desc.LeftTexture.IsValid())
		{
			if (Layer.bUpdateTexture && bIsRunning)
			{
				FRHITexture2D* SrcTexture = Layer.Desc.LeftTexture->GetTexture2D();
				FIntRect DstRect(FIntPoint(0, 0), Layer.SwapchainSize.IntPoint());
				CopyTexture_RenderThread(RHICmdList, SrcTexture, FIntRect(), Layer.LeftSwapchain, DstRect, false, bNoAlpha);
			}

			Quad.eyeVisibility = XR_EYE_VISIBILITY_LEFT;
			Quad.subImage.swapchain = static_cast<FOpenXRSwapchain*>(Layer.LeftSwapchain.Get())->GetHandle();
			PipelinedLayerStateRendering.QuadLayers.Add(Quad);
			PipelinedLayerStateRendering.QuadSwapchains.Add(Layer.LeftSwapchain);
		}
	}

#if !PLATFORM_HOLOLENS
	if (bHiddenAreaMaskSupported && bNeedReBuildOcclusionMesh)
	{
		BuildOcclusionMeshes();
	}
#endif

	if (bIsRunning)
	{
		// Locate the views we will actually be rendering for.
		// This is required to support late-updating the field-of-view.
		LocateViews(PipelinedFrameStateRendering, false);

		SCOPED_NAMED_EVENT(EnqueueFrame, FColor::Red);

		// Reset the update flag on all layers
		ForEachLayer([&](uint32 /* unused */, FOpenXRLayer& Layer)
			{
				Layer.bUpdateTexture = Layer.Desc.Flags & IStereoLayers::LAYER_FLAG_TEX_CONTINUOUS_UPDATE;
			}, false);

		{
			FReadScopeLock Lock(SessionHandleMutex);
		}

		FXRSwapChainPtr ColorSwapchain = PipelinedLayerStateRendering.ColorSwapchain;
		FXRSwapChainPtr DepthSwapchain = PipelinedLayerStateRendering.DepthSwapchain;
		RHICmdList.EnqueueLambda([this, FrameState = PipelinedFrameStateRendering, ColorSwapchain, DepthSwapchain](FRHICommandListImmediate& InRHICmdList)
			{
				OnBeginRendering_RHIThread(FrameState, ColorSwapchain, DepthSwapchain);
			});
	}

	// Snapshot new poses for late update.
	UpdateDeviceLocations(false);
}

void FSpatialLabsDevice::LocateViews(FPipelinedFrameState& PipelineState, bool ResizeViewsArray)
{
	check(PipelineState.FrameState.predictedDisplayTime);
	FReadScopeLock DeviceLock(DeviceMutex);

	uint32_t ViewCount = 0;
	XrViewLocateInfo ViewInfo;
	ViewInfo.type = XR_TYPE_VIEW_LOCATE_INFO;
	ViewInfo.next = nullptr;
	ViewInfo.viewConfigurationType = SelectedViewConfigurationType;
	ViewInfo.space = DeviceSpaces[HMDDeviceId].Space;
	ViewInfo.displayTime = PipelineState.FrameState.predictedDisplayTime;

	XR_ENSURE(xrLocateViews(Session, &ViewInfo, &PipelineState.ViewState, 0, &ViewCount, nullptr));
	if (ResizeViewsArray)
	{
		PipelineState.Views.SetNum(ViewCount, false);
	}
	else
	{
		// PipelineState.Views.Num() can be greater than ViewCount if there is an IOpenXRExtensionPlugin
		// which appends more views with the GetViewLocations callback.
		ensure(PipelineState.Views.Num() >= (int32)ViewCount);
	}

	XR_ENSURE(xrLocateViews(Session, &ViewInfo, &PipelineState.ViewState, PipelineState.Views.Num(), &ViewCount, PipelineState.Views.GetData()));
}

void FSpatialLabsDevice::OnLateUpdateApplied_RenderThread(FRHICommandListImmediate& RHICmdList, const FTransform& NewRelativeTransform)
{
	FHeadMountedDisplayBase::OnLateUpdateApplied_RenderThread(RHICmdList, NewRelativeTransform);

	ensure(IsInRenderingThread());

	for (int32 ViewIndex = 0; ViewIndex < PipelinedLayerStateRendering.ProjectionLayers.Num(); ViewIndex++)
	{
		const XrView& View = PipelinedFrameStateRendering.Views[ViewIndex];
		XrCompositionLayerProjectionView& Projection = PipelinedLayerStateRendering.ProjectionLayers[ViewIndex];

		// Apply the new HMD orientation to each eye pose for the final pose
		FTransform EyePose = ToFTransform(View.pose, GetWorldToMetersScale());
		FTransform NewRelativePoseTransform = EyePose * NewRelativeTransform;
		NewRelativePoseTransform.NormalizeRotation();
		Projection.pose = ToXrPose(NewRelativePoseTransform, GetWorldToMetersScale());
	}

	RHICmdList.EnqueueLambda([this, ProjectionLayers = PipelinedLayerStateRendering.ProjectionLayers](FRHICommandListImmediate& InRHICmdList)
	{
		PipelinedLayerStateRHI.ProjectionLayers = ProjectionLayers;
	});
}

void FSpatialLabsDevice::OnBeginRendering_GameThread()
{
	FReadScopeLock Lock(SessionHandleMutex);
	if (!bIsReady || !bIsRunning)
	{
		// @todo: Sleep here?
		return;
	}

	ensure(IsInGameThread());

	SCOPED_NAMED_EVENT(WaitFrame, FColor::Red);

	XrFrameWaitInfo WaitInfo;
	WaitInfo.type = XR_TYPE_FRAME_WAIT_INFO;
	WaitInfo.next = nullptr;

	XrFrameState FrameState{XR_TYPE_FRAME_STATE};

	XR_ENSURE(xrWaitFrame(Session, &WaitInfo, &FrameState));

	// The pipeline state on the game thread can only be safely modified after xrWaitFrame which will be unblocked by
	// the runtime when xrBeginFrame is called. The rendering thread will clone the game pipeline state before calling
	// xrBeginFrame so the game pipeline state can safely be modified after xrWaitFrame returns.
	FPipelinedFrameState& PipelineState = GetPipelinedFrameStateForThread();
	PipelineState.FrameState = FrameState;
	PipelineState.TrackingSpace = GetTrackingSpace();

	EnumerateViews(PipelineState);
}

bool FSpatialLabsDevice::ReadNextEvent(XrEventDataBuffer* buffer)
{
	// It is sufficient to clear just the XrEventDataBuffer header to XR_TYPE_EVENT_DATA_BUFFER
	XrEventDataBaseHeader* baseHeader = reinterpret_cast<XrEventDataBaseHeader*>(buffer);
	*baseHeader = { XR_TYPE_EVENT_DATA_BUFFER };
	const XrResult xr = xrPollEvent(Instance, buffer);
	//XR_ENSURE(xr);
	if (xr == XR_SUCCESS)
	{
		return true;
	}
	return false;
}

bool FSpatialLabsDevice::OnStartGameFrame(FWorldContext& WorldContext)
{
#if WITH_EDITOR
	ULevelEditorPlaySettings* PlaySettings = GetMutableDefault<ULevelEditorPlaySettings>();

	if (PlaySettings &&
		(PlaySettings->NewWindowPosition != FIntPoint(0, 0) ||
		PlaySettings->NewWindowWidth != SPATIALLABS_SCREEN_RESOLUTION_WIDTH ||
		PlaySettings->NewWindowHeight != SPATIALLABS_SCREEN_RESOLUTION_HEIGHT))
	{
		PlaySettings->NewWindowPosition = FIntPoint(0, 0);
		PlaySettings->NewWindowWidth = SPATIALLABS_SCREEN_RESOLUTION_WIDTH;
		PlaySettings->NewWindowHeight = SPATIALLABS_SCREEN_RESOLUTION_HEIGHT;

		PlaySettings->PostEditChange();
		PlaySettings->SaveConfig();
	}
#endif

	// Only refresh this based on the game world.  When remoting there is also an editor world, which we do not want to have affect the transform.
	if (WorldContext.World()->IsGameWorld())
	{
		RefreshTrackingToWorldTransform(WorldContext);
	}

	// Process all pending messages.
	XrEventDataBuffer event;
	while (ReadNextEvent(&event))
	{
		switch (event.type)
		{
		case XR_TYPE_EVENT_DATA_SESSION_STATE_CHANGED:
		{
			const XrEventDataSessionStateChanged& SessionState =
				reinterpret_cast<XrEventDataSessionStateChanged&>(event);

			CurrentSessionState = SessionState.state;

#if 0
			UE_LOG(LogHMD, Log, TEXT("Session state switching to %s"), OpenXRSessionStateToString(CurrentSessionState));
#endif

			if (SessionState.state == XR_SESSION_STATE_READY)
			{
				if (!GIsEditor)
                {
					GEngine->SetMaxFPS(0);
                }
				bIsReady = true;
				StartSession();
			}
			else if (SessionState.state == XR_SESSION_STATE_SYNCHRONIZED)
			{
				bIsSynchronized = true;
			}
			else if (SessionState.state == XR_SESSION_STATE_IDLE)
			{
				bIsSynchronized = false;
			}
			else if (SessionState.state == XR_SESSION_STATE_STOPPING)
			{
				if (!GIsEditor)
                {
					GEngine->SetMaxFPS(OPENXR_PAUSED_IDLE_FPS);
                }
				bIsReady = false;
				StopSession();
			}
			else if (SessionState.state == XR_SESSION_STATE_EXITING)
			{
				// We need to make sure we unlock the frame rate again when exiting VR while idle
				if (!GIsEditor)
                {
					GEngine->SetMaxFPS(0);
                }
			}

			FApp::SetHasVRFocus(SessionState.state == XR_SESSION_STATE_FOCUSED);

			if (SessionState.state != XR_SESSION_STATE_EXITING && SessionState.state != XR_SESSION_STATE_LOSS_PENDING)
			{
				break;
			}
		}
		// Intentional fall-through
		case XR_TYPE_EVENT_DATA_INSTANCE_LOSS_PENDING:
		{
#if WITH_EDITOR
			if (GIsEditor)
			{
				FSceneViewport* SceneVP = FindSceneViewport();
				if (SceneVP && SceneVP->IsStereoRenderingAllowed())
				{
					TSharedPtr<SWindow> Window = SceneVP->FindWindow();
					Window->RequestDestroyWindow();
				}
			}
			else
#endif//WITH_EDITOR
			{
				// ApplicationWillTerminateDelegate will fire from inside of the RequestExit
				FPlatformMisc::RequestExit(false);
			}

			DestroySession();

			break;
		}
		case XR_TYPE_EVENT_DATA_REFERENCE_SPACE_CHANGE_PENDING:
		{
			const XrEventDataReferenceSpaceChangePending& SpaceChange =
				reinterpret_cast<XrEventDataReferenceSpaceChangePending&>(event);
			if (SpaceChange.referenceSpaceType == TrackingSpaceType)
			{
				OnTrackingOriginChanged();
			}
			break;
		}
		case XR_TYPE_EVENT_DATA_VISIBILITY_MASK_CHANGED_KHR:
		{
			bHiddenAreaMaskSupported = ensure(IsExtensionEnabled(XR_KHR_VISIBILITY_MASK_EXTENSION_NAME));  // Ensure fail indicates a non-conformant openxr implementation.
			bNeedReBuildOcclusionMesh = true;
		}
		}
	}

	GetARCompositionComponent()->StartARGameFrame(WorldContext);

	// Add a display period to the simulation frame state so we're predicting poses for the new frame.
	FPipelinedFrameState& PipelineState = GetPipelinedFrameStateForThread();
	PipelineState.FrameState.predictedDisplayTime += PipelineState.FrameState.predictedDisplayPeriod;

	// Snapshot new poses for game simulation.
	UpdateDeviceLocations(true);

	return true;
}

void FSpatialLabsDevice::OnBeginRendering_RHIThread(const FPipelinedFrameState& InFrameState, FXRSwapChainPtr ColorSwapchain, FXRSwapChainPtr DepthSwapchain)
{
	ensure(IsInRenderingThread() || IsInRHIThread());

	// TODO: Add a hook to resolve discarded frames before we start a new frame.
	checkSlow(!bIsRendering);

	SCOPED_NAMED_EVENT(BeginFrame, FColor::Red);

	FReadScopeLock Lock(SessionHandleMutex);
	if (!bIsRunning)
	{
		return;
	}

	// The layer state will be copied after SetFinalViewRect
	PipelinedFrameStateRHI = InFrameState;

	XrFrameBeginInfo BeginInfo;
	BeginInfo.type = XR_TYPE_FRAME_BEGIN_INFO;
	BeginInfo.next = nullptr;
	XrTime DisplayTime = InFrameState.FrameState.predictedDisplayTime;

	XrResult Result = xrBeginFrame(Session, &BeginInfo);
	if (XR_SUCCEEDED(Result))
	{
		// Only the swapchains are valid to pull out of PipelinedLayerStateRendering
		// Full population is deferred until SetFinalViewRect.
		// TODO Possibly move these Waits to SetFinalViewRect??
		PipelinedLayerStateRHI.ColorSwapchain = ColorSwapchain;
		PipelinedLayerStateRHI.DepthSwapchain = DepthSwapchain;

		// We need a new swapchain image unless we've already acquired one for rendering
		if (!bIsRendering && ColorSwapchain)
		{
			ColorSwapchain->IncrementSwapChainIndex_RHIThread();
			ColorSwapchain->WaitCurrentImage_RHIThread(OPENXR_SWAPCHAIN_WAIT_TIMEOUT);
			if (bDepthExtensionSupported && DepthSwapchain)
			{
				DepthSwapchain->IncrementSwapChainIndex_RHIThread();
				DepthSwapchain->WaitCurrentImage_RHIThread(OPENXR_SWAPCHAIN_WAIT_TIMEOUT);
			}
		}

		bIsRendering = true;
	}
	else
	{
		static bool bLoggedBeginFrameFailure = false;
		if (!bLoggedBeginFrameFailure)
		{
			UE_LOG(LogHMD, Error, TEXT("Unexpected error on xrBeginFrame. Error code was %s."), OpenXRResultToString(Result));
			bLoggedBeginFrameFailure = true;
		}
	}
}

void FSpatialLabsDevice::OnFinishRendering_RHIThread()
{
	ensure(IsInRenderingThread() || IsInRHIThread());

	SCOPED_NAMED_EVENT(EndFrame, FColor::Red);

	if (!bIsRendering)
	{
		return;
	}

	// We need to ensure we release the swap chain images even if the session is not running.
	if (PipelinedLayerStateRHI.ColorSwapchain)
	{
		PipelinedLayerStateRHI.ColorSwapchain->ReleaseCurrentImage_RHIThread();

		if (bDepthExtensionSupported && PipelinedLayerStateRHI.DepthSwapchain)
		{
			PipelinedLayerStateRHI.DepthSwapchain->ReleaseCurrentImage_RHIThread();
		}
	}

	FReadScopeLock Lock(SessionHandleMutex);
	if (bIsRunning)
	{
		TArray<const XrCompositionLayerBaseHeader*> Headers;
		XrCompositionLayerProjection Layer = {};
		if (IsBackgroundLayerVisible())
		{
			Layer.type = XR_TYPE_COMPOSITION_LAYER_PROJECTION;
			Layer.next = nullptr;
			Layer.layerFlags = bProjectionLayerAlphaEnabled ? XR_COMPOSITION_LAYER_BLEND_TEXTURE_SOURCE_ALPHA_BIT : 0;
			Layer.space = PipelinedFrameStateRHI.TrackingSpace;
			Layer.viewCount = PipelinedLayerStateRHI.ProjectionLayers.Num();
			Layer.views = PipelinedLayerStateRHI.ProjectionLayers.GetData();
			Headers.Add(reinterpret_cast<const XrCompositionLayerBaseHeader*>(&Layer));
		}

		for (const XrCompositionLayerQuad& Quad : PipelinedLayerStateRHI.QuadLayers)
		{
			Headers.Add(reinterpret_cast<const XrCompositionLayerBaseHeader*>(&Quad));
		}

		XrFrameEndInfo EndInfo;
		EndInfo.type = XR_TYPE_FRAME_END_INFO;
		EndInfo.next = nullptr;
		EndInfo.displayTime = PipelinedFrameStateRHI.FrameState.predictedDisplayTime;
		EndInfo.environmentBlendMode = SelectedEnvironmentBlendMode;
		EndInfo.layerCount = PipelinedFrameStateRHI.FrameState.shouldRender ? Headers.Num() : 0;
		EndInfo.layers = PipelinedFrameStateRHI.FrameState.shouldRender ? Headers.GetData() : nullptr;

		XR_ENSURE(xrEndFrame(Session, &EndInfo));
	}

	bIsRendering = false;
}

FXRRenderBridge* FSpatialLabsDevice::GetActiveRenderBridge_GameThread(bool /* bUseSeparateRenderTarget */)
{
	return RenderBridge;
}

FIntPoint FSpatialLabsDevice::GetIdealRenderTargetSize() const
{
	const FPipelinedFrameState& PipelineState = GetPipelinedFrameStateForThread();

	FIntPoint Size(EForceInit::ForceInitToZero);
	for (int32 ViewIndex = 0; ViewIndex < PipelineState.ViewConfigs.Num(); ViewIndex++)
	{
		const XrViewConfigurationView& Config = PipelineState.ViewConfigs[ViewIndex];

		// If Mobile Multi-View is active the first two views will share the same position
		Size.X = bIsMobileMultiViewEnabled && ViewIndex < 2 ? FMath::Max(Size.X, (int)Config.recommendedImageRectWidth)
			: Size.X + (int)Config.recommendedImageRectWidth;
		Size.Y = FMath::Max(Size.Y, (int)Config.recommendedImageRectHeight);

		// We always prefer the nearest multiple of 4 for our buffer sizes. Make sure we round up here,
		// so we're consistent with the rest of the engine in creating our buffers.
		QuantizeSceneBufferSize(Size, Size);
	}

	return Size;
}

FIntRect FSpatialLabsDevice::GetFullFlatEyeRect_RenderThread(FTexture2DRHIRef EyeTexture) const
{
	FVector2D SrcNormRectMin(0.05f, 0.2f);
	FVector2D SrcNormRectMax(0.45f, 0.8f);
	if (GetDesiredNumberOfViews(bStereoEnabled) > 2)
	{
		SrcNormRectMin.X /= 2;
		SrcNormRectMax.X /= 2;
	}

	return FIntRect(EyeTexture->GetSizeX() * SrcNormRectMin.X, EyeTexture->GetSizeY() * SrcNormRectMin.Y, EyeTexture->GetSizeX() * SrcNormRectMax.X, EyeTexture->GetSizeY() * SrcNormRectMax.Y);
}

void FSpatialLabsDevice::CopyTexture_RenderThread(FRHICommandListImmediate& RHICmdList, FRHITexture2D* SrcTexture, FIntRect SrcRect, FRHITexture2D* DstTexture, FIntRect DstRect, bool bClearBlack, bool bNoAlpha, ERenderTargetActions RTAction) const
{
	check(IsInRenderingThread());

	const uint32 ViewportWidth = DstRect.Width();
	const uint32 ViewportHeight = DstRect.Height();
	const FIntPoint TargetSize(ViewportWidth, ViewportHeight);

	const float SrcTextureWidth = SrcTexture->GetSizeX();
	const float SrcTextureHeight = SrcTexture->GetSizeY();
	float U = 0.f, V = 0.f, USize = 1.f, VSize = 1.f;
	if (SrcRect.IsEmpty())
	{
		SrcRect.Min.X = 0;
		SrcRect.Min.Y = 0;
		SrcRect.Max.X = SrcTextureWidth;
		SrcRect.Max.Y = SrcTextureHeight;
	}
	else
	{
		U = SrcRect.Min.X / SrcTextureWidth;
		V = SrcRect.Min.Y / SrcTextureHeight;
		USize = SrcRect.Width() / SrcTextureWidth;
		VSize = SrcRect.Height() / SrcTextureHeight;
	}

	FRHITexture * ColorRT = DstTexture->GetTexture2DArray() ? DstTexture->GetTexture2DArray() : DstTexture->GetTexture2D();
	FRHIRenderPassInfo RenderPassInfo(ColorRT, RTAction);
	RHICmdList.BeginRenderPass(RenderPassInfo, TEXT("OpenXRHMD_CopyTexture"));
	{
		if (bClearBlack)
		{
			const FIntRect ClearRect(0, 0, DstTexture->GetSizeX(), DstTexture->GetSizeY());
			RHICmdList.SetViewport(ClearRect.Min.X, ClearRect.Min.Y, 0, ClearRect.Max.X, ClearRect.Max.Y, 1.0f);
			DrawClearQuad(RHICmdList, FLinearColor::Black);
		}

		RHICmdList.SetViewport(DstRect.Min.X, DstRect.Min.Y, 0, DstRect.Max.X, DstRect.Max.Y, 1.0f);

		FGraphicsPipelineStateInitializer GraphicsPSOInit;
		RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
		GraphicsPSOInit.BlendState = bNoAlpha ? TStaticBlendState<>::GetRHI() : TStaticBlendState<CW_RGBA, BO_Add, BF_SourceAlpha, BF_InverseSourceAlpha, BO_Add, BF_One, BF_InverseSourceAlpha>::GetRHI();
		GraphicsPSOInit.RasterizerState = TStaticRasterizerState<>::GetRHI();
		GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, CF_Always>::GetRHI();
		GraphicsPSOInit.PrimitiveType = PT_TriangleList;

		const auto FeatureLevel = GMaxRHIFeatureLevel;
		auto ShaderMap = GetGlobalShaderMap(FeatureLevel);

		TShaderMapRef<FScreenVS> VertexShader(ShaderMap);
		TShaderMapRef<FScreenPS> PixelShader(ShaderMap);

		GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GFilterVertexDeclaration.VertexDeclarationRHI;
		GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
		GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();

		SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit);

		RHICmdList.Transition(FRHITransitionInfo(SrcTexture, ERHIAccess::Unknown, ERHIAccess::SRVMask));

		const bool bSameSize = DstRect.Size() == SrcRect.Size();
		if (bSameSize)
		{
			PixelShader->SetParameters(RHICmdList, TStaticSamplerState<SF_Point>::GetRHI(), SrcTexture);
		}
		else
		{
			PixelShader->SetParameters(RHICmdList, TStaticSamplerState<SF_Bilinear>::GetRHI(), SrcTexture);
		}

		RendererModule->DrawRectangle(
			RHICmdList,
			0, 0,
			ViewportWidth, ViewportHeight,
			U, V,
			USize, VSize,
			TargetSize,
			FIntPoint(1, 1),
			VertexShader,
			EDRF_Default);
	}
	RHICmdList.EndRenderPass();
}

void FSpatialLabsDevice::CopyTexture_RenderThread(FRHICommandListImmediate& RHICmdList, FRHITexture2D* SrcTexture, FIntRect SrcRect, const FXRSwapChainPtr& DstSwapChain, FIntRect DstRect, bool bClearBlack, bool bNoAlpha) const
{
	RHICmdList.EnqueueLambda([DstSwapChain](FRHICommandListImmediate& InRHICmdList)
		{
			DstSwapChain->IncrementSwapChainIndex_RHIThread();
			DstSwapChain->WaitCurrentImage_RHIThread(OPENXR_SWAPCHAIN_WAIT_TIMEOUT);
		});

	// Now that we've enqueued the swapchain wait we can add the commands to do the actual texture copy
	FRHITexture2DArray* DstTexture = DstSwapChain->GetTexture2DArray();
	CopyTexture_RenderThread(RHICmdList, SrcTexture, SrcRect, DstTexture, DstRect, bClearBlack, bNoAlpha, ERenderTargetActions::Clear_Store);

	// Enqueue a command to release the image after the copy is done
	RHICmdList.EnqueueLambda([DstSwapChain](FRHICommandListImmediate& InRHICmdList)
		{
			DstSwapChain->ReleaseCurrentImage_RHIThread();
		});
}

void FSpatialLabsDevice::CopyTexture_RenderThread(FRHICommandListImmediate& RHICmdList, FRHITexture2D* SrcTexture, FIntRect SrcRect, FRHITexture2D* DstTexture, FIntRect DstRect, bool bClearBlack, bool bNoAlpha) const
{
	// FIXME: This should probably use the Load_Store action since the spectator controller does multiple overlaying copies.
	CopyTexture_RenderThread(RHICmdList, SrcTexture, SrcRect, DstTexture, DstRect, bClearBlack, bNoAlpha, ERenderTargetActions::DontLoad_Store);
}

void FSpatialLabsDevice::RenderTexture_RenderThread(class FRHICommandListImmediate& RHICmdList, class FRHITexture2D* BackBuffer, class FRHITexture2D* SrcTexture, FVector2D WindowSize) const
{
	if (SpectatorScreenController)
	{
		SpectatorScreenController->RenderSpectatorScreen_RenderThread(RHICmdList, BackBuffer, SrcTexture, WindowSize);
		OnSpatialLabsRenderTextureCompletedDelegate.Broadcast(RHICmdList, SrcTexture);
	}
}

bool FSpatialLabsDevice::HasHiddenAreaMesh() const
{
	return HiddenAreaMeshes.Num() > 0;
}

bool FSpatialLabsDevice::HasVisibleAreaMesh() const
{
	return VisibleAreaMeshes.Num() > 0;
}

void FSpatialLabsDevice::DrawHiddenAreaMesh_RenderThread(class FRHICommandList& RHICmdList, EStereoscopicPass StereoPass) const
{

}

void FSpatialLabsDevice::DrawVisibleAreaMesh_RenderThread(class FRHICommandList& RHICmdList, EStereoscopicPass StereoPass) const
{

}

//---------------------------------------------------
// OpenXR Action Space Implementation
//---------------------------------------------------

FSpatialLabsDevice::FDeviceSpace::FDeviceSpace(XrAction InAction, XrPath InPath)
	: Action(InAction)
	, Space(XR_NULL_HANDLE)
	, Path(InPath)
{
}

FSpatialLabsDevice::FDeviceSpace::~FDeviceSpace()
{
	DestroySpace();
}

bool FSpatialLabsDevice::FDeviceSpace::CreateSpace(XrSession InSession)
{
	if (Action == XR_NULL_HANDLE || Space != XR_NULL_HANDLE)
	{
		return false;
	}

	XrActionSpaceCreateInfo ActionSpaceInfo;
	ActionSpaceInfo.type = XR_TYPE_ACTION_SPACE_CREATE_INFO;
	ActionSpaceInfo.next = nullptr;
	ActionSpaceInfo.subactionPath = XR_NULL_PATH;
	ActionSpaceInfo.poseInActionSpace = ToXrPose(FTransform::Identity);
	ActionSpaceInfo.action = Action;
	return XR_ENSURE(xrCreateActionSpace(InSession, &ActionSpaceInfo, &Space));
}

void FSpatialLabsDevice::FDeviceSpace::DestroySpace()
{
	if (Space)
	{
		XR_ENSURE(xrDestroySpace(Space));
	}
	Space = XR_NULL_HANDLE;
}

void FSpatialLabsDevice::AddWidget(UUserWidget* Widget, int32 ZOrder) 
{
	UWorld* World = Widget->GetWorld();
	UGameViewportClient* Viewport = World->GetGameViewport();

	// Add to pending vault
	bool WidgetExist = false;
	for (UserWidget PendingUserWidget : PendingUserWidgets) {
		if (Widget == PendingUserWidget.Widget) {
			WidgetExist = true;
			break;
		}
	}
	if (!WidgetExist) {
		PendingUserWidgets.Add({Widget, ZOrder});
	}
}

void FSpatialLabsDevice::RemoveWidget(UUserWidget* Widget) 
{
	UWorld* World = Widget->GetWorld();
	UGameViewportClient* Viewport = World->GetGameViewport();

	if (ViewportLetterBoxes.Contains(Viewport))
	{
		TSharedRef<SWidget> WidgetRef = Widget->TakeWidget();

		FChildren* ChildrenRight = ViewportLetterBoxes[Viewport].BoxRight->GetChildren();
		int NumSlotsRight = ChildrenRight->Num();
		for (int Index = NumSlotsRight - 1; Index >= 0; --Index)// Loop over slots
		{
			TSharedRef<SWidget> Child = ChildrenRight->GetChildAt(Index);

			if (Child == WidgetRef) {
				ViewportLetterBoxes[Viewport].BoxRight->RemoveSlot(Child);
			}
		}

		FChildren* ChildrenLeft = ViewportLetterBoxes[Viewport].BoxLeft->GetChildren();
		int NumSlotsLeft = ChildrenLeft->Num();
		for (int Index = NumSlotsLeft - 1; Index >= 0; --Index)// Loop over slots
		{
			TSharedRef<SWidget> Child = ChildrenLeft->GetChildAt(Index);

			if (Child == WidgetRef) {
				ViewportLetterBoxes[Viewport].BoxLeft->RemoveSlot(Child);
			}
		}

		FChildren* ChildrenControl = ViewportLetterBoxes[Viewport].BoxControl->GetChildren();
		int NumSlotsControl = ChildrenControl->Num();
		for (int Index = NumSlotsControl - 1; Index >= 0; --Index)// Loop over slots
		{
			TSharedRef<SWidget> Child = ChildrenControl->GetChildAt(Index);

			if (Child == WidgetRef) {
				ViewportLetterBoxes[Viewport].BoxControl->RemoveSlot(Child);
			}
		}
	}
}

void FSpatialLabsDevice::ClearWidgets(UWorld* World) 
{
	UGameViewportClient* Viewport = World->GetGameViewport();

	if (ViewportLetterBoxes.Contains(Viewport))
	{
		ViewportLetterBoxes[Viewport].BoxRight->ClearChildren();
		ViewportLetterBoxes[Viewport].BoxLeft->ClearChildren();
		ViewportLetterBoxes[Viewport].BoxControl->ClearChildren();
	}
}

void FSpatialLabsDevice::ViewportResized(FViewport* Viewport, uint32)
{
	UWorld* World = Viewport->GetClient()->GetWorld();
	UGameViewportClient* ViewportObject = World->GetGameViewport();

	if (ViewportObject != nullptr)
	{
		if (!ViewportLetterBoxes.Contains(ViewportObject))
		{
			
			SetupLetterBoxWidget(ViewportObject);
		}
	}

	ResizeLetterboxes();
}

void FSpatialLabsDevice::SetupLetterBoxWidget(UGameViewportClient* Viewport)
{
	// Setup letterbox
	ViewportLetterBoxes.Add(Viewport, {});
	ViewportLetterBox& LetterBox = ViewportLetterBoxes[Viewport];
	ViewportLetterList.Add(&LetterBox);
	LetterBox.Viewport = Viewport;

	FLinearColor LetterboxColor(0, 0, 0, 1.0f);

	// Letterbox layout
	SAssignNew(LetterBox.LetterBox, SVerticalBox);

	LetterBox.LetterBox->AddSlot().AutoHeight()[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().AutoWidth()[
			SAssignNew(LetterBox.LetterBoxContent, SBox)[
				// Content container
				SAssignNew(LetterBox.LetterBoxContentLayout, SConstraintCanvas)
				// Right
				+ SConstraintCanvas::Slot()
				.Anchors(FAnchors(0.f, 0.f, 0.f, 0.f))
				.Offset(FMargin(0, 0, 0, 0))
				.Alignment(FVector2D(0.0, 0.0))
				[
					SAssignNew(LetterBox.BoxRight, SConstraintCanvas)
				]

				+ SConstraintCanvas::Slot()
				.Anchors(FAnchors(0.f, 0.f, 0.f, 0.f))
				.Offset(FMargin(0, 0, 0, 0))
				.Alignment(FVector2D(0.0, 0.0))
				[
					SAssignNew(LetterBox.BoxLeft, SConstraintCanvas)
				]

				// Control 
				+ SConstraintCanvas::Slot()
				.Anchors(FAnchors(0.f, 0.f, 0.f, 0.f))
				.Offset(FMargin(0, 0, 0, 0))
				.Alignment(FVector2D(0.0, 0.0))
				[
					SAssignNew(LetterBox.BoxControl, SConstraintCanvas)
				]
			]
		]
	];

}

void FSpatialLabsDevice::CleanupLetterbox(bool)
{
	ViewportLetterBoxes.Empty();
	ViewportLetterList.Empty();
}

void FSpatialLabsDevice::ResizeLetterboxes()
{
	if (!GWorld || !bStereoEnabled) return;

	for (ViewportLetterBox* Letterbox : this->ViewportLetterList)
	{
		if (Letterbox->LetterBox.IsValid() && Letterbox->LetterBoxContent.IsValid())
		{
			UGameViewportClient* ViewportClient = Letterbox->Viewport;
			UGameViewportClient* CurrentViewportClient = GWorld->GetGameViewport();

			if (ViewportClient && 
				CurrentViewportClient == ViewportClient &&
				ViewportClient->IsValidLowLevel()) {

				FViewport* ViewportInfo = ViewportClient->Viewport;

				if (ViewportInfo != nullptr)
				{
					FGeometry ViewportGeometry = Letterbox->LetterBox->GetTickSpaceGeometry();

					FVector2D AvailableSize = ViewportGeometry.GetLocalSize();

					const FVector2D ViewportHalfSizeCm = FVector2D(SPATIALLABS_SCREEN_WIDTH_CM / 2, SPATIALLABS_SCREEN_HEIGHT_CM / 2);

					FIntPoint ViewportSizePixels = ViewportInfo->GetSizeXY();

					FVector2D Scale = ViewportHalfSizeCm / ViewportSizePixels;
					float UniformScale = FMath::Max(Scale.X, Scale.Y);

					FVector2D ViewportPercentage = FVector2D(Scale.X / UniformScale, Scale.Y / UniformScale);


				}
			}
			
		}
	}
}

void FSpatialLabsDevice::AppendWidgetsToLetterbox(UGameViewportClient* Viewport)
{
	int32 NumPendingWidgets = PendingUserWidgets.Num();
	if (NumPendingWidgets == 0) return;

	if (PendingUserWidgets.Num() >= 0)
	{
		UserWidget& PendingUserWidget = PendingUserWidgets[0];

		TSharedRef<SWidget> WidgetRef = PendingUserWidget.Widget->TakeWidget();
		int32 ZOrder = PendingUserWidget.ZOrder;

		SConstraintCanvas::FSlot& SlotRight = ViewportLetterBoxes[Viewport].BoxRight->AddSlot();
		SlotRight.Anchors(FAnchors(0, 0, 0, 0));
		SlotRight.Offset(FMargin(0, 0, 0, 0));
		SlotRight.Alignment(FVector2D(0.0, 0.0));
		SlotRight.ZOrder(ZOrder);
		SlotRight.AttachWidget(WidgetRef);
		ViewportLetterBoxes[Viewport].BoxRight->SetRenderOpacity(1.0);
		ViewportLetterBoxes[Viewport].BoxRight->SetRenderTransform(FSlateRenderTransform(FScale2D(0.5f, 1.f), FVector2D(960, 0)));

		SConstraintCanvas::FSlot& SlotLeft = ViewportLetterBoxes[Viewport].BoxLeft->AddSlot();
		SlotLeft.Anchors(FAnchors(0, 0, 0, 0));
		SlotLeft.Offset(FMargin(0, 0, 0, 0));
		SlotLeft.Alignment(FVector2D(0.0, 0.0));
		SlotLeft.ZOrder(ZOrder);
		SlotLeft.AttachWidget(WidgetRef);
		ViewportLetterBoxes[Viewport].BoxLeft->SetRenderOpacity(1.0);
		ViewportLetterBoxes[Viewport].BoxLeft->SetRenderTransform(FSlateRenderTransform(FScale2D(0.5f, 1.f), FVector2D(0, 0)));

		SConstraintCanvas::FSlot& SlotControl = ViewportLetterBoxes[Viewport].BoxControl->AddSlot();
		SlotControl.Anchors(FAnchors(0, 0, 0, 0));
		SlotControl.Offset(FMargin(0, 0, 0, 0));
		SlotControl.Alignment(FVector2D(0.0, 0.0));
		SlotControl.ZOrder(ZOrder);
		SlotControl.AttachWidget(WidgetRef);
		ViewportLetterBoxes[Viewport].BoxControl->SetRenderOpacity(0.0);
		
		PendingUserWidgets.RemoveAt(0);
	}

}

void FSpatialLabsDevice::OnTick(UWorld* World, ELevelTick TickType, float DeltaTime)
{
	// Try appending user widgets
	if (World && PendingUserWidgets.Num() > 0) {
		for (ViewportLetterBox* Letterbox : ViewportLetterList)
		{
			UGameViewportClient* GameViewport = World->GetGameViewport();

			if (Letterbox && Letterbox->Viewport && 
				Letterbox->Viewport == GameViewport &&
				Letterbox->Viewport->IsValidLowLevel() && 
				Letterbox->Viewport->GetWindow().IsValid())
			{
				if (ViewportLetterBoxes.Contains(Letterbox->Viewport))
				{
					AppendWidgetsToLetterbox(Letterbox->Viewport);

					Letterbox->Viewport->AddViewportWidgetContent(ViewportLetterBoxes[Letterbox->Viewport].LetterBox.ToSharedRef());

					break;
				}
			}
		}
	}
}

void FSpatialLabsDevice::UpdatePlayerCameraData() {
	if (IsInGameThread())
	{
		if (GWorld != nullptr)
		{
			APlayerController* PlayerController = GWorld->GetFirstPlayerController();
			if (PlayerController != nullptr)
			{
				APlayerCameraManager* CameraManager = PlayerController->PlayerCameraManager;
				if (CameraManager != nullptr)
				{
					FOVPlayerCamera = CameraManager->GetFOVAngle();
				}
			}

			FocalLengthPlayerCamera_CM = (SPATIALLABS_SCREEN_WIDTH_CM / 2) / FMath::Tan(FMath::DegreesToRadians(FOVPlayerCamera / 2));
		}
	}
}

FVector FSpatialLabsDevice::GetCachedEyeLocation(EStereoscopicPass StereoPassType)
{
	uint32 EyeIndex = 0;// Left
	if (StereoPassType == eSSP_RIGHT_EYE)
	{
		EyeIndex = 1;
	}

	float eyePosition[] = { 0.0, 0.0, 0.0 };
	const auto& CoreSettings = FSpatialLabsCoreModule::Get().GetCoreSettings();
	CoreSettings->GetCachedEyePosition(EyeIndex, eyePosition);

	return FVector(eyePosition[0], eyePosition[1], eyePosition[2]);
}

FVector FSpatialLabsDevice::GetCachedEyeScreenSpaceLocation(EStereoscopicPass StereoPassType)
{
	uint32 EyeIndex = 0;// Left
	if (StereoPassType == eSSP_RIGHT_EYE)
	{
		EyeIndex = 1;
	}

	float eyePosition[] = { 0.0, 0.0, 0.0 };
	const auto& CoreSettings = FSpatialLabsCoreModule::Get().GetCoreSettings();
	CoreSettings->GetCachedEyePositionScreenSpace(EyeIndex, eyePosition);

	return FVector(eyePosition[0], eyePosition[1], eyePosition[2]);
}
