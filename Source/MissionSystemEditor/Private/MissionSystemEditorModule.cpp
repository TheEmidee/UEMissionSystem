#include "MissionSystemEditorModule.h"

#include "MSMissionDataAssetTypeActions.h"

#include <Modules/ModuleManager.h>

static const FName MISSION_SYSTEM_MENU_CATEGORY_KEY( TEXT( "Mission System" ) );
static const FText MISSION_SYSTEM_MENU_CATEGORY_KEY_TEXT( NSLOCTEXT( "MissionSystemEditor", "MissionSystemAssetCategory", "Mission System" ) );

IMPLEMENT_MODULE( FMissionSystemEditor, MissionSystemEditor );

FMissionSystemEditor::FMissionSystemEditor()
{
}

void FMissionSystemEditor::StartupModule()
{
    auto & asset_tools = FAssetToolsModule::GetModule().Get();

    const auto bit = asset_tools.RegisterAdvancedAssetCategory( MISSION_SYSTEM_MENU_CATEGORY_KEY, MISSION_SYSTEM_MENU_CATEGORY_KEY_TEXT );

    RegisterAction< FMSMissionDataAssetTypeActions >( bit );
    RegisterAction< FMSMissionObjectiveAssetTypeActions >( bit );
}

void FMissionSystemEditor::ShutdownModule()
{
    if ( !FModuleManager::Get().IsModuleLoaded( "AssetTools" ) )
    {
        return;
    }

    for ( auto action : RegisteredAssetTypeActions )
    {
        FAssetToolsModule::GetModule().Get().UnregisterAssetTypeActions( action.ToSharedRef() );
    }
}
