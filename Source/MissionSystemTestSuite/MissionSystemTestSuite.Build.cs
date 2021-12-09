using UnrealBuildTool;

namespace UnrealBuildTool.Rules
{
    public class MissionSystemTestSuite : ModuleRules
    {
        public MissionSystemTestSuite(ReadOnlyTargetRules Target) : base(Target)
		{
            PublicIncludePaths.AddRange(
                    new string[] {
                    }
                    );

            PublicDependencyModuleNames.AddRange(
                new string[] {
                        "Core",
                        "CoreUObject",
                        "Engine",
                        "GameplayTags",
                        "MissionSystem"
                }
                );

            DynamicallyLoadedModuleNames.AddRange(
                new string[] {
                    // ... add any modules that your module loads dynamically here ...
                }
                );

            if (Target.bBuildEditor == true)
            {
                PrivateDependencyModuleNames.Add("UnrealEd");
            }
        }
    }
}