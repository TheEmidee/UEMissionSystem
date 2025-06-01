using UnrealBuildTool;

public class MissionSystemEditor : ModuleRules
{
    public MissionSystemEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PublicDependencyModuleNames.AddRange(new string[] {
            "Core",
            "MissionSystem"
            });
        
        PrivateDependencyModuleNames.AddRange(new string[] {
            "Core",
            "CoreUObject",
            "Engine",
            "UnrealEd",
            "SlateCore",
            "Slate",
            "PropertyEditor"
            });

        PrivateIncludePathModuleNames.AddRange(new string[] {
            "AssetRegistry",
            "AssetTools"
            });
    }
}
