using UnrealBuildTool;
using System.IO;

public class SpatialLabsCore : ModuleRules
{

    public SpatialLabsCore(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        DefaultBuildSettings = BuildSettingsVersion.V2;

        PublicDependencyModuleNames.AddRange(
            new string[] {
                "Core",
                "CoreUObject",
                "Engine",
                "Projects"
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[] {
                "Core",
                "CoreUObject"
            }
        );

        PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Library/include"));
        PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "Library/lib", "SpatialLabsCoreLib.lib"));
    }
}
