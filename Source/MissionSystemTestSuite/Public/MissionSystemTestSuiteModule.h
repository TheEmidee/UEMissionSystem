#pragma once

#include <CoreMinimal.h>
#include <Modules/ModuleManager.h>

class IMissionSystemTestSuiteModule : public IModuleInterface
{

public:
    static IMissionSystemTestSuiteModule & Get()
    {
        static auto & singleton = FModuleManager::LoadModuleChecked< IMissionSystemTestSuiteModule >( "MissionSystemTestSuiteModule" );
        return singleton;
    }

    static bool IsAvailable()
    {
        return FModuleManager::Get().IsModuleLoaded( "MissionSystemTestSuiteModule" );
    }
};
