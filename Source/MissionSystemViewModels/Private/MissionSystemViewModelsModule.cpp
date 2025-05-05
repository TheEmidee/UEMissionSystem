#include "MissionSystemViewModelsModule.h"

class FMissionSystemViewModelsModule final : public IMissionSystemViewModelsModule
{
public:
    void StartupModule() override;
    void ShutdownModule() override;
};

IMPLEMENT_MODULE( FMissionSystemViewModelsModule, MissionSystemViewModels )

void FMissionSystemViewModelsModule::StartupModule()
{
}

void FMissionSystemViewModelsModule::ShutdownModule()
{
}