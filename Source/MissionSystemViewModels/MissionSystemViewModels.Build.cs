namespace UnrealBuildTool.Rules
{
    public class MissionSystemViewModels : ModuleRules
    {
        public MissionSystemViewModels( ReadOnlyTargetRules Target )
            : base( Target )
        {
            PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

            PrivateIncludePaths.AddRange( 
                new string[]
                {
                    "MissionSystemViewModels/Private"
                }
            );

            PublicDependencyModuleNames.AddRange(
                new string[] { 
                    "Core",
                    "CoreUObject",
                    "Engine",
                    "ModelViewViewModel",
                    "MissionSystem"
                }
            );
        }
    }
}
