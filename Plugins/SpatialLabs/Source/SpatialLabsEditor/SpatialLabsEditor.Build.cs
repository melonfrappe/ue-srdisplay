using UnrealBuildTool;
using System.IO;

public class SpatialLabsEditor : ModuleRules
{

    public SpatialLabsEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        DefaultBuildSettings = BuildSettingsVersion.V2;

        PublicDependencyModuleNames.AddRange(
            new string[] {
                "ApplicationCore",
                "Core",
                "CoreUObject",
                "Engine",
                "InputCore",
                "Projects",
                "RenderCore",
                "RHI",
                "Slate",
                "SlateCore",
                "UnrealEd",
                "UMG",
                "EditorStyle",
                "SpatialLabsCore"
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[] {
                "Core",
                "CoreUObject",
                "Settings",
            }
        );
    }
}
