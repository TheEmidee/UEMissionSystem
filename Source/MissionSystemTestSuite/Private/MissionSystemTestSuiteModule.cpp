#include "MissionSystemTestSuiteModule.h"

#include "MissionSystemModule.h"

class FMissionSystemTestSuiteModule final : public IMissionSystemModule
{
};

IMPLEMENT_MODULE( FMissionSystemTestSuiteModule, MissionSystemTestSuite )