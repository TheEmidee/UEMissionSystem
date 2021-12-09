#pragma once

#include <CoreMinimal.h>
#include <Modules/ModuleInterface.h>
#include <Modules/ModuleManager.h>

class MISSIONSYSTEM_API IMissionSystemModule : public IModuleInterface
{

public:
    static IMissionSystemModule & Get()
    {
        static auto & singleton = FModuleManager::LoadModuleChecked< IMissionSystemModule >( "MissionSystemModule" );
        return singleton;
    }

    static bool IsAvailable()
    {
        return FModuleManager::Get().IsModuleLoaded( "MissionSystemModule" );
    }
};
