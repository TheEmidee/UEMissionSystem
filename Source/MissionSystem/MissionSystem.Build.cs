namespace UnrealBuildTool.Rules
{
    public class MissionSystem : ModuleRules
    {
        public MissionSystem( ReadOnlyTargetRules Target )
            : base( Target )
        {
            PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
            

            PrivatePCHHeaderFile = "Private/MissionSystemPCH.h";

            PrivateIncludePaths.AddRange( 
                new string[]
                {
                    "MissionSystem/Private"
                }
            );

            PublicDependencyModuleNames.AddRange(
                new string[] { 
                    "Core",
                    "CoreUObject",
                    "Engine",
                    "GameplayTags",
                    "DataValidationExtensions",
                    "CoreExtensions",
                    "ModelViewViewModel"
                }
            );
        }
    }
}
