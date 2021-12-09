#include "MissionSystemModule.h"

class FMissionSystemModule final : public IMissionSystemModule
{
public:
    void StartupModule() override;
    void ShutdownModule() override;
};

IMPLEMENT_MODULE( FMissionSystemModule, MissionSystem )

void FMissionSystemModule::StartupModule()
{
}

void FMissionSystemModule::ShutdownModule()
{
}