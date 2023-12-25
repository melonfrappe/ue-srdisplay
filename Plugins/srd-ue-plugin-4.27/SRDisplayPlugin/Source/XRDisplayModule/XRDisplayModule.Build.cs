/*
 * Copyright 2019,2020,2021,2022 Sony Corporation
 */

using System;
using System.IO;
using System.Diagnostics;

using UnrealBuildTool;

public class XRDisplayModule : ModuleRules
{
	public XRDisplayModule(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		CppStandard = CppStandardVersion.Cpp17;
		bEnableExceptions = true;
		BuildVersion Version;
		if (!BuildVersion.TryRead(BuildVersion.GetDefaultFileName(), out Version))
		{
			Version = new BuildVersion();
		}
		string EngineDir = Path.GetFullPath(Target.RelativeEnginePath);

		PrivateIncludePaths.AddRange(
			new string[] {
				Path.Combine(EngineDir, @"Source\Runtime\D3D12RHI\Private"),
				Path.Combine(EngineDir, @"Source\Runtime\D3D12RHI\Private\Windows"),
			}
		);

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"Engine",
				"UMG",
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Slate",
				"SlateCore",
				"HeadMountedDisplay",
				"RHI",
				"Renderer",
				"RenderCore",
				"Projects",
				"ApplicationCore",
				"ImageWrapper",
				"InputCore",
			}
		);

		if (ReadOnlyBuildVersion.Current.MajorVersion == 5)
		{
			PrivateDependencyModuleNames.Add("RHICore");
			AddEngineThirdPartyPrivateStaticDependencies(Target, "DX12");
		}

		if (Target.bBuildEditor == true)
		{
			PrivateDependencyModuleNames.Add("UnrealEd");
		}

		String XrRuntimeRoot = Path.GetFullPath(Path.Combine(ModuleDirectory, "External/xr-runtime/"));
		PublicIncludePaths.Add(Path.Combine(XrRuntimeRoot, "include"));

		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			PrivateDependencyModuleNames.AddRange(
				new string[]
				{
					"D3D12RHI",
				}
			);
			AddEngineThirdPartyPrivateStaticDependencies(Target, "DX12");
			PublicDefinitions.Add("UNREAL_ENGINE_PLUGIN");
		}
	}
}
