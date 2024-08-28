#pragma once

#include <AssetToolsModule.h>
#include <AssetTypeCategories.h>
#include <CoreMinimal.h>
#include <IAssetTools.h>
#include <Modules/ModuleInterface.h>

class IAssetTypeActions;

class FMissionSystemEditor final : public FDefaultModuleImpl
{
public:
    FMissionSystemEditor();

    void StartupModule() override;
    void ShutdownModule() override;

private:
    template < typename _ACTION_TYPE_ >
    void RegisterAction( EAssetTypeCategories::Type category );

    TArray< TSharedPtr< IAssetTypeActions > > RegisteredAssetTypeActions;
};

template < typename _ACTION_TYPE_ >
void FMissionSystemEditor::RegisterAction( EAssetTypeCategories::Type category )
{
    auto & asset_tools = FAssetToolsModule::GetModule().Get();

    const auto action = MakeShared< _ACTION_TYPE_ >( category );
    asset_tools.RegisterAssetTypeActions( action );
    RegisteredAssetTypeActions.Add( action );
}
