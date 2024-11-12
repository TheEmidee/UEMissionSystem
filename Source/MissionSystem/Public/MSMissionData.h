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

    const FGuid & GetGuid() const;
    void PostLoad() override;
    void PostDuplicate( bool duplicate_for_pie ) override;
    void PostEditImport() override;

    // The name of the mission
    UPROPERTY( EditDefaultsOnly, BlueprintReadOnly )
    FText Name;

    // The description of the mission
    UPROPERTY( EditDefaultsOnly, BlueprintReadOnly )
    FText Description;

    // All the actions that must be executed when the mission starts
    UPROPERTY( EditDefaultsOnly, Instanced, Category = "Actions" )
    TArray< TObjectPtr< UMSMissionAction > > StartActions;

    // All the actions that must be executed when the mission ends
    UPROPERTY( EditDefaultsOnly, Instanced, Category = "Actions" )
    TArray< TObjectPtr< UMSMissionAction > > EndActions;

    // The list of objectives to complete
    UPROPERTY( EditDefaultsOnly, Category = "ActiveObjectives" )
    TArray< FMSMissionObjectiveData > Objectives;

    // The missions to start when this mission is complete
    UPROPERTY( EditDefaultsOnly, Category = "Other missions" )
    TArray< UMSMissionData * > NextMissions;

    // The missions to cancel when this mission is started 
    UPROPERTY( EditDefaultsOnly, Category = "Other missions" )
    TArray< UMSMissionData * > MissionsToCancel;

    // Is this mission enabled?
    UPROPERTY( EditDefaultsOnly, Category = "Options" )
    uint8 bEnabled : 1;

    // Set to true to execute the end actions even when this mission is cancelled
    UPROPERTY( EditDefaultsOnly, Category = "Options" )
    uint8 bExecuteEndActionsWhenCancelled : 1;

    // Set to true to start the next missions when this mission is cancelled
    UPROPERTY( EditDefaultsOnly, Category = "Options" )
    uint8 bStartNextMissionsWhenCancelled : 1;

    // Set to true to force the objectives to be completed one after the other
    // Set to false to allow to complete the objectives in any order
    UPROPERTY( EditDefaultsOnly, Category = "Options" )
    uint8 bMustCompleteObjectivesSequentially : 1;

    // Id of the mission, used when the mission history is serialized
    // This is generated automatically
    UPROPERTY( VisibleAnywhere, AdvancedDisplay )
    FGuid MissionId;

#if WITH_EDITOR
    EDataValidationResult IsDataValid( FDataValidationContext & context ) const override;
#endif

private:
    void GenerateGuidIfNeeded( bool force_generation = false );
};

FORCEINLINE const FGuid & UMSMissionData::GetGuid() const
{
    return MissionId;
}