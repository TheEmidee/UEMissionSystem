#pragma once

#include <CoreMinimal.h>
#include <Engine/DeveloperSettings.h>

#include "MSDeveloperSettings.generated.h"

class UMSMissionData;

UCLASS( config = EditorPerProjectUserSettings, MinimalAPI, meta = ( DisplayName = "Mission System" ) )
class UMSDeveloperSettings final : public UDeveloperSettings
{
    GENERATED_BODY()

public:
    UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, config, Category = "Mission System" )
    TSoftObjectPtr< UMSMissionData > FirstMissionOverride;
};
