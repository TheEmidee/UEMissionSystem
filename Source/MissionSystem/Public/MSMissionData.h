#pragma once

#include <CoreMinimal.h>
#include <Engine/DataAsset.h>

#include "MSMissionData.generated.h"

class UMSMissionAction;
class UMSMissionObjective;

USTRUCT( BlueprintType )
struct MISSIONSYSTEM_API FMSMissionObjectiveData
{
    GENERATED_USTRUCT_BODY()

    FMSMissionObjectiveData();
    FMSMissionObjectiveData( const TSubclassOf< UMSMissionObjective > & objective, bool enabled = true );

    UPROPERTY( EditDefaultsOnly )
    TSubclassOf< UMSMissionObjective > Objective;

    UPROPERTY( EditDefaultsOnly )
    uint8 bEnabled : 1;
};

UCLASS( BlueprintType )
class MISSIONSYSTEM_API UMSMissionData final : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY( EditDefaultsOnly )
    TArray< TSubclassOf< UMSMissionAction > > StartActions;

    UPROPERTY( EditDefaultsOnly )
    TArray< FMSMissionObjectiveData > Objectives;

    UPROPERTY( EditDefaultsOnly )
    TArray< TSubclassOf< UMSMissionAction > > EndActions;

    UPROPERTY( EditDefaultsOnly )
    TArray< UMSMissionData * > NextMissions;

    UPROPERTY( EditDefaultsOnly )
    uint8 bExecuteEndActionsWhenCancelled : 1;

    UPROPERTY( EditDefaultsOnly )
    uint8 bStartNextMissionsWhenCancelled : 1;

#if WITH_EDITOR
    EDataValidationResult IsDataValid( TArray< FText > & validation_errors ) override;
#endif
};