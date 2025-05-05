#pragma once

#include <CoreMinimal.h>
#include <Modules/ModuleInterface.h>
#include <Modules/ModuleManager.h>

class MISSIONSYSTEMVIEWMODELS_API IMissionSystemViewModelsModule : public IModuleInterface
{

public:
    static IMissionSystemViewModelsModule & Get()
    {
        static auto & singleton = FModuleManager::LoadModuleChecked< IMissionSystemViewModelsModule >( "MissionSystemViewModelsModule" );
        return singleton;
    }

    static bool IsAvailable()
    {
        return FModuleManager::Get().IsModuleLoaded( "MissionSystemViewModelsModule" );
    }
};
