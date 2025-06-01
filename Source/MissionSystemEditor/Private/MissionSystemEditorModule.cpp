#include "MissionSystemEditorModule.h"

#include "Customizations/MSMissionObjectiveDetailCustomization.h"
#include "MSMissionDataAssetTypeActions.h"
#include "Customizations/MSMissionDataDetailCustomization.h"

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

    auto & property_module = FModuleManager::LoadModuleChecked< FPropertyEditorModule >( "PropertyEditor" );
    property_module.RegisterCustomClassLayout( "MSMissionObjective", FOnGetDetailCustomizationInstance::CreateStatic( &FMsMissionObjectiveDetailCustomization::MakeInstance ) );
    property_module.RegisterCustomClassLayout( "MSMissionData", FOnGetDetailCustomizationInstance::CreateStatic( &FMsMissionDataDetailCustomization::MakeInstance ) );
}

void FMissionSystemEditor::ShutdownModule()
{
    if ( FModuleManager::Get().IsModuleLoaded( "PropertyEditor" ) )
    {
        auto & property_editor_module = FModuleManager::GetModuleChecked< FPropertyEditorModule >( "PropertyEditor" );
        property_editor_module.UnregisterCustomClassLayout( "MSMissionObjective" );
        property_editor_module.UnregisterCustomClassLayout( "MSMissionData" );
    }

    if ( FModuleManager::Get().IsModuleLoaded( "AssetTools" ) )
    {
        for ( auto action : RegisteredAssetTypeActions )
        {
            FAssetToolsModule::GetModule().Get().UnregisterAssetTypeActions( action.ToSharedRef() );
        }
    }
}
