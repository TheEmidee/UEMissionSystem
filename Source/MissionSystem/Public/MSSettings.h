#pragma once

#include <CoreMinimal.h>
#include <Engine/DeveloperSettings.h>

#include "MSSettings.generated.h"

class UMSMissionData;

UCLASS( config = Game, MinimalAPI, meta = ( DisplayName = "Mission System" ) )
class UMSSettings final : public UDeveloperSettings
{
    GENERATED_BODY()

public:
    UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, config, Category = "Mission System" )
    TSoftObjectPtr< UMSMissionData > FirstMission;
};
