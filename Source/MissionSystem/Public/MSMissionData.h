#pragma once

#include "MSMissionAction.h"
#include "MSMissionObjective.h"

#include <CoreMinimal.h>
#include <Engine/DataAsset.h>

#include "MSMissionData.generated.h"

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
    UMSMissionData();

    UPROPERTY( EditDefaultsOnly, Category = "Actions" )
    TArray< TSubclassOf< UMSMissionAction > > StartActions;

    UPROPERTY( EditDefaultsOnly, Category = "Actions" )
    TArray< TSubclassOf< UMSMissionAction > > EndActions;

    UPROPERTY( EditDefaultsOnly, Category = "Objectives" )
    TArray< FMSMissionObjectiveData > Objectives;

    UPROPERTY( EditDefaultsOnly, Category = "Other missions" )
    TArray< UMSMissionData * > NextMissions;

    UPROPERTY( EditDefaultsOnly, Category = "Other missions" )
    TArray< UMSMissionData * > MissionsToCancel;

    UPROPERTY( EditDefaultsOnly, Category = "Options" )
    uint8 bEnabled : 1;

    UPROPERTY( EditDefaultsOnly, Category = "Options" )
    uint8 bExecuteEndActionsWhenCancelled : 1;

    UPROPERTY( EditDefaultsOnly, Category = "Options" )
    uint8 bStartNextMissionsWhenCancelled : 1;

#if WITH_EDITOR
    EDataValidationResult IsDataValid( TArray< FText > & validation_errors ) override;
#endif
};